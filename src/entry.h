#ifndef KPXCCLIENT_ENTRY_H
#define KPXCCLIENT_ENTRY_H

#include <QtCore/QObject>
#include <QtCore/QUuid>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QSharedDataPointer>

#include "kpxcclient_global.h"

namespace KPXCClient {

class Client;
class ClientPrivate;

class EntryData;
class KPXCCLIENT_EXPORT Entry
{
	Q_GADGET

	Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
	Q_PROPERTY(QString title READ title CONSTANT)
	Q_PROPERTY(QString username READ username WRITE setUsername)
	Q_PROPERTY(QString password READ password WRITE setPassword)
	Q_PROPERTY(QString totp READ totp CONSTANT)
	Q_PROPERTY(QMap<QString, QString> extraFields READ extraFields CONSTANT)

public:
	Entry();
	Entry(QString username, QString password);
	Entry(const Entry &other);
	Entry(Entry &&other) noexcept;
	Entry &operator=(const Entry &other);
	Entry &operator=(Entry &&other) noexcept;
	~Entry();

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

	bool operator==(const Entry &other) const;
	bool operator!=(const Entry &other) const;

private:
	friend class KPXCClient::Client;
	friend class KPXCClient::ClientPrivate;
	QSharedDataPointer<EntryData> d;

	void setUuid(QUuid uuid);
	void setTitle(QString title);
	void setTotp(QString totp);
	void setExtraFields(QMap<QString, QString> extraFields);
};

}

Q_DECLARE_METATYPE(KPXCClient::Entry)
Q_DECLARE_TYPEINFO(KPXCClient::Entry, Q_MOVABLE_TYPE);

#endif // KPXCCLIENT_ENTRY_H
