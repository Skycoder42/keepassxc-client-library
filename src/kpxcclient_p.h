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

	KPXCClient::Error lastError = KPXCClient::Error::NoError;
	QString lastErrorString;
	bool reconnectOnUnlock = false;

	KPXCClientPrivate(KPXCClient *q_ptr);

	void setError(KPXCClient::Error error, const QString &msg = {});
};

#endif // KPXCCLIENT_P_H
