#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "Apa102.h"
#include "LedController.h"

// Leader or Follower
#define Leader

// ID
#define ID "0"						// each pi on the network MUST have a unique id--change this setting for different pis

// Timing
#define STARTUP_TIME 20000			// duration for pis to sync up and play startup animation
#define PULL_DEBOUNCE_TIME 2000		// duration within which other people can still pull chain before pulled animation starts
#define REVEAL_TIME 20000			// duration for which tarot cards will be displayed before returning to wait state

// GPIO Settings
#define SERIAL_BAUD 9600			// networking baud rate
#define SPI_BAUD 4000000			// led matrix baud rate
#define ENABLE_DEBUG 0				// when true, stops all calls to pigpio (this disables leds!)
#define NUM_INPUTS 3				// number of inputs for pull-chains on sculpture
#define NETWORK_SIZE 3				// number of pis on Tripdeck network (TODO: evaluate necessity of this)

// Led Settings
// * dont modify these values *
#define MAIN_LEDS 0
#define CENTRE_LEDS 1
#define NO_LEDS 2

// modify these values
#define LED_SETTING NO_LEDS
#define PIXEL_BRIGHTNESS 31
#define RUN_LEDS (LED_SETTING != NO_LEDS)

// variable LED settings for different grids
#if (LED_SETTING == MAIN_LEDS)		// main led grid
#define LED_MATRIX_WIDTH 50
#define LED_MATRIX_HEIGHT 50
#define LED_MATRIX_SPLIT 25
#define LED_GRID_CONFIGURATION_OPTION_A Apa102::VerticalTopLeft		// configuration of first led grid in main light chamber
#define LED_GRID_CONFIGURATION_OPTION_B Apa102::VerticalTopRight	// configuration of second led grid in main light chamber
#define GRID_AB_ORIENTATION LedController::Horizontal				// orientation of grid A and grid B (Horizontal -> A B -- Vertical -> A / B)
#elif (LED_SETTING == CENTRE_LEDS)	 // centre led grid
#define LED_MATRIX_WIDTH 25
#define LED_MATRIX_HEIGHT 30
#define LED_GRID_CONFIGURATION_OPTION Apa102::VerticalTopLeft
#define GRID_AB_ORIENTATION LedController::None
// do nothing						// no leds
#endif // LED_SETTINGS

// Media Folders
#define VIDEO_STARTUP_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/startup/"
#define VIDEO_WAIT_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/wait/"
#define VIDEO_PULLED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/pulled/"
#define VIDEO_REVEAL_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/reveal/"

#define LED_STARTUP_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/startup/"
#define LED_WAIT_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/wait/"
#define LED_PULLED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/pulled/"
#define LED_REVEAL_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/reveal/"

#endif // _SETTINGS_H_