#include "kpxcclient.h"
#include "kpxcclient_p.h"
#include <QtCore/QDebug>
#include <sodium/randombytes.h>
#include <sodium/randombytes_sysrandom.h>
#include <sodium/core.h>

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

KPXCClient::Error KPXCClient::error() const
{
	return d->lastError;
}

QString KPXCClient::errorString() const
{
	return d->lastErrorString;
}

void KPXCClient::connectToKeePass(const QString &keePassPath)
{
	if(d->connector->isConnected()) {
		d->setError(Error::ClientAlreadyConnected);
		return;
	}

	d->reconnectOnUnlock = false;
	d->setError(Error::NoError);
	d->connector->connectToKeePass(keePassPath);
}

void KPXCClient::disconnectFromKeePass()
{
	d->connector->disconnectFromKeePass();
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
	// encrypted channel established -> get db data
	d->connector->sendEncrypted(KPXCClientPrivate::ActionGetDatabaseHash, {},
								d->options.testFlag(Option::TriggerUnlock));
}

void KPXCClient::dbDisconnected()
{
	qDebug() << Q_FUNC_INFO;
	emit disconnected({});
	d->setError(Error::NoError);
	d->reconnectOnUnlock = false;
}

void KPXCClient::dbError(KPXCClient::Error code, const QString &message)
{
	d->setError(code, message);
	disconnectFromKeePass();
}

void KPXCClient::dbLocked()
{
	Q_UNIMPLEMENTED();
}

void KPXCClient::dbUnlocked()
{
	Q_UNIMPLEMENTED();
	if(d->reconnectOnUnlock) {
		d->reconnectOnUnlock = false;
		dbConnected();
	}
}

void KPXCClient::dbMsgRecv(const QString &action, const QJsonObject &message)
{
	Q_UNIMPLEMENTED();
	qDebug() << Q_FUNC_INFO << action << message;
}

void KPXCClient::dbMsgFail(const QString &action, Error code, const QString &message)
{
	if(action == KPXCClientPrivate::ActionGetDatabaseHash &&
	   code == Error::KeePassDatabaseNotOpen &&
	   d->options.testFlag(Option::TriggerUnlock)) {
		d->reconnectOnUnlock = true;
	} else {
		dbError(code, message);
		Q_UNIMPLEMENTED();
		//TODO forward failure to action handler?
	}
}

// ------------- Private implementation -------------

const QString KPXCClientPrivate::ActionGetDatabaseHash{QStringLiteral("get-databasehash")};

bool KPXCClientPrivate::initialized = false;

KPXCClientPrivate::KPXCClientPrivate(KPXCClient *q_ptr) :
	q{q_ptr},
	connector{new KPXCConnector{q_ptr}},
	dbReg{new KPXCDefaultDatabaseRegistry{q_ptr}}
{}

void KPXCClientPrivate::setError(KPXCClient::Error error, const QString &msg)
{
	if(error == lastError)
		return;

	lastError = error;
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
		lastErrorString = msg;
		break;
	// Client errors
	case KPXCClient::Error::ClientAlreadyConnected:
		lastErrorString = KPXCClient::tr("Already connected to a KeePassXC instance");
		break;
	case KPXCClient::Error::ClientKeyGenerationFailed:
		lastErrorString = KPXCClient::tr("Failed to generate session keys");
		break;
	case KPXCClient::Error::ClientReceivedNonceInvalid:
		lastErrorString = KPXCClient::tr("Unexpected nonce received from KeePassXC");
		break;
	case KPXCClient::Error::ClientJsonParseError:
		lastErrorString = KPXCClient::tr("Received JSON-data is invalid. JSON-Error: %1")
						  .arg(msg);
		break;
	case KPXCClient::Error::ClientActionsDontMatch:
		lastErrorString = KPXCClient::tr("Data-Action field of encrypted and unencrypted message are not equal");
		break;
	case KPXCClient::Error::ClientUnsupportedVersion:
		lastErrorString = KPXCClient::tr("Unsupported KeePassXC Version. Must be at least %1, but currently is %2")
						  .arg(KPXCConnector::minimumKeePassXCVersion.toString(), msg);
		break;
	// General errors
	case KPXCClient::Error::NoError:
		lastErrorString = KPXCClient::tr("No Error");
		break;
	case KPXCClient::Error::UnknownError:
	default:
		lastErrorString = KPXCClient::tr("Unknown Error");
		break;
	}
	emit q->errorChanged(lastError, {});
}
