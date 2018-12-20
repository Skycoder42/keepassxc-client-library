#ifndef KPXCCLIENT_CLIENT_P_H
#define KPXCCLIENT_CLIENT_P_H

#include "client.h"
#include "connector_p.h"

namespace KPXCClient {

class ClientPrivate
{
public:
	static const QString ActionGetDatabaseHash;
	static const QString ActionTestAssociate;
	static const QString ActionAssociate;
	static const QString ActionGeneratePassword;
	static const QString ActionGetLogins;
	static const QString ActionSetLogin;
	static const QString ActionLockDatabase;

	static bool initialized;

	Client * const q;
	Connector * const connector;

	IDatabaseRegistry *dbReg;
	Client::Options options = Client::Option::Default;

	QByteArray currentDatabase;
	bool locked = true;

	SecureByteArray _keyCache;

	ClientPrivate(Client *q_ptr);

	void setError(const QString &action,
				  Client::Error error,
				  const QString &msg = {});
	void clear();

	void onDbHash(const QJsonObject &message);
	void onAssoc(const QJsonObject &message);
	void onTestAssoc(const QJsonObject &message);
	void onGeneratePasswd(const QJsonObject &message);
	void onGetLogins(const QJsonObject &message);

	void sendTestAssoc();
	void sendAssoc();
};

}

#endif // KPXCCLIENT_CLIENT_P_H
