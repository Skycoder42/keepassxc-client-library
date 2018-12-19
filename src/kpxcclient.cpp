#include "kpxcclient.h"
#include "kpxcclient_p.h"
#include <QtCore/QDebug>
#include <QtCore/QJsonArray>
#include <sodium/randombytes.h>
#include <sodium/randombytes_sysrandom.h>
#include <sodium/core.h>
#include <sodium/crypto_box.h>

bool KPXCClient::init()
{
	if(KPXCClientPrivate::initialized)
		return true;

	KPXCClientPrivate::initialized = (sodium_init() == 0);
	randombytes_set_implementation(&randombytes_sysrandom_implementation);
	return KPXCClientPrivate::initialized;
}

KPXCClient::KPXCClient(QObject *parent) :
	QObject{parent},
	d{new KPXCClientPrivate{this}}
{
	Q_ASSERT_X(KPXCClientPrivate::initialized, Q_FUNC_INFO, "You must call KPXCClient::init() before creating a KPXCClient");
	connect(this, &KPXCClient::connected,
			this, &KPXCClient::stateChanged);
	connect(this, &KPXCClient::disconnected,
			this, &KPXCClient::stateChanged);
	connect(this, &KPXCClient::databaseOpened,
			this, [this](){
		stateChanged({});
	});
	connect(this, &KPXCClient::databaseClosed,
			this, &KPXCClient::stateChanged);

	connect(d->connector, &KPXCConnector::connected,
			this, &KPXCClient::dbConnected);
	connect(d->connector, &KPXCConnector::disconnected,
			this, &KPXCClient::dbDisconnected);
	connect(d->connector, &KPXCConnector::error,
			this, &KPXCClient::dbError);
	connect(d->connector, &KPXCConnector::locked,
			this, &KPXCClient::dbLocked);
	connect(d->connector, &KPXCConnector::unlocked,
			this, &KPXCClient::dbUnlocked);
	connect(d->connector, &KPXCConnector::messageReceived,
			this, &KPXCClient::dbMsgRecv);
	connect(d->connector, &KPXCConnector::messageFailed,
			this, &KPXCClient::dbMsgFail);
}

KPXCClient::~KPXCClient() = default;

IKPXCDatabaseRegistry *KPXCClient::databaseRegistry() const
{
	return d->dbReg;
}

KPXCClient::Options KPXCClient::options() const
{
	return d->options;
}

KPXCClient::State KPXCClient::state() const
{
	if(d->connector->isConnected())
		return d->locked ? State::Locked : State::Unlocked;
	else if(d->connector->isConnecting())
		return State::Connecting;
	else
		return State::Disconnected;
}

QByteArray KPXCClient::currentDatabase() const
{
	return d->currentDatabase;
}

void KPXCClient::connectToKeePass(const QString &keePassPath)
{
	if(d->connector->isConnected()) {
		d->setError(Error::ClientAlreadyConnected);
		return;
	}

	d->clear();
	d->connector->connectToKeePass(keePassPath);
}

void KPXCClient::disconnectFromKeePass()
{
	d->connector->disconnectFromKeePass();
}

void KPXCClient::openDatabase()
{
	if(state() != State::Locked)
		return; //TODO error?
	d->connector->sendEncrypted(KPXCClientPrivate::ActionGetDatabaseHash, {},
								d->options.testFlag(Option::TriggerUnlock));
}

void KPXCClient::closeDatabase()
{
	if(state() != State::Unlocked)
		return; //TODO error?
	d->connector->sendEncrypted(KPXCClientPrivate::ActionLockDatabase);
}

void KPXCClient::generatePassword()
{
	d->connector->sendEncrypted(KPXCClientPrivate::ActionGeneratePassword);
}

void KPXCClient::getLogins(const QUrl &url, const QUrl &submitUrl, bool httpAuth, bool searchAllDatabases)
{
	QJsonObject message;
	message[QStringLiteral("id")] = d->dbReg->getClientId(d->currentDatabase).name;
	message[QStringLiteral("url")] = url.toString(QUrl::FullyEncoded);
	if(!submitUrl.isEmpty())
		message[QStringLiteral("submitUrl")] = submitUrl.toString(QUrl::FullyEncoded);
	else
		message[QStringLiteral("submitUrl")] = message[QStringLiteral("url")];
	message[QStringLiteral("httpAuth")] = QVariant{httpAuth}.toString();

	QList<IKPXCDatabaseRegistry::ClientId> clientIds;
	if(searchAllDatabases)
		clientIds = d->dbReg->getAllClientIds();
	else
		clientIds.append(d->dbReg->getClientId(d->currentDatabase));
	QJsonArray keys;
	for(const auto &cId : qAsConst(clientIds)) {
		QJsonObject keyInfo;
		keyInfo[QStringLiteral("id")] = cId.name;
		keyInfo[QStringLiteral("key")] = cId.key.toBase64();
		keys.append(keyInfo);
	}
	message[QStringLiteral("keys")] = keys;

	d->connector->sendEncrypted(KPXCClientPrivate::ActionGetLogins, message);
}

void KPXCClient::addLogin(const QUrl &url, const KPXCEntry &entry, const QUrl &submitUrl)
{
	QJsonObject message;
	message[QStringLiteral("id")] = d->dbReg->getClientId(d->currentDatabase).name;
	message[QStringLiteral("url")] = url.toString(QUrl::FullyEncoded);
	if(!submitUrl.isEmpty())
		message[QStringLiteral("submitUrl")] = submitUrl.toString(QUrl::FullyEncoded);
	else
		message[QStringLiteral("submitUrl")] = message[QStringLiteral("url")];
	if(entry.isStored())
		message[QStringLiteral("uuid")] = entry.uuid().toString(QUuid::Id128);
	message[QStringLiteral("login")] = entry.username();
	message[QStringLiteral("password")] = entry.password();

	d->connector->sendEncrypted(KPXCClientPrivate::ActionSetLogin, message);
}

void KPXCClient::setDatabaseRegistry(IKPXCDatabaseRegistry *databaseRegistry)
{
	if (d->dbReg == databaseRegistry)
		return;

	if(d->dbReg)
		dynamic_cast<QObject*>(d->dbReg)->deleteLater();
	d->dbReg = databaseRegistry;
	dynamic_cast<QObject*>(d->dbReg)->setParent(this);
	emit databaseRegistryChanged(d->dbReg, {});
}

void KPXCClient::setOptions(Options options)
{
	if (d->options == options)
		return;

	d->options = options;
	emit optionsChanged(d->options, {});
}

bool KPXCClient::allowDatabase(const QByteArray &databaseHash) const
{
	Q_UNUSED(databaseHash)
	return d->options.testFlag(Option::AllowNewDatabase);
}

void KPXCClient::dbConnected()
{
	emit connected({});
	if(d->options.testFlag(Option::OpenOnConnect))
		openDatabase();
}

void KPXCClient::dbDisconnected()
{
	qDebug() << Q_FUNC_INFO;
	emit disconnected({});
	d->clear();
}

void KPXCClient::dbError(KPXCClient::Error code, const QString &message)
{
	d->setError(code, message);
	disconnectFromKeePass();
}

void KPXCClient::dbLocked()
{
	if(d->locked)
		return;

	d->locked = true;
	emit databaseClosed({});
	if(d->options.testFlag(Option::DisconnectOnClose))
		disconnectFromKeePass();
}

void KPXCClient::dbUnlocked()
{
	if(!d->locked)
		return;

	openDatabase();
}

void KPXCClient::dbMsgRecv(const QString &action, const QJsonObject &message)
{
	if(action == KPXCClientPrivate::ActionGetDatabaseHash)
		d->onDbHash(message);
	else if(action == KPXCClientPrivate::ActionAssociate)
		d->onAssoc(message);
	else if(action == KPXCClientPrivate::ActionTestAssociate)
		d->onTestAssoc(message);
	else if(action == KPXCClientPrivate::ActionGeneratePassword)
		d->onGeneratePasswd(message);
	else if(action == KPXCClientPrivate::ActionGetLogins)
		d->onGetLogins(message);
	else if(action == KPXCClientPrivate::ActionSetLogin)
		emit loginAdded({});
	else if(action == KPXCClientPrivate::ActionLockDatabase)
		dbLocked();
	else {
		d->setError(Error::ClientUnsupportedAction, action);
		disconnectFromKeePass();
	}
}

void KPXCClient::dbMsgFail(const QString &action, Error code, const QString &message)
{
	if(code == Error::KeePassDatabaseNotOpen &&
	   action == KPXCClientPrivate::ActionGetDatabaseHash &&
	   d->options.testFlag(Option::TriggerUnlock)) {
		qInfo() << "Database locked. Waiting for user to unlock";
	} else if(code == Error::KeePassAssociationFailed &&
			  action == KPXCClientPrivate::ActionTestAssociate) {
		qWarning() << "Current association was rejected. Initiation re-association";
		d->dbReg->removeClientId(d->currentDatabase);
		d->sendAssoc();
	} else {
		dbError(code, message);
		Q_UNIMPLEMENTED();
		//TODO forward failure to action handler?
	}
}

// ------------- Private implementation -------------

const QString KPXCClientPrivate::ActionGetDatabaseHash{QStringLiteral("get-databasehash")};
const QString KPXCClientPrivate::ActionTestAssociate{QStringLiteral("test-associate")};
const QString KPXCClientPrivate::ActionAssociate{QStringLiteral("associate")};
const QString KPXCClientPrivate::ActionGeneratePassword{QStringLiteral("generate-password")};
const QString KPXCClientPrivate::ActionGetLogins{QStringLiteral("get-logins")};
const QString KPXCClientPrivate::ActionSetLogin{QStringLiteral("set-login")};
const QString KPXCClientPrivate::ActionLockDatabase{QStringLiteral("lock-database")};

bool KPXCClientPrivate::initialized = false;

KPXCClientPrivate::KPXCClientPrivate(KPXCClient *q_ptr) :
	q{q_ptr},
	connector{new KPXCConnector{q_ptr}},
	dbReg{new KPXCDefaultDatabaseRegistry{q_ptr}}
{}

void KPXCClientPrivate::setError(KPXCClient::Error error, const QString &msg)
{
	QString errorMessage;
	switch(error) {
	// KeePassXC errors -> only use msg description
	case KPXCClient::Error::KeePassDatabaseNotOpen:
	case KPXCClient::Error::KeePassDatabaseHashNotReceived:
	case KPXCClient::Error::KeePassPublicKeyNotReceived:
	case KPXCClient::Error::KeePassCannotDecryptMessage:
	case KPXCClient::Error::KeePassTimeout:
	case KPXCClient::Error::KeePassActionDenied:
	case KPXCClient::Error::KeePassCannotEncryptMessage:
	case KPXCClient::Error::KeePassAssociationFailed:
	case KPXCClient::Error::KeePassKeyChangeFailed:
	case KPXCClient::Error::KeePassEncryptionKeyUnrecognized:
	case KPXCClient::Error::KeePassNoSavedDatabase:
	case KPXCClient::Error::KeePassIncorrectAction:
	case KPXCClient::Error::KeePassEmptyMessageReceived:
	case KPXCClient::Error::KeePassNoUrlProvided:
	case KPXCClient::Error::KeePassNoLoginsFound:
		errorMessage = msg;
		break;
	// Client errors
	case KPXCClient::Error::ClientAlreadyConnected:
		errorMessage = KPXCClient::tr("Already connected to a KeePassXC instance");
		break;
	case KPXCClient::Error::ClientKeyGenerationFailed:
		errorMessage = KPXCClient::tr("Failed to generate session keys");
		break;
	case KPXCClient::Error::ClientReceivedNonceInvalid:
		errorMessage = KPXCClient::tr("Unexpected nonce received from KeePassXC");
		break;
	case KPXCClient::Error::ClientJsonParseError:
		errorMessage = KPXCClient::tr("Received JSON-data is invalid. JSON-Error: %1")
						  .arg(msg);
		break;
	case KPXCClient::Error::ClientActionsDontMatch:
		errorMessage = KPXCClient::tr("Data-Action field of encrypted and unencrypted message are not equal");
		break;
	case KPXCClient::Error::ClientUnsupportedVersion:
		errorMessage = KPXCClient::tr("Unsupported KeePassXC Version. Must be at least %1, but currently is %2")
						  .arg(KPXCConnector::minimumKeePassXCVersion.toString(), msg);
		break;
	case KPXCClient::Error::ClientDatabaseChanged:
		errorMessage = KPXCClient::tr("The opened database in KeePassXC was changed");
		break;
	case KPXCClient::Error::ClientDatabaseRejected:
		errorMessage = KPXCClient::tr("The database hash was not known and thus rejected");
		break;
	case KPXCClient::Error::ClientUnsupportedAction:
		errorMessage = KPXCClient::tr("An unsupported action was received from KeePassXC: %1")
						  .arg(msg);
		break;
	// General errors
	case KPXCClient::Error::UnknownError:
	default:
		errorMessage = KPXCClient::tr("Unknown Error");
		break;
	}
	emit q->errorOccured(error, errorMessage, {});
}

void KPXCClientPrivate::clear()
{
	locked = true;
	currentDatabase.clear();
}

void KPXCClientPrivate::onDbHash(const QJsonObject &message)
{
	const auto dbHash = QByteArray::fromHex(message[QStringLiteral("hash")].toString().toUtf8());
	if(currentDatabase.isEmpty()) {
		currentDatabase = dbHash;
		emit q->currentDatabaseChanged(currentDatabase, {});
	} else if(currentDatabase != dbHash) {
		if(options.testFlag(KPXCClient::Option::AllowDatabaseChange)) {
			currentDatabase = dbHash;
			emit q->currentDatabaseChanged(currentDatabase, {});
		} else {
			setError(KPXCClient::Error::ClientDatabaseChanged);
			q->disconnectFromKeePass();
			return;
		}
	}

	if(dbReg->hasClientId(currentDatabase))
		sendTestAssoc();
	else if(q->allowDatabase(currentDatabase))
		sendAssoc();
	else {
		setError(KPXCClient::Error::ClientDatabaseRejected);
		q->disconnectFromKeePass();
	}
}

void KPXCClientPrivate::onAssoc(const QJsonObject &message)
{
	const auto hash = QByteArray::fromHex(message[QStringLiteral("hash")].toString().toUtf8());
	if(hash != currentDatabase) {
		setError(KPXCClient::Error::ClientDatabaseChanged);
		q->disconnectFromKeePass();
		return;
	}

	IKPXCDatabaseRegistry::ClientId cId;
	cId.name = message[QStringLiteral("id")].toString();
	cId.key = std::move(_keyCache);
	cId.key.makeReadonly();
	dbReg->addClientId(currentDatabase, std::move(cId));
	locked = false;
	emit q->databaseOpened(currentDatabase, {});
}

void KPXCClientPrivate::onTestAssoc(const QJsonObject &message)
{
	const auto hash = QByteArray::fromHex(message[QStringLiteral("hash")].toString().toUtf8());
	if(hash != currentDatabase) {
		setError(KPXCClient::Error::ClientDatabaseChanged);
		q->disconnectFromKeePass();
		return;
	}
	locked = false;
	emit q->databaseOpened(currentDatabase, {});
}

void KPXCClientPrivate::onGeneratePasswd(const QJsonObject &message)
{
	const auto entries = message[QStringLiteral("entries")].toArray();
	QStringList passwords;
	passwords.reserve(entries.size());
	for(const auto entry : entries)
		passwords.append(entry.toObject()[QStringLiteral("password")].toString());
	emit q->passwordsGenerated(passwords, {});
}

void KPXCClientPrivate::onGetLogins(const QJsonObject &message)
{
	const auto jEntries = message[QStringLiteral("entries")].toArray();
	QList<KPXCEntry> entries;
	entries.reserve(jEntries.size());
	for(const auto jEntryVal : jEntries) {
		const auto jEntry = jEntryVal.toObject();
		auto uuidStr = jEntry[QStringLiteral("uuid")].toString();
		uuidStr = uuidStr.mid(0, 8) + QLatin1Char('-') +
				  uuidStr.mid(8, 4) + QLatin1Char('-') +
				  uuidStr.mid(12, 4) + QLatin1Char('-') +
				  uuidStr.mid(16, 4) + QLatin1Char('-') +
				  uuidStr.mid(20, 12);

		KPXCEntry entry;
		entry.setUuid(uuidStr);
		entry.setTitle(jEntry[QStringLiteral("name")].toString());
		entry.setUsername(jEntry[QStringLiteral("login")].toString());
		entry.setPassword(jEntry[QStringLiteral("password")].toString());
		entry.setTotp(jEntry[QStringLiteral("totp")].toString());

		const auto stringFields = jEntry[QStringLiteral("stringFields")].toArray();
		QMap<QString, QString> extraFields;
		for(const auto jFieldVal : stringFields) {
			const auto jField = jFieldVal.toObject();
			for(auto it = jField.constBegin(); it != jField.constEnd(); ++it)
				extraFields.insert(it.key(), it->toString());
		}
		entry.setExtraFields(std::move(extraFields));
		entries.append(entry);
	}
	emit q->loginsReceived(entries, {});
}

void KPXCClientPrivate::sendTestAssoc()
{
	auto cId = dbReg->getClientId(currentDatabase);
	QJsonObject message;
	message[QStringLiteral("id")] = cId.name;
	message[QStringLiteral("key")] = cId.key.toBase64();
	connector->sendEncrypted(ActionTestAssociate, message);
}

void KPXCClientPrivate::sendAssoc()
{
	_keyCache = connector->cryptor()->generateRandom(crypto_box_PUBLICKEYBYTES, SecureByteArray::State::Readonly);
	QJsonObject message;
	message[QStringLiteral("key")] = connector->cryptor()->publicKey().toBase64();
	message[QStringLiteral("idKey")] = _keyCache.toBase64();
	_keyCache.makeNoaccess();
	connector->sendEncrypted(ActionAssociate, message);
}
