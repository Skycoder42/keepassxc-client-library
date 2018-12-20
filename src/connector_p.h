#ifndef KPXCCLIENT_CONNECTOR_P_H
#define KPXCCLIENT_CONNECTOR_P_H

#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>
#include <QtCore/QVersionNumber>
#include <QtCore/QSet>

#include "securebytearray.h"
#include "client.h"
#include "sodiumcryptor_p.h"

namespace KPXCClient {

class Connector : public QObject
{
	Q_OBJECT

public:
	static const QVersionNumber minimumKeePassXCVersion;

	explicit Connector(QObject *parent = nullptr);

	bool isConnected() const;
	bool isConnecting() const;

	SodiumCryptor *cryptor() const;

public Q_SLOTS:
	void connectToKeePass(const QString &target);
	void disconnectFromKeePass();

	void sendEncrypted(const QString &action,
					   QJsonObject message = {},
					   bool triggerUnlock = false);

Q_SIGNALS:
	void connected();
	void disconnected();
	void error(Client::Error code, const QString &message = {});

	void locked();
	void unlocked();
	void messageReceived(const QString &action, const QJsonObject &message);
	void messageFailed(const QString &action, Client::Error code, const QString &message ={});

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
	QSet<SecureByteArray> _allowedNonces;

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

}

#endif // KPXCCLIENT_CONNECTOR_P_H
