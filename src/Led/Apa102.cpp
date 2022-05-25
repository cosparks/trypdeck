#include <iostream>
#include <string>
#include <stdexcept>
#include <pigpio.h>
#include <algorithm>
#include <math.h>

#include "Apa102.h"

Apa102::Apa102(uint32_t numLeds) : _numLeds(numLeds), _activeLeds(numLeds), _x(numLeds), _y(1) {
	// default orientation
	_configurationOption = HorizontalTopLeft;
}

Apa102::Apa102(int32_t x, int32_t y, GridConfigurationOption configurationOption) :
	_x(x), _y(y), _configurationOption(configurationOption) {
	_numLeds = x * y;
	_activeLeds = _numLeds;
}

Apa102::~Apa102() {
	delete[] _spiBuffer;
	spiClose(_handle);
}

void Apa102::init(uint32_t spiChan, uint32_t baud, uint32_t spiFlags) {
	_endframeLength = _calculateEndframe(_activeLeds);
	_spiBufferLength = 4 + (_activeLeds * 4) + _endframeLength;
	_spiBuffer = new uint8_t[_spiBufferLength](); // () initializes array to 0x0
	
	_assignIndexGetterFromOptions();
	_writeEndframe();
	clear();
	
	int result = spiOpen(spiChan, baud, spiFlags);
	if (result < 0) {
		throw std::runtime_error(std::string("Apa102::init Error! Unable to open SPI channel"));
	} else {
		_handle = (uint32_t)result;
	}
}

void Apa102::show() {
	spiWrite(_handle, (char*)_spiBuffer, _spiBufferLength);
}

void Apa102::clear() {
	for (uint32_t i = 4; i < _spiBufferLength - _endframeLength; i += 4) {
		_spiBuffer[i] = 0b11100000;
		_spiBuffer[i + 1] = 0x0;
		_spiBuffer[i + 2] = 0x0;
		_spiBuffer[i + 3] = 0x0;
	}
}

void Apa102::clear(Point p1, Point p2) {
	_doFillAction(p1, p2, [](uint8_t* buf) -> void {
		buf[0] = 0b11100000;
		buf[1] = 0x0;
		buf[2] = 0x0;
		buf[3] = 0x0;
	});
}

void Apa102::setPixel(const Pixel& pixel, uint32_t i) {
	if (i >= _activeLeds) {
		std::string message = "Apa102 Error: led out of active range!  Led " + std::to_string(i) +
			" is outside of active led range " + std::to_string(_activeLeds);
		throw std::runtime_error(message);
	}

	i = (i * 4) + 4;
	_spiBuffer[i] = 0b11100000 | (0b00011111 & pixel.brightness);
	_spiBuffer[i + 1] = pixel.b;
	_spiBuffer[i + 2] = pixel.g;
	_spiBuffer[i + 3] = pixel.r;
}

void Apa102::setPixel(const Pixel& pixel, const Point& point) {
	_assertPointInRange(point);

	uint32_t i = (this->*_getIndexFromPoint)(point);
	_spiBuffer[i] = 0b11100000 | (0b00011111 & pixel.brightness);
	_spiBuffer[i + 1] = pixel.b;
	_spiBuffer[i + 2] = pixel.g;
	_spiBuffer[i + 3] = pixel.r;
}

void Apa102::drawShape(Point topLeft, const Shape& shape) {
	_assertPointInRange(topLeft);
	int32_t shapeY = 0;
	for (int32_t y = topLeft.y; y < topLeft.y + shape.height; y++) {
		int32_t shapeX = 0;
		for (int32_t x = topLeft.x; x < topLeft.x + shape.width; x++) {
			setPixel(shape.map[shapeY * shape.width + shapeX], Point { x % _x, y % _y });
			shapeX++;
		}
		shapeY++;
	}
}

void Apa102::fillArea(Pixel pixel, Point p1, Point p2) {
	_doFillAction(p1, p2, [pixel](uint8_t* buf) -> void {
		buf[0] = 0b11100000 | (0b00011111 & pixel.brightness);
		buf[1] = pixel.b;
		buf[2] = pixel.g;
		buf[3] = pixel.r;
	});
}

void Apa102::setActiveLeds(uint32_t value) {
	_endframeLength = _calculateEndframe(value);
	_activeLeds = value;
	uint32_t newBufferLength = 4 + (4 * value) + _endframeLength;
	uint8_t* temp = new uint8_t[newBufferLength];

	uint32_t minLength = std::min(_spiBufferLength, newBufferLength);
	for (uint32_t i = 0; i < minLength; i++) {
		temp[i] = _spiBuffer[i];
	}

	delete[] _spiBuffer;
	_spiBuffer = temp;
	_spiBufferLength = newBufferLength;

	_writeEndframe();
}

uint32_t Apa102::getActiveLeds() {
	return _activeLeds;
}

uint32_t Apa102::getNumLeds() {
	return _numLeds;
}

void Apa102::_doFillAction(Point& p1, Point& p2, std::function<void(uint8_t*)> action) {
	_assertPointInRange(p1);
	_assertPointInRange(p2);

	int32_t left = std::min(p1.x, p2.x);
	int32_t right = std::max(p1.x, p2.x);
	int32_t bottom = std::min(p1.y, p2.y);
	int32_t top = std::max(p1.y, p2.y);

	for (int32_t x = left; x <= right; x++) {
		for (int32_t y = bottom; y <= top; y++) {
			uint32_t index = (this->*_getIndexFromPoint)(Point {x, y});
			action(&_spiBuffer[index]);
		}
	}
}

void Apa102::_assertPointInRange(const Point& point) {
	if (point.x > _x || point.y > _y) {
		std::string message = "Apa102 Error: Point out of range!  { " + std::to_string(point.x) +
			", " + std::to_string(point.y) + " } is outside of matrix " + std::to_string(_x) + ", " + std::to_string(_y);
		throw std::runtime_error(message);
	}
}

void Apa102::_writeEndframe() {
	for (uint32_t i = _spiBufferLength - _endframeLength; i < _spiBufferLength; i++) {
		_spiBuffer[i] = 0x1;
	}
}

void Apa102::_assignIndexGetterFromOptions() {
	switch (_configurationOption) {
		case HorizontalTopLeft:
			_getIndexFromPoint = &Apa102::_getIndexFromPoint_HorizontalTopLeft;
			break;
		case HorizontalTopRight:
			_getIndexFromPoint = &Apa102::_getIndexFromPoint_HorizontalTopRight;
			break;
		case HorizontalBottomRight:
			_getIndexFromPoint = &Apa102::_getIndexFromPoint_HorizontalBottomRight;
			break;
		case HorizontalBottomLeft:
			_getIndexFromPoint = &Apa102::_getIndexFromPoint_HorizontalBottomLeft;
			break;
		case VerticalTopLeft:
			_getIndexFromPoint = &Apa102::_getIndexFromPoint_VerticalTopLeft;
			break;
		case VerticalTopRight:
			_getIndexFromPoint = &Apa102::_getIndexFromPoint_VerticalTopRight;
			break;
		case VerticalBottomRight:
			_getIndexFromPoint = &Apa102::_getIndexFromPoint_VerticalBottomRight;
			break;
		default:
			_getIndexFromPoint = &Apa102::_getIndexFromPoint_VerticalBottomLeft;
			break;
	}
}

// endframe should be n/2 bits where n is the number of leds in the matrix
// minumum endframe calculated by this method will be 4 bytes or 32 bits
// ex:  for a string of 90 leds we should have at least 45 bits in the endframe
//		 (90 >> 1) = 45;  45 / 8 + 1 = 6;
//		 endframe in this case is 6 bytes == 48 bits
uint32_t Apa102::_calculateEndframe(uint32_t numLeds) {
	bool addOne = ((numLeds >> 1) % 8) != 0;
	uint32_t endFrameLength = ((numLeds >> 1) / 8) + (addOne ? 1 : 0);
	return (endFrameLength < 4) ? 4 : endFrameLength;
}

// Invert x for odd rows from top
uint32_t Apa102::_getIndexFromPoint_HorizontalTopLeft(const Point& point) {
	return 4 + (point.y * _x + fabs((point.y & 0x1) * _x - point.x - (point.y & 0x1))) * 4;
}

// Invert x for even rows from top
uint32_t Apa102::_getIndexFromPoint_HorizontalTopRight(const Point& point) {
	return 4 + (point.y * _x + fabs((~point.y & 0x1) * _x - point.x - (~point.y & 0x1))) * 4;
}

// Invert x for even rows from bottom
uint32_t Apa102::_getIndexFromPoint_HorizontalBottomRight(const Point& point) {
	int32_t relativeY = _y - point.y - 1;
	return 4 + (relativeY * _x + fabs((~relativeY & 0x1) * _x - point.x - (~relativeY & 0x1))) * 4;
}

// Invert x for odd rows from bottom
uint32_t Apa102::_getIndexFromPoint_HorizontalBottomLeft(const Point& point) {
	int32_t y = _y - point.y - 1;
	return 4 + (y * _x + fabs((y & 0x1) * _x - point.x - (y & 0x1))) * 4;
}

// Invert y for odd columns from left
uint32_t Apa102::_getIndexFromPoint_VerticalTopLeft(const Point& point) {
	return 4 + (point.x * _y + fabs((point.x & 0x1) * _y - point.y - (point.x & 0x1))) * 4;
}

// Invert y for odd columns from right
uint32_t Apa102::_getIndexFromPoint_VerticalTopRight(const Point& point) {
	int32_t relativeX = _x - point.x - 1;
	return 4 + (relativeX * _y + fabs((relativeX & 0x1) * _y - point.y - (relativeX & 0x1))) * 4;
}

// Invert y for even columns from right
uint32_t Apa102::_getIndexFromPoint_VerticalBottomRight(const Point& point) {
	int32_t relativeX = _x - point.x - 1;
	return 4 + (relativeX * _y + fabs((~relativeX & 0x1) * _y - point.y - (~relativeX & 0x1))) * 4;
}

// Invert y for even columns from left
uint32_t Apa102::_getIndexFromPoint_VerticalBottomLeft(const Point& point) {
	return 4 + (point.x * _y + fabs((~point.x & 0x1) * _y - point.y - (~point.x & 0x1))) * 4;
}