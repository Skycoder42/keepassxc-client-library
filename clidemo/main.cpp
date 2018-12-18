#include <QCoreApplication>
#include <kpxcclient.h>

#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	KPXCClient::init();

	KPXCClient client;
	QObject::connect(&client, &KPXCClient::errorChanged, [&](KPXCClient::Error error) {
		if(error == KPXCClient::Error::NoError)
			return;
		qDebug() << error << client.errorString();
	});
	QObject::connect(&client, &KPXCClient::currentDatabaseChanged, [&](QByteArray dbHash) {
		qDebug() << dbHash.toHex();
	});
	QObject::connect(&client, &KPXCClient::disconnected,
					 qApp, &QCoreApplication::quit,
					 Qt::QueuedConnection);
	client.connectToKeePass();

	QTimer::singleShot(30000, &client, [&](){
		qDebug("Triggered disconnect");
		client.disconnectFromKeePass();
	});

	return a.exec();
}
