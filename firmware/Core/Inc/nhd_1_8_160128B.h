#ifndef __NHD_1_8_160128B_H__
#define __NHD_1_8_160128B_H__

#include <stdint.h>

#define WHITE   0xFFFF
#define RED     0xF800
#define ORANGE  0xFBE0
#define YELLOW  0xFFE0
#define GREEN   0x07E0
#define BLUE    0x001F
#define VIOLET  0x781F
#define BLACK   0x0000

void nhd_160128b_DisplayImage(uint16_t *img, uint32_t img_w, uint32_t img_h);
void nhd_160128b_FillScreen(uint16_t a);
void nhd_160128b_init(void);
void nhd_160128b_test(void);

#endif  /* !__NHD_1_8_160128B_H__ */
