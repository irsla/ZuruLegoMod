#ifndef __GIF_DECODER_H__
#define __GIF_DECODER_H__

#include <stdint.h>
#include "app_filex.h"

#define SIGNATURE	"GIF"
#define VERSION		"89a"
#define LBL_GCE		0xF9
#define LBL_APP		0xFF
#define LBL_COMMENT	0xFE
#define BLOCK_IMG	0x2C
#define BLOCK_EXT	0x21
#define BLOCK_END	0x3B

#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct table_entry_s {
  int prev;
  int cur;
} table_entry_t;

typedef struct color_s {
  uint8_t	r;
  uint8_t	g;
  uint8_t	b;
} color_t;

typedef struct gif_palette_s {
  int		size;
  color_t	*colors;
} gif_palette_t;

typedef struct gif_s {
  void		*handler;
  unsigned int (*read)(void *handler, uint8_t* buf, uint32_t buf_size, unsigned long *real_size);
  uint8_t	*frame;
  uint32_t	frame_num;
  gif_palette_t palette;
  int		gct_sz;
  uint16_t	width;
  uint16_t	height;
  uint16_t	depth;
  uint8_t	fdsz;
  uint8_t	bgidx;
  uint8_t	aspect;
} gif_t;

typedef struct transparent_flags_s {
  uint8_t	enable:1;
  uint8_t	user_input:1;
  uint8_t	disposal:3;
  uint8_t	res0:3;
} transparent_flags_t;

typedef struct gif_gce_s {
  uint16_t		delay;
  uint8_t		data_len;
  transparent_flags_t	transparent_flags;
  uint8_t		transparent_color_idx;
} gif_gce_t;


typedef struct gif_image_descr_flags_s {
  uint8_t lcl_colr_tbl_size:3;	//Size of Local Color Table     3 Bits
  uint8_t res0:2;		//Reserved                      2 Bits
  uint8_t snort:1;		//Sort Flag                     1 Bit
  uint8_t interlace:1;		//Interlace Flag                1 Bit
  uint8_t local_color_table:1;	//Local Color Table Flag        1 Bit  
} gif_image_descr_flags_t;

typedef struct gif_frame_s {
  uint32_t	bit_pos;
  uint8_t	*data;
  gif_gce_t	gce;
  uint16_t	*output;
  uint16_t	x;
  uint16_t	y;
  uint16_t	width;
  uint16_t	height;
  //  uint8_t	img_desc_color_table_bit;
  gif_image_descr_flags_t img_flags;
  gif_palette_t	palette;
} gif_frame_t;

gif_t *gif_init(void *handler, unsigned int (*read)(void *handler, uint8_t* buf, uint32_t buf_size, unsigned long *real_size));
void gif_destroy(gif_t *gif);
int gif_get_hdr(gif_t *gif);
int gif_get_img(gif_t *gif, gif_frame_t *frame);

#endif  /* __GIF_DECODER_H__ */
