/*
 *  baul-python-object.h - Generation of wrapper objects for baul
 *                           extension objects in python.
 *
 *  Copyright (C) 2003 Novell, Inc.
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
 *  Author: Dave Camp <dave@ximian.com>
 *
 */

#ifndef BAUL_PYTHON_OBJECT_H
#define BAUL_PYTHON_OBJECT_H

#include <Python.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef struct _BaulPythonObject       BaulPythonObject;
typedef struct _BaulPythonObjectClass  BaulPythonObjectClass;

struct _BaulPythonObject {
  GObject parent_slot;
  PyObject *instance;
};

struct _BaulPythonObjectClass {
    GObjectClass parent_slot;
    PyObject *type;
};

GType baul_python_object_get_type (GTypeModule *module, PyObject *type);

G_END_DECLS

#endif
