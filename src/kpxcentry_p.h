#ifndef KPXCENTRY_P_H
#define KPXCENTRY_P_H

#include "kpxcentry.h"

class KPXCEntryData : public QSharedData
{
public:
	KPXCEntryData() = default;
	KPXCEntryData(QString &&username, QString &&password);
	KPXCEntryData(const KPXCEntryData &other) = default;

	QUuid uuid;
	QString title;
	QString username;
	QString password;
	QString totp;
	QMap<QString, QString> extraFields;
};

#endif // KPXCENTRY_P_H
