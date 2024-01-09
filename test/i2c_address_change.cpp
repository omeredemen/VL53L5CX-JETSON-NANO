#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vl53l5cx_api.hpp"

#define DEVICE_NUMBER  3
#define BUFFER_MAX 4
#define DIRECTION_MAX 40
#define VALUE_MAX 30
#define LOW 0
#define DELAY 500000

VL53L5CX_Configuration 	Dev[DEVICE_NUMBER];

/**
 * jetson nano quark board üzerinde bulunan GPIO pinlerine bağlı
 * vl53l5cx'lerin i2c addresslerini değiştrimek için aşağıdaki 
 * GPIO'lar kullanılır.
 * - 26. Pin GPI01 → 149 (sysfs)
 * - 28. Pin GPI02 → 62  (sysfs)
 * - 34. Pin GPI05 → 63  (sysfs)
 * - 36. Pin GPI06 → 64  (sysfs)
 * - 38. Pin GPI07 → 168 (sysfs)
 * - 40. Pin GPI08 → 202 (sysfs)
*/
int control_pin_0[DEVICE_NUMBER] = { 149 ,62, 65 };
int control_pin_1[DEVICE_NUMBER] = { 64, 168, 202 };
unsigned char  	address = 0x29;

static int exportPin(int );
static int unexportPin(int );
static int directionPin(int );
static int writePin(int , int );
int pin_start(int control_pin[3], char bus);

int main(){

	if(pin_start(control_pin_1, 1) == -1){
		printf("i2c-1 unsuccessful\n");
		return -1;
	}

	usleep(DELAY);

	if(pin_start(control_pin_0, 0) == -1){
		printf("i2c-0 unsuccessful\n");
		return -1;
	}

	printf("successfully completed\n");
	return 0;

}

int pin_start(int gpio_pin[3], char bus){
	int status;
	for (int i = 0; i < DEVICE_NUMBER; i++)
	{
		if(unexportPin(gpio_pin[i])){
			printf("pin %d not close\n", gpio_pin[i]);
			return -1;
		}
		usleep(DELAY);
		if(exportPin(gpio_pin[i])){
			printf("pin %d not open\n", gpio_pin[i]);
			return -1;
		}
		usleep(DELAY);
		if(directionPin(gpio_pin[i])){
			printf("pin %d could not be set as output\n", gpio_pin[i]);
			return -1;
		}
		usleep(DELAY);
		if(writePin(gpio_pin[i], 0)){
			printf("pin %d not become low\n", gpio_pin[i]);
			return -1;
		}
		usleep(DELAY);
	}

	printf("opening devices to change their i2c adresses...\n");

	for(int i = 0; i < DEVICE_NUMBER; i++){
		if(vl53l5cx_comms_init(&Dev[i].platform, bus)){
			printf("VL53L5CX comms init failed %d\n", gpio_pin[i]);
			exit(-1);
		}
		usleep(DELAY);
		if(vl53l5cx_set_i2c_address(&Dev[i], (address+i+1)<<1)){
			printf("VL53L5CX address could not change (%d)\n",i);
			exit(-1);
		}
		Dev[i].platform.address= 0x30;
		usleep(DELAY);
		writePin(gpio_pin[i], 1);
		usleep(DELAY);
		vl53l5cx_comms_close(&Dev[i].platform);
		usleep(DELAY);
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////
static int directionPin(int pin)
{
	char path[DIRECTION_MAX];
	int fd;
	static const char s_directions_str[]  = "in\0out";
	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio direction for writing: %d!\n", pin);
		return(1);
	}
	if (-1 == write(fd, &s_directions_str[3], 3)) {
		fprintf(stderr, "Failed to set direction!\n");
		return(1);
	}
	close(fd);
	return(0);
}

/////////////////////////////////////////////////////////////////////
static int writePin(int pin,int value)
{
	static const char s_values_str[] = "01";
	char path[VALUE_MAX];
	int fd;
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if (-1 == fd) {
		fprintf(stderr, "Failed to open gpio value for writing: %d!\n", pin);
		return(1);
	}

	if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
		fprintf(stderr, "Failed to write value!\n");
		return(1);
	}

	close(fd);
	return(0);
}

///////////////////////////////////////////////////////////////////
static int exportPin(int pin)
{

    char buffer[BUFFER_MAX];
    ssize_t bytes_written;

    int fd = open("/sys/class/gpio/export",01);

    if(fd==-1) 
    {
        printf("Failed to open unexport for writing: %d!\n", pin);
        return 1;
    }

    bytes_written = snprintf(buffer,BUFFER_MAX,"%d",pin);
    write(fd,buffer,bytes_written);
    close(fd);
    return 0;

}


static int unexportPin(int pin)
{
    char buffer[BUFFER_MAX];
    ssize_t bytes_written;
    int fd = open ("/sys/class/gpio/unexport", 01);
    if (-1 == fd) {
		fprintf(stderr, "Failed to open unexport for writing!\n");
		return(1);
	}

    bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}