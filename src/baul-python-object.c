/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 *  Copyright (C) 2004 Novell, Inc.
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

#include <config.h>

#define NO_IMPORT

#include "baul-python-object.h"
#include "baul-python.h"

#include <libbaul-extension/baul-extension-types.h>

#include <pygobject.h>

/* Baul extension headers */
#include <libbaul-extension/baul-file-info.h>
#include <libbaul-extension/baul-info-provider.h>
#include <libbaul-extension/baul-column-provider.h>
#include <libbaul-extension/baul-location-widget-provider.h>
#include <libbaul-extension/baul-menu-item.h>
#include <libbaul-extension/baul-menu-provider.h>
#include <libbaul-extension/baul-property-page-provider.h>

#include <string.h>

#define METHOD_PREFIX ""

static GObjectClass *parent_class;

/* These macros assumes the following things:
 *   a METHOD_NAME is defined with is a string
 *   a goto label called beach
 *   the return value is called ret
 */

#define CHECK_METHOD_NAME(self)                                        \
	if (!PyObject_HasAttrString(self, METHOD_NAME))                    \
		goto beach;

#define CHECK_OBJECT(object)										   \
  	if (object->instance == NULL)									   \
  	{																   \
  		g_object_unref (object);									   \
  		goto beach;													   \
  	}																   \

#define CONVERT_LIST(py_files, files)                                  \
	{                                                                  \
		GList *l;                                                      \
        py_files = PyList_New(0);                                      \
		for (l = files; l; l = l->next)                                \
		{                                                              \
			PyList_Append(py_files, pygobject_new((GObject*)l->data)); \
		}                                                              \
	}

#define HANDLE_RETVAL(py_ret)                                          \
    if (!py_ret)                                                       \
    {                                                                  \
        PyErr_Print();                                                 \
		goto beach;                                                    \
 	}                                                                  \
 	else if (py_ret == Py_None)                                        \
 	{                                                                  \
 		goto beach;                                                    \
	}

#define HANDLE_LIST(py_ret, type, type_name)                           \
    {                                                                  \
        Py_ssize_t i = 0;                                              \
    	if (!PySequence_Check(py_ret) || PyUnicode_Check(py_ret))      \
    	{                                                              \
    		PyErr_SetString(PyExc_TypeError,                           \
    						METHOD_NAME " must return a sequence");    \
    		goto beach;                                                \
    	}                                                              \
    	for (i = 0; i < PySequence_Size (py_ret); i++)                 \
    	{                                                              \
    		PyGObject *py_item;                                        \
    		py_item = (PyGObject*)PySequence_GetItem (py_ret, i);      \
    		if (!pygobject_check(py_item, &Py##type##_Type))           \
    		{                                                          \
    			PyErr_SetString(PyExc_TypeError,                       \
    							METHOD_NAME                            \
    							" must return a sequence of "          \
    							type_name);                            \
    			goto beach;                                            \
    		}                                                          \
    		ret = g_list_append (ret, (type*) g_object_ref(py_item->obj));  \
            Py_DECREF(py_item);                                        \
    	}                                                              \
    }


static void
free_pygobject_data(gpointer data, gpointer user_data)
{
	/* Some BaulFile objects are cached and not freed until baul
		itself is closed.  Since PyGObject stores data that must be freed by
		the Python interpreter, we must always free it before the interpreter
		is finalized. */
	g_object_set_data((GObject *)data, "PyGObject::instance-data", NULL);
}

static void
free_pygobject_data_list(GList *list)
{
	if (list == NULL)
		return;

	g_list_foreach(list, (GFunc)free_pygobject_data, NULL);
}

static PyObject *
baul_python_boxed_new (PyTypeObject *type, gpointer boxed, gboolean free_on_dealloc)
{
	PyGBoxed *self = (PyGBoxed *) type->tp_alloc (type, 0);
	self->gtype = pyg_type_from_object ( (PyObject *) type);
	self->boxed = boxed;
	self->free_on_dealloc = free_on_dealloc;

	return (PyObject *) self;
}

#define METHOD_NAME "get_property_pages"
static GList *
baul_python_object_get_property_pages (BaulPropertyPageProvider *provider,
										   GList 						*files)
{
	BaulPythonObject *object = (BaulPythonObject*)provider;
    PyObject *py_files, *py_ret = NULL;
    GList *ret = NULL;
	PyGILState_STATE state = pyg_gil_state_ensure();

  	debug_enter();

	CHECK_OBJECT(object);
	CHECK_METHOD_NAME(object->instance);

	CONVERT_LIST(py_files, files);

    py_ret = PyObject_CallMethod(object->instance, METHOD_PREFIX METHOD_NAME,
								 "(N)", py_files);
	HANDLE_RETVAL(py_ret);

	HANDLE_LIST(py_ret, BaulPropertyPage, "Baul.PropertyPage");

 beach:
	Py_XDECREF(py_ret);
	pyg_gil_state_release(state);
    return ret;
}
#undef METHOD_NAME


static void
baul_python_object_property_page_provider_iface_init (BaulPropertyPageProviderIface *iface)
{
	iface->get_pages = baul_python_object_get_property_pages;
}

#define METHOD_NAME "get_widget"
static GtkWidget *
baul_python_object_get_widget (BaulLocationWidgetProvider *provider,
								   const char 				 	  *uri,
								   GtkWidget 					  *window)
{
	BaulPythonObject *object = (BaulPythonObject*)provider;
	GtkWidget *ret = NULL;
	PyObject *py_ret = NULL;
	PyGObject *py_ret_gobj;
	PyObject *py_uri = NULL;
	PyGILState_STATE state = pyg_gil_state_ensure();

	debug_enter();

	CHECK_OBJECT(object);
	CHECK_METHOD_NAME(object->instance);

	py_uri = PyUnicode_FromString(uri);

	py_ret = PyObject_CallMethod(object->instance, METHOD_PREFIX METHOD_NAME,
								 "(NN)", py_uri,
								 pygobject_new((GObject *)window));
	HANDLE_RETVAL(py_ret);

	py_ret_gobj = (PyGObject *)py_ret;
	if (!pygobject_check(py_ret_gobj, &PyGtkWidget_Type))
	{
		PyErr_SetString(PyExc_TypeError,
					    METHOD_NAME "should return a gtk.Widget");
		goto beach;
	}
	ret = (GtkWidget *)g_object_ref(py_ret_gobj->obj);

 beach:
	Py_XDECREF(py_ret);
	pyg_gil_state_release(state);
	return ret;
}
#undef METHOD_NAME

static void
baul_python_object_location_widget_provider_iface_init (BaulLocationWidgetProviderIface *iface)
{
	iface->get_widget = baul_python_object_get_widget;
}

#define METHOD_NAME "get_file_items"
static GList *
baul_python_object_get_file_items (BaulMenuProvider *provider,
									   GtkWidget 			*window,
									   GList 				*files)
{
	BaulPythonObject *object = (BaulPythonObject*)provider;
    GList *ret = NULL;
    PyObject *py_ret = NULL, *py_files;
	PyGILState_STATE state = pyg_gil_state_ensure();

  	debug_enter();

	CHECK_OBJECT(object);

	if (PyObject_HasAttrString(object->instance, "get_file_items_full"))
	{
		CONVERT_LIST(py_files, files);
		py_ret = PyObject_CallMethod(object->instance, METHOD_PREFIX "get_file_items_full",
									 "(NNN)",
									 pygobject_new((GObject *)provider),
									 pygobject_new((GObject *)window),
									 py_files);
	}
	else if (PyObject_HasAttrString(object->instance, "get_file_items"))
	{
		CONVERT_LIST(py_files, files);
		py_ret = PyObject_CallMethod(object->instance, METHOD_PREFIX METHOD_NAME,
									 "(NN)",
									 pygobject_new((GObject *)window),
									 py_files);
	}
	else
	{
		goto beach;
	}

	HANDLE_RETVAL(py_ret);

	HANDLE_LIST(py_ret, BaulMenuItem, "Baul.MenuItem");

 beach:
 	free_pygobject_data_list(files);
	Py_XDECREF(py_ret);
	pyg_gil_state_release(state);
    return ret;
}
#undef METHOD_NAME

#define METHOD_NAME "get_background_items"
static GList *
baul_python_object_get_background_items (BaulMenuProvider *provider,
											 GtkWidget 			  *window,
											 BaulFileInfo 	  *file)
{
	BaulPythonObject *object = (BaulPythonObject*)provider;
    GList *ret = NULL;
    PyObject *py_ret = NULL;
	PyGILState_STATE state = pyg_gil_state_ensure();

  	debug_enter();

	CHECK_OBJECT(object);

	if (PyObject_HasAttrString(object->instance, "get_background_items_full"))
	{
		py_ret = PyObject_CallMethod(object->instance, METHOD_PREFIX "get_background_items_full",
									 "(NNN)",
									 pygobject_new((GObject *)provider),
									 pygobject_new((GObject *)window),
									 pygobject_new((GObject *)file));
	}
	else if (PyObject_HasAttrString(object->instance, "get_background_items"))
	{
		py_ret = PyObject_CallMethod(object->instance, METHOD_PREFIX METHOD_NAME,
									 "(NN)",
									 pygobject_new((GObject *)window),
									 pygobject_new((GObject *)file));
	}
	else
	{
		goto beach;
	}

	HANDLE_RETVAL(py_ret);

	HANDLE_LIST(py_ret, BaulMenuItem, "Baul.MenuItem");

 beach:
	free_pygobject_data(file, NULL);
	Py_XDECREF(py_ret);
	pyg_gil_state_release(state);
    return ret;
}
#undef METHOD_NAME

static void
baul_python_object_menu_provider_iface_init (BaulMenuProviderIface *iface)
{
	iface->get_background_items = baul_python_object_get_background_items;
	iface->get_file_items = baul_python_object_get_file_items;
}

#define METHOD_NAME "get_columns"
static GList *
baul_python_object_get_columns (BaulColumnProvider *provider)
{
	BaulPythonObject *object = (BaulPythonObject*)provider;
    GList *ret = NULL;
    PyObject *py_ret = NULL;
	PyGILState_STATE state = pyg_gil_state_ensure();                                    \

	debug_enter();

	CHECK_OBJECT(object);
	CHECK_METHOD_NAME(object->instance);

    py_ret = PyObject_CallMethod(object->instance, METHOD_PREFIX METHOD_NAME,
								 NULL);

	HANDLE_RETVAL(py_ret);

	HANDLE_LIST(py_ret, BaulColumn, "Baul.Column");

 beach:
	if (py_ret != NULL)
		Py_XDECREF(py_ret);
	pyg_gil_state_release(state);
    return ret;
}
#undef METHOD_NAME

static void
baul_python_object_column_provider_iface_init (BaulColumnProviderIface *iface)
{
	iface->get_columns = baul_python_object_get_columns;
}


#define METHOD_NAME "cancel_update"
static void
baul_python_object_cancel_update (BaulInfoProvider 		*provider,
									  BaulOperationHandle 	*handle)
{
	BaulPythonObject *object = (BaulPythonObject*)provider;
	PyGILState_STATE state = pyg_gil_state_ensure();
	PyObject *py_handle = baul_python_boxed_new (_PyBaulOperationHandle_Type, handle, FALSE);

  	debug_enter();

	CHECK_OBJECT(object);
	CHECK_METHOD_NAME(object->instance);

    PyObject_CallMethod(object->instance,
								 METHOD_PREFIX METHOD_NAME, "(NN)",
								 pygobject_new((GObject*)provider),
								 py_handle);

 beach:
	pyg_gil_state_release(state);
}
#undef METHOD_NAME

#define METHOD_NAME "update_file_info"
static BaulOperationResult
baul_python_object_update_file_info (BaulInfoProvider 		*provider,
										 BaulFile 				*file,
										 GClosure 					*update_complete,
										 BaulOperationHandle   **handle)
{
	BaulPythonObject *object = (BaulPythonObject*)provider;
    BaulOperationResult ret = BAUL_OPERATION_COMPLETE;
    PyObject *py_ret = NULL;
	PyGILState_STATE state = pyg_gil_state_ensure();
	static volatile gssize handle_generator = 1;

  	debug_enter();

	CHECK_OBJECT(object);

	*handle = NULL;

	if (PyObject_HasAttrString(object->instance, "update_file_info_full"))
	{
        PyObject *py_handle;
		void *h;

        /* Generate a new handle with a default value. */
        do {
            h = (BaulOperationHandle *) g_atomic_pointer_add (&handle_generator, 1);
        } while (!h);
        py_handle = baul_python_boxed_new (_PyBaulOperationHandle_Type,
                                           h, FALSE);


		py_ret = PyObject_CallMethod(object->instance,
									 METHOD_PREFIX "update_file_info_full", "(NNNN)",
									 pygobject_new((GObject*)provider),
									 py_handle,
									 pyg_boxed_new(G_TYPE_CLOSURE, update_complete, TRUE, TRUE),
									 pygobject_new((GObject*)file));
		*handle = (void *) ((PyGBoxed *) py_handle)->boxed;
	}
	else if (PyObject_HasAttrString(object->instance, "update_file_info"))
	{
		py_ret = PyObject_CallMethod(object->instance,
									 METHOD_PREFIX METHOD_NAME, "(N)",
									 pygobject_new((GObject*)file));
	}
	else
	{
		goto beach;
	}

	HANDLE_RETVAL(py_ret);

	if (!PyLong_Check(py_ret))
	{
		PyErr_SetString(PyExc_TypeError,
						METHOD_NAME " must return None or a int");
		goto beach;
	}

	ret = PyLong_AsLong(py_ret);

    if (!*handle && ret == BAUL_OPERATION_IN_PROGRESS)
        ret = BAUL_OPERATION_FAILED;

 beach:
 	free_pygobject_data(file, NULL);
	Py_XDECREF(py_ret);
	pyg_gil_state_release(state);
    return ret;
}
#undef METHOD_NAME

static void
baul_python_object_info_provider_iface_init (BaulInfoProviderIface *iface)
{
	iface->cancel_update = baul_python_object_cancel_update;
	iface->update_file_info = baul_python_object_update_file_info;
}

static void
baul_python_object_instance_init (BaulPythonObject *object)
{
	BaulPythonObjectClass *class;
  	debug_enter();

	class = (BaulPythonObjectClass*)(((GTypeInstance*)object)->g_class);

	object->instance = PyObject_CallObject(class->type, NULL);
	if (object->instance == NULL)
		PyErr_Print();
}

static void
baul_python_object_finalize (GObject *object)
{
  	debug_enter();

	if (((BaulPythonObject *)object)->instance != NULL)
		Py_DECREF(((BaulPythonObject *)object)->instance);
}

static void
baul_python_object_class_init (BaulPythonObjectClass *class,
								   gpointer 				  class_data)
{
	debug_enter();

	parent_class = g_type_class_peek_parent (class);

	class->type = (PyObject*)class_data;

	G_OBJECT_CLASS (class)->finalize = baul_python_object_finalize;
}

GType
baul_python_object_get_type (GTypeModule *module,
								 PyObject 	*type)
{
	GTypeInfo *info;
	const char *type_name;
	GType gtype;

	static const GInterfaceInfo property_page_provider_iface_info = {
		(GInterfaceInitFunc) baul_python_object_property_page_provider_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo location_widget_provider_iface_info = {
		(GInterfaceInitFunc) baul_python_object_location_widget_provider_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo menu_provider_iface_info = {
		(GInterfaceInitFunc) baul_python_object_menu_provider_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo column_provider_iface_info = {
		(GInterfaceInitFunc) baul_python_object_column_provider_iface_init,
		NULL,
		NULL
	};

	static const GInterfaceInfo info_provider_iface_info = {
		(GInterfaceInitFunc) baul_python_object_info_provider_iface_init,
		NULL,
		NULL
	};

	debug_enter_args("type=%s", PyUnicode_AsUTF8(PyObject_GetAttrString(type, "__name__")));
	info = g_new0 (GTypeInfo, 1);

	info->class_size = sizeof (BaulPythonObjectClass);
	info->class_init = (GClassInitFunc)baul_python_object_class_init;
	info->instance_size = sizeof (BaulPythonObject);
	info->instance_init = (GInstanceInitFunc)baul_python_object_instance_init;

	info->class_data = type;
	Py_INCREF(type);

	type_name = g_strdup_printf("%s+BaulPython",
								PyUnicode_AsUTF8(PyObject_GetAttrString(type, "__name__")));

	gtype = g_type_module_register_type (module,
										 G_TYPE_OBJECT,
										 type_name,
										 info, 0);

	if (PyObject_IsSubclass(type, (PyObject*)&PyBaulPropertyPageProvider_Type))
	{
		g_type_module_add_interface (module, gtype,
									 BAUL_TYPE_PROPERTY_PAGE_PROVIDER,
									 &property_page_provider_iface_info);
	}

	if (PyObject_IsSubclass(type, (PyObject*)&PyBaulLocationWidgetProvider_Type))
	{
		g_type_module_add_interface (module, gtype,
									 BAUL_TYPE_LOCATION_WIDGET_PROVIDER,
									 &location_widget_provider_iface_info);
	}

	if (PyObject_IsSubclass(type, (PyObject*)&PyBaulMenuProvider_Type))
	{
		g_type_module_add_interface (module, gtype,
									 BAUL_TYPE_MENU_PROVIDER,
									 &menu_provider_iface_info);
	}

	if (PyObject_IsSubclass(type, (PyObject*)&PyBaulColumnProvider_Type))
	{
		g_type_module_add_interface (module, gtype,
									 BAUL_TYPE_COLUMN_PROVIDER,
									 &column_provider_iface_info);
	}

	if (PyObject_IsSubclass(type, (PyObject*)&PyBaulInfoProvider_Type))
	{
		g_type_module_add_interface (module, gtype,
									 BAUL_TYPE_INFO_PROVIDER,
									 &info_provider_iface_info);
	}

	return gtype;
}
