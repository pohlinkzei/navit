/**
 * Navit, a modular navigation system.
 * Copyright (C) 2005-2016 Navit Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <time.h>
#include "config.h"
#include "debug.h"
#include "item.h"
#include "xmlconfig.h"
#include "plugin.h"
#include "util.h"
#include "event.h"
#include "callback.h"
#include "service.h"

struct object_func service_func;
struct service_priv;





/**
 * Generic get function
 *
 * @param this_ Pointer to a service structure
 * @param type The attribute type to look for
 * @param attr Pointer to a {@code struct attr} to store the attribute
 * @param iter A service attr_iter. This is only used for generic attributes; for attributes specific to the service object it is ignored.
 * @return True for success, false for failure
 */
int
service_get_attr(struct service *this_, enum attr_type type, struct attr *attr, struct attr_iter *iter)
{
	int ret;

	ret=this_->meth.get_attr(this_->priv, type, attr);
	if (ret)
		return ret;

	return attr_generic_get_attr(this_->attrs, NULL, type, attr, iter);
}

/**
 * Generic set function
 *
 * @param this_ A service
 * @param attr The attribute to set
 * @return False on success, true on failure
 */
int
service_set_attr(struct service *this_, struct attr *attr)
{
	int ret=1;
	if (this_->meth.set_attr)
		ret=this_->meth.set_attr(this_->priv, attr);
	if (ret == 1 && attr->type != attr_navit)
		this_->attrs=attr_generic_set_attr(this_->attrs, attr);
	return ret != 0;
}

/**
 * Generic add function
 *
 * @param this_ A service
 * @param attr The attribute to add
 *
 * @return true if the attribute was added, false if not.
 */
int
service_add_attr(struct service *this_, struct attr *attr)
{
	int ret=1;
	switch (attr->type) {
	case attr_callback:
		callback_list_add(this_->cbl, attr->u.callback);
		break;
	default:
		break;
	}
	if (ret)
		this_->attrs=attr_generic_add_attr(this_->attrs, attr);
	return ret;
}

/**
 * @brief Generic remove function.
 *
 * Used to remove a callback from the service.
 * @param this_ A service
 * @param attr
 */
int
service_remove_attr(struct service *this_, struct attr *attr)
{
	struct callback *cb;
	switch (attr->type) {
	case attr_callback:
		callback_list_remove(this_->cbl, attr->u.callback);
		break;
	default:
		this_->attrs=attr_generic_remove_attr(this_->attrs, attr);
		return 0;
	}
	return 1;
}




/**
 * @brief Creates a new service
 *
 * @param parent
 * @param attrs Points to a null-terminated array of pointers to the attributes
 * for the new service type.
 *
 * @return The newly created service object
 */
struct service *
service_new(struct navit* navit, struct attr** attrs, struct attr* parent)
{
	struct service *this_;
	struct attr* type;
	struct service_priv *(*servicetype_new) (struct service_methods *
						 meth,
						 struct callback_list *cbl,
						 struct attr ** attrs);

	dbg(lvl_debug, "enter\n");

	type = attr_search(attrs, NULL, attr_type);
	if (!type) {
		dbg(lvl_error, "incomplete service definition: missing type attribute!\n");
		return NULL;
	}

	servicetype_new = plugin_get_service_type(type->u.str);
	if(!servicetype_new){
		dbg(lvl_error, "Wrong type: %s\n", type->u.str);
		return NULL;
	}
	
	this_ = g_new0(struct service, 1);
	this_->func=&service_func;
	navit_object_ref((struct navit_object *)this_);
	
	//TODO: Read important attrs here
	
	this_->cbl = callback_list_new();
	this_->priv = servicetype_new(&this_->meth, this_->cbl, attrs);
	if (!this_->priv) {
		dbg(lvl_error, "servicetype_new failed\n");
		callback_list_destroy(this_->cbl);
		g_free(this_);
		return NULL;
	}
	this_->attrs=attr_list_dup(attrs);

	dbg(lvl_debug, "leave\n");

	return this_;
}


struct object_func service_func = {
	attr_service,
	(object_func_new)service_new,
	(object_func_get_attr)service_get_attr,
	(object_func_iter_new)NULL,
	(object_func_iter_destroy)NULL,
	(object_func_set_attr)service_set_attr,
	(object_func_add_attr)service_add_attr,
	(object_func_remove_attr)service_remove_attr,
	(object_func_init)NULL,
	(object_func_destroy)NULL,
	(object_func_dup)NULL,
	(object_func_ref)navit_object_ref,
	(object_func_unref)navit_object_unref,
};
