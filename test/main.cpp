#include <unistd.h>
#include <signal.h>
#include <dlfcn.h>

#include <stdio.h>
#include <string.h>

#include "vl53l5cx_api.hpp"

#define DELAY 100000

VL53L5CX_Configuration dev1;
uint8_t status, isAlive, isReady,i;
VL53L5CX_ResultsData 	Results;

int main()
{	
	status = vl53l5cx_comms_init(&dev1.platform, 1);
	if(status) return -1;

	status = vl53l5cx_is_alive(&dev1, &isAlive);
	if(!isAlive || status) {
		printf("%d\n", status);
		return status;
	}
	printf("------------------");

	status = vl53l5cx_init(&dev1);
	if(status) return status;

	status = vl53l5cx_set_resolution(&dev1, VL53L5CX_RESOLUTION_8X8);
	if(status)
	{
		printf("vl53l5cx_set_resolution failed, status %u\n", status);
		return status;
	}
	status = vl53l5cx_set_ranging_frequency_hz(&dev1, 30);

	status = vl53l5cx_start_ranging(&dev1);
	vl53l5cx_check_data_ready(&dev1, &isReady);

	while(i<40)
	{
		status = vl53l5cx_check_data_ready(&dev1, &isReady);
		if(isReady)
		{
			printf("\nSensor 1:\n");

			vl53l5cx_get_ranging_data(&dev1, &Results);
			for(int a = 0; a < 64; a++)
			{
				if((a%8)==0) printf("\n");

				printf("|%4d mm|",Results.distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*a]);
			}
			printf("\n");
			i++;
		}
		usleep(5);
	}

	status = vl53l5cx_stop_ranging(&dev1);

	vl53l5cx_comms_close(&dev1.platform);
	

	return 0;
}
