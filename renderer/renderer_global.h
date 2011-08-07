#ifndef RENDERER_GLOBAL_H
#define RENDERER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined ( R_BUILD )
#  define RENDERERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define RENDERERSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // RENDERER_GLOBAL_H
