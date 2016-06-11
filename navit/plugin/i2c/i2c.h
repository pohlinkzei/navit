#ifndef I2C_H
#define I2C_H
#include <unistd.h>
#include <stdio.h>
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
#ifdef USE_AUDIO_FRAMEWORK
#include <audio.h>
#endif

#define AUDIO_STR_LENGTH 32
#define CRC_POLYNOME 0xAB
char x[] = {0x66, 0x55,0x44, 0x33, 0x22, 0x11};
char result[6] = {0,};
unsigned char i2ctxdata[128] = {0,};
unsigned char i2crxdata[128] = {0,};
struct i2c *i2c_plugin;

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef signed char int8_t;
typedef signed short int int16_t;

struct i2c_nav_data{
	int distance_to_next_turn;
	int nav_status;
	int next_turn;
};

struct i2c{
	struct navit* nav;
    int device;
    int last_status;
    int last_next_turn;
    struct callback* callback;
    int timeout;
    GList* connected_devices;
    struct i2c_nav_data* navigation_data;
};
/*
typedef struct connected_devices{
	char* name;
	uint8_t addr;
	char* icon;
	void* rx_data;
	void* tx_data;
	uint8_t (*serialize_tx)(void *tx, uint8_t size, volatile uint8_t buffer[size]);
	uint8_t (*serialize_rx)(void *rx, uint8_t size, volatile uint8_t buffer[size]);
	uint8_t (*deserialize_rx)(void *rx, uint8_t size, volatile uint8_t buffer[size]); 
}connected_device_t;*/


struct connected_devices{
	char* name;
	uint8_t addr;
	char* icon;
	void* rx_data;
	void* tx_data;
	uint8_t rx_size;
	uint8_t tx_size;
	uint8_t (*serialize_tx)(void *tx_data, uint8_t size, volatile uint8_t buffer[size]);
	uint8_t (*serialize_rx)(void *rx_data, uint8_t size, volatile uint8_t buffer[size]);
	uint8_t (*deserialize_rx)(void *rx_data, uint8_t size, volatile uint8_t buffer[size]); 
};

typedef struct txdataLSG{
	bool AL;
	bool TFL;
	bool ZV;
	bool LED;
	uint8_t time_in;
	uint8_t time_out;
}tx_lsg_t;
	
typedef struct rxdataLSG{
	bool AL;
	bool TFL;
	bool ZV;
	bool LED;
	uint8_t time_in;
	uint8_t time_out;
}rx_lsg_t;

typedef struct txdataV2V{
	uint8_t time;
	uint8_t temperature;
}tx_v2v_t;
	
typedef struct rxdataV2V{
	uint8_t time;
	uint8_t temperature;
}rx_v2v_t;

typedef struct txdataWFS{
	uint8_t time;
}tx_wfs_t;
	
typedef struct rxdataWFS{
	uint8_t time;
}rx_wfs_t;

typedef struct txdataMFA{
	uint8_t radio_text[AUDIO_STR_LENGTH];
	uint8_t navigation_next_turn;
	long distance_to_next_turn;
	//navigation active?
	uint8_t cal_water_temperature;
	uint8_t cal_voltage;
	uint8_t cal_oil_temperature;
	uint8_t cal_consumption;	
	// other values from pwm module?
}tx_mfa_t;
	
typedef struct rxdataMFA{
	uint8_t radio_text[AUDIO_STR_LENGTH];
	uint8_t navigation_next_turn;
	long distance_to_next_turn;
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
	int consumption;
	int average_consumption;
	int range;
	int speed;
	int average_speed;
	int rpm;
}rx_mfa_t;

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
struct i2c* get_i2c_plugin(void);

#endif
