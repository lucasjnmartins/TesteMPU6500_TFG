#include <stdio.h>
#include <stdbool.h>
#include <sys/_intsup.h>
#include <unistd.h>
#include "driver/i2c_master.h"
#include "hal/i2c_types.h"
#include "soc/gpio_num.h"

#define DATA_LENGTH 14
void app_main(void)
{
	i2c_master_bus_config_t i2c_mst_config = {
    	.clk_source = I2C_CLK_SRC_DEFAULT,
	    .i2c_port = I2C_NUM_0,
	    .scl_io_num = GPIO_NUM_22,
	    .sda_io_num = GPIO_NUM_21,
	    .glitch_ignore_cnt = 7,
	    .flags.enable_internal_pullup = 1
	};
	i2c_master_bus_handle_t bus_handle;
	
	ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));
	
	i2c_device_config_t dev_cfg = {
	    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
	    .device_address = 0x68,
	    .scl_speed_hz = 100000,
	};
	
	i2c_master_dev_handle_t dev_handle;
	ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
	uint8_t activate_mpu[2];
    
    uint8_t gyro_data_address[6] = {0x43, 0x44, 0x45, 0x46, 0x47, 0x48};
    uint8_t gyro_data_read[6];
    int16_t gyro_combined_values[3];
    
    uint8_t acel_data_address[6] = {0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40};
    uint8_t acel_data_read[6];
    int16_t acel_combined_values[3];
    
    activate_mpu[0] = 0x6B;
    activate_mpu[1] = 0x00;
    
    i2c_master_transmit(dev_handle, activate_mpu, 2, 50);
    
    while (true) {
		for(uint8_t i=0; i<6; i+=2) {
			i2c_master_transmit_receive(dev_handle, &gyro_data_address[i], 1, &gyro_data_read[i], 1, 50);
			i2c_master_transmit_receive(dev_handle, &gyro_data_address[i+1], 1, &gyro_data_read[i+1], 1, 50);
			
			i2c_master_transmit_receive(dev_handle, &acel_data_address[i], 1, &acel_data_read[i], 1, 50);
			i2c_master_transmit_receive(dev_handle, &acel_data_address[i+1], 1, &acel_data_read[i+1], 1, 50);
			
			gyro_combined_values[i/2] = (int16_t)((gyro_data_read[i] << 8) | gyro_data_read[i+1]);
			acel_combined_values[i/2] = (int16_t)((acel_data_read[i] << 8) | acel_data_read[i+1]);
		}
        
        printf("\nGyro - X: %05d  /  Y: %05d  /  Z: %05d  \nAcel - X: %05d  /  Y: %05d  /  Z: %05d  ", gyro_combined_values[0], gyro_combined_values[1], gyro_combined_values[2], acel_combined_values[0], acel_combined_values[1], acel_combined_values[2]);
        usleep(100000);
    }
}
