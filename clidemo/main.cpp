#include <QCoreApplication>
#include <kpxcclient.h>

#include <QDebug>
#include <kpxcconnector_p.h>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	KPXCClient::init();

	KPXCConnector connector;
	QObject::connect(&connector, &KPXCConnector::disconnected,
					 qApp, &QCoreApplication::quit);
	connector.connectToKeePass(QStringLiteral("keepassxc-proxy"));

	QTimer::singleShot(5000, &connector, [&](){
		qDebug("Triggered disconnect");
		connector.disconnectFromKeePass();
	});

	return a.exec();
}
