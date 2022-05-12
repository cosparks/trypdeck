#include "Apa102.h"

Apa102::Apa102(uint32_t numLeds) : _numLeds(numLeds), _activeLeds(numLeds), _x(numLeds), _y(1) { }

Apa102::Apa102(uint32_t x, uint32_t y) : _x(x), _y(y) {
	_numLeds = x * y;
	_activeLeds = _numLeds;
}

Apa102::~Apa102() {
	delete[] _spiBuffer;
}

void Apa102::init(uint32_t spiChan, uint32_t baud, uint32_t spiFlags) {
	_endframeLength = _calculateEndframe(_activeLeds);
	_spiBufferLength = 4 + (_activeLeds * 4) + _endframeLength;
	_spiBuffer = new uint8_t[_spiBufferLength];

	_writeEndframe();
	
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

void Apa102::setPixel(Pixel pixel, Point point) {
	_assertPointInRange(point);

	uint32_t i = _getIndexFromPoint(point);
	_spiBuffer[i] = 0b11100000 | (0b00011111 & pixel.brightness);
	_spiBuffer[i + 1] = pixel.g;
	_spiBuffer[i + 2] = pixel.b;
	_spiBuffer[i + 3] = pixel.r;
}

void Apa102::fillArea(Pixel pixel, Point p1, Point p2) {
	_doFillAction(p1, p2, [pixel](uint8_t* buf) -> void {
		buf[0] = 0b11100000 | (0b00011111 & pixel.brightness);
		buf[1] = pixel.g;
		buf[2] = pixel.b;
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

void Apa102::_doFillAction(Point p1, Point p2, std::function<void(uint8_t*)> action) {
	_assertPointInRange(p1);
	_assertPointInRange(p2);

	uint8_t left = std::min(p1.x, p2.x);
	uint8_t right = std::max(p1.x, p2.x);
	uint8_t bottom = std::min(p1.y, p2.y);
	uint8_t top = std::max(p1.y, p2.y);

	for (int x = left; x <= right; x++) {
		for (int y = bottom; y <= top; y++) {
			int index = 4 + (x * _y + y) * 4;
			action(&_spiBuffer[index]);
		}
	}
}

uint32_t Apa102::_getIndexFromPoint(Point point) {
	return 4 + (point.x * _y + point.y) * 4;
}

void Apa102::_assertPointInRange(Point point) {
	if (point.x > _x || point.y > _y) {
		throw std::runtime_error(std::string("Apa102 Error: Point out of range!"));
	}
}

void Apa102::_writeEndframe() {
	for (uint32_t i = _spiBufferLength - _endframeLength; i < _spiBufferLength; i++) {
		_spiBuffer[i] = 0x1;
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