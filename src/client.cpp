#include "client.h"
#include "client_p.h"
#include "defaultdatabaseregistry.h"
#include <QtCore/QDebug>
#include <QtCore/QJsonArray>
#include <sodium/randombytes.h>
#include <sodium/randombytes_sysrandom.h>
#include <sodium/core.h>
#include <sodium/crypto_box.h>
using namespace KPXCClient;

bool KPXCClient::init()
{
	if(ClientPrivate::initialized)
		return true;

	ClientPrivate::initialized = (sodium_init() == 0);
	randombytes_set_implementation(&randombytes_sysrandom_implementation);
	return ClientPrivate::initialized;
}

Client::Client(QObject *parent) :
	QObject{parent},
	d{new ClientPrivate{this}}
{
	Q_ASSERT_X(ClientPrivate::initialized, Q_FUNC_INFO, "You must call KPXCClient::init() before creating a KPXCClient");
	connect(this, &Client::connected,
			this, &Client::stateChanged);
	connect(this, &Client::disconnected,
			this, &Client::stateChanged);
	connect(this, &Client::databaseOpened,
			this, [this](){
		stateChanged({});
	});
	connect(this, &Client::databaseClosed,
			this, &Client::stateChanged);

	connect(d->connector, &Connector::connected,
			this, &Client::dbConnected);
	connect(d->connector, &Connector::disconnected,
			this, &Client::dbDisconnected);
	connect(d->connector, &Connector::error,
			this, &Client::dbError);
	connect(d->connector, &Connector::locked,
			this, &Client::dbLocked);
	connect(d->connector, &Connector::unlocked,
			this, &Client::dbUnlocked);
	connect(d->connector, &Connector::messageReceived,
			this, &Client::dbMsgRecv);
	connect(d->connector, &Connector::messageFailed,
			this, &Client::dbMsgFail);
}

Client::~Client() = default;

IDatabaseRegistry *Client::databaseRegistry() const
{
	return d->dbReg;
}

Client::Options Client::options() const
{
	return d->options;
}

Client::State Client::state() const
{
	if(d->connector->isConnected())
		return d->locked ? State::Locked : State::Unlocked;
	else if(d->connector->isConnecting())
		return State::Connecting;
	else
		return State::Disconnected;
}

QByteArray Client::currentDatabase() const
{
	return d->currentDatabase;
}

void Client::connectToKeePass(const QString &keePassPath)
{
	if(d->connector->isConnected()) {
		d->setError({}, Error::ClientAlreadyConnected);
		return;
	}

	d->clear();
	d->connector->connectToKeePass(keePassPath);
}

void Client::disconnectFromKeePass()
{
	d->connector->disconnectFromKeePass();
}

void Client::openDatabase()
{
	if(state() != State::Locked)
		return;
	d->connector->sendEncrypted(ClientPrivate::ActionGetDatabaseHash, {},
								d->options.testFlag(Option::TriggerUnlock));
}

void Client::closeDatabase()
{
	if(state() != State::Unlocked)
		return;
	d->connector->sendEncrypted(ClientPrivate::ActionLockDatabase);
}

void Client::generatePassword()
{
	d->connector->sendEncrypted(ClientPrivate::ActionGeneratePassword);
}

void Client::getLogins(const QUrl &url, const QUrl &submitUrl, bool httpAuth, bool searchAllDatabases)
{
	QJsonObject message;
	message[QStringLiteral("id")] = d->dbReg->getClientId(d->currentDatabase).name;
	message[QStringLiteral("url")] = url.toString(QUrl::FullyEncoded);
	if(!submitUrl.isEmpty())
		message[QStringLiteral("submitUrl")] = submitUrl.toString(QUrl::FullyEncoded);
	else
		message[QStringLiteral("submitUrl")] = message[QStringLiteral("url")];
	message[QStringLiteral("httpAuth")] = QVariant{httpAuth}.toString();

	QList<IDatabaseRegistry::ClientId> clientIds;
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

	d->connector->sendEncrypted(ClientPrivate::ActionGetLogins, message);
}

void Client::addLogin(const QUrl &url, const Entry &entry, const QUrl &submitUrl)
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

	d->connector->sendEncrypted(ClientPrivate::ActionSetLogin, message);
}

void Client::setDatabaseRegistry(IDatabaseRegistry *databaseRegistry)
{
	if (d->dbReg == databaseRegistry)
		return;

	if(d->dbReg)
		dynamic_cast<QObject*>(d->dbReg)->deleteLater();
	d->dbReg = databaseRegistry;
	dynamic_cast<QObject*>(d->dbReg)->setParent(this);
	emit databaseRegistryChanged(d->dbReg, {});
}

void Client::setOptions(Options options)
{
	if (d->options == options)
		return;

	d->options = options;
	emit optionsChanged(d->options, {});
}

bool Client::allowDatabase(const QByteArray &databaseHash) const
{
	Q_UNUSED(databaseHash)
	return d->options.testFlag(Option::AllowNewDatabase);
}

void Client::dbConnected()
{
	emit connected({});
	if(d->options.testFlag(Option::OpenOnConnect))
		openDatabase();
}

void Client::dbDisconnected()
{
	qDebug() << Q_FUNC_INFO;
	emit disconnected({});
	d->clear();
}

void Client::dbError(Client::Error code, const QString &message)
{
	d->setError({}, code, message);
}

void Client::dbLocked()
{
	if(d->locked)
		return;

	d->locked = true;
	emit databaseClosed({});
	if(d->options.testFlag(Option::DisconnectOnClose))
		disconnectFromKeePass();
}

void Client::dbUnlocked()
{
	if(!d->locked)
		return;

	openDatabase();
}

void Client::dbMsgRecv(const QString &action, const QJsonObject &message)
{
	if(action == ClientPrivate::ActionGetDatabaseHash)
		d->onDbHash(message);
	else if(action == ClientPrivate::ActionAssociate)
		d->onAssoc(message);
	else if(action == ClientPrivate::ActionTestAssociate)
		d->onTestAssoc(message);
	else if(action == ClientPrivate::ActionGeneratePassword)
		d->onGeneratePasswd(message);
	else if(action == ClientPrivate::ActionGetLogins)
		d->onGetLogins(message);
	else if(action == ClientPrivate::ActionSetLogin)
		emit loginAdded({});
	else if(action == ClientPrivate::ActionLockDatabase)
		dbLocked();
	else
		d->setError(action, Error::ClientUnsupportedAction, action);
}

void Client::dbMsgFail(const QString &action, Error code, const QString &message)
{
	if(code == Error::KeePassDatabaseNotOpen &&
	   action == ClientPrivate::ActionGetDatabaseHash &&
	   d->options.testFlag(Option::TriggerUnlock)) {
		qInfo() << "Database locked. Waiting for user to unlock";
	} else if(code == Error::KeePassAssociationFailed &&
			  action == ClientPrivate::ActionTestAssociate) {
		qWarning() << "Current association was rejected. Initiation re-association";
		d->dbReg->removeClientId(d->currentDatabase);
		d->sendAssoc();
	} else
		d->setError(action, code, message);
}

// ------------- Private implementation -------------

const QString ClientPrivate::ActionGetDatabaseHash{QStringLiteral("get-databasehash")};
const QString ClientPrivate::ActionTestAssociate{QStringLiteral("test-associate")};
const QString ClientPrivate::ActionAssociate{QStringLiteral("associate")};
const QString ClientPrivate::ActionGeneratePassword{QStringLiteral("generate-password")};
const QString ClientPrivate::ActionGetLogins{QStringLiteral("get-logins")};
const QString ClientPrivate::ActionSetLogin{QStringLiteral("set-login")};
const QString ClientPrivate::ActionLockDatabase{QStringLiteral("lock-database")};

bool ClientPrivate::initialized = false;

ClientPrivate::ClientPrivate(Client *q_ptr) :
	q{q_ptr},
	connector{new Connector{q_ptr}},
	dbReg{new DefaultDatabaseRegistry{q_ptr}}
{}

void ClientPrivate::setError(const QString &action, Client::Error error, const QString &msg)
{
	QString errorMessage;
	switch(error) {
	// KeePassXC errors -> only use msg description
	case Client::Error::KeePassDatabaseNotOpen:
	case Client::Error::KeePassDatabaseHashNotReceived:
	case Client::Error::KeePassPublicKeyNotReceived:
	case Client::Error::KeePassCannotDecryptMessage:
	case Client::Error::KeePassTimeout:
	case Client::Error::KeePassActionDenied:
	case Client::Error::KeePassCannotEncryptMessage:
	case Client::Error::KeePassAssociationFailed:
	case Client::Error::KeePassKeyChangeFailed:
	case Client::Error::KeePassEncryptionKeyUnrecognized:
	case Client::Error::KeePassNoSavedDatabase:
	case Client::Error::KeePassIncorrectAction:
	case Client::Error::KeePassEmptyMessageReceived:
	case Client::Error::KeePassNoUrlProvided:
	case Client::Error::KeePassNoLoginsFound:
		errorMessage = msg;
		break;
	// Client errors
	case Client::Error::ClientAlreadyConnected:
		errorMessage = Client::tr("Already connected to a KeePassXC instance");
		break;
	case Client::Error::ClientKeyGenerationFailed:
		errorMessage = Client::tr("Failed to generate session keys");
		break;
	case Client::Error::ClientReceivedNonceInvalid:
		errorMessage = Client::tr("Unexpected nonce received from KeePassXC");
		break;
	case Client::Error::ClientJsonParseError:
		errorMessage = Client::tr("Received JSON-data is invalid. JSON-Error: %1")
						  .arg(msg);
		break;
	case Client::Error::ClientUnsupportedVersion:
		errorMessage = Client::tr("Unsupported KeePassXC Version. Must be at least %1, but currently is %2")
						  .arg(Connector::minimumKeePassXCVersion.toString(), msg);
		break;
	case Client::Error::ClientDatabaseChanged:
		errorMessage = Client::tr("The opened database in KeePassXC was changed");
		break;
	case Client::Error::ClientDatabaseRejected:
		errorMessage = Client::tr("The database hash was not known and thus rejected");
		break;
	case Client::Error::ClientUnsupportedAction:
		errorMessage = Client::tr("An unsupported action was received from KeePassXC: %1")
						  .arg(msg);
		break;
	// General errors
	case Client::Error::UnknownError:
	default:
		errorMessage = Client::tr("Unknown Error");
		break;
	}

	auto unrecoverable = true;
	switch (error) {
	// KeePassXC errors -> only use msg description
	case Client::Error::KeePassDatabaseNotOpen:
	case Client::Error::KeePassTimeout:
	case Client::Error::KeePassActionDenied:
	case Client::Error::KeePassAssociationFailed:
	case Client::Error::KeePassIncorrectAction:
	case Client::Error::KeePassEmptyMessageReceived:
	case Client::Error::KeePassNoUrlProvided:
	case Client::Error::KeePassNoLoginsFound:
	// Client errors
	case Client::Error::ClientAlreadyConnected:
	case Client::Error::ClientDatabaseChanged:
		unrecoverable = false;
		break;
	default:
		break;
	}

	emit q->errorOccured(error, errorMessage, action, unrecoverable, {});
	if(unrecoverable)
		q->disconnectFromKeePass();
}

void ClientPrivate::clear()
{
	locked = true;
	currentDatabase.clear();
}

void ClientPrivate::onDbHash(const QJsonObject &message)
{
	const auto dbHash = QByteArray::fromHex(message[QStringLiteral("hash")].toString().toUtf8());
	if(currentDatabase.isEmpty()) {
		currentDatabase = dbHash;
		emit q->currentDatabaseChanged(currentDatabase, {});
	} else if(currentDatabase != dbHash) {
		if(options.testFlag(Client::Option::AllowDatabaseChange)) {
			currentDatabase = dbHash;
			emit q->currentDatabaseChanged(currentDatabase, {});
		} else {
			setError(ActionGetDatabaseHash, Client::Error::ClientDatabaseChanged);
			return;
		}
	}

	if(dbReg->hasClientId(currentDatabase))
		sendTestAssoc();
	else if(q->allowDatabase(currentDatabase))
		sendAssoc();
	else
		setError(ActionGetDatabaseHash, Client::Error::ClientDatabaseRejected);
}

void ClientPrivate::onAssoc(const QJsonObject &message)
{
	const auto hash = QByteArray::fromHex(message[QStringLiteral("hash")].toString().toUtf8());
	if(hash != currentDatabase) {
		setError(ActionAssociate, Client::Error::ClientDatabaseChanged);
		return;
	}

	IDatabaseRegistry::ClientId cId;
	cId.name = message[QStringLiteral("id")].toString();
	cId.key = std::move(_keyCache);
	cId.key.makeReadonly();
	dbReg->addClientId(currentDatabase, std::move(cId));
	locked = false;
	emit q->databaseOpened(currentDatabase, {});
}

void ClientPrivate::onTestAssoc(const QJsonObject &message)
{
	const auto hash = QByteArray::fromHex(message[QStringLiteral("hash")].toString().toUtf8());
	if(hash != currentDatabase) {
		setError(ActionTestAssociate, Client::Error::ClientDatabaseChanged);
		return;
	}
	locked = false;
	emit q->databaseOpened(currentDatabase, {});
}

void ClientPrivate::onGeneratePasswd(const QJsonObject &message)
{
	const auto entries = message[QStringLiteral("entries")].toArray();
	QStringList passwords;
	passwords.reserve(entries.size());
	for(const auto entry : entries)
		passwords.append(entry.toObject()[QStringLiteral("password")].toString());
	emit q->passwordsGenerated(passwords, {});
}

void ClientPrivate::onGetLogins(const QJsonObject &message)
{
	const auto jEntries = message[QStringLiteral("entries")].toArray();
	QList<Entry> entries;
	entries.reserve(jEntries.size());
	for(const auto jEntryVal : jEntries) {
		const auto jEntry = jEntryVal.toObject();
		auto uuidStr = jEntry[QStringLiteral("uuid")].toString();
		uuidStr = uuidStr.mid(0, 8) + QLatin1Char('-') +
				  uuidStr.mid(8, 4) + QLatin1Char('-') +
				  uuidStr.mid(12, 4) + QLatin1Char('-') +
				  uuidStr.mid(16, 4) + QLatin1Char('-') +
				  uuidStr.mid(20, 12);

		Entry entry;
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

void ClientPrivate::sendTestAssoc()
{
	auto cId = dbReg->getClientId(currentDatabase);
	QJsonObject message;
	message[QStringLiteral("id")] = cId.name;
	message[QStringLiteral("key")] = cId.key.toBase64();
	connector->sendEncrypted(ActionTestAssociate, message);
}

void ClientPrivate::sendAssoc()
{
	_keyCache = connector->cryptor()->generateRandom(crypto_box_PUBLICKEYBYTES, SecureByteArray::State::Readonly);
	QJsonObject message;
	message[QStringLiteral("key")] = connector->cryptor()->publicKey().toBase64();
	message[QStringLiteral("idKey")] = _keyCache.toBase64();
	_keyCache.makeNoaccess();
	connector->sendEncrypted(ActionAssociate, message);
}
