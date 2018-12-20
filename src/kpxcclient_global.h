#ifndef KPXCCLIENT_GLOBAL_H
#define KPXCCLIENT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(KPXCCLIENT_LIBRARY)
#  define KPXCCLIENT_EXPORT Q_DECL_EXPORT
#else
#  define KPXCCLIENT_EXPORT Q_DECL_IMPORT
#endif

namespace KPXCClient {

KPXCCLIENT_EXPORT bool init();

}

#endif // KPXCCLIENT_GLOBAL_H
