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
#ifndef NAVIT_SERVICE_H
#define NAVIT_SERVICE_H

#include "glib_slice.h"
#include "item.h"
#include "callback.h"
#include "xmlconfig.h"


#ifdef __cplusplus
extern "C" {
#endif 
 
struct service_priv;

struct service_methods {
	void (*plugin)(struct service_priv *priv);
	int (*set_attr)(struct service_priv *priv, struct attr *attr);
	int (*get_attr)(struct service_priv *priv,enum attr_type type, struct attr *attr);
	GList* (*get_properties)(struct service_priv *priv);
	struct service_property* (*set_property)(struct service_priv *priv, struct service_property* sp);
};

struct service {
	NAVIT_OBJECT
	struct callback_list* cbl;
	struct service_methods meth;
	struct service_priv* priv;
	char* name;
};

struct service_property{
	char *name;
	struct service_property* parent;
	int ro;
	int num_children;
	GList* children;
	void* value;
};


struct service* service_new(struct attr* parent, struct attr** attrs);
void service_destroy(struct service *this_);
int service_get_attr(struct service *this_, enum attr_type type, struct attr *attr, struct attr_iter *iter);
int service_set_attr(struct service *this_, struct attr *attr);
int service_add_attr(struct service *this_, struct attr *attr);
int service_remove_attr(struct service *this_, struct attr *attr);

#ifdef __cplusplus
}
#endif

#endif
