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

#ifndef I2C_DEVICES_H
#define I2C_DEVICES_H

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
#include <util.h>
#include <service.h>
#include <i2c.h>

#ifdef USE_AUDIO_FRAMEWORK
#include <audio.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif 

#define AUDIO_STR_LENGTH 38

typedef struct txdataSTUB{
	uint32_t distance_to_next_turn;
	char radio_text[AUDIO_STR_LENGTH];
	uint8_t navigation_next_turn;
	uint8_t calibration;
}tx_stub_t;
	
typedef struct rxdataSTUB{
	uint32_t distance_to_next_turn;
	char radio_text[AUDIO_STR_LENGTH];
	uint8_t navigation_next_turn;
	uint8_t calibration;
}rx_stub_t;

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
	char radio_text[AUDIO_STR_LENGTH];
	
	uint8_t navigation_next_turn;
	//navigation active?
	uint8_t cal_water_temperature;
	uint8_t cal_voltage;
	uint8_t cal_oil_temperature;
	uint8_t cal_consumption;	
	uint8_t cal_speed;
	// other values from pwm module?
}tx_mfa_t;
	
typedef struct rxdataMFA{
	
	uint32_t distance_to_next_turn;
	uint16_t voltage;
	uint16_t consumption;
	uint16_t average_consumption;
	uint16_t range;
	uint16_t speed;
	uint16_t average_speed;
	int16_t rpm;
	char radio_text[AUDIO_STR_LENGTH];
	uint8_t navigation_next_turn;	//navigation active?
	uint8_t cal_water_temperature;
	uint8_t cal_voltage;
	uint8_t cal_oil_temperature;
	uint8_t cal_consumption;	
	uint8_t cal_speed;
	// read only
	int8_t water_temperature;
	int8_t ambient_temperature;
	int8_t oil_temperature;
	uint8_t dummy;
}rx_mfa_t;

extern rx_lsg_t *rx_lsg;
extern tx_lsg_t *tx_lsg;
extern rx_pwm_t *rx_pwm;
extern tx_pwm_t *tx_pwm;
extern rx_wfs_t *rx_wfs;
extern tx_wfs_t *tx_wfs;
extern rx_v2v_t *rx_v2v;
extern tx_v2v_t *tx_v2v;
extern rx_mfa_t *rx_mfa;
extern tx_mfa_t *tx_mfa;

extern tx_stub_t *tx_stub;
extern rx_stub_t *rx_stub;

int init_i2c_devices(struct service_priv *this);
#ifdef __cplusplus
}
#endif
#endif
