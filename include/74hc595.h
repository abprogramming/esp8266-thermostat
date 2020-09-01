#ifndef __74HC595_H__
#define __74HC595_H__

/**
 * Minimalistic driver for the 74HC595
 * shift register, up to 32 bits
 */

void shift_init(void);
void shift_out(uint32_t value, size_t bits);

#endif
