/*
 *  baul-python.c - Baul Python extension
 *
 *  Copyright (C) 2004 Johan Dahlin
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef CAJA_PYTHON_H
#define CAJA_PYTHON_H

#include <glib-object.h>
#include <glib/gprintf.h>
#include <Python.h>

#if defined(NO_IMPORT)
#define CAJA_PYTHON_VAR_DECL extern
#else
#define CAJA_PYTHON_VAR_DECL
#endif

typedef enum {
    CAJA_PYTHON_DEBUG_MISC = 1 << 0,
} BaulPythonDebug;

extern BaulPythonDebug baul_python_debug;

#define debug(x) { if (baul_python_debug & CAJA_PYTHON_DEBUG_MISC) \
                       g_printf( "baul-python:" x "\n"); }
#define debug_enter()  { if (baul_python_debug & CAJA_PYTHON_DEBUG_MISC) \
                             g_printf("%s: entered\n", __FUNCTION__); }
#define debug_enter_args(x, y) { if (baul_python_debug & CAJA_PYTHON_DEBUG_MISC) \
                                     g_printf("%s: entered " x "\n", __FUNCTION__, y); }


CAJA_PYTHON_VAR_DECL PyTypeObject *_PyGtkWidget_Type;
#define PyGtkWidget_Type (*_PyGtkWidget_Type)

CAJA_PYTHON_VAR_DECL PyTypeObject *_PyBaulColumn_Type;
#define PyBaulColumn_Type (*_PyBaulColumn_Type)

CAJA_PYTHON_VAR_DECL PyTypeObject *_PyBaulColumnProvider_Type;
#define PyBaulColumnProvider_Type (*_PyBaulColumnProvider_Type)

CAJA_PYTHON_VAR_DECL PyTypeObject *_PyBaulInfoProvider_Type;
#define PyBaulInfoProvider_Type (*_PyBaulInfoProvider_Type)

CAJA_PYTHON_VAR_DECL PyTypeObject *_PyBaulLocationWidgetProvider_Type;
#define PyBaulLocationWidgetProvider_Type (*_PyBaulLocationWidgetProvider_Type)

CAJA_PYTHON_VAR_DECL PyTypeObject *_PyBaulMenu_Type;
#define PyBaulMenu_Type (*_PyBaulMenu_Type)

CAJA_PYTHON_VAR_DECL PyTypeObject *_PyBaulMenuItem_Type;
#define PyBaulMenuItem_Type (*_PyBaulMenuItem_Type)

CAJA_PYTHON_VAR_DECL PyTypeObject *_PyBaulMenuProvider_Type;
#define PyBaulMenuProvider_Type (*_PyBaulMenuProvider_Type)

CAJA_PYTHON_VAR_DECL PyTypeObject *_PyBaulPropertyPage_Type;
#define PyBaulPropertyPage_Type (*_PyBaulPropertyPage_Type)

CAJA_PYTHON_VAR_DECL PyTypeObject *_PyBaulPropertyPageProvider_Type;
#define PyBaulPropertyPageProvider_Type (*_PyBaulPropertyPageProvider_Type)

CAJA_PYTHON_VAR_DECL PyTypeObject *_PyBaulOperationHandle_Type;
#define PyBaulOperationHandle_Type (*_PyBaulOperationHandle_Type)

#endif /* CAJA_PYTHON_H */
