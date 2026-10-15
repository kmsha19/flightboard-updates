#pragma once
#define PROGMEM
inline unsigned short pgm_read_word(const unsigned short* p){return *p;}
inline unsigned char pgm_read_byte(const unsigned char* p){return *p;}
