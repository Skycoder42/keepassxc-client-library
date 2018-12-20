#include "defaultdatabaseregistry.h"
#include "defaultdatabaseregistry_p.h"
using namespace KPXCClient;

IDatabaseRegistry::IDatabaseRegistry() = default;

IDatabaseRegistry::~IDatabaseRegistry() = default;



DefaultDatabaseRegistry::DefaultDatabaseRegistry(QObject *parent) :
	QObject{parent},
	d{new DefaultDatabaseRegistryPrivate{}}
{}

DefaultDatabaseRegistry::DefaultDatabaseRegistry(bool persistent, QObject *parent) :
	DefaultDatabaseRegistry{parent}
{
	if(persistent) {
		d->settings = new QSettings{this};
		reloadSettings();
	}
}

DefaultDatabaseRegistry::DefaultDatabaseRegistry(QSettings *settings, QObject *parent) :
	DefaultDatabaseRegistry{true, parent}
{
	d->settings = settings;
	reloadSettings();
}

DefaultDatabaseRegistry::~DefaultDatabaseRegistry() = default;

bool DefaultDatabaseRegistry::hasClientId(const QByteArray &databaseHash)
{
	return d->clientIds.contains(databaseHash);
}

IDatabaseRegistry::ClientId DefaultDatabaseRegistry::getClientId(const QByteArray &databaseHash)
{
	auto &clientId = d->clientIds[databaseHash];
	SecureByteArray::StateLocker _{&clientId.key, SecureByteArray::State::Readonly};
	return clientId;
}

QList<IDatabaseRegistry::ClientId> DefaultDatabaseRegistry::getAllClientIds()
{
	QList<ClientId> idList;
	for(auto it = d->clientIds.begin(); it != d->clientIds.end(); ++it) {
		SecureByteArray::StateLocker _{&it->key, SecureByteArray::State::Readonly};
		idList.append(*it);
	}
	return idList;
}

void DefaultDatabaseRegistry::addClientId(const QByteArray &databaseHash, ClientId clientId)
{
	auto it = d->clientIds.insert(databaseHash, clientId); //only shallow copy because of QSharedData
	clientId = {}; //drop reference to prevent deep copy because of state change

	if(d->settings) {
		d->settings->beginGroup(DefaultDatabaseRegistryPrivate::SettingsGroupKey);
		d->settings->beginGroup(QString::fromUtf8(databaseHash.toHex()));
		d->settings->setValue(DefaultDatabaseRegistryPrivate::SettingsNameKey, it->name);
		d->settings->setValue(DefaultDatabaseRegistryPrivate::SettingsKeyKey, it->key.toBase64());
		d->settings->endGroup();
		d->settings->endGroup();
	}
	it->key.setState(SecureByteArray::State::Noaccess);
}

void DefaultDatabaseRegistry::removeClientId(const QByteArray &databaseHash)
{
	if(d->clientIds.remove(databaseHash) > 0 &&
	   d->settings) {
		d->settings->beginGroup(DefaultDatabaseRegistryPrivate::SettingsGroupKey);
		d->settings->remove(QString::fromUtf8(databaseHash.toHex()));
		d->settings->endGroup();
	}
}

bool DefaultDatabaseRegistry::isPersistent() const
{
	return d->settings;
}

QSettings *DefaultDatabaseRegistry::settings() const
{
	return d->settings;
}

void DefaultDatabaseRegistry::reloadSettings()
{
	if(!d->settings)
		return;

	d->clientIds.clear();
	d->settings->beginGroup(DefaultDatabaseRegistryPrivate::SettingsGroupKey);
	const auto keys = d->settings->childGroups();
	for(const auto &key : keys) {
		d->settings->beginGroup(key);
		ClientId cId;
		cId.name = d->settings->value(DefaultDatabaseRegistryPrivate::SettingsNameKey).toString();
		cId.key = SecureByteArray::fromBase64(d->settings->value(DefaultDatabaseRegistryPrivate::SettingsKeyKey).toString());
		d->settings->endGroup();
		auto it = d->clientIds.insert(QByteArray::fromHex(key.toUtf8()), cId);
		it->key.setState(SecureByteArray::State::Noaccess);
	}
	d->settings->endGroup();
}

void DefaultDatabaseRegistry::setPersistent(bool persistent)
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

void DefaultDatabaseRegistry::setPersistent(QSettings *settings, bool takeOwnership)
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

const QString DefaultDatabaseRegistryPrivate::SettingsGroupKey{QStringLiteral("KPXCClientRegistry")};
const QString DefaultDatabaseRegistryPrivate::SettingsNameKey{QStringLiteral("name")};
const QString DefaultDatabaseRegistryPrivate::SettingsKeyKey{QStringLiteral("key")};
