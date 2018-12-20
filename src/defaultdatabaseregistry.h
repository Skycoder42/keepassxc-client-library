#ifndef KPXCCLIENT_DEFAULTDATABASEREGISTRY_H
#define KPXCCLIENT_DEFAULTDATABASEREGISTRY_H

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>

#include "kpxcclient_global.h"
#include "idatabaseregistry.h"

namespace KPXCClient {

class DefaultDatabaseRegistryPrivate;
class KPXCCLIENT_EXPORT DefaultDatabaseRegistry : public QObject, public IDatabaseRegistry
{
	Q_OBJECT
	Q_INTERFACES(KPXCClient::IDatabaseRegistry)

	Q_PROPERTY(bool persistent READ isPersistent WRITE setPersistent NOTIFY persistentChanged)

public:
	explicit DefaultDatabaseRegistry(QObject *parent = nullptr);
	explicit DefaultDatabaseRegistry(bool persistent, QObject *parent = nullptr);
	explicit DefaultDatabaseRegistry(QSettings *settings, QObject *parent = nullptr);
	~DefaultDatabaseRegistry() override;

	// IKPXCDatabaseRegistry interface
	bool hasClientId(const QByteArray &databaseHash) override;
	ClientId getClientId(const QByteArray &databaseHash) override;
	QList<ClientId> getAllClientIds() override;
	void addClientId(const QByteArray &databaseHash, ClientId clientId) override;
	void removeClientId(const QByteArray &databaseHash) override;

	bool isPersistent() const;
	QSettings *settings() const;

public Q_SLOTS:
	void reloadSettings();

	void setPersistent(bool persistent);
	void setPersistent(QSettings *settings, bool takeOwnership = false);

Q_SIGNALS:
	void persistentChanged(bool persistent, QPrivateSignal);

private:
	QScopedPointer<DefaultDatabaseRegistryPrivate> d;
};

}

#endif // KPXCCLIENT_DEFAULTDATABASEREGISTRY_H
