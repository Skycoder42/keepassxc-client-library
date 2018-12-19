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

	Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
	Q_PROPERTY(QString title READ title CONSTANT)
	Q_PROPERTY(QString username READ username WRITE setUsername)
	Q_PROPERTY(QString password READ password WRITE setPassword)
	Q_PROPERTY(QString totp READ totp CONSTANT)
	Q_PROPERTY(QMap<QString, QString> extraFields READ extraFields CONSTANT)

public:
	KPXCEntry();
	KPXCEntry(QString username, QString password);
	KPXCEntry(const KPXCEntry &other);
	KPXCEntry(KPXCEntry &&other) noexcept;
	KPXCEntry &operator=(const KPXCEntry &other);
	KPXCEntry &operator=(KPXCEntry &&other) noexcept;
	~KPXCEntry();

	bool isStored() const;

	QUuid uuid() const;
	QString title() const;
	QString username() const;
	QString password() const;
	QString totp() const;
	QMap<QString, QString> extraFields() const;
	QString extraField(const QString &name) const;

	void setUsername(QString username);
	void setPassword(QString password);

	bool operator==(const KPXCEntry &other) const;
	bool operator!=(const KPXCEntry &other) const;

private:
	friend class KPXCClient;
	friend class KPXCClientPrivate;
	QSharedDataPointer<KPXCEntryData> d;

	void setUuid(QUuid uuid);
	void setTitle(QString title);
	void setTotp(QString totp);
	void setExtraFields(QMap<QString, QString> extraFields);
};

Q_DECLARE_METATYPE(KPXCEntry)
Q_DECLARE_TYPEINFO(KPXCEntry, Q_MOVABLE_TYPE);

#endif // KPXCENTRY_H
