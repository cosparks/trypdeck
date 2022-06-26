#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "Apa102.h"
#include "LedController.h"

// Leader or Follower
#define Leader

// Media Folders
#define VIDEO_STARTUP_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/startup/"
#define VIDEO_WAIT_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/wait/"
#define VIDEO_PULLED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/pulled/"
#define VIDEO_REVEAL_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/reveal/"

#define LED_STARTUP_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/startup/"
#define LED_WAIT_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/wait/"
#define LED_PULLED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/pulled/"
#define LED_REVEAL_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/reveal/"

// GPIO Settings
#define SERIAL_BAUD 9600 // networking
#define SPI_BAUD 4000000 // led matrix
#define ENABLE_DEBUG 0	// when true, stops all calls to pigpio (this disables leds!)
#define NUM_INPUTS 3

// Led Settings
// * dont modify these values *
#define MAIN_LEDS 0
#define CENTRE_LEDS 1
#define NO_LEDS 2

// modify these values
#define LED_SETTING NO_LEDS		// change this to set pi to run different LED setups
#define PIXEL_BRIGHTNESS 31
#define RUN_LEDS (LED_SETTING != NO_LEDS)

#if (LED_SETTING == MAIN_LEDS)
// main led grid
#define LED_MATRIX_WIDTH 50
#define LED_MATRIX_HEIGHT 50
#define LED_MATRIX_SPLIT 25
#define LED_GRID_CONFIGURATION_OPTION_A Apa102::VerticalTopLeft		// configuration of first led grid in main light chamber
#define LED_GRID_CONFIGURATION_OPTION_B Apa102::VerticalTopRight	// configuration of second led grid in main light chamber
#define GRID_AB_ORIENTATION LedController::Horizontal			// orientation of grid A and grid B (Horizontal -> A B -- Vertical -> A / B)
#elif (LED_SETTING == CENTRE_LEDS)
// centre led grid
#define LED_MATRIX_WIDTH 25
#define LED_MATRIX_HEIGHT 30
#define LED_GRID_CONFIGURATION_OPTION Apa102::VerticalTopLeft
#define GRID_AB_ORIENTATION LedController::None
#endif
// leds are off
#endif