#ifndef _LED_CONTROLLER_H_
#define _LED_CONTROLLER_H_

#include "Apa102.h"

/**
 * @brief Led Controller which controls two Apa102 pixel grids via the raspberry pi's two SPI busses
 */
class LedController {
    public:
        enum SplitOrientationOption { Horizontal, Vertical, None };
        LedController(int32_t width, int32_t height, int32_t centre, SplitOrientationOption option,
            Apa102::GridConfigurationOption gridOptionA, Apa102::GridConfigurationOption gridOptionB);
        ~LedController();
        void init(uint32_t spiBaud);
        void clear();
        void show();
        void setPixel(const Pixel& pixel, const Point& point);
		int32_t getWidth();
		int32_t getHeight();

    private:
        Apa102* _ledGridA = NULL;
        Apa102* _ledGridB = NULL;
        int32_t _width;
        int32_t _height;
        int32_t _centre;
        SplitOrientationOption _splitOption;
        Apa102::GridConfigurationOption _gridOptionA;
        Apa102::GridConfigurationOption _gridOptionB;

        inline void _setPixelHorizontal(const Pixel& pixel, const Point& point);
        inline void _setPixelVertical(const Pixel& pixel, const Point& point);
        inline void _clear();
        inline void _show();
};

#endif