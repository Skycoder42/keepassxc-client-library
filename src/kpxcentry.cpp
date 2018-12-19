#include "kpxcentry.h"
#include "kpxcentry_p.h"

KPXCEntry::KPXCEntry() :
	d{new KPXCEntryData{}}
{}

KPXCEntry::KPXCEntry(QString username, QString password) :
	d{new KPXCEntryData{std::move(username), std::move(password)}}
{}

KPXCEntry::KPXCEntry(const KPXCEntry &other) = default;

KPXCEntry::KPXCEntry(KPXCEntry &&other) noexcept = default;

KPXCEntry &KPXCEntry::operator=(const KPXCEntry &other) = default;

KPXCEntry &KPXCEntry::operator=(KPXCEntry &&other) noexcept = default;

KPXCEntry::~KPXCEntry() = default;

bool KPXCEntry::isStored() const
{
	return !d->uuid.isNull();
}

QUuid KPXCEntry::uuid() const
{
	return d->uuid;
}

QString KPXCEntry::title() const
{
	return d->title;
}

QString KPXCEntry::username() const
{
	return d->username;
}

QString KPXCEntry::password() const
{
	return d->password;
}

QString KPXCEntry::totp() const
{
	return d->totp;
}

QMap<QString, QString> KPXCEntry::extraFields() const
{
	return d->extraFields;
}

QString KPXCEntry::extraField(const QString &name) const
{
	return d->extraFields.value(name);
}

void KPXCEntry::setUsername(QString username)
{
	d->username = std::move(username);
}

void KPXCEntry::setPassword(QString password)
{
	d->password = std::move(password);
}

void KPXCEntry::setUuid(QUuid uuid)
{
	d->uuid = std::move(uuid);
}

void KPXCEntry::setTitle(QString title)
{
	d->title = std::move(title);
}

void KPXCEntry::setTotp(QString totp)
{
	d->totp = std::move(totp);
}

void KPXCEntry::setExtraFields(QMap<QString, QString> extraFields)
{
	d->extraFields = std::move(extraFields);
}

bool KPXCEntry::operator==(const KPXCEntry &other) const
{
	return d == other.d || (
		d->uuid == other.d->uuid &&
		d->title == other.d->title &&
		d->username == other.d->username &&
		d->password == other.d->password &&
		d->totp == other.d->totp &&
		d->extraFields == other.d->extraFields);
}

bool KPXCEntry::operator!=(const KPXCEntry &other) const
{
	return d != other.d && (
		d->uuid != other.d->uuid ||
		d->title != other.d->title ||
		d->username != other.d->username ||
		d->password != other.d->password ||
		d->totp != other.d->totp ||
		d->extraFields != other.d->extraFields);
}

KPXCEntryData::KPXCEntryData(QString &&username, QString &&password) :
	username{std::move(username)},
	password{std::move(password)}
{}
