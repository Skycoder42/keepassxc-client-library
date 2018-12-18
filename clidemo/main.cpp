#include <QCoreApplication>
#include <kpxcclient.h>

#include <QDebug>
#include <QTimer>

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setOrganizationName("baum42");

	KPXCClient::init();

	KPXCClient client;
	static_cast<KPXCDefaultDatabaseRegistry*>(client.databaseRegistry())->setPersistent(true);

	QObject::connect(&client, &KPXCClient::errorChanged, [&](KPXCClient::Error error) {
		if(error == KPXCClient::Error::NoError)
			return;
		qDebug() << error << client.errorString();
	});
	QObject::connect(&client, &KPXCClient::databaseOpened, [&](QByteArray dbHash) {
		qDebug() << "[[MAIN]]" << "Connected to database:" << dbHash.toHex();
	});
	QObject::connect(&client, &KPXCClient::databaseClosed, [&]() {
		qDebug() << "[[MAIN]]" << "Database connection closed";
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
