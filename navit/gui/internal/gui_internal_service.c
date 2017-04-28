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
 
#include <glib.h>
#include <navit/main.h>
#include <navit/debug.h>
#include <navit/point.h>
#include <navit/navit.h>
#include <navit/callback.h>
#include <navit/color.h>
#include <navit/event.h>

#include "time.h"
#include "gui_internal.h"
#include "gui_internal_menu.h"
#include "coord.h"
#include "service.h"
#include "gui_internal_widget.h"
#include "gui_internal_priv.h"
#include "gui_internal_command.h"
#include "gui_internal_service.h"

void gui_internal_service_devicelist (struct gui_priv *this, struct widget *wm, void *data);
struct service_priv;
struct service;
struct navit_service;

void
increase_value(struct gui_priv *this, struct widget *wm, void *data){
	struct service_property *prop = (struct service_property *) data;
	(*(int*)prop->value)++; 
	gui_internal_service_property (this, wm, prop);
}

void
decrease_value(struct gui_priv *this, struct widget *wm, void *data){
	struct service_property *prop = (struct service_property *) data;
	(*(int*)prop->value)--; 
	gui_internal_service_property (this, wm, prop);
}



/**
 * @brief   Show
 * @param[in]   this    - pointer to the gui_priv
 *              wm      - pointer the the parent widget
 *              data    - pointer to arbitrary data
 *
 * @return  nothing
 *
 * 
 *
 */
void
gui_internal_service_root (struct gui_priv *this, struct widget *wm, void *data)
{
    struct widget *wb, *w, *wbm;
    struct widget *tbl, *row;
    dbg (lvl_error, "Showing rootlist\n");
    gui_internal_prune_menu_count (this, 1, 0);
    wb = gui_internal_menu (this, "..,-");
    wb->background = this->background;
    w = gui_internal_box_new (this, gravity_top_center | orientation_vertical | flags_expand | flags_fill);
    gui_internal_widget_append (wb, w);
	GList* services = navit_get_services(this->nav);
	int i=0;
    tbl = gui_internal_widget_table_new (this, gravity_left_top | flags_fill | flags_expand | orientation_vertical, 1);
    gui_internal_widget_append (w, tbl);
	dbg (lvl_error, "Showing %p\n",services);
    while(services) {
		struct service* service = navit_get_service(this->nav, i++);
		if(service){
			dbg (lvl_error, "Showing Service %p\n", service);
			struct attr attr;
			if(service_get_attr(service, attr_type, &attr, NULL)){
				row = gui_internal_widget_table_row_new (this, gravity_left | flags_fill | orientation_horizontal);
				gui_internal_widget_append (tbl, row);
				//add rows
				wbm = gui_internal_button_new_with_callback (this,
								 attr.u.str ,image_new_s (this,/**/ (attr.u.str) ? (attr.u.str) : /**/("gui_active")),
								 gravity_left_center |
								 orientation_horizontal | flags_fill, gui_internal_service_devicelist, service);

			
				gui_internal_widget_append (row, wbm);
			}
		}
		services = g_list_next(services);
		
	}

    gui_internal_menu_render (this);

}

struct widget *
gui_internal_service_edit_property(struct gui_priv *this, struct service_property *p){
	struct widget *wl, *wb;
	int nitems = 2, nrows;
	
	wl = gui_internal_box_new(this, gravity_left_center | orientation_horizontal_vertical | flags_fill);
	wl->w = this->root.w;
	wl->cols = this->root.w / this->icon_s;
	nrows = nitems / wl->cols + (nitems % wl->cols > 0);
	wl->h = this->icon_l *nrows;
	if(p->root){
		wb = gui_internal_button_new_with_callback(this, " back ", NULL, gravity_left_center | orientation_horizontal, gui_internal_service_devicelist, p->root);
	}else{
		wb = gui_internal_button_new_with_callback(this, " back ", NULL, gravity_left_center | orientation_horizontal, gui_internal_service_root, NULL);
	}
	gui_internal_widget_append(wl, wb);
	if(p->ro == 0){
		wb = gui_internal_button_new_with_callback(this, " - ", NULL, gravity_left_center | orientation_horizontal, decrease_value, p);
		gui_internal_widget_append(wl, wb);
	}
	switch(p->type){
		case integer8:{
			dbg(lvl_error, "\nName: %s\nvalue: %i\n", p->name, *((char*) p->value));
			wb = gui_internal_label_new(this, g_strdup_printf("Integer %i", *((char*) p->value)));
			break;
		}
		case integer8u:{
			dbg(lvl_error, "\nName: %s\nvalue: %i\n", p->name, *((unsigned char*) p->value));
			wb = gui_internal_label_new(this, g_strdup_printf("Integer %i", *((unsigned char*) p->value)));
			break;
		}
		case integer16:{
			dbg(lvl_error, "\nName: %s\nvalue: %i\n", p->name, *((short*) p->value));
			wb = gui_internal_label_new(this, g_strdup_printf("Integer %i", *((short*) p->value)));
			break;
		}
		case integer16u:{
			dbg(lvl_error, "\nName: %s\nvalue: %i\n", p->name, *((unsigned short*) p->value));
			wb = gui_internal_label_new(this, g_strdup_printf("Integer %i", *((unsigned short*) p->value)));
			break;
		}
		case integer32:{
			dbg(lvl_error, "\nName: %s\nvalue: %i\n", p->name, *((int*) p->value));
			wb = gui_internal_label_new(this, g_strdup_printf("Integer %i", *((int*) p->value)));
			break;
		}
		case integer32u:{
			dbg(lvl_error, "\nName: %s\nvalue: %i\n", p->name, *((unsigned int*) p->value));
			wb = gui_internal_label_new(this, g_strdup_printf("Integer %i", *((unsigned int*) p->value)));
			break;
		}
		case integer64:{
			dbg(lvl_error, "\nName: %s\nvalue: %i\n", p->name, *((long*) p->value));
			wb = gui_internal_label_new(this, g_strdup_printf("Integer %i", *((long*) p->value)));
			break;
		}case integer64u:{
			dbg(lvl_error, "\nName: %s\nvalue: %i\n", p->name, *((unsigned long*) p->value));
			wb = gui_internal_label_new(this, g_strdup_printf("Integer %i", *((unsigned long*) p->value)));
			break;
		}
		case string:{
			dbg(lvl_error, "\nName: %s\nvalue: %s\n", p->name, (char*) p->value );
			wb = gui_internal_label_new(this, g_strdup_printf("String: %s", p->value));
			break;
		}
		case structure:{
			dbg(lvl_error, "\nName: %s\nvalue: %p\n", p->name, p->value );
			wb = gui_internal_label_new(this, g_strdup_printf("Structure: %p", p->value));
			break;
		}
		
		case boolean:{
			if(p->value){
				dbg(lvl_error, "\nName: %s\nvalue: ON", p->name);
				wb = gui_internal_label_new(this, g_strdup_printf("Bool: ON"));
			}else{
				dbg(lvl_error, "\nName: %s\nvalue: OFF", p->name);
				wb = gui_internal_label_new(this, g_strdup_printf("Bool: OFF"));
			}
			break;
		}
	}
	gui_internal_widget_append(wl, wb);
	if(p->ro == 0){
		wb = gui_internal_button_new_with_callback(this, " + ", NULL, gravity_left_center | orientation_horizontal, increase_value, p);
		gui_internal_widget_append(wl, wb);
	}
	return wl;
	
	
	
}

void
gui_internal_service_property (struct gui_priv *this, struct widget *wm, void *data)
{
	struct widget *wb, *w, *wbm;
    struct widget *tbl, *row;
    int index=0;
    
    char* name = "";
    gui_internal_prune_menu_count (this, 1, 0);
    GList* properties = NULL;
	struct service_property *prop = (struct service_property *) data;
	dbg (lvl_error, "Showing properties %p\n", prop->name);
	if(prop)
		properties = prop->children; 
	
	wb = gui_internal_menu (this, g_strdup_printf("Property: %s", prop->name));
	wb->background = this->background;
    w = gui_internal_box_new (this, gravity_top_center | orientation_vertical | flags_expand | flags_fill);
    gui_internal_widget_append (wb, w);	
	if(prop->value != NULL){
			dbg (lvl_error, "Value: %p, %i\n", prop->value, prop->value);
			gui_internal_widget_append (w, gui_internal_service_edit_property(this, prop));
	}
	
	tbl = gui_internal_widget_table_new (this, gravity_left_top | flags_fill | flags_expand | orientation_vertical, 1);
	gui_internal_widget_append (w, tbl);
	
	while(properties){
		struct service_property *p = properties->data;

		row = gui_internal_widget_table_row_new (this, gravity_left | flags_fill | orientation_horizontal);
		wbm = gui_internal_button_new_with_callback (this,
						 p->name,image_new_s (this,/* (service->icon) ? (service->icon) : */("gui_inactive")),
						 gravity_left_center |
						 orientation_horizontal | flags_fill, gui_internal_service_property, p);

		gui_internal_widget_append (tbl, row);
		gui_internal_widget_append (row, wbm);	
		properties = g_list_next(properties);
	}
     gui_internal_menu_render (this);
}

/**
 * @brief   
 * @param[in]   this    - pointer to the gui_priv
 *              wm      - pointer the the parent widget
 *              data    - pointer to arbitrary data
 *
 * @return  nothing
 *
 *
 *
 */
void
gui_internal_service_devicelist (struct gui_priv *this, struct widget *wm, void *data)
{
    struct widget *wb, *w, *wbm;
    struct widget *tbl, *row;
    int index=0;
    char* name = "";
    gui_internal_prune_menu_count (this, 1, 0);
    GList *properties = NULL;
    //*
    struct service_priv* service = (struct service*) data;
    if(service){
		struct attr attr;
		if(service_get_attr(service, attr_type, &attr, NULL))
			name = g_strdup(attr.u.str);
		
		wb = gui_internal_menu (this, g_strdup_printf ("%s Devices",name));
		properties = service_get_properties(service);
		dbg (lvl_error, "Service %s, Properties: %p\n", name, properties);
	}else{
		dbg (lvl_error, "Service not available\n");
		wb = gui_internal_menu (this, g_strdup_printf ("Service not available"));
    }
    //*/
    
    wb->background = this->background;
    w = gui_internal_box_new (this, gravity_top_center | orientation_vertical | flags_expand | flags_fill);
    gui_internal_widget_append (wb, w);
	tbl = gui_internal_widget_table_new (this, gravity_left_top | flags_fill | flags_expand | orientation_vertical, 1);
	gui_internal_widget_append (w, tbl);
	
	while(properties){
		struct service_property *p = properties->data;
		if(p){
			dbg (lvl_error, "Service Property %p, Name: %s\n", p, (p->name)?p->name:"NULL");
			row = gui_internal_widget_table_row_new (this, gravity_left | flags_fill | orientation_horizontal);
			wbm = gui_internal_button_new_with_callback (this,
							 p->name,image_new_s (this,/* (service->icon) ? (service->icon) : */("gui_active")),
							 gravity_left_center |
							 orientation_horizontal | flags_fill, gui_internal_service_property, p);

			gui_internal_widget_append (tbl, row);
			gui_internal_widget_append (row, wbm);	
			properties = g_list_next(properties);
		}
	}
    
	
    gui_internal_menu_render (this);
}
