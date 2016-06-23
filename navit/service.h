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



#ifdef __cplusplus
extern "C" {
#endif 
 
struct service_priv;

struct service_methods{
	int (*plugin)(struct service_priv *priv);
	int (*set_attr)(struct service_priv *priv, struct attr *attr);
	int (*get_attr)(struct service_priv *priv,enum attr_type type, struct attr *attr);
};

struct service {
	NAVIT_OBJECT
	char* name;
	struct callback_list* cbl;
	struct service_methods meth;
	struct service_priv* priv;
};


struct service* service_new(struct navit* navit, struct attr** attrs, struct attr* parent);
void service_destroy(struct service *this_);
int service_get_attr(struct service *this_, enum attr_type type, struct attr *attr, struct attr_iter *iter);
int service_set_attr(struct service *this_, struct attr *attr);
int service_add_attr(struct service *this_, struct attr *attr);
int service_remove_attr(struct service *this_, struct attr *attr);

#ifdef __cplusplus
}
#endif

#endif
