#include "Apa102.h"

struct APA102_Frame APA102_CreateFrame(uint8_t brightness, uint8_t r, uint8_t g, uint8_t b) {
    struct APA102_Frame led = {
		.r = r,
		.g = g,
		.b = b,
		.brightness = brightness,
	};
    return led;
}

struct APA102 APA102_Init(int n_leds) {
    struct APA102 strip = {
		.n_leds = n_leds
	};

    if(spiOpen(0, 6000000, 0) < 0) {
        printf("WiringPiSPISetup failed\n");
    }
    return strip;
}

void APA102_Begin() {
    uint8_t buf[1];
    int i;

    for(i = 0; i < 4; i++) {
        buf[0] = 0x00;
        spiWrite(0, (char*)buf, 1);
    }
}

void APA102_End() {
    uint8_t buf[1];
    int i;

    for(i = 0; i < 4; i++) {
        buf[0] = 0xFF;
        spiWrite(0, (char*)buf, 1);
    }
}

void APA102_WriteLED(uint32_t position, struct APA102_Frame led) {
    uint8_t led_frame[position * 4 + 8] = {};
	// int randR = rand() % 256;
	// int randG = rand() % 256;
	// int randB = rand() % 256;
    if(led.brightness > 31) {
        led.brightness = 31;
    }

	// FILL W/ COLOUR
	// uint32_t i;
	// for (i = 4; i <= (position * 4 + 4); i += 4) {
	// 	led_frame[i] = 0b11100000 | (0b00011111 & led.brightness);
	// 	led_frame[i+1] = led.b;
	// 	led_frame[i+2] = led.g;
	// 	led_frame[i+3] = led.r;
	// }

	// CLEAR
	uint32_t i;
	for (i = 4; i <= (position * 4 + 4); i += 4) {
		led_frame[i] = 0b11100000;
	}

	// uint32_t i = (position * 4 + 4);
	led_frame[i] = 0b11100000 | (0b00011111 & led.brightness);
	led_frame[i+1] = led.b;
	led_frame[i+2] = led.g;
	led_frame[i+3] = led.r;

	// terminating byte
	led_frame[i+4] = 1;
	led_frame[i+5] = 1;
	led_frame[i+6] = 1;
	led_frame[i+7] = 1;

    spiWrite(0, (char*)led_frame, i + 4);
}

void APA102_Fill(struct APA102 strip, struct APA102_Frame led) {
    uint8_t led_frame[1024];
    int i;

    if(led.brightness > 31) {
        led.brightness = 31;
    }

    // APA102_Begin();
	for (i = 0; i <= 4; i++) {
		led_frame[i] = 0x0;
	}

    for(i = 4; i < strip.n_leds; i+=4) {
        led_frame[i] = 0b11100000 | (0b00011111 & led.brightness);
        led_frame[i + 1] = led.b;
        led_frame[i + 2] = led.g;
        led_frame[i + 3] = led.r;
    }

	for (; i <= strip.n_leds + 4; i++) {
		led_frame[i] = 0xFF;
	}

	spiWrite(0, (char*)led_frame, i);
    // APA102_End();
}

void APA102_Stripes(struct APA102* strip, struct APA102_Frame* led, int stripe_size, int gap_size, int offset) {
    uint8_t led_frame[4];
    int i, ctr;

    ctr = offset;
    if(ctr < 0) {
        ctr = 0;
    }

    while(ctr > gap_size + stripe_size) {
        ctr -= gap_size+stripe_size;
        if(ctr < 0) {
            ctr = 0;
        }
    }

    if(led->brightness > 31) {
        led->brightness = 31;
    }

    APA102_Begin();
    for(i = 0; i < strip->n_leds; i++) {
        if(ctr < stripe_size) {
            led_frame[0] = 0b11100000 | (0b00011111 & led->brightness);
            led_frame[1] = led->b;
            led_frame[2] = led->g;
            led_frame[3] = led->r;
        } else {
            led_frame[0] = 0b11100000;
            led_frame[1] = 0x00;
            led_frame[2] = 0x00;
            led_frame[3] = 0x00;
        }

        spiWrite(0, (char*)led_frame, 4);

        ctr++;
        if(ctr >= stripe_size + gap_size) {
            ctr = 0;
        }
    }
    APA102_End();
}

void APA102_MultiStripes(struct APA102* strip, struct APA102_Frame** leds, int stripe_size, int gap_size, int offset, int coffset) {
    uint8_t led_frame[4];
    int i, ctr, cctr, clen;
    struct APA102_Frame* ref;

    ref = leds[0];
    clen = 0;
    cctr = 0;

    while(1) {
        clen++;
        ref = leds[clen];
        if(ref == 0) {
            break;
        }
    }

    cctr = coffset;

    if(clen == 0) {
        printf("APA102_MultiStripes Error: leds must contain at least one color\n");
    }

    ctr = offset;
    if(ctr < 0) {
        ctr = 0;
    }

    while(ctr > gap_size + stripe_size) {
        ctr -= gap_size+stripe_size;
        if(ctr < 0) {
            ctr = 0;
        }
    }

    APA102_Begin();
    for(i = 0; i < strip->n_leds; i++) {

        if(ctr < stripe_size) {
            led_frame[0] = 0b11100000 | (0b00011111 & leds[cctr]->brightness);
            led_frame[1] = leds[cctr]->b;
            led_frame[2] = leds[cctr]->g;
            led_frame[3] = leds[cctr]->r;
        } else {
            led_frame[0] = 0b11100000;
            led_frame[1] = 0x00;
            led_frame[2] = 0x00;
            led_frame[3] = 0x00;
        }

        spiWrite(0, (char*)led_frame, 4);

        ctr++;
        if(ctr >= stripe_size + gap_size) {
            ctr = 0;
            cctr++;
                if(cctr == clen) {
                    cctr = 0;
                }
        }
    }
    APA102_End();
}