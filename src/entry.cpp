#include "entry.h"
#include "entry_p.h"
using namespace KPXCClient;

Entry::Entry() :
	d{new EntryData{}}
{}

Entry::Entry(QString username, QString password) :
	d{new EntryData{std::move(username), std::move(password)}}
{}

Entry::Entry(const Entry &other) = default;

Entry::Entry(Entry &&other) noexcept = default;

Entry &Entry::operator=(const Entry &other) = default;

Entry &Entry::operator=(Entry &&other) noexcept = default;

Entry::~Entry() = default;

bool Entry::isStored() const
{
	return !d->uuid.isNull();
}

QUuid Entry::uuid() const
{
	return d->uuid;
}

QString Entry::title() const
{
	return d->title;
}

QString Entry::username() const
{
	return d->username;
}

QString Entry::password() const
{
	return d->password;
}

QString Entry::totp() const
{
	return d->totp;
}

QMap<QString, QString> Entry::extraFields() const
{
	return d->extraFields;
}

QString Entry::extraField(const QString &name) const
{
	return d->extraFields.value(name);
}

void Entry::setUsername(QString username)
{
	d->username = std::move(username);
}

void Entry::setPassword(QString password)
{
	d->password = std::move(password);
}

void Entry::setUuid(QUuid uuid)
{
	d->uuid = std::move(uuid);
}

void Entry::setTitle(QString title)
{
	d->title = std::move(title);
}

void Entry::setTotp(QString totp)
{
	d->totp = std::move(totp);
}

void Entry::setExtraFields(QMap<QString, QString> extraFields)
{
	d->extraFields = std::move(extraFields);
}

bool Entry::operator==(const Entry &other) const
{
	return d == other.d || (
		d->uuid == other.d->uuid &&
		d->title == other.d->title &&
		d->username == other.d->username &&
		d->password == other.d->password &&
		d->totp == other.d->totp &&
		d->extraFields == other.d->extraFields);
}

bool Entry::operator!=(const Entry &other) const
{
	return d != other.d && (
		d->uuid != other.d->uuid ||
		d->title != other.d->title ||
		d->username != other.d->username ||
		d->password != other.d->password ||
		d->totp != other.d->totp ||
		d->extraFields != other.d->extraFields);
}

EntryData::EntryData(QString &&username, QString &&password) :
	username{std::move(username)},
	password{std::move(password)}
{}
