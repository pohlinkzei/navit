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
#include "gui_internal_media.h"
#include "gui_internal_command.h"
#include <plugin/i2c/i2c.h>

static void
tracks_free (gpointer data)
{
	if (data != NULL)
	{
		struct audio_track *track = data;
		g_free(track->name);
		g_free (track);
	}
}

/**
 * @brief   play a track from the playlist
 * @param[in]   this    - pointer to the gui_priv
 *              wm      - pointer the the parent widget
 *              date    - pointer to arbitrary data
 * /
void
media_play_track (struct gui_priv *this, struct widget *wm, void *data)
{
    dbg (lvl_info, "Got a request to play a specific track : %i\n", wm->c.x);
    audio_play_track(this->nav, wm->c.x);
    gui_internal_prune_menu(this, NULL);
    //gui_internal_media_show_playlist (this, NULL, NULL);
}

/**
 * @brief   Build a player 'toolbar'
 * @param[in]   this    - pointer to the gui_priv
 *
 * @return  the resulting widget
 *
 * Builds a widget containing buttons to browse the root playlist, some buttons to control the player
 * The buttons are mapped to the player actions
 */
static struct widget *
gui_internal_media_playlist_toolbar (struct gui_priv *this)
{
    struct widget *wl, *wb;
    int nitems, nrows;
    GList* actions = audio_get_actions(this->nav);
    wl = gui_internal_box_new (this, gravity_left_center | orientation_horizontal_vertical | flags_fill);
    wl->background = this->background;
    wl->w = this->root.w;
    wl->cols = this->root.w / this->icon_s;
    nitems = 2;
    nrows = nitems / wl->cols + (nitems % wl->cols > 0);
    wl->h = this->icon_l * nrows;
    wb = gui_internal_button_new_with_callback (this, "Playlists",
						image_new_s (this, "media_category"),
						gravity_left_center |
						orientation_horizontal, gui_internal_media_show_rootlist, NULL);
    gui_internal_widget_append (wl, wb); 
    while(actions){
		struct audio_actions *aa = actions->data;
		actions = g_list_next(actions);
		if(aa->icon && aa->action){		
		gui_internal_widget_append (wl, wb =
				gui_internal_button_new_with_callback (this, NULL,
					   image_new_s (this, aa->icon),
					   gravity_left_center
					   |
					   orientation_horizontal,
					   media_action_do, aa->action));
    		}
    }
    return wl;
}

/**
 * @brief   Show the playlists in the root playlist
 * @param[in]   this    - pointer to the gui_priv
 *              wm      - pointer the the parent widget
 *              data    - pointer to arbitrary data
 *
 * @return  nothing
 *
 * Display a list of the playlists in the root playlist
 *
 */
void
gui_internal_i2c_devicelist (struct gui_priv *this, struct widget *wm, void *data)
{
    struct widget *wb, *w, *wbm;
    struct widget *tbl, *row;
    dbg (lvl_info, "Showing rootlist\n");
    

    gui_internal_prune_menu_count (this, 1, 0);
    wb = gui_internal_menu (this, "I2C Config");
    wb->background = this->background;
    w = gui_internal_box_new (this, gravity_top_center | orientation_vertical | flags_expand | flags_fill);
    gui_internal_widget_append (wb, w);


    tbl = gui_internal_widget_table_new (this, gravity_left_top | flags_fill | flags_expand | orientation_vertical, 1);
    gui_internal_widget_append (w, tbl);

    while(connected_devices) {
      struct i2c_device * i2cd=connected_devices->data;
      
      connected_devices=g_list_next(connected_devices);
	  row = gui_internal_widget_table_row_new (this, gravity_left | flags_fill | orientation_horizontal);
	  gui_internal_widget_append (tbl, row);
	  wbm = gui_internal_button_new_with_callback (this,
						     pl->name,image_new_s (this, (i2cd->icon) ? (i2cd->icon) : ("gui_ok")),
						     gravity_left_center |
						     orientation_horizontal | flags_fill, i2c_show_device, NULL);

	  gui_internal_widget_append (row, wbm);
	  wbm->c.x = i2cd->name;
      }

    gui_internal_menu_render (this);

}

/**
 * @brief   Show the tracks in the current playlist
 * @param[in]   this    - pointer to the gui_priv
 *              wm      - pointer the the parent widget
 *              data    - pointer to arbitrary data
 *
 * @return  nothing
 *
 * Display a list of the tracks in the current playlist
 *
 */
void
gui_internal_i2c_show_device (struct gui_priv *this, struct widget *wm, void *data)
{
    struct widget *wb, *w, *wbm;
    struct widget *tbl, *row;
    int index=0;
    gui_internal_prune_menu_count (this, 1, 0);
#ifndef USE_I2C
    wb = gui_internal_menu (this, g_strdup_printf ("I2C not available"));
    wb->background = this->background;
    w = gui_internal_box_new (this, gravity_top_center | orientation_vertical | flags_expand | flags_fill);
    gui_internal_widget_append (wb, w);
#else
    
    wb = gui_internal_menu (this, g_strdup_printf ("I2C > %s", NULL));
    wb->background = this->background;
    w = gui_internal_box_new (this, gravity_top_center | orientation_vertical | flags_expand | flags_fill);
    gui_internal_widget_append (wb, w);
    gui_internal_widget_append (w, gui_internal_media_playlist_toolbar (this));
    tbl = gui_internal_widget_table_new (this, gravity_left_top | flags_fill | flags_expand | orientation_vertical, 1);
    gui_internal_widget_append (w, tbl);
    while(tracks) {
		struct audio_track * track=tracks->data;
		tracks=g_list_next(tracks);
		row = gui_internal_widget_table_row_new (this, gravity_left | flags_fill | orientation_horizontal);
		gui_internal_widget_append (tbl, row);
		wbm = gui_internal_button_new_with_callback (
						this, 
						track->name, 
						image_new_s(this, track->icon), 
						gravity_left_center | orientation_horizontal | flags_fill, 
						media_play_track, 
						NULL);
		wbm->c.x = track->index;
		gui_internal_widget_append (row, wbm);
    }
#endif
	g_list_free_full(tracks, tracks_free);
    gui_internal_menu_render (this);
}


