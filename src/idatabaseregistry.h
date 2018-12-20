#ifndef KPXCCLIENT_IDATABASEREGISTRY_H
#define KPXCCLIENT_IDATABASEREGISTRY_H

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QByteArray>

#include "kpxcclient_global.h"
#include "securebytearray.h"

namespace KPXCClient {

class KPXCCLIENT_EXPORT IDatabaseRegistry
{
	Q_DISABLE_COPY(IDatabaseRegistry)
public:
	struct KPXCCLIENT_EXPORT ClientId
	{
		QString name;
		SecureByteArray key;
	};

	IDatabaseRegistry();
	virtual ~IDatabaseRegistry();

	virtual bool hasClientId(const QByteArray &databaseHash) = 0;
	virtual ClientId getClientId(const QByteArray &databaseHash) = 0;
	virtual QList<ClientId> getAllClientIds() = 0;

	virtual void addClientId(const QByteArray &databaseHash, ClientId clientId) = 0;
	virtual void removeClientId(const QByteArray &databaseHash) = 0;
};

}

#define KPXCClient_IDatabaseRegistryIid "de.skycoder42.kpxcclient.IDatabaseRegistry"
Q_DECLARE_INTERFACE(KPXCClient::IDatabaseRegistry, KPXCClient_IDatabaseRegistryIid)

#endif // KPXCCLIENT_IDATABASEREGISTRY_H
