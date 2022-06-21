#ifndef _LED_CONTROLLER_H_
#define _LED_CONTROLLER_H_

#include "Apa102.h"

/**
 * @brief Led Controller which controls two Apa102 pixel grids via the raspberry pi's two SPI busses
 */
class LedController {
    public:
        enum SplitConfigurationOption { Horizontal, Vertical };
        LedController(int32_t width, int32_t height, int32_t centre, SplitConfigurationOption option);
        ~LedController();
        void init(uint32_t spiBaud, Apa102::GridConfigurationOption optionA, Apa102::GridConfigurationOption optionB);
        void clear();
        void show();
        void setPixel(const Pixel& pixel, const Point& point);

    private:
        Apa102* _ledGridA;
        Apa102* _ledGridB;
        int32_t _width;
        int32_t _height;
        int32_t _centre;
        SplitConfigurationOption _option;

        inline void _setPixelHorizontal(const Pixel& pixel, const Point& point);
        inline void _setPixelVertical(const Pixel& pixel, const Point& point);
        inline void _clear();
        inline void _show();
};

#endif