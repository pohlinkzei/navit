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
#include "gui_internal_widget.h"
#include "gui_internal_priv.h"
#include "gui_internal_command.h"
#include "gui_internal_service.h"

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
    dbg (lvl_info, "Showing rootlist\n");
    
    gui_internal_prune_menu_count (this, 1, 0);
    wb = gui_internal_menu (this, "..,-");
    wb->background = this->background;
    w = gui_internal_box_new (this, gravity_top_center | orientation_vertical | flags_expand | flags_fill);
    gui_internal_widget_append (wb, w);


    tbl = gui_internal_widget_table_new (this, gravity_left_top | flags_fill | flags_expand | orientation_vertical, 1);
    gui_internal_widget_append (w, tbl);

    while(0) {
      row = gui_internal_widget_table_row_new (this, gravity_left | flags_fill | orientation_horizontal);
	  gui_internal_widget_append (tbl, row);
	  //add rows
	  gui_internal_widget_append (row, wbm);
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
gui_internal_service (struct gui_priv *this, struct widget *wm, void *data)
{
    struct widget *wb, *w, *wbm;
    struct widget *tbl, *row;
    int index=0;
    gui_internal_prune_menu_count (this, 1, 0);
    wb = gui_internal_menu (this, g_strdup_printf ("Service not available"));
    wb->background = this->background;
    w = gui_internal_box_new (this, gravity_top_center | orientation_vertical | flags_expand | flags_fill);
    gui_internal_widget_append (wb, w);

    gui_internal_menu_render (this);
}
