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
#include <i2c_devices.h>

#define CRC_POLYNOME 0xAB

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

uint8_t deserialize_stub_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_stub_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
uint8_t serialize_stub_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);
GList* init_stub_properties(void* rx_data, void* tx_data, struct service_property *parent);

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

///////////////////////////////////////////////////////////////////////////
// STUB 
///////////////////////////////////////////////////////////////////////////
//*
uint8_t serialize_stub_txdata(void *tx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(tx_stub_t)){
		dbg(lvl_debug,"size: %i, struct: %i\n",size,sizeof(tx_stub_t));
		return 0;
	}
	char str[2560] = {0,};
	tx_stub_t* tx = (tx_stub_t*) tx_data;
	dbg(lvl_debug,"\nserialize_stub_txdata:%i\n%s\n%i\n",size,tx->radio_text, tx->navigation_next_turn, tx->calibration);
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
	buffer[37] = tx->calibration;
	
	sprintf(str,"stub:");
	for(i=0;i<size; i++){
		char buf[6] = {0,};
		sprintf(buf, " 0x%02X", buffer[i]);
		strcat(str, buf);
	}
	dbg(lvl_debug,"%s\n", str);
	return 1;
}

uint8_t serialize_stub_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_stub_t)){
		return 0;
	}
	rx_stub_t* rx = (rx_stub_t*) rx_data;
	dbg(lvl_debug,"\nserialize_stub_rxdata:%i\n%s\n%i\n%i\n%",size,rx->radio_text, rx->navigation_next_turn, rx->calibration);
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
	buffer[AUDIO_STR_LENGTH + 5] = rx->calibration;
	
	
	sprintf(str,"stub:");
	for(i=0;i<size; i++){
		char buf[6] = {0,};
		sprintf(buf, " 0x%02X", buffer[i]);
		strcat(str, buf);
	}
	dbg(lvl_debug,"%s\n", str);
	return 1;
}

uint8_t deserialize_stub_rxdata(void *rx_data, uint8_t size, volatile uint8_t buffer[size]){
	if(size != sizeof(rx_stub_t)){
		return 0;
	}
	rx_stub_t* rx = (rx_stub_t*) rx_data;
	uint8_t i;
	char str[2560] = {0,};
	dbg(lvl_debug,"\ndeserialize_stub_rxdata:%i\n", size);
	sprintf(str,"stub:");
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
	rx->calibration = buffer[AUDIO_STR_LENGTH + 5];
	
	return 1;
}

GList* init_stub_properties(void *rx_data, void* tx_data, struct service_property *parent){
	GList* list = NULL;
	rx_stub_t* rx = (rx_stub_t*) rx_data;
	rx_stub_t* tx = (rx_stub_t*) tx_data;
	struct service_property *p = g_new0(struct service_property,1);
	
	if(rx->calibration == tx->calibration){
		p->name = g_strdup("Calibration");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->calibration;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	
	p->name = g_strdup("Navigation Next Turn");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->navigation_next_turn;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Distance To Next");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->distance_to_next_turn;
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
	
	return list;
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
		p->value = (void*) &rx->pwm_freq;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}

	if(rx->cal_voltage == tx->cal_voltage){
		p->name = g_strdup("Calibration Voltage");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->cal_voltage;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_temperature == tx->cal_temperature){
		p->name = g_strdup("Calibration Temperature");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->cal_temperature;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->water_value == tx->water_value){
		p->name = g_strdup("Enable Temperature");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->water_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->time_value == tx->time_value){
		p->name = g_strdup("Enable Time");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->time_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	
	p->name = g_strdup("Battery Voltage");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->vbat;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Water Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->water_temp;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Device Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->fet_temp;
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
		p->value = (void*) &rx->cal_water_temperature;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_voltage == tx->cal_voltage){
		p->name = g_strdup("Voltage Calibration");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->cal_voltage;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_oil_temperature == tx->cal_oil_temperature){
		p->name = g_strdup("Oil Temperature Calibration");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->cal_oil_temperature;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_consumption == tx->cal_consumption){
		p->name = g_strdup("Consumption Calibration");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->cal_consumption;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	
	p->name = g_strdup("Battery Voltage");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->voltage;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Water Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->water_temperature;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Ambient Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->ambient_temperature;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Oil Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->oil_temperature;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Ambient Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->ambient_temperature;
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
	p->value = (void*) &rx->consumption;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Average Consumption");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->average_consumption;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Speed");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->speed;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Average Speed");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->average_speed;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Speed");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->speed;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
		
	p->name = g_strdup("RPM");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->rpm;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
			
	p->name = g_strdup("Range");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->range;
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
		p->value = (void*) &rx->AL;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}

	if(rx->TFL == tx->TFL){
		p->name = g_strdup("Daytime Running Light");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->TFL;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->ZV == tx->ZV){
		p->name = g_strdup("Central Locking");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->ZV;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->LED == tx->LED){
		p->name = g_strdup("LED Setting");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->LED;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->time_in == tx->time_in){
		p->name = g_strdup("Delay Time Inside");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->time_in;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->time_out == tx->time_out){
		p->name = g_strdup("Delay Time Outside");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->time_out;
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
		p->value = (void*) &rx->pwm_freq;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}

	if(rx->cal_voltage == tx->cal_voltage){
		p->name = g_strdup("Calibration Voltage");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->cal_voltage;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->cal_temperature == tx->cal_temperature){
		p->name = g_strdup("Calibration Temperature");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->cal_temperature;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->water_value == tx->water_value){
		p->name = g_strdup("Enable Temperature");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->water_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	if(rx->time_value == tx->time_value){
		p->name = g_strdup("Enable Time");
		p->ro = 0;
		p->num_children = 0;
		p->parent = parent;
		p->children = NULL;
		p->value = (void*) &rx->time_value;
		list = g_list_append(list, p);
		p=g_new0(struct service_property,1);
	}
	
	p->name = g_strdup("Battery Voltage");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->vbat;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Water Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->water_temp;
	list = g_list_append(list, p);
	p=g_new0(struct service_property,1);
	
	p->name = g_strdup("Device Temperature");
	p->ro = 1;
	p->num_children = 0;
	p->parent = parent;
	p->children = NULL;
	p->value = (void*) &rx->fet_temp;
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
	}else if(this->stub){
		struct connected_devices *cd = g_new0(struct connected_devices, 1);
		rx_stub = (rx_stub_t*) malloc(sizeof(rx_stub_t)); 
		tx_stub = (tx_stub_t*) malloc(sizeof(tx_stub_t)); 
		cd->name = g_strdup("I2Cstub");
		cd->icon = "gui_inactive";
		cd->rx_data = rx_stub;
		cd->tx_data = tx_stub;
		cd->init_properties = init_stub_properties;
		cd->serialize_rx = serialize_stub_rxdata;
		cd->serialize_tx = serialize_stub_txdata;
		cd->deserialize_rx = deserialize_stub_rxdata;
		cd->num_properties = 3;
		dbg(lvl_info, "Appending a Device %p\n", cd);
		this->connected_devices = g_list_append(this->connected_devices, cd);
	}
	return 1;
}
