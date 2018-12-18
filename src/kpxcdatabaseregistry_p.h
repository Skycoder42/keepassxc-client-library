#ifndef KPXCDATABASEREGISTRY_P_H
#define KPXCDATABASEREGISTRY_P_H

#include "kpxcdatabaseregistry.h"
#include "securebytearray.h"

#include <variant>

#include <QtCore/QPointer>
#include <QtCore/QSettings>
#include <QtCore/QHash>

class KPXCDefaultDatabaseRegistryPrivate
{
public:
	static const QString SettingsGroupKey;
	static const QString SettingsNameKey;
	static const QString SettingsKeyKey;

	QPointer<QSettings> settings;
	QHash<QByteArray, IKPXCDatabaseRegistry::ClientId> clientIds;
};

#endif // KPXCDATABASEREGISTRY_P_H
