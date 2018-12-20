#ifndef KPXCCLIENT_DEFAULTDATABASEREGISTRY_P_H
#define KPXCCLIENT_DEFAULTDATABASEREGISTRY_P_H

#include "defaultdatabaseregistry.h"

#include <variant>

#include <QtCore/QPointer>
#include <QtCore/QSettings>
#include <QtCore/QHash>

namespace KPXCClient {

class DefaultDatabaseRegistryPrivate
{
public:
	static const QString SettingsGroupKey;
	static const QString SettingsNameKey;
	static const QString SettingsKeyKey;

	QPointer<QSettings> settings;
	QHash<QByteArray, IDatabaseRegistry::ClientId> clientIds;
};

}

#endif // KPXCCLIENT_DEFAULTDATABASEREGISTRY_P_H
