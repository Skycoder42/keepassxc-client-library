#ifndef KPXCENTRY_H
#define KPXCENTRY_H

#include <QtCore/QObject>
#include <QtCore/QUuid>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QSharedDataPointer>

#include "kpxcclient_global.h"

class KPXCEntryData;
class KPXCCLIENT_EXPORT KPXCEntry
{
	Q_GADGET

	Q_PROPERTY(QUuid uuid READ uuid WRITE setUuid)
	Q_PROPERTY(QString title READ title WRITE setTitle)
	Q_PROPERTY(QString username READ username WRITE setUsername)
	Q_PROPERTY(QString password READ password WRITE setPassword)
	Q_PROPERTY(QString totp READ totp WRITE setTotp)
	Q_PROPERTY(QMap<QString, QString> extraFields READ extraFields WRITE setExtraFields)

public:
	KPXCEntry();
	KPXCEntry(const KPXCEntry &other);
	KPXCEntry(KPXCEntry &&other) noexcept;
	KPXCEntry &operator=(const KPXCEntry &other);
	KPXCEntry &operator=(KPXCEntry &&other) noexcept;
	~KPXCEntry();

	bool isValid() const;
	explicit operator bool() const;
	bool operator!() const;

	QUuid uuid() const;
	QString title() const;
	QString username() const;
	QString password() const;
	QString totp() const;
	QMap<QString, QString> extraFields() const;
	QString extraField(const QString &name) const;

	void setUuid(QUuid uuid);
	void setTitle(QString title);
	void setUsername(QString username);
	void setPassword(QString password);
	void setTotp(QString totp);
	void setExtraFields(QMap<QString, QString> extraFields);
	void setExtraField(const QString &name, const QString &value);
	bool removeExtraField(const QString &name);

	bool operator==(const KPXCEntry &other) const;
	bool operator!=(const KPXCEntry &other) const;

private:
	QSharedDataPointer<KPXCEntryData> d;
};

Q_DECLARE_METATYPE(KPXCEntry)
Q_DECLARE_TYPEINFO(KPXCEntry, Q_MOVABLE_TYPE);

#endif // KPXCENTRY_H
