#ifndef KPXCCLIENT_P_H
#define KPXCCLIENT_P_H

#include "kpxcclient.h"
#include "kpxcconnector_p.h"

class KPXCClientPrivate
{
public:
	static const QString ActionGetDatabaseHash;

	static bool initialized;

	KPXCClient * const q;
	KPXCConnector * const connector;

	IKPXCDatabaseRegistry *dbReg;
	KPXCClient::Options options = KPXCClient::Option::Default;

	QByteArray currentDatabase;
	bool locked = true;
	KPXCClient::Error lastError = KPXCClient::Error::NoError;
	QString lastErrorString;

	KPXCClientPrivate(KPXCClient *q_ptr);

	void setError(KPXCClient::Error error, const QString &msg = {});
	void clear();

	void onDbHash(const QJsonObject &message);
};

#endif // KPXCCLIENT_P_H
