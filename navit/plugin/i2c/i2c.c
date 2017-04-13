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

#include <service.h>
#include <i2c.h>
#include <i2c_devices.h>
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

struct navigation;

static struct service_methods i2c_service_meth = {
		i2c_get_plugin,
		i2c_set_attr,
		i2c_get_attr,
		i2c_get_properties,
		i2c_set_property,
};


int i2c_set_attr(struct service_priv *priv, struct attr *attr){
	dbg(lvl_info, "i2c_set_attr(struct service_priv=%p, struct attr=%p)\n", priv,  attr);
	switch(attr->type){
		case attr_navit:
			dbg(lvl_info, "Attr_Navit\n");
			priv->nav = attr->u.navit;
			break;
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
	dbg(lvl_info, "i2c_get_attr(struct service_priv=%p, enum attr_type=%i (%s), struct attr=%p)\n", priv, type, attr_to_name(type), attr);
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
	dbg(lvl_info, "\n");
	int dev_h = open(device, O_RDWR);
	if(dev_h < 0){
		perror("Error: Can't open I2C device!\n");
		return 0;
	}
	return dev_h;
}

unsigned long check_ioctl(int device){
	dbg(lvl_info, "\n");
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
	dbg(lvl_info, "\n");
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


void get_audio_data(struct service_priv* this, char audio_str[AUDIO_STR_LENGTH]){
	int i = 0;
	gchar str[256] = {0,};
#ifdef USE_AUDIO_FRAMEWORK 
	strcpy(str, audio_get_current_track(this->nav));
	strcat(str, " - ");
	strcat(str, audio_get_current_playlist(this->nav));
#else
	sprintf(str, "No Audio Plugin found! ");
#endif
for (i=0; i< AUDIO_STR_LENGTH; i++){
		audio_str[i] = str[i];
	}
	audio_str[i] = ' ';
}


int get_next_turn_by_name(char* name){
	if(name){
		if(!strcmp(name, "null")){
			return -1;
		}
		
		/* Possible maneuver types:
		 * nav_none                    (default, change wherever we encounter it – unless the maneuver is a merge, which has only merge_or_exit)
		 * nav_straight                (set below)
		 * nav_keep_{left|right}       (set below)
		 * nav_{right|left}_{1..3}     (set below)
		 * nav_turnaround              (TODO: when we have a U turn without known direction? Needs full implementation!)
		 * nav_turnaround_{left|right} (set below)
		 * nav_roundabout_{r|l}{1..8}  (set below, special handling)
		 * nav_exit_{left|right}       (do not set here)
		 * nav_merge_{left|right}      (do not set here)
		 * nav_destination             (if this is set, leave it)
		 * nav_position                (do not set here)
		 */
		// 					012345678901234567890
		if(!strcmp(name , "nav_straight")){
			return STRAIGHT;
		} 
		// 					012345678901234567890
		if(!strcmp(name , "nav_destination")){
			return DEST;
		} 
		if(!strncmp(name , "nav_left_", 9)){
			int number;
			sscanf(&name[9], "%i", &number);
			return number;
		}
		// 					012345678901234567890
		if(!strncmp(name , "nav_right_", 10)){
			int number;
			sscanf(&name[10], "%i", &number);
			return 3+number;
		}
		// 					012345678901234567890
		if(!strncmp(name , "nav_roundabout_l", 16)){
			int number;
			sscanf(&name[16], "%i", &number);
			return 6 + number;
		}
		if(!strncmp(name , "nav_roundabout_r", 16)){
			int number;
			sscanf(&name[16], "%i", &number);
			return 14 + number;
		}
		
		if(!strcmp(name , "nav_keep_left")){
			return KEEPL;
		}
		if(!strcmp(name , "nav_keep_right")){
			return KEEPR;
		}
		
		if(!strcmp(name , "nav_exit_left")){
			return EXITL;
		}
		if(!strcmp(name , "nav_exit_right")){
			return EXITR;
		}
		
		if(!strcmp(name , "nav_merge_left")){
			return MERGEL;
		}
		
		if(!strcmp(name , "nav_merge_right")){
			return MERGER;
		}
		
		if(!strncmp(name , "nav_turnaround_left", 10)){
			return TURNL;
		}
		
		if(!strncmp(name , "nav_turnaround_right", 10)){
			return TURNR;
		}
	}
	return -1;
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
	struct map *map = NULL;
	struct map_rect *mr = NULL;
	struct item *item = NULL;
	enum nav_status status = 0;
	char* name = NULL;
	struct navit* navit = this->nav;
	if(nav_data && navit){
		nav = navit_get_navigation(navit);
		if (nav){
			map = navigation_get_map(nav);
			struct attr item1;
			navigation_get_attr(nav, attr_length, &item1, NULL);
			nav_data->distance_to_next_turn = round_distance(item1.u.num);
			
			if (navigation_get_attr(nav, attr_nav_status, &attr, NULL)){
				status = attr.u.num;
				nav_data->nav_status = status;
			}
		}
		if (map)
			mr = map_rect_new(map, NULL);
		if (mr)
			while ((item = map_rect_get_item(mr))
				   && (item->type == type_nav_position || item->type == type_nav_none));
		if (item) {
			name = item_to_name(item->type);
		}
		nav_data->next_turn = get_next_turn_by_name(name);
	}
	return nav_data;
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
		if(navigation_data){
			tx_mfa->distance_to_next_turn = navigation_data->distance_to_next_turn;
			tx_mfa->navigation_next_turn = navigation_data->next_turn + (navigation_data->nav_status << 4);
			dbg(lvl_info,"Send NAV data: %i, %i\n", tx_mfa->distance_to_next_turn,tx_mfa->navigation_next_turn);
		}
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
	}else if(this->stub){
		struct i2c_nav_data* navigation_data = get_navigation_data(this);
		get_audio_data(this, tx_stub->radio_text);
		dbg(lvl_error, "\nradio text: %s\ncalibration: %i\nnav_next: %i\ndistance_to_nxt: %i\n", tx_stub->radio_text, tx_stub->calibration, tx_stub->navigation_next_turn, tx_stub->distance_to_next_turn);
		/*i2c_set_property();*/
		if(navigation_data){
			tx_stub->distance_to_next_turn = navigation_data->distance_to_next_turn;
			tx_stub->navigation_next_turn = navigation_data->next_turn + (navigation_data->nav_status << 4);
			dbg(lvl_info,"Send NAV data: %i, %i (0x%02X)\n", tx_stub->distance_to_next_turn,tx_stub->navigation_next_turn, tx_stub->navigation_next_turn);
		}
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
	if(this->stub){
		while(num_devices--){
			if(this->connected_devices->data){
				struct connected_devices* cd = this->connected_devices->data;
				
				dbg(lvl_info, "\nstart PROCESS DATA: 0x%02X, %s\n\n", cd->addr, cd->name);
				rx_task(this->device, cd);
				tx_task(this->device, cd);
				process_i2c_data(this, cd);
				dbg(lvl_info, "\nend PROCESS DATA: 0x%02X, %s\n\n", cd->addr, cd->name);
				
				if(this->connected_devices->next)
					this->connected_devices = this->connected_devices->next;
				else
					break;
			}
		}
	}
}

GList* i2c_get_properties(struct service_priv *priv){
	dbg(lvl_info,"Found Properties %p\n", priv->properties);
	return priv->properties;
}

struct service_property* i2c_set_property(struct service_priv *priv, struct service_property* sp){
	//*
	GList* service_properties = priv->properties;
	while(service_properties->next){
		struct service_property *service_property = (struct service_property*) service_properties->data;
		if(!strcmp(sp->name, service_property->name)){
			if(!service_property->ro){
				service_property->children = sp->children;
				service_property->value = sp->value;
				service_property->num_children = sp->num_children;
				service_property->parent = sp->parent;
				service_property->root = sp->root;
			}
			return service_property;
		}
		service_properties = service_properties->next;
	}
	//*/
	return NULL;
}
/*
struct service_property* get_device_property (GList* cd_list, char* name){
	struct service_property* dev_prop;
	struct connected_devices* cd;
	cd = (struct connected_devices*) cd_list->data;
	while(cd_list->next){
		cd = (struct connected_devices*) cd_list->data;
		dev_prop = (struct service_property*) cd->properties->data;
		if(strcmp(dev_prop->name, name)){
			return dev_prop;
		}
		cd_list = cd_list->next;
	}
	
};
*/

void
i2c_get_plugin(struct service_priv* p){
	p = i2c_plugin;
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



// todo fix....
GList* init_properties(struct service_priv *this){
	GList *prop = NULL;

	this->connected_devices = g_list_first(this->connected_devices);
	int num_devices = g_list_length(this->connected_devices);
	dbg(lvl_info, "%i connected devices\n", num_devices);
	if(this->device || this->stub){
		while(num_devices--){
			if(this->connected_devices->data){
				struct connected_devices* cd = this->connected_devices->data;
				struct service_property *sp = g_new0(struct service_property,1);
				dbg(lvl_info, "init device %s\n", cd->name);
				sp->name = cd->name;
				sp->parent = NULL;
				sp->ro = 0;
				sp->num_children = cd->num_properties;
				dbg(lvl_info, "parent = %p, ro = %i, num_children = %i\n", sp->parent, sp->ro, sp->num_children);
				//uint8_t cnt = cd->num_properties;
				//while(cnt--){
					
					sp->children = cd->init_properties(cd->rx_data, cd->tx_data, sp);
					cd->properties = sp->children;
				//}
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
    if ((attr = attr_search (attrs, NULL, attr_source))){
		i2c_plugin->source = g_strdup(attr->u.str);
		dbg (lvl_info, "Navit: %s\n", i2c_plugin->source);
	}
   
    dbg (lvl_debug,  "Callback created successfully\n");
	i2c_plugin->attrs=attrs;
	//i2c_plugin->nav=nav;
	/* Init i2c Bus & local data 
	//*/ 
	i2c_plugin->navigation_data = g_new0(struct i2c_nav_data, 1);

	i2c_plugin->device = open_i2c(i2c_plugin->source);
	if(!i2c_plugin->device){
		dbg(lvl_error, "no i2c ioctl found\n");
		i2c_plugin->stub = 1;
		//no i2c ioctl found: We are probably on a testing vm... set dummy data
	}else{
		check_ioctl(i2c_plugin->device);
	}
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
	dbg(lvl_info, "Methods: %p\n",meth);
    *meth=i2c_service_meth;
    dbg(lvl_info, "Methods: Plugin: %p, Get: %p, Set: %p\n",meth->plugin, meth->get_attr, meth->set_attr);
   
	
    return i2c_plugin;
    //*/
}



/**
* @brief plugin entry point
*/
void
plugin_init(void)
{ 
    dbg(lvl_error, "registering service category i2c-service\n");
    plugin_register_category_service("i2c_service", i2c_service_new);
}
