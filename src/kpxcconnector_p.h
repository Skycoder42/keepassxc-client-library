#ifndef KPXCCONNECTOR_P_H
#define KPXCCONNECTOR_P_H

#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>
#include <QtCore/QVersionNumber>

#include "securebytearray.h"
#include "kpxcclient.h"
#include "sodiumcryptor_p.h"

class KPXCConnector : public QObject
{
	Q_OBJECT

public:
	static const QVersionNumber minimumKeePassXCVersion;

	explicit KPXCConnector(QObject *parent = nullptr);

	bool isConnected() const;

public Q_SLOTS:
	void connectToKeePass(const QString &target);
	void disconnectFromKeePass();

	void sendEncrypted(const QString &action,
					   QJsonObject message = {},
					   bool triggerUnlock = false);

Q_SIGNALS:
	void connected();
	void disconnected();
	void error(KPXCClient::Error code, const QString &message = {});

	void locked();
	void unlocked();
	void messageReceived(const QString &action, const QJsonObject &message);
	void messageFailed(const QString &action, KPXCClient::Error code, const QString &message ={});

private Q_SLOTS:
	void started();
	void finished(int exitCode, QProcess::ExitStatus exitStatus);
	void procError(QProcess::ProcessError error);
	void stdOutReady();
	void stdErrReady();

private:
	QProcess *_process = nullptr;

	SodiumCryptor *_cryptor;
	SecureByteArray _serverKey;
	SecureByteArray _clientId;
	SecureByteArray _nonce; //TODO remove or per action

	enum {
		PhaseConnecting,
		PhaseConnected,
		PhaseEof,
		PhaseTerminate,
		PhaseKill
	} _connectPhase = PhaseKill;
	QTimer *_disconnectTimer;

	void sendMessage(const QJsonObject &message);
	void cleanup();

	QJsonObject readMessageData();
	bool performChecks(const QString &action, const QJsonObject &message);
	void handleChangePublicKeys(const QString &publicKey);
};

#endif // KPXCCONNECTOR_P_H
