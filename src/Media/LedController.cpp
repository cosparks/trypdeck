#include <stdexcept>

#include "settings.h"
#include "LedController.h"

LedController::LedController(int32_t width, int32_t height, int32_t centre, SplitConfigurationOption option,
    Apa102::GridConfigurationOption gridOptionA, Apa102::GridConfigurationOption gridOptionB) : 
        _width(width), _height(height), _centre(centre), _splitOption(option), _gridOptionA(gridOptionA), _gridOptionB(gridOptionB) { }

LedController::~LedController() {
    delete _ledGridA;
    delete _ledGridB;
}

void LedController::init(uint32_t spiBaud) {
    switch (_splitOption) {
        case Horizontal:
            _ledGridA = new Apa102(_centre, _height, _gridOptionA);
            _ledGridB = new Apa102(_width - _centre, _height, _gridOptionB);
            break;
        case Vertical:
            _ledGridA = new Apa102(_width, _centre, _gridOptionA);
            _ledGridB = new Apa102(_width, _height - _centre, _gridOptionB);
            break;
        default:
            _ledGridA = new Apa102(_width, _height, _gridOptionA);
            _ledGridB = NULL;
            break;
    }

    _ledGridA->init(0, SPI_BAUD, 0);
    if (_splitOption != None)
        _ledGridB->init(1, SPI_BAUD, 0);

    _clear();
    _show();
}

void LedController::clear() {
    _clear();
}

void LedController::show() {
    _show();
}

void LedController::setPixel(const Pixel& pixel, const Point& point) {
    switch (_splitOption) {
        case Horizontal:
            _setPixelHorizontal(pixel, point);
            break;
        case Vertical:
            _setPixelVertical(pixel, point);
            break;
        default:
            _ledGridA->setPixel(pixel, point);
            break;
    }
}

void LedController::_clear() {
    _ledGridA->clear();
    if (_ledGridB)
        _ledGridB->clear();
}

void LedController::_show() {
    _ledGridA->show();
    if (_ledGridB)
        _ledGridB->show();
}

void LedController::_setPixelHorizontal(const Pixel& pixel, const Point& point) {
    if (point.x <_centre) {
        _ledGridA->setPixel(pixel, point);
    } else {
        _ledGridB->setPixel(pixel, Point { point.x - _centre, point.y });
    }
}


void LedController::_setPixelVertical(const Pixel& pixel, const Point& point) {
    if (point.y <_centre) {
        _ledGridA->setPixel(pixel, point);
    } else {
        _ledGridB->setPixel(pixel, Point { point.x, point.y - _centre });
    }
}