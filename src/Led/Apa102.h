#ifndef _APA102_H_
#define _APA102_H_

#include <string>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include <pigpio.h>

struct Pixel {
	uint8_t brightness;
    uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct Point {
	uint32_t x;
	uint32_t y;
};

struct Shape {
	uint32_t width;
	uint32_t height;
	const Pixel* map;
};

class Apa102 {
	public:
		/**
		 * @brief constructs Apa102 object
		 * @note strip is treated as an x * y matrix of numLeds * 1
		*/
		Apa102(uint32_t numLeds);

		/**
		 * @brief constructs Apa102 object with led matrix of x * y
		*/
		Apa102(uint32_t x, uint32_t y);

		/**
		 * @brief frees memory allocated for buffer
		 */
		~Apa102();

		/**
		 * @brief opens pigpio SPI and initializes led buffer
		*/
		void init(uint32_t spiChan, uint32_t baud, uint32_t spiFlags);

		/**
		 * @brief displays current pixels in matrix
		 */
		void show();

		/**
		 * @brief turns off all leds in matrix
		 */
		void clear();

		/**
		 * @brief clears all pixels between p1 and p2
		 */
		void clear(Point p1, Point p2);

		/**
		 * @brief sets led by index, regardless of grid
		*/ 
		void setPixel(const Pixel& pixel, uint32_t led);

		/**
		 * @brief sets values for pixel at point
		 */
		void setPixel(const Pixel& pixel, const Point& point);

		/**
		 * @brief draws a pixel bitmap from top left corner
		 * 
		 */
		void drawShape(Point topLeft, const Shape& shape);

		/**
		 * @brief fills area from p1 to p2
		 */
		void fillArea(Pixel pixel, Point p1, Point p2);

		/**
		 * @brief changes number of active leds
		 * @note changing this value will cause buffer to be resized
		 */
		void setActiveLeds(uint32_t);

		/**
		 * @brief gets the number of active leds in the matrix
		 */
		uint32_t getActiveLeds();

		/**
		 * @brief gets the number of leds in the matrix
		 */
		uint32_t getNumLeds();

		private:
			uint32_t _numLeds;
			uint32_t _activeLeds;
			int32_t _handle = -1;
			uint32_t _endframeLength;
			uint32_t _x;
			uint32_t _y;
			uint8_t* _spiBuffer;
			uint32_t _spiBufferLength = 0;

			void _doFillAction(Point& p1, Point& p2, std::function<void(uint8_t*)> action);

			uint32_t _getIndexFromPoint(const Point& point);

			// uint32_t _getIndexFromPoint(const uint32_t& x, const uint32_t& y);

			void _writeEndframe();

			void _assertPointInRange(const Point& point);

			static uint32_t _calculateEndframe(uint32_t);
};

#endif