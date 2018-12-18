#ifndef KPXCCLIENT_H
#define KPXCCLIENT_H

#include <QtCore/QScopedPointer>
#include <QtCore/QObject>
#include <QtCore/QJsonObject>

#include "kpxcclient_global.h"
#include "kpxcdatabaseregistry.h"

class KPXCClientPrivate;
class KPXCCLIENT_EXPORT KPXCClient : public QObject
{
	Q_OBJECT

	Q_PROPERTY(IKPXCDatabaseRegistry* databaseRegistry READ databaseRegistry WRITE setDatabaseRegistry NOTIFY databaseRegistryChanged)
	Q_PROPERTY(bool allowNewDatabase READ doesAllowNewDatabase WRITE setAllowNewDatabase NOTIFY allowNewDatabaseChanged)
	Q_PROPERTY(bool triggerUnlock READ triggersUnlock WRITE setTriggerUnlock NOTIFY triggerUnlockChanged)

public:
	enum class Error {
		NoError = 0,
		UnknownError = -1,

		// KeePassXC internal errors
		KeePassDatabaseNotOpen = 0x0001,
		KeePassDatabaseHashNotReceived = 0x0002,
		KeePassPublicKeyNotReceived = 0x0003,
		KeePassCannotDecryptMessage = 0x0004,
		KeePassTimeout = 0x0005,
		KeePassActionDenied = 0x0006,
		KeePassCannotEncryptMessage = 0x0007,
		KeePassAssociationFailed = 0x0008,
		KeePassKeyChangeFailed = 0x0009,
		KeePassEncryptionKeyUnrecognized = 0x000A,
		KeePassNoSavedDatabase = 0x000B,
		KeePassIncorrectAction = 0x000C,
		KeePassEmptyMessageReceived = 0x000D,
		KeePassNoUrlProvided = 0x000E,
		KeePassNoLoginsFound = 0x000F,

		// Client library errors
		ClientAlreadyConnected = 0x00010000,
		ClientKeyGenerationFailed = 0x00020000,
		ClientReceivedNonceInvalid = 0x00030000,
		ClientJsonParseError = 0x00040000,
		ClientActionsDontMatch = 0x00050000,
		ClientUnsupportedVersion = 0x00060000
	};
	Q_ENUM(Error)

	static bool init();

	explicit KPXCClient(QObject *parent = nullptr);
	~KPXCClient() override;

	IKPXCDatabaseRegistry* databaseRegistry() const;
	bool doesAllowNewDatabase() const;

	Error error() const;
	QString errorString() const;
	bool triggersUnlock() const;

public Q_SLOTS:
	void connectToKeePass(const QString &keePassPath = QStringLiteral("keepassxc-proxy"));
	void disconnectFromKeePass();

	void setDatabaseRegistry(IKPXCDatabaseRegistry* databaseRegistry);
	void setAllowNewDatabase(bool allowNewDatabase);
	void setTriggerUnlock(bool triggerUnlock);

Q_SIGNALS:
	void connected(QPrivateSignal);
	void disconnected(QPrivateSignal);

	void databaseRegistryChanged(IKPXCDatabaseRegistry* databaseRegistry, QPrivateSignal);
	void allowNewDatabaseChanged(bool allowNewDatabase, QPrivateSignal);
	void triggerUnlockChanged(bool triggerUnlock, QPrivateSignal);
	void errorChanged(Error error, QPrivateSignal);

protected:
	virtual bool allowDatabase(const QByteArray &databaseHash) const;

private Q_SLOTS:
	void dbConnected();
	void dbDisconnected();
	bool dbError(Error code, const QString &message);
	void dbLocked();
	void dbUnlocked();
	void dbMsgRecv(const QString &action, const QJsonObject &message);
	void dbMsgFail(const QString &action, Error code, const QString &message);

private:
	friend class KPXCClientPrivate;
	QScopedPointer<KPXCClientPrivate> d;
};

#endif // KPXCCLIENT_H
