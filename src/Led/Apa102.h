#ifndef _APA102_H_
#define _APA102_H_

#include <functional>
#include <algorithm>
#include <pigpio.h>
#include <exception>

#define NUM_LEDS 300
#define ACTIVE_LEDS 60

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
		int init();

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
		 * @brief sets values for pixel at point
		 */
		void setPixel(Pixel pixel, Point point);

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
			uint32_t _endframeLength;
			uint32_t _x;
			uint32_t _y;
			uint8_t* _spiBuffer;
			uint32_t _spiBufferLength = 0;

			void _doFillAction(Point p1, Point p2, std::function<void(uint8_t*)> action);

			uint32_t _getIndexFromPoint(Point point);

			void _writeEndframe();

			void _writePixelToBuffer(Pixel pixel, uint32_t i);

			void _assertPointInRange(Point point);

			static uint32_t _calculateEndframe(uint32_t);
};

// struct APA102 {
// 	int n_leds;
// };

// struct APA102_Frame {
//   uint8_t r, g, b, brightness;
// };
// struct APA102_Animation;

// //APA102_MakeLED(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b): Creates a LED-frame
// struct APA102_Frame APA102_CreateFrame(uint8_t, uint8_t, uint8_t, uint8_t);

// //APA102_Init(int n_leds): Initializes a LED-strip with the given number of LEDS
// struct APA102 APA102_Init(int);

// /*
//   Base API functions for controlling single LEDs
// */
// //APA102_Begin(): Begin a write
// void APA102_Begin();
// //APA102_End(): End a write
// void APA102_End();
// //APA102_WriteLED(APA102_Frame): Write a single LED frame
// void APA102_WriteLED(uint32_t, struct APA102_Frame);
// // Turn off all leds
// void APA102_Clear();

// void APA102_WriteLEDSegment(uint32_t, uint32_t, struct APA102_Frame);

// /*
//   'high-level' functions for controlling the entire strip
// */
// //APA102_Fill(APA102* strip, APA102_Frame led): Fill the entire strip with a single color
// void APA102_Fill(struct APA102, struct APA102_Frame);
// //APA102_Stripes(APA102* strip, APA102_Frame led, int stripe_size, int gap_size, int offset): Fill the entire strip with stripes and holes in them
// void APA102_Stripes(struct APA102*, struct APA102_Frame*, int, int, int);
// //APA102_MultiStripes(APA102* strip, APA102_Frame* leds, int stripe_size, int gap_size, int offset, int color_offset): Fill the intire strip with multi-colored stripes and holes in them. Colors are contained in a NULL-terminated array
// void APA102_MultiStripes(struct APA102*, struct APA102_Frame**, int, int, int, int);

// /*
//   Animation functions
// */
// //APA102_KillAnimation(struct APA102_Animation* anim): Kills a running animation
// void APA102_KillAnimation(struct APA102_Animation*);
// //APA102_BlinkAnimation(APA102* strip, APA102_Frame* led, int interval): A blinking animation using the given color and interval
// struct APA102_Animation* APA102_BlinkAnimation(struct APA102*, struct APA102_Frame*, int);
// //APA102_PulseAnimation(APA102* strip, APA102_Frame* led, int interval): A pulsing animation using the given color and interval
// struct APA102_Animation* APA102_PulseAnimation(struct APA102*, struct APA102_Frame*, int);
// //APA102_StripesAnimation(APA102* strip, APA102_Frame* led, int interval, int stripe_size, int gap_size, int direction)
// struct APA102_Animation* APA102_StripesAnimation(struct APA102*, struct APA102_Frame*, int, int, int, int);
// //APA102_MultiStripesAnimation(APA102* strip, APA102_Frame** led, int interval, int stripe_size, int gap_size, int direction)
// struct APA102_Animation* APA102_MultiStripesAnimation(struct APA102*, struct APA102_Frame**, int, int, int, int);
// //APA102_FadeAnimation(APA102* strip, APA102_Frame** leds, int interval)
// struct APA102_Animation* APA102_FadeAnimation(struct APA102*, struct APA102_Frame**, int);

#endif