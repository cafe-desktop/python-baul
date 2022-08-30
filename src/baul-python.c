/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 *  Copyright (C) 2004,2005 Johan Dahlin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <Python.h>
#include <structmember.h>
#include <pygobject.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include "baul-python.h"
#include "baul-python-object.h"

#include <libbaul-extension/baul-extension-types.h>

static const GDebugKey baul_python_debug_keys[] = {
	{"misc", CAJA_PYTHON_DEBUG_MISC},
};
static const guint baul_python_ndebug_keys = sizeof (baul_python_debug_keys) / sizeof (GDebugKey);
BaulPythonDebug baul_python_debug;

static gboolean baul_python_init_python(void);

static GArray *all_types = NULL;
static GList *all_pyfiles = NULL;


/* Baul.OperationHandle value access. */
static PyObject *
baul_operationhandle_get_handle(PyGBoxed *self, void *closure)
{
	return PyLong_FromSsize_t((Py_ssize_t) (size_t) self->boxed);
}

static int
baul_operationhandle_set_handle(PyGBoxed *self, PyObject *value, void *closure)
{
	Py_ssize_t val = PyLong_AsSsize_t(value);

	if (!PyErr_Occurred()) {
		if (val) {
			self->boxed = (void *) val;
			return 0;
		}
		PyErr_SetString(PyExc_ValueError, "invalid operation handle value");
	}
	return -1;
}

static PyGetSetDef baul_operationhandle_handle = {
	"handle",
	(getter) baul_operationhandle_get_handle,
	(setter) baul_operationhandle_set_handle,
	"Operation handle value",
	NULL
};


static inline gboolean
np_init_pygobject(void)
{
    PyObject *gobject = pygobject_init (PYGOBJECT_MAJOR_VERSION, PYGOBJECT_MINOR_VERSION, PYGOBJECT_MICRO_VERSION);

    if (gobject == NULL) {
        PyErr_Print ();
        return FALSE;
    }

	return TRUE;
}

static void
baul_python_load_file(GTypeModule *type_module,
						  const gchar *filename)
{
	PyObject *main_module, *main_locals, *locals, *key, *value;
	PyObject *module;
	GType gtype;
	Py_ssize_t pos = 0;

	debug_enter_args("filename=%s", filename);

	main_module = PyImport_AddModule("__main__");
	if (main_module == NULL)
	{
		g_warning("Could not get __main__.");
		return;
	}

	main_locals = PyModule_GetDict(main_module);
	module = PyImport_ImportModuleEx((char *) filename, main_locals, main_locals, NULL);
	if (!module)
	{
		PyErr_Print();
		return;
	}

	locals = PyModule_GetDict(module);

	while (PyDict_Next(locals, &pos, &key, &value))
	{
		if (!PyType_Check(value))
			continue;

		if (PyObject_IsSubclass(value, (PyObject*)&PyBaulColumnProvider_Type) ||
			PyObject_IsSubclass(value, (PyObject*)&PyBaulInfoProvider_Type) ||
			PyObject_IsSubclass(value, (PyObject*)&PyBaulLocationWidgetProvider_Type) ||
			PyObject_IsSubclass(value, (PyObject*)&PyBaulMenuProvider_Type) ||
			PyObject_IsSubclass(value, (PyObject*)&PyBaulPropertyPageProvider_Type))
		{
			gtype = baul_python_object_get_type(type_module, value);
			g_array_append_val(all_types, gtype);

			all_pyfiles = g_list_append(all_pyfiles, (gchar*)filename);
		}
	}

	debug("Loaded python modules");
}

static void
baul_python_load_dir (GTypeModule *module,
						  const char  *dirname)
{
	GDir *dir;
	const char *name;
	gboolean initialized = FALSE;

	debug_enter_args("dirname=%s", dirname);

	dir = g_dir_open(dirname, 0, NULL);
	if (!dir)
		return;

	while ((name = g_dir_read_name(dir)))
	{
		if (g_str_has_suffix(name, ".py"))
		{
			char *modulename;
			int len;

			len = strlen(name) - 3;
			modulename = g_new0(char, len + 1 );
			strncpy(modulename, name, len);

			if (!initialized)
			{
				PyObject *sys_path, *py_path;

				/* n-p python part is initialized on demand (or not
				* at all if no extensions are found) */
				if (!baul_python_init_python())
				{
					g_warning("baul_python_init_python failed");
					g_dir_close(dir);
					break;
				}

				/* sys.path.insert(0, dirname) */
				sys_path = PySys_GetObject("path");
				py_path = PyUnicode_FromString(dirname);
				PyList_Insert(sys_path, 0, py_path);
				Py_DECREF(py_path);
			}
			baul_python_load_file(module, modulename);
		}
	}
}

static gboolean
baul_python_init_python (void)
{
	PyObject *gi, *require_version, *args, *baul, *descr;
	GModule *libpython;
	wchar_t *argv[] = { L"baul", NULL };

	if (Py_IsInitialized())
		return TRUE;

  	debug("g_module_open " PY_LIB_LOC "/libpython" PYTHON_VERSION PYTHON_ABIFLAGS "." G_MODULE_SUFFIX ".1.0");
	libpython = g_module_open(PY_LIB_LOC "/libpython" PYTHON_VERSION PYTHON_ABIFLAGS "." G_MODULE_SUFFIX ".1.0", 0);
	if (!libpython)
		g_warning("g_module_open libpython failed: %s", g_module_error());

	debug("Py_Initialize");
	Py_Initialize();
	if (PyErr_Occurred())
	{
		PyErr_Print();
		return FALSE;
	}

	debug("PySys_SetArgv");
	PySys_SetArgv(1, argv);
	if (PyErr_Occurred())
	{
		PyErr_Print();
		return FALSE;
	}

	debug("Sanitize the python search path");
	PyRun_SimpleString("import sys; sys.path = list(filter(None, sys.path))");
	if (PyErr_Occurred())
	{
		PyErr_Print();
		return FALSE;
	}

	/* import gobject */
  	debug("init_pygobject");
	if (!np_init_pygobject())
	{
		g_warning("pygobject initialization failed");
		return FALSE;
	}

	/* import baul */
	g_setenv("INSIDE_CAJA_PYTHON", "", FALSE);
	debug("import baul");
	gi = PyImport_ImportModule ("gi");
	if (!gi) {
		PyErr_Print();
		return FALSE;
	}

	require_version = PyObject_GetAttrString (gi, (char *) "require_version");
	args = PyTuple_Pack (2, PyUnicode_FromString ("Baul"),
	PyUnicode_FromString ("2.0"));
	PyObject_CallObject (require_version, args);
	Py_DECREF (require_version);
	Py_DECREF (args);
	Py_DECREF (gi);
	baul = PyImport_ImportModule("gi.repository.Baul");
	if (!baul)
	{
		PyErr_Print();
		return FALSE;
	}

	_PyGtkWidget_Type = pygobject_lookup_class(GTK_TYPE_WIDGET);
	g_assert(_PyGtkWidget_Type != NULL);

#define IMPORT(x, y) \
    _PyBaul##x##_Type = (PyTypeObject *)PyObject_GetAttrString(baul, y); \
	if (_PyBaul##x##_Type == NULL) { \
		PyErr_Print(); \
		return FALSE; \
	}

	IMPORT(Column, "Column");
	IMPORT(ColumnProvider, "ColumnProvider");
	IMPORT(InfoProvider, "InfoProvider");
	IMPORT(LocationWidgetProvider, "LocationWidgetProvider");
	IMPORT(Menu, "Menu");
	IMPORT(MenuItem, "MenuItem");
	IMPORT(MenuProvider, "MenuProvider");
	IMPORT(PropertyPage, "PropertyPage");
	IMPORT(PropertyPageProvider, "PropertyPageProvider");
	IMPORT(OperationHandle, "OperationHandle");

#undef IMPORT

	/* Add the "handle" member to the OperationHandle type. */
	descr = PyDescr_NewGetSet(_PyBaulOperationHandle_Type,
							  &baul_operationhandle_handle);
    if (!descr) {
		PyErr_Print();
		return FALSE;
	}
	if (PyDict_SetItemString(_PyBaulOperationHandle_Type->tp_dict,
						     baul_operationhandle_handle.name, descr)) {
		Py_DECREF(descr);
		PyErr_Print();
		return FALSE;
	}
	Py_DECREF(descr);

	return TRUE;
}

void
baul_module_initialize(GTypeModule *module)
{
	gchar *user_extensions_dir;
	const gchar *env_string;

	env_string = g_getenv("CAJA_PYTHON_DEBUG");
	if (env_string != NULL)
	{
		baul_python_debug = g_parse_debug_string(env_string,
													 baul_python_debug_keys,
													 baul_python_ndebug_keys);
		env_string = NULL;
    }

	debug_enter();

	all_types = g_array_new(FALSE, FALSE, sizeof(GType));

	// Look in the new global path, $DATADIR/baul-python/extensions
	baul_python_load_dir(module, DATADIR "/baul-python/extensions");

	// Look in XDG_DATA_DIR, ~/.local/share/baul-python/extensions
	user_extensions_dir = g_build_filename(g_get_user_data_dir(),
		"baul-python", "extensions", NULL);
	baul_python_load_dir(module, user_extensions_dir);
}

void
baul_module_shutdown(void)
{
	debug_enter();

	if (Py_IsInitialized())
		Py_Finalize();

	g_array_free(all_types, TRUE);
	g_list_free (all_pyfiles);
}

void
baul_module_list_types(const GType **types,
						   int          *num_types)
{
	debug_enter();

	*types = (GType*)all_types->data;
	*num_types = all_types->len;
}

void
baul_module_list_pyfiles(GList **pyfiles)
{
	debug_enter();

	*pyfiles = all_pyfiles;
}
