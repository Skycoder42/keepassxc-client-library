#include "kpxcconnector_p.h"
#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QStandardPaths>
#include <QThread>
#include <chrono>

KPXCConnector::KPXCConnector(QObject *parent) :
	QObject{parent},
	_cryptor{new SodiumCryptor{this}},
	_disconnectTimer{new QTimer{this}}
{
	using namespace std::chrono_literals;
	_disconnectTimer->setInterval(500ms);
	_disconnectTimer->setSingleShot(true);
	_disconnectTimer->setTimerType(Qt::CoarseTimer);
	connect(_disconnectTimer, &QTimer::timeout,
			this, &KPXCConnector::disconnectFromKeePass);
}

void KPXCConnector::connectToKeePass(const QString &target)
{
	if(_process) {
		//emit error
		return;
	}

	if(!_cryptor->createKeys())
		; //emit error
	_serverKey.deallocate();
	_clientId = _cryptor->generateRandomNonce(SecureByteArray::State::Readonly);
	_nonce = _cryptor->generateRandomNonce(SecureByteArray::State::Readonly);

	const auto procPath = QStandardPaths::findExecutable(target);
	if(procPath.isEmpty())
		; //emit error
	_process = new QProcess{this};
	_process->setProgram(procPath);
	connect(_process, &QProcess::started,
			this, &KPXCConnector::started);
	connect(_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			this, &KPXCConnector::finished);
	connect(_process, &QProcess::errorOccurred,
			this, &KPXCConnector::procError);
	connect(_process, &QProcess::readyReadStandardOutput,
			this, &KPXCConnector::stdOutReady);
	connect(_process, &QProcess::readyReadStandardError,
			this, &KPXCConnector::stdErrReady);

	_disconnectPhase = PhaseConnecting;
	_process->start();
}

void KPXCConnector::disconnectFromKeePass()
{
	if(!_process)
		return;

	switch(_disconnectPhase) {
	case PhaseConnected:
		qDebug() << "Disconnecting Phase: Sending EOF";
		_disconnectPhase = PhaseEof;
		_process->closeWriteChannel();
		_disconnectTimer->start();
		break;
	case PhaseConnecting:
	case PhaseEof:
		qDebug() << "Disconnecting Phase: Sending SIGTERM";
		_disconnectPhase = PhaseTerminate;
		_process->terminate();
		_disconnectTimer->start();
		break;
	case PhaseTerminate:
		qDebug() << "Disconnecting Phase: Sending SIGKILL";
		_disconnectPhase = PhaseKill;
		_process->kill();
		_disconnectTimer->start();
		break;
	case PhaseKill:
		qDebug() << "Disconnecting Phase: Dropping process";
		cleanup();
		break;
	default:
		Q_UNREACHABLE();
		break;
	}
}

void KPXCConnector::reconnectToKeePass()
{

}

void KPXCConnector::started()
{
	_disconnectPhase = PhaseConnected;
	QJsonObject keysMessage;
	keysMessage[QStringLiteral("action")] = QStringLiteral("change-public-keys");
	keysMessage[QStringLiteral("publicKey")] = _cryptor->publicKey().toBase64();
	keysMessage[QStringLiteral("nonce")] = _nonce.toBase64();
	keysMessage[QStringLiteral("clientID")] = _clientId.toBase64();
	_nonce.increment(true);
	sendMessage(keysMessage);
}

void KPXCConnector::finished(int exitCode, QProcess::ExitStatus exitStatus)
{
	cleanup();
	emit disconnected();
}

void KPXCConnector::procError(QProcess::ProcessError error)
{
	qCritical() << error << _process->errorString();
}

void KPXCConnector::stdOutReady()
{
	qDebug() << "stdout" << _process->readAllStandardOutput()
			 << _nonce.toBase64();
}

void KPXCConnector::stdErrReady()
{
	qWarning() << "stderr" << _process->readAllStandardError();
}

void KPXCConnector::sendMessage(const QJsonObject &message)
{
	const auto data = QJsonDocument{message}.toJson(QJsonDocument::Compact);
	QByteArray length{sizeof(quint32), 0};
	*reinterpret_cast<quint32*>(length.data()) = data.size();
	_process->write(length + data);
}

void KPXCConnector::cleanup()
{
	_disconnectTimer->stop();
	if(_process) {
		_process->disconnect(this);
		_process->deleteLater();
		_process = nullptr;
	}
	_cryptor->dropKeys();
	_serverKey.deallocate();
	_clientId.deallocate();
	_nonce.deallocate();
	_disconnectPhase = PhaseKill;
}
