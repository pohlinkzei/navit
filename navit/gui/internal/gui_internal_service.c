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
    dbg (lvl_error, "Showing devicelist %p\n", data);
    char* name = "";
    gui_internal_prune_menu_count (this, 1, 0);
    //*
    struct service_priv* service = (struct service*) data;
    if(service){
		struct attr attr;
		if(service_get_attr(service, attr_type, &attr, NULL))
			name = g_strdup(attr.u.str);
		dbg (lvl_error, "Service %s not implemented\n", data);
		wb = gui_internal_menu (this, g_strdup_printf ("Service %s not implemented",name));
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
	row = gui_internal_widget_table_row_new (this, gravity_left | flags_fill | orientation_horizontal);
	wbm = gui_internal_button_new_with_callback (this,
						 /*service->name*/ "device",image_new_s (this,/* (service->icon) ? (service->icon) : */("gui_inactive")),
						 gravity_left_center |
						 orientation_horizontal | flags_fill, gui_internal_service_root, NULL);

		gui_internal_widget_append (tbl, row);
		gui_internal_widget_append (row, wbm);	
	
	
    
	
    gui_internal_menu_render (this);
}
