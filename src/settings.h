#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <termios.h>
#include "Apa102.h"
#include "LedController.h"

// Leader or Follower
#define Leader

// IDs
#define ID '0'						// change this setting for different followers: each follower on the network MUST have a unique id
#define LEADER_ID '0'				// do not change this ID

// Timing -- Leader only
#define STARTUP_TIME 20000			// duration for pis to sync up and play startup animation
#define PULL_DEBOUNCE_TIME 2000		// duration within which other people can still pull chain before pulled animation starts
#define PULL_TO_PRE_REVEAL 6000		// amount of time between chain pull (on any pi) and pre-reveal state
#define PRE_REVEAL_TO_REVEAL 800	// amount of time between pre-reveal and reveal state
#define REVEAL_TIME 20000			// duration for which tarot cards will be displayed before returning to wait state

// GPIO Settings (general)
#define SERIAL_BAUD B9600			// networking baud rate
#define SERIAL_BUFFER_SIZE 64		// largest possible serial message size
#define SPI_BAUD 4000000			// led matrix baud rate
#define NUM_INPUTS 2				// number of inputs for pull-chains on sculpture
#define NUM_FOLLOWERS 1				// number of pis on TripdeckMediaManager network (TODO: evaluate necessity of this)

// Buttons -- Leader only
#define LEADER_BUTTON_ID '0'
#define FOLLOWER_1_BUTTON_ID '1'
#define FOLLOWER_2_BUTTON_ID '2'
#define RESET_BUTTON_ID 'x'
#define LEADER_BUTTON_PIN 1
#define PIN_BUTTON_2 2
#define PIN_BUTTON_3 3
#define RESET_BUTTON_PIN 4

// debug
#define ENABLE_VISUAL_DEBUG 0		// when true, stops all calls to pigpio (only necessary if initializing leds)
#define ENABLE_SERIAL_DEBUG 1		// when true, prints out serial read/write data

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
#else
// do nothing						// no leds
#endif // LED_SETTINGS

// Media Folders
#define VIDEO_CONNECTING_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/connecting/"
#define VIDEO_CONNECTED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/connected/"
#define VIDEO_WAIT_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/wait/"
#define VIDEO_PULLED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/pulled/"
#define VIDEO_REVEAL_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/video/reveal/"

#define LED_CONNECTING_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/connecting/"
#define LED_CONNECTED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/connected/"
#define LED_WAIT_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/wait/"
#define LED_PULLED_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/pulled/"
#define LED_REVEAL_DIRECTORY "/home/trypdeck/projects/tripdeck_basscoast/media/led/reveal/"

#endif // _SETTINGS_H_