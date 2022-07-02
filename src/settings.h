#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <termios.h>
#include "Apa102.h"
#include "LedController.h"

// Debug
#define ENABLE_VISUAL_DEBUG 0		// when true, stops all calls to pigpio (only necessary if initializing leds)
#define ENABLE_SERIAL_DEBUG 1		// when true, prints out serial read/write data
#define ENABLE_MEDIA_DEBUG 1	// when true, prints out messages regarding file system state

/////////////////////////////////////
// Trypdeck

// Leader or Follower
#define Follower

// IDs
#define ID '3'							// change this setting for different followers: each follower on the network MUST have a unique id
#define LEADER_ID '0'					// do not change this ID

// Timing -- Leader only
#define STARTUP_TIME 20000				// duration for pis to sync up and play startup animation
#define PULL_TO_PRE_REVEAL_TIME 5000	// amount of time between chain pull (on any pi) and pre-reveal state
#define PRE_REVEAL_TO_REVEAL_TIME 1500	// amount of time between pre-reveal and reveal state
#define REVEAL_TIME 10000				// duration for which tarot cards will be displayed before returning to wait state

// Timing -- Leader and Follower
#define DEFAULT_PING_INTERVAL 1000		// intervals on which Leader / Follower ping one another
#define LED_WAIT_TIME 0					// Led decodes much faster than video, so a short pause may be needed

// GPIO Settings (general)
#define SERIAL_BAUD B38400				// networking baud rate
#define SERIAL_BUFFER_SIZE 32			// largest possible serial message size
#define SPI_BAUD 4000000				// led matrix baud rate
#define NUM_INPUTS 2					// number of inputs for pull-chains on sculpture
#define NUM_FOLLOWERS 2					// number of pis on TripdeckMediaManager network (TODO: evaluate necessity of this)

// Buttons -- Leader only
#define LEADER_BUTTON_ID '0'
#define FOLLOWER_1_BUTTON_ID '3'
#define FOLLOWER_2_BUTTON_ID '4'
#define RESET_BUTTON_ID 'x'
#define SHUTDOWN_BUTTON_ID 'y'
#define LEADER_BUTTON_PIN 1
#define PIN_BUTTON_2 2
#define PIN_BUTTON_3 3
#define RESET_BUTTON_PIN 4
#define SHUTDOWN_BUTTON_PIN 5

// Video Settings
#define RUN_OMX_PLAYER 0

// Led Settings
// * dont modify these values *
#define MAIN_LEDS 0
#define CENTRE_LEDS 1
#define NO_LEDS 2

// * modify these values *
#define LED_SETTING NO_LEDS 				// * important * main led setting
#define PIXEL_BRIGHTNESS 31 				// global pixel brightness

// * dont modify this value *
#define RUN_LEDS (LED_SETTING != NO_LEDS)

// variable LED settings for different grids
#if (LED_SETTING == MAIN_LEDS)		// Top Matrix * do not change this line *
// * modify these values *
#define LED_MATRIX_WIDTH 50
#define LED_MATRIX_HEIGHT 50
#define LED_MATRIX_SPLIT 25
#define GRID_AB_ORIENTATION LedController::Horizontal				// orientation of grid A and grid B (Horizontal -> A B -- Vertical -> A / B)
#define LED_GRID_CONFIGURATION_OPTION_A Apa102::VerticalTopLeft		// configuration of first led grid in main light chamber
#define LED_GRID_CONFIGURATION_OPTION_B Apa102::VerticalTopRight	// configuration of second led grid in main light chamber
#elif (LED_SETTING == CENTRE_LEDS)	 // Centre Matrix * do no change this line *
// * modify these values *
#define LED_MATRIX_WIDTH 10
#define LED_MATRIX_HEIGHT 50
#define LED_GRID_CONFIGURATION_OPTION Apa102::VerticalBottomRight
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