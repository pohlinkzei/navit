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

#include "i2c.h"


struct navigation_itm;
struct navigation;

uint8_t deserialize_pwm_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_pwm_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_pwm_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);

uint8_t deserialize_mfa_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_mfa_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_mfa_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);

uint8_t deserialize_wfs_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_wfs_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_wfs_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);

uint8_t deserialize_v2v_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_v2v_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_v2v_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);

uint8_t deserialize_lsg_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_lsg_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_lsg_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);

struct i2c_nav_data* get_navigation_data(struct i2c *this);
void get_audio_data(struct i2c *this, uint8_t audio_str[AUDIO_STR_LENGTH]);

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
	dbg(lvl_debug,"crc: %p, %i\n",data, crc);
	while(len-->0){
		//dbg(lvl_debug,"crc: %p, %i\n",data, crc);
		crc = crc8(crc, *data++);
	}
	dbg(lvl_debug,"\ncrc: %i\n",crc);
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

int init_i2c_devices(struct i2c *this){
	int port, res;
	this->connected_devices = NULL;
	if(this->device){
		for(port = 0; port < 127; port++){
			if(ioctl(this->device, I2C_SLAVE, port) < 0){
				dbg(lvl_debug,"Error: No I2C_SLAVE found!\n");
			}else{
				res = i2c_smbus_read_byte(this->device);
				if (res >= 0){
					dbg(lvl_debug,"I2C device found at 0x%02x, val = 0x%02x\n",port, res);
					struct connected_devices *cd = g_new0(struct connected_devices, 1);
					cd->addr = port;
					if(port == calculateID("MFA")){
						
						rx_mfa = (rx_mfa_t*) malloc(sizeof(rx_mfa_t)); 
						tx_mfa = (tx_mfa_t*) malloc(sizeof(tx_mfa_t));
						cd->name = g_strdup("MFA");
						cd->icon = "gui_active";
						cd->rx_data = rx_mfa;
						cd->tx_data = tx_mfa;
						cd->serialize_rx = serialize_mfa_rxdata;
						cd->serialize_tx = serialize_mfa_txdata;
						cd->deserialize_rx = deserialize_mfa_rxdata;
						cd->rx_size = sizeof(rx_mfa_t);
						cd->tx_size = sizeof(tx_mfa_t);
					}else if(port == calculateID("LSG")){
						rx_lsg = (rx_lsg_t*) malloc(sizeof(rx_lsg_t)); 
						tx_lsg = (tx_lsg_t*) malloc(sizeof(tx_lsg_t)); 
						cd->name = g_strdup("LSG");
						cd->icon = "gui_active";
						cd->rx_data = rx_lsg;
						cd->tx_data = tx_lsg;
						cd->serialize_rx = serialize_lsg_rxdata;
						cd->serialize_tx = serialize_lsg_txdata;
						cd->deserialize_rx = deserialize_lsg_rxdata;
						cd->rx_size = sizeof(rx_lsg_t);
						cd->tx_size = sizeof(tx_lsg_t);
					}else if(port == calculateID("WFS")){
						rx_wfs = (rx_wfs_t*) malloc(sizeof(rx_wfs_t)); 
						tx_wfs = (tx_wfs_t*) malloc(sizeof(tx_wfs_t)); 
						cd->name = g_strdup("WFS");
						cd->icon = "gui_active";
						cd->rx_data = rx_wfs;
						cd->tx_data = tx_wfs;
						cd->serialize_rx = serialize_wfs_rxdata;
						cd->serialize_tx = serialize_wfs_txdata;
						cd->deserialize_rx = deserialize_wfs_rxdata;
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
						cd->serialize_rx = serialize_pwm_rxdata;
						cd->serialize_tx = serialize_pwm_txdata;
						cd->deserialize_rx = deserialize_pwm_rxdata;
						cd->rx_size = sizeof(rx_pwm_t);
						cd->tx_size = sizeof(tx_pwm_t);
					}else if(port == calculateID("V2V")){
						rx_v2v = (rx_v2v_t*) malloc(sizeof(rx_v2v_t)); 
						tx_v2v = (tx_v2v_t*) malloc(sizeof(tx_v2v_t)); 
						cd->name = g_strdup("V2V");
						cd->icon = "gui_active";
						cd->rx_data = rx_v2v;
						cd->tx_data = tx_v2v;
						cd->serialize_rx = serialize_v2v_rxdata;
						cd->serialize_tx = serialize_v2v_txdata;
						cd->deserialize_rx = deserialize_v2v_rxdata;
						cd->rx_size = sizeof(rx_v2v_t);
						cd->tx_size = sizeof(tx_v2v_t);
					}else{
						cd->name = g_strdup("I2C");
						cd->icon = "gui_inactive";
						return 0;
					}
					this->connected_devices = g_list_append(this->connected_devices, cd);
				}
			}
		}
	}
	return 1;
}

void get_audio_data(struct i2c* this, uint8_t audio_str[AUDIO_STR_LENGTH]){
	int i = 0;
	gchar str[256] = {0,};
#ifdef USE_AUDIO_FRAMEWORK 
	strcpy(str, audio_get_current_track(this->nav));
	strcat(str, " - ");
	strcat(str, audio_get_current_playlist(this->nav));
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
get_navigation_data(struct i2c* this){
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

//*////////////////////////////////////////////////////////////////////////
// MFA 
///////////////////////////////////////////////////////////////////////////

uint8_t serialize_mfa_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(tx_mfa_t)){
		dbg(lvl_debug,"size: %i, struct: %i\n",size,sizeof(tx_mfa_t));
		return 0;
	}
	tx_mfa_t* tx = (tx_mfa_t*) tx_data;
	dbg(lvl_debug,"\nserialize_mfa_txdata\n");
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
	dbg(lvl_debug,"mfa: ");
	for(i=0;i<size; i++){
		dbg(lvl_debug,"0x%02X ",buffer[i]);
	}
	dbg(lvl_debug,"\n");
	return 1;
}

uint8_t serialize_mfa_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_mfa_t)){
		return 0;
	}
	rx_mfa_t* rx = (rx_mfa_t*) rx_data;
	dbg(lvl_debug,"\nserialize_mfa_rxdata\n");
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
	
	dbg(lvl_debug,"mfa: ");
	for(i=0;i<size; i++){
		dbg(lvl_debug,"0x%02X ",buffer[i]);
	}
	dbg(lvl_debug,"\n");
	return 1;
}

uint8_t deserialize_mfa_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_mfa_t)){
		return 0;
	}
	rx_mfa_t* rx = (rx_mfa_t*) rx_data;
	uint8_t i;
	dbg(lvl_debug,"\ndeserialize_mfa_rxdata\n");
	dbg(lvl_debug,"mfa: ");
	for(i=0;i<size; i++){
		dbg(lvl_debug,"0x%02X ",buffer[i]);
	}
	dbg(lvl_debug,"\n");
	
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
	dbg(lvl_debug,"\nserialize_lsg_txdata %p\n", tx);
	//dbg(lvl_debug,"lsg: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
	return 1;
}

uint8_t serialize_lsg_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_lsg_t)){
		return 0;
	}
	rx_lsg_t* rx = (rx_lsg_t*) rx_data;
	dbg(lvl_debug,"\nserialize_lsg_rxdata %p\n", rx);
	//dbg(lvl_debug,"lsg: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	
	return 1;
}

uint8_t deserialize_lsg_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_lsg_t)){
		return 0;
	}
	rx_lsg_t* rx = (rx_lsg_t*) rx_data;
	dbg(lvl_debug,"\ndeserialize_lsg_rxdata %p\n", rx);
	//dbg(lvl_debug,"lsg: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	return 1;
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

//*////////////////////////////////////////////////////////////////////////
// V2V 
///////////////////////////////////////////////////////////////////////////
//*
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
	//dbg(lvl_debug,"v2v: 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
	return 1;
}

//*////////////////////////////////////////////////////////////////////////
// GENERIC I2C FUNCTIONS 
///////////////////////////////////////////////////////////////////////////

uint8_t rx_task(int device, struct connected_devices* cd){
	uint8_t i;
	uint8_t rx_size = cd->rx_size;
	uint8_t i2crxdata[rx_size+1];
	if(!device) return 1;
	dbg(lvl_debug,"Read i2c\n");
	dbg(lvl_debug,"rx_pwm_t: %i, rx_pwm: %i\n",sizeof(rx_pwm_t), rx_size);
	
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
				if(!deserialize_pwm_rxdata(cd->rx_data, rx_size, i2crxdata)){
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
	dbg(lvl_debug,"rx_pwm_t: %i, rx_pwm: %i\n",sizeof(tx_pwm_t), tx_size);	
	dbg(lvl_debug,"// serialize tx object %i\n", tx_size);
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

void process_i2c_data(struct i2c* this, struct connected_devices* cd){
	uint8_t port = cd->addr;
	if(port == calculateID("MFA")){
		struct i2c_nav_data* navigation_data = get_navigation_data(this);
		get_audio_data(this, tx_mfa->radio_text);
		tx_mfa->distance_to_next_turn = navigation_data->distance_to_next_turn;
		tx_mfa->navigation_next_turn = navigation_data->nav_status && (navigation_data->nav_status << 4);
	}else if(port == calculateID("LSG")){
		
	}else if(port == calculateID("WFS")){
		
	}else if(port == calculateID("PWM")){
		
		
		/*	
		tx_pwm->pwm_freq = 10000;
		tx_pwm->cal_temperature = 5;
		tx_pwm->cal_voltage = 5;
		tx_pwm->time_value = 10;
		tx_pwm->water_value = 35;
		*/	
		tx_pwm->pwm_freq += 500;
		if(tx_pwm->pwm_freq > 30000) tx_pwm->pwm_freq = 1000;
	}else if(port == calculateID("V2V")){
		
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
			dbg(lvl_debug,"Read %i: 0x%02X 0x%02X\n", i, (uint8_t) data[i], device);
		}
}
}

///////////////////////////////////////////////////////////////////////////
// MAIN
///////////////////////////////////////////////////////////////////////////

static void 
i2c_task(struct i2c *this){

	int num_devices = g_list_length(this->connected_devices);
	dbg(lvl_info, "%i connected devices\n", num_devices);
	if(this->device){
		do{
			if(this->connected_devices->data){
				struct connected_devices* cd = this->connected_devices->data;
				if(0==select_slave(this->device, cd->addr)){
					rx_task(this->device, cd);
					tx_task(this->device, cd);
					process_i2c_data(this, cd);
				}
				if(this->connected_devices->next)
					this->connected_devices = this->connected_devices->next;
				else
					break;
			}
		}
		while(num_devices--);
	
		
	}
}

static void 
i2c_main(struct i2c *this, struct navit *nav){
	
	this->callback = callback_new_1(callback_cast(i2c_task), this);
	this->timeout = event_add_timeout(500, 1, this->callback);

	return;
}

static void 
i2c_init(struct i2c *this, struct navit *nav)
{
	this->nav=nav;
	this->navigation_data = g_new0(struct i2c_nav_data, 1);
	dbg(lvl_debug,"I2C Test\n");
	this->device = open_i2c("/dev/i2c-1");
	check_ioctl(this->device);
	if(init_i2c_devices(this))
		navit_add_callback(nav,callback_new_attr_1(callback_cast(i2c_main),attr_graphics_ready, this));
	else
		dbg(lvl_error, "No I2C Devices found\n");
}



void
plugin_init(void)
{
	struct attr callback; 
	i2c_plugin = g_new0(struct i2c, 1);
	callback.type=attr_callback;
	callback.u.callback=callback_new_attr_1(callback_cast(i2c_init),attr_navit,i2c_plugin);
	config_add_attr(config, &callback);
	dbg(lvl_debug,"hello i2c\n\n");
}

