#ifndef MIKROTIK_GLOBAL_H
#define MIKROTIK_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SRC_LIBRARY)
#  define MIKROTIKSHARED_EXPORT Q_DECL_EXPORT
#else
#  define SRCSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // MIKROTIK_GLOBAL_H
