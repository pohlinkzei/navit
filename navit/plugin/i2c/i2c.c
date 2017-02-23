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
#ifdef USE_AUDIO_FRAMEWORK
#include <audio.h>
#endif


#define AUDIO_STR_LENGTH 38
#define CRC_POLYNOME 0xAB
char x[] = {0x66, 0x55,0x44, 0x33, 0x22, 0x11};
char result[6] = {0,};
unsigned char i2ctxdata[128] = {0,};
unsigned char i2crxdata[128] = {0,};
struct service_priv *i2c_plugin;

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

typedef struct txdataLSG{
	uint8_t AL;
	uint8_t TFL;
	uint8_t ZV;
	uint8_t LED;
	uint8_t time_in;
	uint8_t time_out;
}tx_lsg_t;
	
typedef struct rxdataLSG{
	uint8_t AL;
	uint8_t TFL;
	uint8_t ZV;
	uint8_t LED;
	uint8_t time_in;
	uint8_t time_out;
}rx_lsg_t;

typedef struct txdataV2V{
	uint16_t pwm_freq;
	uint8_t cal_temperature;
	uint8_t cal_voltage;
	uint8_t water_value;
	uint8_t time_value;
}tx_v2v_t;
	
typedef struct rxdataV2V{
	uint16_t pwm_freq;
	uint8_t cal_temperature;
	uint8_t cal_voltage;
	uint8_t water_value;
	uint8_t time_value;
	uint16_t vbat;
	uint8_t water_temp;
	uint8_t fet_temp;
}rx_v2v_t;

typedef struct txdataPWM{
	uint16_t pwm_freq;
	uint8_t cal_temperature;
	uint8_t cal_voltage;
	uint8_t water_value;
	uint8_t time_value;
}tx_pwm_t;
	
typedef struct rxdataPWM{
	uint16_t pwm_freq;
	uint8_t cal_temperature;
	uint8_t cal_voltage;
	uint8_t water_value;
	uint8_t time_value;
	uint16_t vbat;
	uint8_t water_temp;
	uint8_t fet_temp;
}rx_pwm_t;

typedef struct txdataWFS{
	uint8_t time;
}tx_wfs_t;
	
typedef struct rxdataWFS{
	uint8_t time;
}rx_wfs_t;

typedef struct txdataMFA{
	uint32_t distance_to_next_turn;
	uint8_t radio_text[AUDIO_STR_LENGTH];
	
	uint8_t navigation_next_turn;
	//navigation active?
	uint8_t cal_water_temperature;
	uint8_t cal_voltage;
	uint8_t cal_oil_temperature;
	uint8_t cal_consumption;	
	// other values from pwm module?
}tx_mfa_t;
/*
typedef struct txdataMFA{
	uint8_t radio_text[AUDIO_STR_LENGTH];
	uint8_t navigation_next_turn;
	uint32_t distance_to_next_turn;
	//navigation active?
	uint8_t cal_water_temperature;
	uint8_t cal_voltage;
	uint8_t cal_oil_temperature;
	uint8_t cal_consumption;	
	// other values from pwm module?
}tx_mfa_t;*/
	
typedef struct rxdataMFA{
	
	uint32_t distance_to_next_turn;
	uint16_t voltage;
	uint16_t consumption;
	uint16_t average_consumption;
	uint16_t range;
	uint16_t speed;
	uint16_t average_speed;
	uint16_t rpm;
	uint8_t radio_text[AUDIO_STR_LENGTH];
	uint8_t navigation_next_turn;	//navigation active?
	uint8_t cal_water_temperature;
	uint8_t cal_voltage;
	uint8_t cal_oil_temperature;
	uint8_t cal_consumption;
	// read only
	int8_t water_temperature;
	int8_t ambient_temperature;
	int8_t oil_temperature;
}rx_mfa_t;
/*
typedef struct rxdataMFA{
	uint8_t radio_text[AUDIO_STR_LENGTH];
	uint8_t navigation_next_turn;
	uint32_t distance_to_next_turn;
	//navigation active?
	uint8_t cal_water_temperature;
	uint8_t cal_voltage;
	uint8_t cal_oil_temperature;
	uint8_t cal_consumption;
	// read only
	uint16_t voltage;
	int8_t water_temperature;
	int8_t ambient_temperature;
	int8_t oil_temperature;
	uint16_t consumption;
	uint16_t average_consumption;
	uint16_t range;
	uint16_t speed;
	uint16_t average_speed;
	uint16_t rpm;
}rx_mfa_t;
*/


rx_lsg_t *rx_lsg = NULL;
tx_lsg_t *tx_lsg = NULL;
rx_pwm_t *rx_pwm = NULL;
tx_pwm_t *tx_pwm = NULL;
rx_wfs_t *rx_wfs = NULL;
tx_wfs_t *tx_wfs = NULL;
rx_v2v_t *rx_v2v = NULL;
tx_v2v_t *tx_v2v = NULL;
rx_mfa_t *rx_mfa = NULL;
tx_mfa_t *tx_mfa = NULL;

void read_i2c_frame(int device, uint8_t* data, uint8_t size);
//*
void i2c_get_plugin(struct service_priv* p);
int i2c_set_attr(struct service_priv *priv, struct attr *attr);
int i2c_get_attr(struct service_priv *priv,enum attr_type type, struct attr *attr);
GList* i2c_get_properties(struct service_priv *priv);
struct service_property* i2c_set_property(struct service_priv *priv, struct service_property* sp);
/*
static struct service_methods i2c_service_meth = {
		i2c_get_plugin,
		i2c_set_attr,
		i2c_get_attr,
};
*/

struct navigation_itm;
struct navigation;

uint8_t deserialize_pwm_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_pwm_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_pwm_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);
GList* init_pwm_properties(void* rx_data, void* tx_data, struct service_property *parent);

uint8_t deserialize_mfa_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_mfa_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_mfa_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);
GList* init_mfa_properties(void* rx_data, void* tx_data, struct service_property *parent);

uint8_t deserialize_wfs_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_wfs_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_wfs_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);
GList* init_wfs_properties(void* rx_data, void* tx_data, struct service_property *parent);

uint8_t deserialize_v2v_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_v2v_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_v2v_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);
GList* init_v2v_properties(void* rx_data, void* tx_data, struct service_property *parent);

uint8_t deserialize_lsg_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_lsg_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_lsg_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);
GList* init_lsg_properties(void* rx_data, void* tx_data, struct service_property *parent);

struct i2c_nav_data* get_navigation_data(struct service_priv *this);
void get_audio_data(struct service_priv *this, uint8_t audio_str[AUDIO_STR_LENGTH]);

static struct service_methods i2c_service_meth = {
		i2c_get_plugin,
		i2c_set_attr,
		i2c_get_attr,
		i2c_get_properties,
		i2c_set_property,
};


int i2c_set_attr(struct service_priv *priv, struct attr *attr){
	dbg(lvl_error, "i2c_set_attr(struct service_priv=%p, struct attr=%p)\n", priv,  attr);
	switch(attr->type){
		case attr_name:
			priv->name = attr->u.str;
			break;
		/*
		case attr_property:
			//priv->properties->data = attr->u.str;
			break;
		//*/
		default:
		break;
	}
		return 1;
}

int i2c_get_attr(struct service_priv *priv,enum attr_type type, struct attr *attr){
	dbg(lvl_error, "i2c_get_attr(struct service_priv=%p, enum attr_type=%p (%s), struct attr=%p)\n", priv, type, attr_to_name(type), attr);
	switch(type){
		case attr_type:
			attr->u.str = priv->name;
			break;
		/*
		case attr_property:
			//priv->properties->data = attr->u.str;
			break;
		//*/
		default:
		break;
	}
		return 1;
}



uint8_t calculateID(char* name){
	//calculate an ID from the first 3 Letter of its name
	uint8_t ID;
	ID = (name[0]-48) * 3 + (name[1]-48) * 2 + (name[2]-48);
	ID = ID & 0x7F;
	dbg(lvl_debug,"Name: %s, ID = 0x%02X\n",name, ID);
	return ID ;
}
/*
void init_i2c_data(void){
	 
}
*/
uint8_t crc8(uint8_t crc, uint8_t data){
	uint8_t i, _data = 0;
	_data = crc ^ data;
	for(i=0; i<8; i++){
		if((_data & 0x80) != 0){
			_data <<= 1;
			_data ^= 0x07;
		}else{
			_data <<= 1;
		}
	}
	return _data;
}

uint8_t calculateCRC8(uint8_t crc, uint8_t* data, uint8_t len){
	//dbg(lvl_debug,"crc: %p, %i\n",data, crc);
	while(len-->0){
		//dbg(lvl_debug,"crc: %p, %i\n",data, crc);
		crc = crc8(crc, *data++);
	}
	//dbg(lvl_debug,"\ncrc: %i\n",crc);
	return crc;
}


int open_i2c(const char* device){
	int dev_h = open(device, O_RDWR);
	if(dev_h < 0){
		perror("Error: Can't open I2C device!\n");
		return 0;
	}
	return dev_h;
}

unsigned long check_ioctl(int device){
	unsigned long funcs;
	if(ioctl(device, I2C_FUNCS, &funcs) < 0){
		perror("Error: No I2C functions found!\n");
		return 0;
	}
	if(funcs & I2C_FUNC_I2C){
		dbg(lvl_debug,"I2C_FUNC_I2C found.\n");
	}
	if(funcs & I2C_FUNC_SMBUS_BYTE){
		dbg(lvl_debug,"I2C_FUNC_SMBUS_BYTE found.\n");
	}	
	return funcs;
}

void scan_i2c_bus(int device){
	int port, res;
	if(device){
		for(port = 0; port < 127; port++){
			if(ioctl(device, I2C_SLAVE, port) < 0){
				dbg(lvl_debug,"Error: No I2C_SLAVE found!\n");
			}else{
				res = i2c_smbus_read_byte(device);
				if (res >= 0){
					dbg(lvl_debug,"I2C device found at 0x%02x, val = 0x%02x\n",port, res);
				}
			}
		}
	}
}

int select_slave(int device, uint8_t addr){
	int res;
	if(device){
		dbg(lvl_debug,"Probe Address 0x%02X: %i\n", addr, ioctl(device, I2C_SLAVE, addr));
		res = i2c_smbus_read_byte_data(device, 0);
		if (res >= 0){
			dbg(lvl_debug,"I2C device found at 0x%02x, val = 0x%02x\n",addr, res);
			return 0;
		}
	}
	return 1;
}

int init_i2c_devices(struct service_priv *this){
	int port, res;
	this->connected_devices = NULL;
	if(this->device){
		for(port = 0; port < 127; port++){
			if(ioctl(this->device, I2C_SLAVE, port) < 0){
				dbg(lvl_debug,"Error: No I2C_SLAVE found!\n");
			}else{
				res = i2c_smbus_read_byte(this->device);
				if (res >= 0){
					dbg(lvl_info,"I2C device found at 0x%02x, val = 0x%02x\n",port, res);
					struct connected_devices *cd = g_new0(struct connected_devices, 1);
					cd->addr = port;
					if(port == calculateID("MFA")){
						
						rx_mfa = (rx_mfa_t*) malloc(sizeof(rx_mfa_t)); 
						tx_mfa = (tx_mfa_t*) malloc(sizeof(tx_mfa_t));
						cd->name = g_strdup("MFA");
						cd->icon = "gui_active";
						cd->rx_data = rx_mfa;
						cd->tx_data = tx_mfa;
						cd->num_properties = 17;
						cd->serialize_rx = serialize_mfa_rxdata;
						cd->serialize_tx = serialize_mfa_txdata;
						cd->deserialize_rx = deserialize_mfa_rxdata;
						cd->init_properties = init_mfa_properties;
						cd->rx_size = sizeof(rx_mfa_t);
						cd->tx_size = sizeof(tx_mfa_t);
					}else if(port == calculateID("LSG")){
						rx_lsg = (rx_lsg_t*) malloc(sizeof(rx_lsg_t)); 
						tx_lsg = (tx_lsg_t*) malloc(sizeof(tx_lsg_t)); 
						cd->name = g_strdup("LSG");
						cd->icon = "gui_active";
						cd->rx_data = rx_lsg;
						cd->tx_data = tx_lsg;
						cd->num_properties = 6;
						cd->serialize_rx = serialize_lsg_rxdata;
						cd->serialize_tx = serialize_lsg_txdata;
						cd->deserialize_rx = deserialize_lsg_rxdata;
						cd->init_properties = init_lsg_properties;
						cd->rx_size = sizeof(rx_lsg_t);
						cd->tx_size = sizeof(tx_lsg_t);
					}else if(port == calculateID("WFS")){
						rx_wfs = (rx_wfs_t*) malloc(sizeof(rx_wfs_t)); 
						tx_wfs = (tx_wfs_t*) malloc(sizeof(tx_wfs_t)); 
						cd->name = g_strdup("WFS");
						cd->icon = "gui_active";
						cd->rx_data = rx_wfs;
						cd->tx_data = tx_wfs;
						cd->num_properties = 0;
						cd->serialize_rx = serialize_wfs_rxdata;
						cd->serialize_tx = serialize_wfs_txdata;
						cd->deserialize_rx = deserialize_wfs_rxdata;
						cd->init_properties = init_wfs_properties;
						cd->rx_size = sizeof(rx_wfs_t);
						cd->tx_size = sizeof(tx_wfs_t);
					}else if(port == calculateID("PWM")){
						rx_pwm = (rx_pwm_t*) malloc(sizeof(rx_pwm_t)); 
						tx_pwm = (tx_pwm_t*) malloc(sizeof(tx_pwm_t)); 
						tx_pwm->pwm_freq = 10000;
						tx_pwm->cal_temperature = 5;
						tx_pwm->cal_voltage = 5;
						tx_pwm->time_value = 10;
						tx_pwm->water_value = 35;
						cd->name = g_strdup("PWM");
						cd->icon = "gui_active";
						cd->rx_data = rx_pwm;
						cd->tx_data = tx_pwm;
						cd->num_properties = 8;
						cd->serialize_rx = serialize_pwm_rxdata;
						cd->serialize_tx = serialize_pwm_txdata;
						cd->deserialize_rx = deserialize_pwm_rxdata;
						cd->init_properties = init_pwm_properties;
						cd->rx_size = sizeof(rx_pwm_t);
						cd->tx_size = sizeof(tx_pwm_t);
					}else if(port == calculateID("V2V")){
						rx_v2v = (rx_v2v_t*) malloc(sizeof(rx_v2v_t)); 
						tx_v2v = (tx_v2v_t*) malloc(sizeof(tx_v2v_t)); 
						cd->name = g_strdup("V2V");
						cd->icon = "gui_active";
						cd->rx_data = rx_v2v;
						cd->tx_data = tx_v2v;
						cd->num_properties = 8;
						cd->serialize_rx = serialize_v2v_rxdata;
						cd->serialize_tx = serialize_v2v_txdata;
						cd->deserialize_rx = deserialize_v2v_rxdata;
						cd->init_properties = init_v2v_properties;
						cd->rx_size = sizeof(rx_v2v_t);
						cd->tx_size = sizeof(tx_v2v_t);
					}else{
						cd->name = g_strdup("I2C");
						cd->icon = "gui_inactive";
						cd->init_properties = NULL;
						return 0;
					}
					dbg(lvl_info, "Appending a Device %p\n", cd);
					this->connected_devices = g_list_append(this->connected_devices, cd);
				}
			}
		}
	}
	return 1;
}

void get_audio_data(struct service_priv* this, uint8_t audio_str[AUDIO_STR_LENGTH]){
	int i = 0;
	gchar str[256] = {0,};
#ifdef USE_AUDIO_FRAMEWORK 
	strcpy(str, audio_get_current_track(this->nav));
	strcat(str, " - ");
	strcat(str, audio_get_current_playlist(this->nav));
#else
	sprintf(str, "No Audio Plugin found!");
#endif
for (i=0; i< AUDIO_STR_LENGTH; i++){
		audio_str[i] = str[i];
	}
}


int get_next_turn_by_name(char* name){
		//tdb
		return 0;
}	

int
round_distance(int distance)
{
	if (distance >= 10000)
		return ((distance + 500) / 1000) * 1000;
	else if (distance >= 1000)
		return ((distance+50) / 100) * 100;
	else if (distance >= 300)
		return ((distance+ 13) / 25) * 25;
	else if (distance >= 50)
		return ((distance + 5) / 10) * 10;
	else 
		return distance;
	
}
struct i2c_nav_data*
get_navigation_data(struct service_priv* this){
	struct i2c_nav_data* nav_data = this->navigation_data;
	
	struct attr attr;
	struct navigation *nav = NULL;
	struct navigation_itm *nav_itm = NULL;
	struct map *map = NULL;
	struct map_rect *mr = NULL;
	struct item *item = NULL;
	uint8_t status = 0, navigation_next_turn = 0;
	char* name = "null";
	struct navit* navit = this->nav;
	if(nav_data){
	if (navit)
		nav = navit_get_navigation(navit);
	if (nav){
		map = navigation_get_map(nav);
		struct attr item1;
		navigation_get_attr(nav, attr_length, &item1, NULL);
		dbg(lvl_info, "length: %i\n", round_distance(item1.u.num));
		nav_data->distance_to_next_turn = round_distance(item1.u.num);
		
		if (navigation_get_attr(nav, attr_nav_status, &attr, NULL)){
			uint8_t status = attr.u.num;
			uint8_t status2 = (status == 3) ? 4 : status;

			if ((status2 != this->last_status) && (status2 != status_invalid)) {
				this->last_status = status2;
				dbg(lvl_info, "status=%s\n", nav_status_to_text(status2));
				nav_data->nav_status = status;
			}
		}
	}
	if (map)
		mr = map_rect_new(map, NULL);
	if (mr)
		while ((item = map_rect_get_item(mr))
		       && (item->type == type_nav_position || item->type == type_nav_none));
	if (item) {
		name = item_to_name(item->type);
		//navigation.item[1].length[named]
	}
	dbg(lvl_info, "name=%s\n", name);
	nav_data->next_turn = get_next_turn_by_name(name);
	}
	return nav_data;
}

///////////////////////////////////////////////////////////////////////////
// PWM 
///////////////////////////////////////////////////////////////////////////
//*
uint8_t serialize_pwm_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(tx_pwm_t)){
		dbg(lvl_debug,"size: %i, struct: %i\n",size,sizeof(tx_pwm_t));
		return 0;
	}
	tx_pwm_t* tx = (tx_pwm_t*) tx_data;
	dbg(lvl_debug,"\nserialize_pwm_txdata\n");
	dbg(lvl_debug,"PWM:\nfreq: %i\ntemp: %i\nvtg: %i\nwatertemp_sh: %i\ntime_sh: %i\n",tx->pwm_freq, tx->cal_temperature, tx->cal_voltage, tx->water_value, tx->time_value);
		buffer[0] = (uint8_t) ((tx->pwm_freq & 0xFF00) >> 8);
	buffer[1] = (uint8_t) (tx->pwm_freq & 0x00FF);
	buffer[2] = tx->cal_temperature;
	buffer[3] = tx->cal_voltage;
	buffer[4] = tx->water_value;
	buffer[5] = tx->time_value;
	dbg(lvl_debug,"PWM: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	return 1;
}

uint8_t serialize_pwm_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_pwm_t)){
		return 0;
	}
	rx_pwm_t* rx = (rx_pwm_t*) rx_data;
	dbg(lvl_debug,"\nserialize_pwm_rxdata\n");
	dbg(lvl_debug,"PWM:\nfreq: %i\ntemp: %i\nvtg: %i\nwatertemp_sh: %i\ntime_sh: %i\nvbat: %i\nwatertemp: %i\nfettemp:%i\n",rx->pwm_freq, rx->cal_temperature, rx->cal_voltage, rx->water_value, rx->time_value, rx->vbat, rx->water_temp, rx->fet_temp);
	buffer[0] = (uint8_t) ((rx->pwm_freq & 0xFF00) >> 8);
	buffer[1] = (uint8_t) (rx->pwm_freq & 0x00FF);
	buffer[2] = rx->cal_temperature;
	buffer[3] = rx->cal_voltage;
	buffer[4] = rx->water_value;
	buffer[5] = rx->time_value;
	buffer[6] = (uint8_t) ((rx->vbat & 0xFF00) >> 8);
	buffer[7] = (uint8_t) (rx->vbat & 0x00FF);
	buffer[8] = rx->water_temp;
	buffer[9] = rx->fet_temp;
	dbg(lvl_debug,"PWM: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	
	return 1;
}

uint8_t deserialize_pwm_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_pwm_t)){
		return 0;
	}
	rx_pwm_t* rx = (rx_pwm_t*) rx_data;
	dbg(lvl_debug,"\ndeserialize_pwm_rxdata\n");
	dbg(lvl_debug,"PWM: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	rx->pwm_freq = ((uint16_t) (buffer[0]) << 8) + buffer[1];
	rx->cal_temperature = buffer[2];
	rx->cal_voltage = buffer[3];
	rx->water_value = buffer[4];
	rx->time_value = buffer[5];
	rx->vbat = ((uint16_t) (buffer[6]) << 8) + buffer[7];
	rx->water_temp = buffer[8];
	rx->fet_temp = buffer[9];
	dbg(lvl_debug,"PWM:\nfreq: %i\ntemp: %i\nvtg: %i\nwatertemp_sh: %i\ntime_sh: %i\nvbat: %i\nwatertemp: %i\nfettemp:%i\n",rx->pwm_freq, rx->cal_temperature, rx->cal_voltage, rx->water_value, rx->time_value, rx->vbat, rx->water_temp, rx->fet_temp);
	return 1;
}

GList* init_pwm_properties(void *rx_data, void* tx_data, struct service_property *parent){
	GList* list = NULL;
	rx_pwm_t* rx = (rx_pwm_t*) rx_data;
	rx_pwm_t* tx = (rx_pwm_t*) tx_data;
	struct service_property *p = g_new0(struct service_property,1);
	if(rx->pwm_freq == tx->pwm_freq){
		p->name = g_strdup("Frequency");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->pwm_freq;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}

	if(rx->cal_voltage == tx->cal_voltage){
		p->name = g_strdup("Calibration Voltage");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->cal_voltage;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_temperature == tx->cal_temperature){
		p->name = g_strdup("Calibration Temperature");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->cal_temperature;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->water_value == tx->water_value){
		p->name = g_strdup("Enable Temperature");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->water_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->time_value == tx->time_value){
		p->name = g_strdup("Enable Time");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->time_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	
	p->name = g_strdup("Battery Voltage");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->vbat;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Water Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->water_temp;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Device Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->fet_temp;
	list = g_list_append(list, p);
	
	return list;
}

//*////////////////////////////////////////////////////////////////////////
// MFA 
///////////////////////////////////////////////////////////////////////////

uint8_t serialize_mfa_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(tx_mfa_t)){
		dbg(lvl_debug,"size: %i, struct: %i\n",size,sizeof(tx_mfa_t));
		return 0;
	}
	char str[2560] = {0,};
	tx_mfa_t* tx = (tx_mfa_t*) tx_data;
	dbg(lvl_debug,"\nserialize_mfa_txdata:%i\n%s\n%i\n%i\n%i\n%i\n",size,tx->radio_text, tx->navigation_next_turn, tx->cal_water_temperature, tx->cal_voltage, tx->cal_oil_temperature, tx->cal_consumption);
	uint8_t i;
	for(i=0;i<32;i++){
		buffer[i] = tx->radio_text[i];
	}
	buffer[32] = tx->navigation_next_turn;//or status
	buffer[33] = (uint8_t) ((tx->distance_to_next_turn & 0xFF000000) >> 24);
	buffer[34] = (uint8_t) ((tx->distance_to_next_turn & 0x00FF0000) >> 16);
	buffer[35] = (uint8_t) ((tx->distance_to_next_turn & 0x0000FF00) >> 8);
	buffer[36] = (uint8_t) ((tx->distance_to_next_turn & 0x000000FF));
	//navigation active?
	buffer[37] = tx->cal_water_temperature;
	buffer[38] = tx->cal_voltage;
	buffer[39] = tx->cal_oil_temperature;
	buffer[40] = tx->cal_consumption;	
	sprintf(str,"mfa:");
	for(i=0;i<size; i++){
		char buf[6] = {0,};
		sprintf(buf, " 0x%02X", buffer[i]);
		strcat(str, buf);
	}
	dbg(lvl_debug,"%s\n", str);
	return 1;
}

uint8_t serialize_mfa_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_mfa_t)){
		return 0;
	}
	rx_mfa_t* rx = (rx_mfa_t*) rx_data;
	dbg(lvl_debug,"\nserialize_mfa_rxdata:%i\n%s\n%i\n%i\n%i\n%i\n",size,rx->radio_text, rx->navigation_next_turn, rx->cal_water_temperature, rx->cal_voltage, rx->cal_oil_temperature, rx->cal_consumption);
	char str[2560] = {0,};
	uint8_t i;
	for(i=0;i<AUDIO_STR_LENGTH;i++){
		buffer[i] = rx->radio_text[i];
	}
	buffer[AUDIO_STR_LENGTH] = rx->navigation_next_turn;//or status
	buffer[AUDIO_STR_LENGTH + 1] = (uint8_t) ((rx->distance_to_next_turn & 0xFF000000) >> 24);
	buffer[AUDIO_STR_LENGTH + 2] = (uint8_t) ((rx->distance_to_next_turn & 0x00FF0000) >> 16);
	buffer[AUDIO_STR_LENGTH + 3] = (uint8_t) ((rx->distance_to_next_turn & 0x0000FF00) >> 8);
	buffer[AUDIO_STR_LENGTH + 4] = (uint8_t) ((rx->distance_to_next_turn & 0x000000FF));
	//navigation active?
	buffer[AUDIO_STR_LENGTH + 5] = rx->cal_water_temperature;
	buffer[AUDIO_STR_LENGTH + 6] = rx->cal_voltage;
	buffer[AUDIO_STR_LENGTH + 7] = rx->cal_oil_temperature;
	buffer[AUDIO_STR_LENGTH + 8] = rx->cal_consumption;
	// read only
	buffer[AUDIO_STR_LENGTH + 9] = (uint8_t) ((rx->voltage & 0xFF00) >> 8);
	buffer[AUDIO_STR_LENGTH + 10] = (uint8_t) ((rx->voltage & 0x00FF));
	buffer[AUDIO_STR_LENGTH + 11] = rx->water_temperature;
	buffer[AUDIO_STR_LENGTH + 12] = rx->ambient_temperature;
	buffer[AUDIO_STR_LENGTH + 13] = rx->oil_temperature;
	buffer[AUDIO_STR_LENGTH + 14] = (uint8_t) ((rx->consumption & 0xFF00) >> 8);
	buffer[AUDIO_STR_LENGTH + 15] = (uint8_t) ((rx->consumption & 0x00FF));
	buffer[AUDIO_STR_LENGTH + 16] = (uint8_t) ((rx->average_consumption & 0xFF00) >> 8);
	buffer[AUDIO_STR_LENGTH + 17] = (uint8_t) ((rx->average_consumption & 0x00FF));
	buffer[AUDIO_STR_LENGTH + 18] = (uint8_t) ((rx->range & 0xFF00) >> 8);
	buffer[AUDIO_STR_LENGTH + 19] = (uint8_t) ((rx->range & 0x00FF));
	buffer[AUDIO_STR_LENGTH + 20] = (uint8_t) ((rx->speed & 0xFF00) >> 8);
	buffer[AUDIO_STR_LENGTH + 21] = (uint8_t) ((rx->speed & 0x00FF));
	buffer[AUDIO_STR_LENGTH + 22] = (uint8_t) ((rx->average_speed & 0xFF00) >> 8);
	buffer[AUDIO_STR_LENGTH + 23] = (uint8_t) ((rx->average_speed & 0x00FF));
	buffer[AUDIO_STR_LENGTH + 24] = (uint8_t) ((rx->rpm & 0xFF00) >> 8);
	buffer[AUDIO_STR_LENGTH + 25] = (uint8_t) ((rx->rpm & 0x00FF));
	
	sprintf(str,"mfa:");
	for(i=0;i<size; i++){
		char buf[6] = {0,};
		sprintf(buf, " 0x%02X", buffer[i]);
		strcat(str, buf);
	}
	dbg(lvl_debug,"%s\n", str);
	return 1;
}

uint8_t deserialize_mfa_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_mfa_t)){
		return 0;
	}
	rx_mfa_t* rx = (rx_mfa_t*) rx_data;
	uint8_t i;
	char str[2560] = {0,};
	dbg(lvl_debug,"\ndeserialize_mfa_rxdata:%i\n", size);
	sprintf(str,"mfa:");
	for(i=0;i<size; i++){
		char buf[6] = {0,};
		sprintf(buf, " 0x%02X", buffer[i]);
		strcat(str, buf);
	}
	dbg(lvl_debug,"%s\n", str);
	
	for(i=0;i<AUDIO_STR_LENGTH;i++){
		rx->radio_text[i] = buffer[i];
	}
	
	rx->navigation_next_turn = buffer[AUDIO_STR_LENGTH];
	rx->distance_to_next_turn = ((long) buffer[AUDIO_STR_LENGTH + 1] << 24) 
		+ ((long) buffer[AUDIO_STR_LENGTH + 2] << 16) 
		+ ((long) buffer[AUDIO_STR_LENGTH + 3] << 8) 
		+ buffer[AUDIO_STR_LENGTH + 4];
	//navigation active?
	rx->cal_water_temperature = buffer[AUDIO_STR_LENGTH + 5];
	rx->cal_voltage = buffer[AUDIO_STR_LENGTH + 6];
	rx->cal_oil_temperature = buffer[AUDIO_STR_LENGTH + 7];
	rx->cal_consumption = buffer[AUDIO_STR_LENGTH + 8];
	// read only
	rx->voltage = ((uint16_t) buffer[AUDIO_STR_LENGTH + 9] << 8) + buffer[AUDIO_STR_LENGTH + 10];
	rx->water_temperature = buffer[AUDIO_STR_LENGTH + 11];
	rx->ambient_temperature = buffer[AUDIO_STR_LENGTH + 12];
	rx->oil_temperature = buffer[AUDIO_STR_LENGTH + 13];
	rx->consumption = ((uint16_t) buffer[AUDIO_STR_LENGTH + 14] << 8) + buffer[AUDIO_STR_LENGTH + 15];
	rx->average_consumption = ((uint16_t) buffer[AUDIO_STR_LENGTH + 16] << 8) + buffer[AUDIO_STR_LENGTH + 17];
	rx->range = ((uint16_t) buffer[AUDIO_STR_LENGTH + 18] << 8) + buffer[AUDIO_STR_LENGTH + 19];
	rx->speed = ((uint16_t) buffer[AUDIO_STR_LENGTH + 20] << 8) + buffer[AUDIO_STR_LENGTH + 21];
	rx->average_speed = ((uint16_t) buffer[22] << 8) + buffer[AUDIO_STR_LENGTH + 23];
	rx->rpm = (uint16_t) (buffer[AUDIO_STR_LENGTH + 24] << 8) + buffer[AUDIO_STR_LENGTH + 25];
	return 1;
}
GList* init_mfa_properties(void *rx_data, void* tx_data, struct service_property *parent){
	GList* list = NULL;
	rx_mfa_t* rx = (rx_mfa_t*) rx_data;
	rx_mfa_t* tx = (rx_mfa_t*) tx_data;
	struct service_property *p = g_new0(struct service_property,1);
	
	
	if(rx->cal_water_temperature == tx->cal_water_temperature){
		p->name = g_strdup("Water Temperature Calibration");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->cal_water_temperature;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_voltage == tx->cal_voltage){
		p->name = g_strdup("Voltage Calibration");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->cal_voltage;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_oil_temperature == tx->cal_oil_temperature){
		p->name = g_strdup("Oil Temperature Calibration");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->cal_oil_temperature;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_consumption == tx->cal_consumption){
		p->name = g_strdup("Consumption Calibration");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->cal_consumption;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	
	p->name = g_strdup("Battery Voltage");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->voltage;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Water Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->water_temperature;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Ambient Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->ambient_temperature;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Oil Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->oil_temperature;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Ambient Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->ambient_temperature;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Radio Text");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) g_strdup(rx->radio_text);
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Consumption");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->consumption;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Average Consumption");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->average_consumption;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Speed");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->speed;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Average Speed");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->average_speed;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Speed");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->speed;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
		
	p->name = g_strdup("RPM");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->rpm;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
			
	p->name = g_strdup("Range");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->range;
	list = g_list_append(list, p);

	return list;
}

//*////////////////////////////////////////////////////////////////////////
// LSG 
///////////////////////////////////////////////////////////////////////////
//*
uint8_t serialize_lsg_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(tx_lsg_t)){
		dbg(lvl_debug,"size: %i, struct: %i\n",size,sizeof(tx_lsg_t));
		return 0;
	}
	tx_lsg_t* tx = (tx_lsg_t*) tx_data;
	dbg(lvl_debug,"serialize_lsg_txdata %p\n", tx);
	buffer[0] =  tx->AL;
	buffer[1] =  tx->TFL;
	buffer[2] =  tx->ZV;
	buffer[3] =  tx->LED;
	buffer[4] =  tx->time_in;
	buffer[5] =  tx->time_out;
	//dbg(lvl_debug,"lsg: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	//dbg(lvl_debug, "AL: %i\nTFL: %i\nZV: %i\nLED: %i\nIn: %i\nout: %i\n\n", tx->AL, tx->TFL, tx->ZV, tx->LED, tx->time_in, tx->time_out );
	return 1;
}

uint8_t serialize_lsg_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_lsg_t)){
		dbg(lvl_debug,"size: %i, struct: %i\n",size,sizeof(rx_lsg_t));
		return 0;
	}
	rx_lsg_t* rx = (rx_lsg_t*) rx_data;
	dbg(lvl_debug,"serialize_lsg_rxdata %p\n", rx);
	buffer[0] =  rx->AL;
	buffer[1] =  rx->TFL;
	buffer[2] =  rx->ZV;
	buffer[3] =  rx->LED;
	buffer[4] =  rx->time_in;
	buffer[5] =  rx->time_out;
	//dbg(lvl_debug,"lsg: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	//dbg(lvl_debug, "AL: %i\nTFL: %i\nZV: %i\nLED: %i\nIn: %i\nout: %i\n\n", rx->AL, rx->TFL, rx->ZV, rx->LED, rx->time_in, rx->time_out );
	return 1;
}

uint8_t deserialize_lsg_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_lsg_t)){
		dbg(lvl_debug,"size: %i, struct: %i\n",size,sizeof(tx_lsg_t));
		return 0;
	}
	rx_lsg_t* rx = (rx_lsg_t*) rx_data;
	dbg(lvl_debug,"deserialize_lsg_rxdata %p\n", rx);
	rx->AL = buffer[0];
	rx->TFL = buffer[1];
	rx->ZV = buffer[2];
	rx->LED = buffer[3];
	rx->time_in = buffer[4];
	rx->time_out = buffer[5];
	dbg(lvl_debug,"****LSG:****\n 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	dbg(lvl_debug, "AL: %i\nTFL: %i\nZV: %i\nLED: %i\nIn: %i\nout: %i\n\n", rx->AL, rx->TFL, rx->ZV, rx->LED, rx->time_in, rx->time_out );
	return 1;
}

GList* init_lsg_properties(void *rx_data, void* tx_data, struct service_property *parent){
	GList* list = NULL;
	rx_lsg_t* rx = (rx_lsg_t*) rx_data;
	rx_lsg_t* tx = (rx_lsg_t*) tx_data;
	struct service_property *p = g_new0(struct service_property,1);
	if(rx->AL == tx->AL){
		p->name = g_strdup("Automatic Light");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->AL;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}

	if(rx->TFL == tx->TFL){
		p->name = g_strdup("Daytime Running Light");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->TFL;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->ZV == tx->ZV){
		p->name = g_strdup("Central Locking");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->ZV;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->LED == tx->LED){
		p->name = g_strdup("LED Setting");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->LED;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->time_in == tx->time_in){
		p->name = g_strdup("Delay Time Inside");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->time_in;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->time_out == tx->time_out){
		p->name = g_strdup("Delay Time Outside");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->time_out;
		list = g_list_append(list, p);
	}
	return list;
}


//*////////////////////////////////////////////////////////////////////////
// WFS
///////////////////////////////////////////////////////////////////////////

uint8_t serialize_wfs_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(tx_wfs_t)){
		dbg(lvl_debug,"size: %i, struct: %i\n",size,sizeof(tx_wfs_t));
		return 0;
	}
	tx_wfs_t* tx = (tx_wfs_t*) tx_data;
	dbg(lvl_debug,"\nserialize_wfs_txdata %p\n", tx);
	//dbg(lvl_debug,"wfs: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	return 1;
}

uint8_t serialize_wfs_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_wfs_t)){
		return 0;
	}
	rx_wfs_t* rx = (rx_wfs_t*) rx_data;
	dbg(lvl_debug,"\nserialize_wfs_rxdata %p\n", rx);
	//dbg(lvl_debug,"wfs: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	
	return 1;
}

uint8_t deserialize_wfs_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_wfs_t)){
		return 0;
	}
	rx_wfs_t* rx = (rx_wfs_t*) rx_data;
	dbg(lvl_debug,"\ndeserialize_wfs_rxdata%p\n", rx);
	//dbg(lvl_debug,"wfs: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	return 1;
}

GList* init_wfs_properties(void *rx_data, void* tx_data, struct service_property *parent){
	GList* list = NULL;
/*	
	rx_wfs_t* rx = (rx_wfs_t*) rx_data;
	rx_wfs_t* tx = (rx_wfs_t*) tx_data;
	struct service_property *p = g_new0(struct service_property,1);
	if(rx->wfs_freq == tx->wfs_freq){
		p->name = g_strdup("Frequency");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->wfs_freq;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}

	if(rx->cal_voltage == tx->cal_voltage){
		p->name = g_strdup("Calibration Voltage");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->cal_voltage;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_temperature == tx->cal_temperature){
		p->name = g_strdup("Calibration Temperature");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->cal_temperature;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->water_value == tx->water_value){
		p->name = g_strdup("Enable Temperature");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->water_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->time_value == tx->time_value){
		p->name = g_strdup("Enable Time");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->time_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->v == tx->time_value){
		p->name = g_strdup("Enable Time");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->time_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	
	p->name = g_strdup("Battery Voltage");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->vbat;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Water Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->water_temp;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Device Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->fet_temp;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
//*/
	return list;
}


//*////////////////////////////////////////////////////////////////////////
// V2V 
///////////////////////////////////////////////////////////////////////////
//*
uint8_t serialize_v2v_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(tx_v2v_t)){
		dbg(lvl_debug,"size: %i, struct: %i\n",size,sizeof(tx_v2v_t));
		return 0;
	}
	tx_v2v_t* tx = (tx_v2v_t*) tx_data;
	dbg(lvl_info,"\nserialize_v2v_txdata\n");
	dbg(lvl_info,"v2v:\nfreq: %i\ntemp: %i\nvtg: %i\nwatertemp_sh: %i\ntime_sh: %i\n",tx->pwm_freq, tx->cal_temperature, tx->cal_voltage, tx->water_value, tx->time_value);
		buffer[0] = (uint8_t) ((tx->pwm_freq & 0xFF00) >> 8);
	buffer[1] = (uint8_t) (tx->pwm_freq & 0x00FF);
	buffer[2] = tx->cal_temperature;
	buffer[3] = tx->cal_voltage;
	buffer[4] = tx->water_value;
	buffer[5] = tx->time_value;
	dbg(lvl_info,"v2v: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	return 1;
}

uint8_t serialize_v2v_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_v2v_t)){
		return 0;
	}
	rx_v2v_t* rx = (rx_v2v_t*) rx_data;
	dbg(lvl_info,"\nserialize_v2v_rxdata\n");
	dbg(lvl_info,"v2v:\nfreq: %i\ntemp: %i\nvtg: %i\nwatertemp_sh: %i\ntime_sh: %i\nvbat: %i\nwatertemp: %i\nfettemp:%i\n",rx->pwm_freq, rx->cal_temperature, rx->cal_voltage, rx->water_value, rx->time_value, rx->vbat, rx->water_temp, rx->fet_temp);
	buffer[0] = (uint8_t) ((rx->pwm_freq & 0xFF00) >> 8);
	buffer[1] = (uint8_t) (rx->pwm_freq & 0x00FF);
	buffer[2] = rx->cal_temperature;
	buffer[3] = rx->cal_voltage;
	buffer[4] = rx->water_value;
	buffer[5] = rx->time_value;
	buffer[6] = (uint8_t) ((rx->vbat & 0xFF00) >> 8);
	buffer[7] = (uint8_t) (rx->vbat & 0x00FF);
	buffer[8] = rx->water_temp;
	buffer[9] = rx->fet_temp;
	dbg(lvl_info,"v2v: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	
	return 1;
}

uint8_t deserialize_v2v_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_v2v_t)){
		return 0;
	}
	rx_v2v_t* rx = (rx_v2v_t*) rx_data;
	dbg(lvl_info,"\ndeserialize_v2v_rxdata\n");
	dbg(lvl_info,"v2v: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	rx->pwm_freq = ((uint16_t) (buffer[0]) << 8) + buffer[1];
	rx->cal_temperature = buffer[2];
	rx->cal_voltage = buffer[3];
	rx->water_value = buffer[4];
	rx->time_value = buffer[5];
	rx->vbat = ((uint16_t) (buffer[6]) << 8) + buffer[7];
	rx->water_temp = buffer[8];
	rx->fet_temp = buffer[9];
	dbg(lvl_info,"v2v:\nfreq: %i\ntemp: %i\nvtg: %i\nwatertemp_sh: %i\ntime_sh: %i\nvbat: %i\nwatertemp: %i\nfettemp:%i\n",rx->pwm_freq, rx->cal_temperature, rx->cal_voltage, rx->water_value, rx->time_value, rx->vbat, rx->water_temp, rx->fet_temp);
	return 1;
}

GList* init_v2v_properties(void *rx_data, void* tx_data, struct service_property *parent){
	GList* list = NULL;
	rx_v2v_t* rx = (rx_v2v_t*) rx_data;
	rx_v2v_t* tx = (rx_v2v_t*) tx_data;
//*	
	struct service_property *p = g_new0(struct service_property,1);
	if(rx->pwm_freq == tx->pwm_freq){
		p->name = g_strdup("Frequency");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->pwm_freq;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}

	if(rx->cal_voltage == tx->cal_voltage){
		p->name = g_strdup("Calibration Voltage");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->cal_voltage;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_temperature == tx->cal_temperature){
		p->name = g_strdup("Calibration Temperature");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->cal_temperature;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->water_value == tx->water_value){
		p->name = g_strdup("Enable Temperature");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->water_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->time_value == tx->time_value){
		p->name = g_strdup("Enable Time");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) rx->time_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	
	p->name = g_strdup("Battery Voltage");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->vbat;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Water Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->water_temp;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Device Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) rx->fet_temp;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	return list;
//*/	
}

/*
uint8_t serialize_v2v_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(tx_v2v_t)){
		dbg(lvl_debug,"size: %i, struct: %i\n",size,sizeof(tx_v2v_t));
		return 0;
	}tx_v2v_t* tx = (tx_v2v_t*) tx_data;
	dbg(lvl_debug,"\nserialize_v2v_txdata %p\n", tx);
	//dbg(lvl_debug,"v2v: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	return 1;
}

uint8_t serialize_v2v_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_v2v_t)){
		return 0;
	}
	rx_v2v_t* rx = (rx_v2v_t*) rx_data;
	dbg(lvl_debug,"\nserialize_v2v_rxdata %p\n", rx);
	//dbg(lvl_debug,"v2v: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	
	return 1;
}

uint8_t deserialize_v2v_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_v2v_t)){
		return 0;
	}
	rx_v2v_t* rx = (rx_v2v_t*) rx_data;
	dbg(lvl_debug,"\ndeserialize_v2v_rxdata %p\n", rx);
	
	dbg(lvl_debug,"PWM: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	rx->pwm_freq = ((uint16_t) (buffer[0]) << 8) + buffer[1];
	rx->cal_temperature = buffer[2];
	rx->cal_voltage = buffer[3];
	rx->water_value = buffer[4];
	rx->time_value = buffer[5];
	rx->vbat = ((uint16_t) (buffer[6]) << 8) + buffer[7];
	rx->water_temp = buffer[8];
	rx->fet_temp = buffer[9];
	dbg(lvl_debug,"****V2V:****\nfreq: %i\ntemp: %i\nvtg: %i\nwatertemp_sh: %i\ntime_sh: %i\nvbat: %i\nwatertemp: %i\nfettemp:%i\n",rx->pwm_freq, rx->cal_temperature, rx->cal_voltage, rx->water_value, rx->time_value, rx->vbat, rx->water_temp, rx->fet_temp);
	return 1;
	//dbg(lvl_debug,"v2v: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	return 1;
}
//*/
//*////////////////////////////////////////////////////////////////////////
// GENERIC I2C FUNCTIONS 
///////////////////////////////////////////////////////////////////////////

uint8_t rx_task(int device, struct connected_devices* cd){
	uint8_t i;
	uint8_t rx_size = cd->rx_size;
	uint8_t i2crxdata[rx_size+1];
	if(!device) return 1;
	dbg(lvl_debug,"Read i2c\n");
	dbg(lvl_debug,"rx_t: %i, \n", rx_size);
	
	read_i2c_frame(device, i2crxdata, rx_size+1);
	
	uint8_t rx_crc = calculateCRC8(CRC_POLYNOME, i2crxdata, rx_size);
	dbg(lvl_debug,"// check rx crc: 0x%02X 0x%02X\n",rx_crc,i2crxdata[rx_size]);
	if(rx_crc == i2crxdata[rx_size]){
		dbg(lvl_debug,"//crc is correct\n");
		uint8_t ser_rx[rx_size];
		
		if(cd->serialize_rx(cd->rx_data, rx_size, ser_rx)){
			dbg(lvl_debug,"// check if new data differs from current data object\n");
			uint8_t ok = 0;
			for(i=0; i<rx_size; i++){
				dbg(lvl_debug,"check i2crxdata[%i] 0x%02X != ser_rx[%i] 0x%02X\n",i,i2crxdata[i],i,ser_rx[i]);
				if(i2crxdata[i] != ser_rx[i]) ok = 1;
			}
			if(ok){
				dbg(lvl_debug,"//we got new data -> replace the old object \n");
				if(!cd->deserialize_rx(cd->rx_data, rx_size, i2crxdata)){
					dbg(lvl_debug,"failed to replace\n");
					return 1;
				}else{
					dbg(lvl_debug,"// everything went fine -> clean the buffer\n");
					for(i=0; i <= rx_size; i++){
						i2crxdata[i] = 0;
					}
				}
			}else return 1;
		}else return 1;
	}else return 1;
	dbg(lvl_debug,"// end of rx data procession\n");
	return 0;
	
}	
uint8_t tx_task(int device, struct connected_devices *cd){
	uint8_t i;
	uint8_t tx_size = cd->tx_size;	
	uint8_t i2ctxdata[tx_size+1];
	if(!device) return 1;
	dbg(lvl_debug,"tx_t: %i\n", tx_size);	
	//dbg(lvl_debug,"// serialize tx object %i\n", tx_size);
	if(cd->serialize_tx(cd->tx_data, tx_size, i2ctxdata)){
		dbg(lvl_debug,"//calculate CRC and append to i2ctxdata\n");
		i2ctxdata[tx_size] = calculateCRC8(CRC_POLYNOME, i2ctxdata, tx_size);
		for(i=0;i<tx_size;i++){
			i2c_smbus_write_byte_data(device, i, i2ctxdata[i]);
			dbg(lvl_debug,"Write %i: 0x%02X\n", i, i2ctxdata[i]);
		}
		i2c_smbus_write_byte_data(device, i,i2ctxdata[tx_size]); 
		dbg(lvl_debug,"Write CRC: 0x%02X\n", i2ctxdata[tx_size]);
	}else return 1;
	return 0;
}

void process_i2c_data(struct service_priv* this, struct connected_devices* cd){
	uint8_t port = cd->addr;
	dbg(lvl_info,"Process data on addr: 0x%02X\n",port);
	if(port == calculateID("MFA")){
		
		struct i2c_nav_data* navigation_data = get_navigation_data(this);
		get_audio_data(this, tx_mfa->radio_text);
		tx_mfa->distance_to_next_turn = navigation_data->distance_to_next_turn;
		tx_mfa->navigation_next_turn = navigation_data->nav_status && (navigation_data->nav_status << 4);
		dbg(lvl_info,"Send NAV data: %i, %i\n", tx_mfa->distance_to_next_turn,tx_mfa->navigation_next_turn);
	}else if(port == calculateID("LSG")){
		tx_lsg->AL = 1;
		tx_lsg->TFL = 0;
		tx_lsg->ZV = 0;
		tx_lsg->time_in++;
		if(tx_lsg->time_in > 160) tx_lsg->time_in = 0;
		tx_lsg->time_out--;
		if(tx_lsg->time_out == 0) tx_lsg->time_out = 200;
	}else if(port == calculateID("WFS")){
		
	}else if(port == calculateID("PWM")){

		tx_pwm->pwm_freq += 500;
		if(tx_pwm->pwm_freq > 30000) tx_pwm->pwm_freq = 1000;
	}else if(port == calculateID("V2V")){
		tx_v2v->pwm_freq += 50;
		if(tx_v2v->pwm_freq > 1000) tx_v2v->pwm_freq = 100;
	}else{
		dbg(lvl_error, "Invalid i2c data\n");
	}
}


//*////////////////////////////////////////////////////////////////////////
// TEST 
///////////////////////////////////////////////////////////////////////////
void read_i2c_frame(int device, uint8_t* data, uint8_t size){
	uint8_t i;
	if(device){
		dbg(lvl_debug,"Read %i bytes\n", size);
		for(i=0;i<size;i++){	
			if(i==0){
				data[i] = i2c_smbus_read_byte_data(device, i);
			}else{
				data[i] = i2c_smbus_read_byte(device);
			}
			dbg(lvl_debug,"Read %i: 0x%02X\n", i, (uint8_t) data[i]);
		}
}
}

///////////////////////////////////////////////////////////////////////////
// MAIN
///////////////////////////////////////////////////////////////////////////

static void 
i2c_task(struct service_priv *this){
	this->connected_devices = g_list_first(this->connected_devices);
	int num_devices = g_list_length(this->connected_devices);
	dbg(lvl_info, "%i connected devices\n", num_devices);
	if(this->device){
		while(num_devices--){
			if(this->connected_devices->data){
				struct connected_devices* cd = this->connected_devices->data;
				if(0==select_slave(this->device, cd->addr)){
					dbg(lvl_info, "\nstart PROCESS DATA: 0x%02X, %s\n\n", cd->addr, cd->name);
					rx_task(this->device, cd);
					tx_task(this->device, cd);
					process_i2c_data(this, cd);
					dbg(lvl_info, "\nend PROCESS DATA: 0x%02X, %s\n\n", cd->addr, cd->name);
				}
				if(this->connected_devices->next)
					this->connected_devices = this->connected_devices->next;
				else
					break;
			}
		}
		
	
		
	}
}

GList* i2c_get_properties(struct service_priv *priv){
	dbg(lvl_error,"Found Properties %p\n", priv->properties);
	return priv->properties;
}

struct service_property* i2c_set_property(struct service_priv *priv, struct service_property* sp){
	return NULL;
}



void
i2c_get_plugin(struct service_priv* p){
	p = &i2c_plugin;
	return 1;
}
/*
static struct service_priv *
i2c_new(struct navit *nav, struct service_methods *meth,
        struct attr **attrs)
{
	struct attr callback; 
	i2c_plugin = g_new0(struct service_priv, 1);
	dbg(lvl_error, "i2cplugin: %p\n", i2c_plugin);
	callback.type=attr_callback;
	callback.u.callback=callback_new_attr_1(callback_cast(i2c_init),attr_navit,i2c_plugin);
	config_add_attr(config, &callback);
	dbg(lvl_info,"hello i2c\n\n");
    navit_add_callback(nav, callback_new_attr_1(callback_cast(i2c_init), attr_graphics_ready, i2c_plugin));
    return (struct osd_priv *) i2c_plugin;
}
*/
/**
 * @brief	The plugin entry point
 *
 * @return	nothing
 *
 * The plugin entry point
 *
 * /
void
plugin_init(void)
{

	struct attr callback; 
	i2c_plugin = g_new0(struct service_priv, 1);
	dbg(lvl_error, "i2cplugin: %p\n", i2c_plugin);
	callback.type=attr_callback;
	callback.u.callback=callback_new_attr_1(callback_cast(i2c_init),attr_navit,i2c_plugin);
	config_add_attr(config, &callback);
	dbg(lvl_info,"hello i2c\n\n");
	
}
//*/




GList* init_properties(struct service_priv *this){
	GList *prop;

	this->connected_devices = g_list_first(this->connected_devices);
	int num_devices = g_list_length(this->connected_devices);
	dbg(lvl_error, "%i connected devices\n", num_devices);
	if(this->device){
		while(num_devices--){
			if(this->connected_devices->data){
				struct connected_devices* cd = this->connected_devices->data;
				struct service_property *sp = g_new0(struct service_property,1);
				dbg(lvl_error, "init device %s\n", cd->name);
				sp->name = cd->name;
				sp->parent = NULL;
				sp->ro = 0;
				sp->num_children = cd->num_properties;
				dbg(lvl_error, "parent = %p, ro = %i, num_children = %i\n", sp->parent, sp->ro, sp->num_children);
				uint8_t cnt = cd->num_properties;
				while(cnt--){
					sp->children = cd->init_properties(cd->rx_data, cd->tx_data, sp);
				}
				prop = g_list_append(prop, sp);
				if(this->connected_devices->next)
					this->connected_devices = this->connected_devices->next;
				else
					break;
			}
		}

	}
	return prop;
}


static struct service_priv *
i2c_service_new(struct service_methods *
						 meth,
						 struct callback_list *cbl,
						 struct attr ** attrs) 
{
	//*
    struct service_priv *i2c_plugin = g_new0 (struct service_priv, 1);
    struct attr *attr;
    if ((attr = attr_search (attrs, NULL, attr_source)))
      {
          i2c_plugin->source = g_strdup(attr->u.str);
          //dbg (lvl_info, "found source attr: %s\n", attr->u.str);
      }
   
    dbg (lvl_debug,  "Callback created successfully\n");
	i2c_plugin->attrs=attrs;
	//i2c_plugin->nav=nav;
	/* Init i2c Bus & local data 
	*/ 
	i2c_plugin->navigation_data = g_new0(struct i2c_nav_data, 1);
	
	dbg(lvl_debug,"I2C Test\n");

	i2c_plugin->device = open_i2c(i2c_plugin->source);
	check_ioctl(i2c_plugin->device);
	if(init_i2c_devices(i2c_plugin)){
		i2c_plugin->task = callback_new_1 (callback_cast (i2c_task), i2c_plugin);
		i2c_plugin->timeout = event_add_timeout(1000, 1,  i2c_plugin->task);
		i2c_plugin->callback = callback_new_1 (callback_cast (i2c_task), i2c_plugin);
		i2c_plugin->timeout = event_add_timeout(10000, 1,  i2c_plugin->callback);
		dbg (lvl_debug,  "Callbacks created successfully\n");
	}else{
		dbg(lvl_error, "No I2C Devices found\n");
	}
	i2c_plugin->name = g_strdup("I2C Service");
	i2c_plugin->properties = init_properties(i2c_plugin);
	
	i2c_plugin->cbl = cbl;
	dbg(lvl_error, "Methods: %p\n",meth);
    *meth=i2c_service_meth;
    dbg(lvl_error, "Methods: Plugin: %p, Get: %p, Set: %p\n",meth->plugin, meth->get_attr, meth->set_attr);
   
	
    return i2c_plugin;
    //*/
}



/**
* @brief plugin entry point
*/
void
plugin_init(void)
{ 
    dbg(lvl_info, "registering service category i2c-service\n");
    plugin_register_service_category("i2c_service", i2c_service_new);
}
