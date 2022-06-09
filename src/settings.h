#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "Apa102.h"

// system
#define ENABLE_DEBUG 0	// when true, stops all calls to pigpio

// media files
#define WAIT_VIDEO_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/loop/"
#define CARD_VIDEO_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/card/"
#define LED_ANIMATION_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/anim/"

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