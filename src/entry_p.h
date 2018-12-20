#ifndef KPXCCLIENT_ENTRY_P_H
#define KPXCCLIENT_ENTRY_P_H

#include "entry.h"

namespace KPXCClient {

class EntryData : public QSharedData
{
public:
	EntryData() = default;
	EntryData(QString &&username, QString &&password);
	EntryData(const EntryData &other) = default;

	QUuid uuid;
	QString title;
	QString username;
	QString password;
	QString totp;
	QMap<QString, QString> extraFields;
};

}

#endif // KPXCCLIENT_ENTRY_P_H
