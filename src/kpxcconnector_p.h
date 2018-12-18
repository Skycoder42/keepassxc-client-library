#ifndef KPXCCONNECTOR_P_H
#define KPXCCONNECTOR_P_H

#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>

#include "securebytearray_p.h"
#include "sodiumcryptor_p.h"

class KPXCConnector : public QObject
{
	Q_OBJECT

public:
	explicit KPXCConnector(QObject *parent = nullptr);

public Q_SLOTS:
	void connectToKeePass(const QString &target);
	void disconnectFromKeePass();
	void reconnectToKeePass();

signals:
	void connected();
	void disconnected();

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
	SecureByteArray _nonce;

	enum {
		PhaseConnecting,
		PhaseConnected,
		PhaseEof,
		PhaseTerminate,
		PhaseKill
	} _disconnectPhase = PhaseKill;
	QTimer *_disconnectTimer;

	void sendMessage(const QJsonObject &message);
	void cleanup();
};

#endif // KPXCCONNECTOR_P_H
