#ifndef _APA102_H_
#define _APA102_H_

#include <functional>

struct Pixel {
	uint8_t brightness;
    uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct Point {
	int32_t x;
	int32_t y;
};

struct Shape {
	int32_t width;
	int32_t height;
	const Pixel* map;
};

/**
 * @brief Simple class for controlling Apa102 strip or grid using PIGPIO
 * @note you MUST call init() before making any other method calls
 */
class Apa102 {
	public:
		enum GridConfigurationOption { HorizontalTopLeft, HorizontalTopRight, HorizontalBottomRight, HorizontalBottomLeft,
			VerticalTopLeft, VerticalTopRight, VerticalBottomRight, VerticalBottomLeft,};

		/**
		 * @brief constructs Apa102 object
		 * @note strip will be treated as an x * y matrix of numLeds * 1, where {0,0} is the first led from SPI input
		*/
		Apa102(uint32_t numLeds);

		/**
		 * @brief constructs Apa102 object with led matrix of x * y.
		 * @param x width of grid
		 * @param y height of grid
		 * @param orientationOption the orientation of led strips which form the grid
		*/
		Apa102(int32_t x, int32_t y, GridConfigurationOption orientationOption);

		/**
		 * @brief frees memory allocated for buffer
		 */
		~Apa102();

		/**
		 * @brief opens pigpio SPI, sets led grid configuration and initializes led buffer
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
		void setPixel(const Pixel& pixel, uint32_t i);

		/**
		 * @brief sets values for pixel at point
		 */
		void setPixel(const Pixel& pixel, const Point& point);

		/**
		 * @brief draws a pixel bitmap from top left corner
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
			int32_t _x;
			int32_t _y;
			GridConfigurationOption _configurationOption;
			uint32_t (Apa102::*_getIndexFromPoint)(const Point&);
			// SPI
			uint32_t _endframeLength;
			int32_t _handle = -1;
			uint8_t* _spiBuffer;
			uint32_t _spiBufferLength = 0;

			void _doFillAction(Point& p1, Point& p2, std::function<void(uint8_t*)> action);
			void _writeEndframe();
			void _assertPointInRange(const Point& point);
			void _assignIndexGetterFromOptions();
			static uint32_t _calculateEndframe(uint32_t);

			// index getters for different led configurations:
			//		each possible led configuration has a separate method which will be assigned to _getIndexFromPoint--
			//		this is an optimization to avoid branching, as _getIndexFromPoint will likely be called hundreds of
			//		thousands if not millions of times per second
			uint32_t _getIndexFromPoint_HorizontalTopLeft(const Point& point);
			uint32_t _getIndexFromPoint_HorizontalTopRight(const Point& point);
			uint32_t _getIndexFromPoint_HorizontalBottomLeft(const Point& point);
			uint32_t _getIndexFromPoint_HorizontalBottomRight(const Point& point);
			uint32_t _getIndexFromPoint_VerticalTopLeft(const Point& point);
			uint32_t _getIndexFromPoint_VerticalTopRight(const Point& point);
			uint32_t _getIndexFromPoint_VerticalBottomLeft(const Point& point);
			uint32_t _getIndexFromPoint_VerticalBottomRight(const Point& point);
};

#endif