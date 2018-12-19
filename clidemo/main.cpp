#include <QCoreApplication>
#include <kpxcclient.h>

#include <QDebug>
#include <QTimer>

#ifdef USE_CTRL_SIGNALS
#include <QCtrlSignals>
#endif

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setOrganizationName("baum42");

	KPXCClient::init();

	KPXCClient client;
	static_cast<KPXCDefaultDatabaseRegistry*>(client.databaseRegistry())->setPersistent(true);

#ifdef USE_CTRL_SIGNALS
	QCtrlSignalHandler::instance()->registerForSignal(QCtrlSignalHandler::SigInt);
	QCtrlSignalHandler::instance()->registerForSignal(QCtrlSignalHandler::SigTerm);
	QObject::connect(QCtrlSignalHandler::instance(), &QCtrlSignalHandler::ctrlSignal,
					 &client, &KPXCClient::disconnectFromKeePass);
#endif

	QObject::connect(&client, &KPXCClient::errorOccured, [&](KPXCClient::Error error, QString msg) {
		qDebug() << error << msg;
	});
	QObject::connect(&client, &KPXCClient::databaseOpened, [&](QByteArray dbHash) {
		qDebug() << "[[MAIN]]" << "Connected to database:" << dbHash.toHex();
		client.generatePassword();
	});
	QObject::connect(&client, &KPXCClient::passwordsGenerated, [&](QStringList pwds) {
		qDebug() << "[[MAIN]]" << "Passwords received:" << pwds;
		client.addLogin(QStringLiteral("https://example.com"), {"name", pwds.first()});
	});
	QObject::connect(&client, &KPXCClient::loginsReceived, [&](QList<KPXCEntry> entries) {
		qDebug() << "[[MAIN]]" << "Entries received:" << entries.size();
		KPXCEntry changeEntry;
		for(auto entry : entries) {
			qDebug() << "  >>" << entry.uuid()
					 << entry.title()
					 << entry.username()
					 << entry.password()
					 << entry.totp()
					 << entry.extraFields();
			if(entry.username() == "name")
				changeEntry = entry;
		}
		if(changeEntry.isStored()) {
			changeEntry.setUsername("name2");
			changeEntry.setPassword(changeEntry.password() + "*");
			client.addLogin(QStringLiteral("https://example.com"), changeEntry);
		} else
			client.closeDatabase();
	});
	QObject::connect(&client, &KPXCClient::loginAdded, [&]() {
		qDebug() << "[[MAIN]]" << "Login data was accepted by keepass";
		client.getLogins(QStringLiteral("https://example.com/baum42"));
	});
	QObject::connect(&client, &KPXCClient::databaseClosed, [&]() {
		qDebug() << "[[MAIN]]" << "Database connection closed";
	});
	QObject::connect(&client, &KPXCClient::disconnected,
					 qApp, &QCoreApplication::quit,
					 Qt::QueuedConnection);
	client.connectToKeePass();

	return a.exec();
}
