#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "Apa102.h"

// system
#define ENABLE_DEBUG 0	// when true, stops all calls to pigpio

// media files
#define VIDEO_STARTUP_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/video/startup/"
#define VIDEO_WAIT_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/video/wait/"
#define VIDEO_PULLED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/video/pulled/"
#define VIDEO_REVEAL_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/video/reveal/"

#define LED_STARTUP_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/led/startup/"
#define LED_WAIT_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/led/wait/"
#define LED_PULLED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/led/pulled/"
#define LED_REVEAL_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/led/reveal/"

// gpio
#define SERIAL_BAUD 9600
#define SPI_BAUD 4000000

// led
#define LED_MATRIX_WIDTH 53
#define LED_MATRIX_HEIGHT 10
#define PIXEL_BRIGHTNESS 31
#define LED_GRID_CONFIGURATION_OPTION Apa102::HorizontalTopLeft
#define CACHE_RGB_FRAMES 0

#endif