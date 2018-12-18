#include "kpxcdatabaseregistry.h"
#include "kpxcdatabaseregistry_p.h"

IKPXCDatabaseRegistry::IKPXCDatabaseRegistry() = default;

IKPXCDatabaseRegistry::~IKPXCDatabaseRegistry() = default;



KPXCDefaultDatabaseRegistry::KPXCDefaultDatabaseRegistry(QObject *parent) :
	QObject{parent},
	d{new KPXCDefaultDatabaseRegistryPrivate{}}
{}

KPXCDefaultDatabaseRegistry::KPXCDefaultDatabaseRegistry(bool persistent, QObject *parent) :
	KPXCDefaultDatabaseRegistry{parent}
{
	if(persistent) {
		d->settings = new QSettings{this};
		reloadSettings();
	}
}

KPXCDefaultDatabaseRegistry::KPXCDefaultDatabaseRegistry(QSettings *settings, QObject *parent) :
	KPXCDefaultDatabaseRegistry{true, parent}
{
	d->settings = settings;
	reloadSettings();
}

KPXCDefaultDatabaseRegistry::~KPXCDefaultDatabaseRegistry() = default;

bool KPXCDefaultDatabaseRegistry::hasClientId(const QByteArray &databaseHash)
{
	return d->clientIds.contains(databaseHash);
}

IKPXCDatabaseRegistry::ClientId KPXCDefaultDatabaseRegistry::getClientId(const QByteArray &databaseHash)
{
	auto &clientId = d->clientIds[databaseHash];
	SecureByteArray::StateLocker _{&clientId.key, SecureByteArray::State::Readonly};
	return clientId;
}

void KPXCDefaultDatabaseRegistry::addClientId(const QByteArray &databaseHash, ClientId clientId)
{
	auto it = d->clientIds.insert(databaseHash, clientId); //only shallow copy because of QSharedData
	clientId = {}; //drop reference to prevent deep copy because of state change

	if(d->settings) {
		d->settings->beginGroup(KPXCDefaultDatabaseRegistryPrivate::SettingsGroupKey);
		d->settings->beginGroup(QString::fromUtf8(databaseHash.toHex()));
		d->settings->setValue(KPXCDefaultDatabaseRegistryPrivate::SettingsNameKey, it->name);
		d->settings->setValue(KPXCDefaultDatabaseRegistryPrivate::SettingsKeyKey, it->key.toBase64());
		d->settings->endGroup();
		d->settings->endGroup();
	}
	it->key.setState(SecureByteArray::State::Noaccess);
}

void KPXCDefaultDatabaseRegistry::removeClientId(const QByteArray &databaseHash)
{
	if(d->clientIds.remove(databaseHash) > 0 &&
	   d->settings) {
		d->settings->beginGroup(KPXCDefaultDatabaseRegistryPrivate::SettingsGroupKey);
		d->settings->remove(QString::fromUtf8(databaseHash.toHex()));
		d->settings->endGroup();
	}
}

bool KPXCDefaultDatabaseRegistry::isPersistent() const
{
	return d->settings;
}

QSettings *KPXCDefaultDatabaseRegistry::settings() const
{
	return d->settings;
}

void KPXCDefaultDatabaseRegistry::reloadSettings()
{
	if(!d->settings)
		return;

	d->clientIds.clear();
	d->settings->beginGroup(KPXCDefaultDatabaseRegistryPrivate::SettingsGroupKey);
	const auto keys = d->settings->childGroups();
	for(const auto &key : keys) {
		d->settings->beginGroup(key);
		ClientId cId;
		cId.name = d->settings->value(KPXCDefaultDatabaseRegistryPrivate::SettingsNameKey).toString();
		cId.key = SecureByteArray::fromBase64(d->settings->value(KPXCDefaultDatabaseRegistryPrivate::SettingsKeyKey).toString());
		d->settings->endGroup();
		auto it = d->clientIds.insert(QByteArray::fromHex(key.toUtf8()), cId);
		it->key.setState(SecureByteArray::State::Noaccess);
	}
	d->settings->endGroup();
}

void KPXCDefaultDatabaseRegistry::setPersistent(bool persistent)
{
	if (isPersistent() == persistent)
		return;

	if(persistent) {
		d->settings = new QSettings{this};
		reloadSettings();
	} else {
		if(d->settings->parent() == this)
			d->settings->deleteLater();
		d->settings.clear();
	}
	emit persistentChanged(persistent, {});
}

void KPXCDefaultDatabaseRegistry::setPersistent(QSettings *settings, bool takeOwnership)
{
	if(d->settings == settings)
		return;

	if(d->settings->parent() == this)
		d->settings->deleteLater();
	d->settings = settings;
	if(takeOwnership)
		d->settings->setParent(this);
	reloadSettings();
}

// ------------- Private implementation -------------

const QString KPXCDefaultDatabaseRegistryPrivate::SettingsGroupKey{QStringLiteral("KPXCClientRegistry")};
const QString KPXCDefaultDatabaseRegistryPrivate::SettingsNameKey{QStringLiteral("name")};
const QString KPXCDefaultDatabaseRegistryPrivate::SettingsKeyKey{QStringLiteral("key")};
