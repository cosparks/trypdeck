#include <pigpio.h>
#include <iostream>

#include "Clock.h"
#include "LedController.h"

// TEST PARAMS
#define SPI_BAUD 4000000
#define TEST_PIXEL_BRIGHTNESS 31
#define LED_MATRIX_WIDTH 10
#define LED_MATRIX_HEIGHT 51
#define LED_GRID_CONFIGURATION_OPTION Apa102::VerticalTopLeft
#define GRID_AB_ORIENTATION LedController::None
#define LED_MATRIX_SPLIT 0

#define TEST_LED_INTERVAL 80		// led test update interval
#define TEST_RUN_TIME 60000			// led test duration in milliseconds

bool initializeGpio();

/**
 * @brief This test helps to cofirm that an led grid is configured properly
 *
 * @note Red dots should move along each edge: left, right, top and bottom
 */
int main(int argv, char** argc) {
	if (!initializeGpio()) {
		return -1;
	}

	LedController controller(LED_MATRIX_WIDTH, LED_MATRIX_HEIGHT, LED_MATRIX_SPLIT, GRID_AB_ORIENTATION, LED_GRID_CONFIGURATION_OPTION, LED_GRID_CONFIGURATION_OPTION);
	int64_t lastLedUpdateTime = 0;

	controller.init(SPI_BAUD);

	int32_t x = 0;
	int32_t y = 0;
	bool horizontal = true;

	while (true) {
		int64_t currentTime = Clock::instance().millis();

		if (currentTime >= TEST_RUN_TIME) {
			break;
		}

        if (lastLedUpdateTime + TEST_LED_INTERVAL <= currentTime) {
            controller.clear();
            controller.setPixel(Pixel { TEST_PIXEL_BRIGHTNESS, 0xFF, 0, 0 }, Point { x, y });
            controller.show();

            // move point on either horizontal or vertical edges
            if (horizontal) {
                x = (x + 1) % LED_MATRIX_WIDTH;
                y = ((x == 0) ? LED_MATRIX_HEIGHT - 1 : y);
                if ((y == LED_MATRIX_HEIGHT - 1) && (x == LED_MATRIX_WIDTH - 1)) {
                    horizontal = false;
                    x = 0;
                    y = 0;
                }
            } else {
                y = (y + 1) % LED_MATRIX_HEIGHT;
                x = ((y == 0) ? LED_MATRIX_WIDTH - 1 : x);
                if ((y == LED_MATRIX_HEIGHT - 1) && (x == LED_MATRIX_WIDTH - 1)) {
                    horizontal = true;
                    x = 0;
                    y = 0;
                }

            }

            lastLedUpdateTime = currentTime;
        }
	}

	std::cout << "Test complete.  Returning" << std::endl;
	gpioTerminate();
	return 1;
}

bool initializeGpio() {
	if (gpioInitialise() < 0) {
		std::cout << "PI GPIO Initialization failed" << std::endl;
		return false;
	}
	else {
		std::cout << "PI GPIO Initialization successful" << std::endl;
		return true;
	}
}
