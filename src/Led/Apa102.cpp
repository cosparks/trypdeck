#include "Apa102.h"

Apa102::Apa102(uint32_t numLeds) : _numLeds(numLeds), _activeLeds(numLeds), _x(numLeds), _y(1) { }

Apa102::Apa102(uint32_t x, uint32_t y) : _x(x), _y(y) {
	_numLeds = x * y;
	_activeLeds = _numLeds;
}

Apa102::~Apa102() {
	delete[] _spiBuffer;
}

int Apa102::init() {
	_endframeLength = _calculateEndframe(_activeLeds);
	_spiBufferLength = 4 + (_activeLeds * 4) + _endframeLength;
	_spiBuffer = new uint8_t[_spiBufferLength];

	_writeEndframe();

	return spiOpen(0, 6000000, 0);
}

void Apa102::show() {
	spiWrite(0, (char*)_spiBuffer, _spiBufferLength);
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

	for (int x = left; x < right; x++) {
		for (int y = bottom; y < top; y++) {
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
		throw std::runtime_error(std::string("Error: Point out of range!"));
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

// struct APA102_Frame APA102_CreateFrame(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b) {
//     struct APA102_Frame led = {
// 		.r = r,
// 		.g = g,
// 		.b = b,
// 		.brightness = brightness,
// 	};
//     return led;
// }

// void APA102_WriteLED(uint32_t position, struct APA102_Frame led) {
// 	uint32_t endFrameLength = (((position) >> 1) / 8 + ((position) >> 1) % 8);
// 	endFrameLength = endFrameLength < 4 ? 4 : (endFrameLength + endFrameLength % 4);
// 	uint32_t size = (position + 1) * 4 + 4 + endFrameLength;
//     uint8_t led_frame[size] = {};
	
//     if(led.brightness > 31) {
//         led.brightness = 31;
//     }

// 	// Fill with color
// 	// uint32_t i;
// 	// for (i = 4; i < (position * 4 + 4); i += 4) {
// 	// 	led_frame[i] = 0b11100000 | (0b00011111 & led.brightness);
// 	// 	led_frame[i+1] = led.b;
// 	// 	led_frame[i+2] = led.g;
// 	// 	led_frame[i+3] = led.r;
// 	// }

// 	// Clear everything up to position
// 	uint32_t i;
// 	for (i = 4; i < (position * 4 + 4); i += 4) {
// 		led_frame[i] = 0b11100000;
// 	}

// 	// Fill led at position
// 	led_frame[i] = 0b11100000 | (0b00011111 & led.brightness);
// 	led_frame[i+1] = led.b;
// 	led_frame[i+2] = led.g;
// 	led_frame[i+3] = led.r;

// 	// Build End Frame    (I = N * 4 + 4, where N is the position)
// 	//	 0    1    2    3        4    5    6    7			 I    I+1  I+2  I+3      I+4  I+...
// 	// ( 0x00 0x00 0x00 0x00 ) ( 0x0A 0x00 0x00 0x00 ) ... ( 0xFF 0xFF 0x00 0x00 ) ( 0xFF ... )
// 	for (i = i + 4; i < size; i += 4) {
// 		led_frame[i] = 1;
// 		led_frame[i+1] = 1;
// 		led_frame[i+2] = 1;
// 		led_frame[i+3] = 1;
// 	}

// 	spiWrite(0, (char*)led_frame, i);
// }

// void APA102_WriteLEDSegment(uint32_t start, uint32_t end, struct APA102_Frame led) {
// 	uint32_t endFrameLength = (((end) >> 1) / 8 + ((end) >> 1) % 8);
// 	endFrameLength = endFrameLength < 4 ? 4 : (endFrameLength + endFrameLength % 4);
// 	uint32_t size = (end + 1) * 4 + 4 + endFrameLength;
//     uint8_t led_frame[size] = {};
	
//     if(led.brightness > 31) {
//         led.brightness = 31;
//     }

// 	// Clear everything up to start
// 	uint32_t i;
// 	for (i = 4; i < (start * 4 + 4); i += 4) {
// 		led_frame[i] = 0b11100000;
// 	}

// 	// Fill segment
// 	for (; i < (end * 4 + 4); i += 4) {
// 		led_frame[i] = 0b11100000 | (0b00011111 & led.brightness);
// 		led_frame[i+1] = led.b;
// 		led_frame[i+2] = led.g;
// 		led_frame[i+3] = led.r;
// 	}

// 	// fill end frame
// 	for (; i < size; i += 4) {
// 		led_frame[i] = 1;
// 		led_frame[i+1] = 1;
// 		led_frame[i+2] = 1;
// 		led_frame[i+3] = 1;
// 	}

// 	spiWrite(0, (char*)led_frame, i);
// }

// void APA102_Clear() {
// 	uint32_t endFrameLength = (((NUM_LEDS) >> 1) / 8 + ((NUM_LEDS) >> 1) % 8);
// 	endFrameLength = endFrameLength < 4 ? 4 : (endFrameLength + endFrameLength % 4);
// 	uint32_t size = NUM_LEDS * 4 + endFrameLength + 4;
//     uint8_t led_frame[size] = {};

// 	// Fill with color
// 	uint32_t i;
// 	for (i = 4; i < size; i += 4) {
// 		led_frame[i] = 0b11100000;
// 	}

// 	spiWrite(0, (char*)led_frame, size);
// }

// void APA102_Fill(struct APA102 strip, struct APA102_Frame led) {
//     uint8_t led_frame[1024];
//     int i;

//     if(led.brightness > 31) {
//         led.brightness = 31;
//     }

//     // APA102_Begin();
// 	for (i = 0; i <= 4; i++) {
// 		led_frame[i] = 0x0;
// 	}

//     for(i = 4; i < strip.n_leds; i+=4) {
//         led_frame[i] = 0b11100000 | (0b00011111 & led.brightness);
//         led_frame[i + 1] = led.b;
//         led_frame[i + 2] = led.g;
//         led_frame[i + 3] = led.r;
//     }

// 	for (; i <= strip.n_leds + 4; i++) {
// 		led_frame[i] = 0xFF;
// 	}

// 	spiWrite(0, (char*)led_frame, i);
//     // APA102_End();
// }

// void APA102_Stripes(struct APA102* strip, struct APA102_Frame* led, int stripe_size, int gap_size, int offset) {
//     uint8_t led_frame[4];
//     int i, ctr;

//     ctr = offset;
//     if(ctr < 0) {
//         ctr = 0;
//     }

//     while(ctr > gap_size + stripe_size) {
//         ctr -= gap_size+stripe_size;
//         if(ctr < 0) {
//             ctr = 0;
//         }
//     }

//     if(led->brightness > 31) {
//         led->brightness = 31;
//     }

//     APA102_Begin();
//     for(i = 0; i < strip->n_leds; i++) {
//         if(ctr < stripe_size) {
//             led_frame[0] = 0b11100000 | (0b00011111 & led->brightness);
//             led_frame[1] = led->b;
//             led_frame[2] = led->g;
//             led_frame[3] = led->r;
//         } else {
//             led_frame[0] = 0b11100000;
//             led_frame[1] = 0x00;
//             led_frame[2] = 0x00;
//             led_frame[3] = 0x00;
//         }

//         spiWrite(0, (char*)led_frame, 4);

//         ctr++;
//         if(ctr >= stripe_size + gap_size) {
//             ctr = 0;
//         }
//     }
//     APA102_End();
// }

// void APA102_MultiStripes(struct APA102* strip, struct APA102_Frame** leds, int stripe_size, int gap_size, int offset, int coffset) {
//     uint8_t led_frame[4];
//     int i, ctr, cctr, clen;
//     struct APA102_Frame* ref;

//     ref = leds[0];
//     clen = 0;
//     cctr = 0;

//     while(1) {
//         clen++;
//         ref = leds[clen];
//         if(ref == 0) {
//             break;
//         }
//     }

//     cctr = coffset;

//     if(clen == 0) {
//         printf("APA102_MultiStripes Error: leds must contain at least one color\n");
//     }

//     ctr = offset;
//     if(ctr < 0) {
//         ctr = 0;
//     }

//     while(ctr > gap_size + stripe_size) {
//         ctr -= gap_size+stripe_size;
//         if(ctr < 0) {
//             ctr = 0;
//         }
//     }

//     APA102_Begin();
//     for(i = 0; i < strip->n_leds; i++) {

//         if(ctr < stripe_size) {
//             led_frame[0] = 0b11100000 | (0b00011111 & leds[cctr]->brightness);
//             led_frame[1] = leds[cctr]->b;
//             led_frame[2] = leds[cctr]->g;
//             led_frame[3] = leds[cctr]->r;
//         } else {
//             led_frame[0] = 0b11100000;
//             led_frame[1] = 0x00;
//             led_frame[2] = 0x00;
//             led_frame[3] = 0x00;
//         }

//         spiWrite(0, (char*)led_frame, 4);

//         ctr++;
//         if(ctr >= stripe_size + gap_size) {
//             ctr = 0;
//             cctr++;
//                 if(cctr == clen) {
//                     cctr = 0;
//                 }
//         }
//     }
//     APA102_End();
// }