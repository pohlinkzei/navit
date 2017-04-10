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

#ifndef I2C_H
#define I2C_H

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>

#include <math.h>

#include <glib.h>
#include <time.h>


#include <string.h>
#include <errno.h>

#include <config.h>
#include <config_.h>
#include <navit.h>
#include <coord.h>
#include <point.h>
#include <plugin.h>
#include <debug.h>
#include <item.h>
#include <xmlconfig.h>
#include <attr.h>
#include <layout.h>
#include <navigation.h>
#include <command.h>
#include <callback.h>
#include <graphics.h>
#include <track.h>
#include <vehicle.h>
#include <vehicleprofile.h>
#include <map.h>
#include <event.h>
#include <mapset.h>
#include <osd.h>
#include <route.h>
#include <search.h>
#include <callback.h>
#include <gui.h>
#include <util.h>
#include <service.h>
#include <i2c_devices.h>
#ifdef USE_AUDIO_FRAMEWORK
#include <audio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif 

#define AUDIO_STR_LENGTH 38
struct navigation_itm;
struct navigation;

struct i2c_nav_data{
	int distance_to_next_turn;
	int nav_status;
	int next_turn;
};

struct service_priv{
	struct navit* nav;
	struct callback_list *cbl;
	struct attr** attrs;
	struct service* parent;
	char* source;
	char* name;
	char* icon;
	GList* properties;
    int device;
    int last_status;
    int last_next_turn;
    struct callback* task;
    struct callback* callback;
    int timeout;
    int stub;
    GList* connected_devices;
    struct i2c_nav_data* navigation_data;
};

struct connected_devices{
	char* name;
	uint8_t addr;
	char* icon;
	void* rx_data;
	void* tx_data;
	uint8_t rx_size;
	uint8_t tx_size;
	uint8_t num_properties;
	uint8_t (*serialize_tx)(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);
	uint8_t (*serialize_rx)(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
	uint8_t (*deserialize_rx)(void *rx_data, uint8_t size, volatile uint8_t buffer[size]); 
	GList* (*init_properties)(void *rx_data, void* tx_data, struct service_property *parent);
};


void read_i2c_frame(int device, uint8_t* data, uint8_t size);
//*
uint8_t calculateID(char* name);
void i2c_get_plugin(struct service_priv* p);
int i2c_set_attr(struct service_priv *priv, struct attr *attr);
int i2c_get_attr(struct service_priv *priv,enum attr_type type, struct attr *attr);
GList* i2c_get_properties(struct service_priv *priv);
struct service_property* i2c_set_property(struct service_priv *priv, struct service_property* sp);
struct i2c_nav_data* get_navigation_data(struct service_priv *this);
void get_audio_data(struct service_priv *this, uint8_t audio_str[AUDIO_STR_LENGTH]);
#ifdef __cplusplus
}
#endif
#endif
