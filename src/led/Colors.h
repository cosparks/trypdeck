#ifndef _COLORS_H_
#define _COLORS_H_

#include "Apa102.h"

const Pixel Red = { 31, 0xFF, 0x0, 0x0 };
const Pixel Orange = { 31, 0xFF, 0x7F, 0x0 };
const Pixel Yellow = { 31, 0xFF, 0xFF, 0x0 };
const Pixel Green = { 31, 0x0, 0xFF, 0x0 };
const Pixel Blue= { 31, 0x0, 0x0, 0xFF };
const Pixel Indigo= { 31, 0x4B, 0x0, 0x82 };
const Pixel Violet = { 31, 0x94, 0x0, 0xD3 };

const Pixel Black = { 31, 0x0, 0x0, 0x0 };
const Pixel White = { 31, 0xFF, 0xFF, 0xFF };

const Pixel roygbiv[] = { Red, Orange, Yellow, Green, Blue, Indigo, Violet, White };

#endif