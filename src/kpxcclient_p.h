#ifndef KPXCCLIENT_P_H
#define KPXCCLIENT_P_H

#include "kpxcclient.h"
#include "kpxcconnector_p.h"

class KPXCClientPrivate
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

	KPXCClient * const q;
	KPXCConnector * const connector;

	IKPXCDatabaseRegistry *dbReg;
	KPXCClient::Options options = KPXCClient::Option::Default;

	QByteArray currentDatabase;
	bool locked = true;

	SecureByteArray _keyCache;

	KPXCClientPrivate(KPXCClient *q_ptr);

	void setError(KPXCClient::Error error, const QString &msg = {});
	void clear();

	void onDbHash(const QJsonObject &message);
	void onAssoc(const QJsonObject &message);
	void onTestAssoc(const QJsonObject &message);
	void onGeneratePasswd(const QJsonObject &message);
	void onGetLogins(const QJsonObject &message);

	void sendTestAssoc();
	void sendAssoc();
};

#endif // KPXCCLIENT_P_H
