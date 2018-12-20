#include <QCoreApplication>
#include <client.h>
#include <defaultdatabaseregistry.h>

#include <QDebug>
#include <QTimer>

#ifdef USE_CTRL_SIGNALS
#include <QCtrlSignals>
#endif

using namespace KPXCClient;

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);
	QCoreApplication::setOrganizationName("baum42");

	KPXCClient::init();

	Client client;
	static_cast<DefaultDatabaseRegistry*>(client.databaseRegistry())->setPersistent(true);

#ifdef USE_CTRL_SIGNALS
	QCtrlSignalHandler::instance()->registerForSignal(QCtrlSignalHandler::SigInt);
	QCtrlSignalHandler::instance()->registerForSignal(QCtrlSignalHandler::SigTerm);
	QObject::connect(QCtrlSignalHandler::instance(), &QCtrlSignalHandler::ctrlSignal,
					 &client, &Client::disconnectFromKeePass);
#endif

	QObject::connect(&client, &Client::errorOccured, [&](Client::Error error, QString msg, QString action, bool unrecoverable) {
		qDebug() << error << msg << action << unrecoverable;
	});
	QObject::connect(&client, &Client::databaseOpened, [&](QByteArray dbHash) {
		qDebug() << "[[MAIN]]" << "Connected to database:" << dbHash.toHex();
		client.generatePassword();
	});
	QObject::connect(&client, &Client::passwordsGenerated, [&](QStringList pwds) {
		qDebug() << "[[MAIN]]" << "Passwords received:" << pwds;
		client.addLogin(QStringLiteral("https://example.com"), {"name", pwds.first()});
	});
	QObject::connect(&client, &Client::loginsReceived, [&](QList<Entry> entries) {
		qDebug() << "[[MAIN]]" << "Entries received:" << entries.size();
		Entry changeEntry;
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
	QObject::connect(&client, &Client::loginAdded, [&]() {
		qDebug() << "[[MAIN]]" << "Login data was accepted by keepass";
		client.getLogins(QStringLiteral("https://example.com/baum42"));
	});
	QObject::connect(&client, &Client::databaseClosed, [&]() {
		qDebug() << "[[MAIN]]" << "Database connection closed";
	});
	QObject::connect(&client, &Client::disconnected,
					 qApp, &QCoreApplication::quit,
					 Qt::QueuedConnection);
	client.connectToKeePass();

	return a.exec();
}
