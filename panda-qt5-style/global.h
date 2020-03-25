#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(PROXYSTYLE_LIBRARY)
#  define PROXYSTYLESHARED_EXPORT Q_DECL_EXPORT
#else
#  define PROXYSTYLESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // GLOBAL_H