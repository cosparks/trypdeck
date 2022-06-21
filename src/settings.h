#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "Apa102.h"
#include "LedController.h"

// media files
#define VIDEO_STARTUP_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/startup/"
#define VIDEO_WAIT_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/wait/"
#define VIDEO_PULLED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/pulled/"
#define VIDEO_REVEAL_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/reveal/"

#define LED_STARTUP_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/startup/"
#define LED_WAIT_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/wait/"
#define LED_PULLED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/pulled/"
#define LED_REVEAL_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/reveal/"

// gpio
#define SERIAL_BAUD 9600 // networking
#define SPI_BAUD 4000000 // led matrix
#define ENABLE_DEBUG 0	// when true, stops all calls to pigpio (this disables leds!)

// led 
#define PIXEL_BRIGHTNESS 31
// led (main)
#define MAIN_LED_MATRIX_WIDTH 50
#define MAIN_LED_MATRIX_HEIGHT 50
#define MAIN_LED_MATRIX_SPLIT 25
#define MAIN_LED_GRID_CONFIGURATION_OPTION_A Apa102::VerticalTopLeft    // configuration of first led grid in main light chamber
#define MAIN_LED_GRID_CONFIGURATION_OPTION_B Apa102::VerticalTopRight   // configuration of second led grid in main light chamber
#define MAIN_LED_GRID_AB_ORIENTATION LedController::Horizontal          // orientation of grid A and grid B (Horizontal -> A B -- Vertical -> A / B)
// led (centre)
#define CENTRE_LED_MATRIX_WIDTH 25
#define CENTER_LED_MATRIX_HEIGHT 30
#define CENTRE_LED_GRID_CONFIGURATION_OPTION Apa102::VerticalTopLeft    // configuration of first led grid in main light chamber

#endif