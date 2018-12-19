#ifndef KPXCDATABASEREGISTRY_H
#define KPXCDATABASEREGISTRY_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>
#include <QtCore/QScopedPointer>
#include <QtCore/QSettings>

#include "kpxcclient_global.h"
#include "securebytearray.h"

class KPXCCLIENT_EXPORT IKPXCDatabaseRegistry
{
	Q_DISABLE_COPY(IKPXCDatabaseRegistry)
public:
	struct KPXCCLIENT_EXPORT ClientId
	{
		QString name;
		SecureByteArray key;
	};

	IKPXCDatabaseRegistry();
	virtual ~IKPXCDatabaseRegistry();

	virtual bool hasClientId(const QByteArray &databaseHash) = 0;
	virtual ClientId getClientId(const QByteArray &databaseHash) = 0;
	virtual QList<ClientId> getAllClientIds() = 0;

	virtual void addClientId(const QByteArray &databaseHash, ClientId clientId) = 0;
	virtual void removeClientId(const QByteArray &databaseHash) = 0;
};

#define IKPXCDatabaseRegistryIid "de.skycoder42.kpxcclient.IKPXCDatabaseRegistry"
Q_DECLARE_INTERFACE(IKPXCDatabaseRegistry, IKPXCDatabaseRegistryIid)



class KPXCDefaultDatabaseRegistryPrivate;
class KPXCCLIENT_EXPORT KPXCDefaultDatabaseRegistry : public QObject, public IKPXCDatabaseRegistry
{
	Q_OBJECT
	Q_INTERFACES(IKPXCDatabaseRegistry)

	Q_PROPERTY(bool persistent READ isPersistent WRITE setPersistent NOTIFY persistentChanged)

public:
	explicit KPXCDefaultDatabaseRegistry(QObject *parent = nullptr);
	explicit KPXCDefaultDatabaseRegistry(bool persistent, QObject *parent = nullptr);
	explicit KPXCDefaultDatabaseRegistry(QSettings *settings, QObject *parent = nullptr);
	~KPXCDefaultDatabaseRegistry() override;

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
	QScopedPointer<KPXCDefaultDatabaseRegistryPrivate> d;
};

#endif // KPXCDATABASEREGISTRY_H
