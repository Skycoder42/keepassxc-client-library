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
	QCoreApplication::setOrganizationName(QStringLiteral("Skycoder42"));
	QCoreApplication::setOrganizationDomain(QStringLiteral("de.skycoder42"));

	// init the client
	KPXCClient::init();

	// create a client
	Client client;
	client.setOptions(client.options() | Client::Option::DisconnectOnClose);
	// optional: make database registrations persistent
	static_cast<DefaultDatabaseRegistry*>(client.databaseRegistry())->setPersistent(true);

#ifdef USE_CTRL_SIGNALS
	QCtrlSignalHandler::instance()->registerForSignal(QCtrlSignalHandler::SigInt);
	QCtrlSignalHandler::instance()->registerForSignal(QCtrlSignalHandler::SigTerm);
	QObject::connect(QCtrlSignalHandler::instance(), &QCtrlSignalHandler::ctrlSignal,
					 &client, &Client::disconnectFromKeePass);
#endif

	// display errors
	QObject::connect(&client, &Client::errorOccured, [&](Client::Error error, QString msg, QString action, bool unrecoverable) {
		qDebug() << error << msg << action << unrecoverable;
	});

	// on database opened: let it generate a new password
	QObject::connect(&client, &Client::databaseOpened, [&](QByteArray dbHash) {
		qDebug() << "[[MAIN]]" << "Connected to database:" << dbHash.toHex();
		client.generatePassword();
	});

	// once the password was generated, add a new entry that uses that password
	QObject::connect(&client, &Client::passwordsGenerated, [&](QStringList pwds) {
		qDebug() << "[[MAIN]]" << "Passwords received:" << pwds;
		client.addLogin(QStringLiteral("https://example.com"), {QStringLiteral("name"), pwds.first()});
	});

	// after the new login was created, display all logins for that URL
	QObject::connect(&client, &Client::loginAdded, [&]() {
		qDebug() << "[[MAIN]]" << "Login data was accepted by keepass";
		client.getLogins(QStringLiteral("https://example.com"));
	});

	// first trigger: print all the received logins and modify the previously created one
	// second trigger: print all the received logins, then lock the database (after a timeout so it can save the changes)
	QObject::connect(&client, &Client::loginsReceived, [&](QList<Entry> entries) {
		qDebug() << "[[MAIN]]" << "Entries received:" << entries.size();
		Entry changeEntry;
		for(const auto &entry : entries) {
			qDebug() << "  >>" << entry.uuid()
					 << entry.title()
					 << entry.username()
					 << entry.password()
					 << entry.totp()
					 << entry.extraFields();
			if(entry.username() == QStringLiteral("name"))
				changeEntry = entry;
		}
		if(changeEntry.isStored()) {
			changeEntry.setUsername("name2");
			changeEntry.setPassword(changeEntry.password() + "*");
			client.addLogin(QStringLiteral("https://example.com"), changeEntry);
		} else {
			QTimer::singleShot(1000, &client, &Client::closeDatabase);
		}
	});

	// If the database was closed, print it to the log (the previous option triggeres a disconnect)
	QObject::connect(&client, &Client::databaseClosed, [&]() {
		qDebug() << "[[MAIN]]" << "Database connection closed";
	});

	// once the client has disconnected close the application
	QObject::connect(&client, &Client::disconnected,
					 qApp, &QCoreApplication::quit,
					 Qt::QueuedConnection);

	// call connect to initate the previously prepare events
	client.connectToKeePass();
	return a.exec();
}
