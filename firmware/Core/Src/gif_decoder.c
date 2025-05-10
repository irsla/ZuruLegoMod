#include <stdio.h>
#include "gif_decoder.h"

#define TABLE_MAX_SIZE	(4096)  // lets try 17 bits
table_entry_t	table[TABLE_MAX_SIZE];

uint32_t	table_idx = 0;
uint32_t	clear_id = 0;
uint32_t	end_of_info = 0;

void clear_table(int n) {
  table_idx = 0;

  for (table_idx = 0; table_idx < (1 << (n - 1)); table_idx++) {
    table[table_idx].prev = -1;
    table[table_idx].cur = table_idx;
  }
  // clear
  clear_id = table_idx;
  table[clear_id].prev = -1;
  table[clear_id].cur = table_idx++;
  // end of information
  end_of_info = table_idx;
  table[end_of_info].prev = -1;
  table[end_of_info].cur = table_idx++;
}

int getNbits(gif_frame_t *frame, int n) {
  uint32_t cur_byte = 0;
  uint32_t nb_bits_left_in_byte = 0;
  uint32_t to_take = 0;
  int res = 0;
  uint32_t mask = 0;
  uint32_t taken = 0;

  while (n > 0) {
    cur_byte = frame->bit_pos / 8;
    nb_bits_left_in_byte = 8 - frame->bit_pos % 8;
    to_take = MIN(nb_bits_left_in_byte, n);
    mask = (1 << to_take) - 1;
    res += ((frame->data[cur_byte] >> (8 - nb_bits_left_in_byte)) & mask) << taken;
    n -= to_take;
    taken += to_take;
    frame->bit_pos += to_take;
  }

  return res;
}

uint8_t FirstPixel (int c) {
  uint8_t cur;
  do {
    cur = table[c].cur;
    c = table[c].prev;
  } while (c != -1);
  return cur;
}

int Power2 (int x) {
  return (x & (x - 1)) == 0;
}

void gif_update_output(gif_t *gif, gif_frame_t *frame, uint16_t *output, uint32_t *output_pos, int cur) {
  int i = 0;
  int prev = cur;
  gif_palette_t *palette;
  uint32_t pos;

  palette =  &gif->palette;
  if (frame->palette.size != 0) {
    palette = &frame->palette;
  }

  while (prev != -1) {
    prev = table[prev].prev;
    i++;
  }

  *output_pos += i;
  prev = cur;
  while (prev != -1) {
    uint32_t rgb;

    if (frame->gce.transparent_flags.enable == 0 ||
	table[prev].cur != frame->gce.transparent_color_idx) {
      int r = palette->colors[table[prev].cur].r;
      int g = palette->colors[table[prev].cur].g;
      int b = palette->colors[table[prev].cur].b;
      rgb = ((r >> 3) << 11) | ((g >> 2) << 5) | b >> 3;
      asm("REV16 %0,%1" : "=r"(rgb) : "r"(rgb));

      // compute index in output
      pos = *output_pos  - 1;
      // add empty lines
      uint32_t real_pos = (160 * ((128-gif->height)/2 + frame->y + (pos) / frame->width))  - (frame->x + ((pos) % frame->width)) - ((160 - gif->width) / 2);
      // reverse image
      real_pos = (160*128) - real_pos;
      // store color
      output[real_pos] = rgb;
    }
    *output_pos -= 1;
    prev = table[prev].prev;
  }

  *output_pos +=  i;
}

void gif_lzw_decompress(gif_t *gif, gif_frame_t *frame, uint16_t *output) {
  uint8_t	stop = 0;
  uint8_t	depth = gif->depth + 1;
  uint32_t	output_pos = 0;
  int		prev;
  int		cur = -1;

  clear_table(depth);
  prev = clear_id;

  while (stop == 0) {
    prev = cur;
    cur = getNbits(frame, depth);

    if (cur == clear_id) {
      depth = gif->depth + 1;
      clear_table(depth);
      cur = -1;
    } else if (cur == end_of_info) {
      stop = 1;
      continue;
    } else if (prev == -1) {
      // this is the first entry after a clear
      // save cur
      gif_update_output(gif, frame, output, &output_pos, cur);
    } else {
      if (table_idx < TABLE_MAX_SIZE) {
	table[table_idx].prev = prev;
	if (cur == -1) {
	  table[table_idx].cur = FirstPixel(prev);
	} else {
	  table[table_idx].cur = FirstPixel(cur);
	}
	table_idx += 1;
	if (Power2(table_idx) && table_idx < TABLE_MAX_SIZE) {
	  depth += 1;
	  if (depth > 12) {
	    while(1);
	  }
	}
      }
      // save cur
      gif_update_output(gif, frame, output, &output_pos, cur);
    }
  }
  return;
}

gif_t *gif_init(void *handler, unsigned int (*read)(void *handler, uint8_t* buf, uint32_t buf_size, unsigned long *real_size)) {
  gif_t *gif;

  gif = calloc(1, sizeof(gif_t));
  //gif->file = file;
  gif->handler = handler;
  gif->read = read;
  gif->frame_num = 0;

  return (gif);
}

void gif_destroy(gif_t *gif) {
  free(gif->palette.colors);
  free(gif);
}

int gif_get_hdr(gif_t *gif) {
  UINT	status;
  ULONG	real_size;
  char sigver[3];
  char num[2];

  status = gif->read(gif->handler, sigver, 3, &real_size);
  if (status != FX_SUCCESS ||
      real_size != 3 ||
      strncmp(sigver, SIGNATURE, 3) != 0) {
    return -1;
  }

  status = gif->read(gif->handler, sigver, 3, &real_size);
  if (status != FX_SUCCESS ||
      real_size != 3 ||
      strncmp(sigver, VERSION, 3) != 0) {
    return -1;
  }

  status = gif->read(gif->handler, num, 2, &real_size);
  gif->width = num[0] + (((uint16_t) num[1]) << 8);

  status = gif->read(gif->handler, num, 2, &real_size);
  gif->height = num[0] + (((uint16_t) num[1]) << 8);

  status = gif->read(gif->handler, &gif->fdsz, 1, &real_size);
  if (status != FX_SUCCESS || (gif->fdsz & 0x80) == 0) {
    return -1;
  }

  gif->depth = ((gif->fdsz >> 4) & 7) + 1;
  gif->palette.size = 1 << ((gif->fdsz & 0x07) + 1);
  if (gif->palette.colors == NULL) {
    gif->palette.colors = calloc(gif->palette.size, sizeof(color_t));
  }

  // background color index
  status = gif->read(gif->handler, &gif->bgidx, 1, &real_size);
  // default pixel aspect ratio
  status = gif->read(gif->handler, &gif->aspect, 1, &real_size);

  // Global Color Table
  status = gif->read(gif->handler, gif->palette.colors, 3*gif->palette.size, &real_size);

  return status;
}

int gif_skip(gif_t *gif, int nBytes) {
  ULONG		real_size;
  uint8_t	d;

  for (int i = 0; i < nBytes; i++) {
    gif->read(gif->handler, &d, 1, &real_size);
  }

  return 0;
}

int gif_get_img(gif_t *gif, gif_frame_t *frame) {
  UINT		status;
  ULONG		real_size;
  uint8_t	d;
  uint8_t	data_size;
  uint16_t	count = 0;
  uint32_t	total_data;


  // graphic Control Extention
  do {
    status = gif->read(gif->handler, &d, 1, &real_size);
    if (status != FX_SUCCESS) {
      return -1;
    }
    switch (d) {
    case BLOCK_EXT:
      {
	status = gif->read(gif->handler, &d, 1, &real_size);
	switch (d) {
	case LBL_GCE:
	  // read and skip FOR NOW timing happens here

	  status = gif->read(gif->handler, &frame->gce.data_len, 1, &real_size);
	  status = gif->read(gif->handler, &frame->gce.transparent_flags, 1, &real_size);
	  status = gif->read(gif->handler, &frame->gce.delay, 2, &real_size);
	  status = gif->read(gif->handler, &frame->gce.transparent_color_idx, 1, &real_size);
	  status = gif->read(gif->handler, &d, 1, &real_size);
	  if (d != 0) {
	    return -1;
	  }
	  break;
	case LBL_APP:
	case LBL_COMMENT:
	  // read and skip FOR NOW
	  status = gif->read(gif->handler, &d, 1, &real_size);
	  gif_skip(gif, d);
	  // read sub block size
	  status = gif->read(gif->handler, &d, 1, &real_size);
	  if (d == 0x00) {
	    // found and end block early -> exit
	    break;
	  }
	  // skip len
	  gif_skip(gif, d);
	  // look for the end block
	  do {
	    status = gif->read(gif->handler, &d, 1, &real_size);
	  } while (d != 0);
	  break;
	default:
	  // read and skip
	  status = gif->read(gif->handler, &d, 1, &real_size);
	  gif_skip(gif, d);
	}
      }
      break;
    case BLOCK_IMG:
      {
	status = gif->read(gif->handler, &frame->x, 2, &real_size);
	status = gif->read(gif->handler, &frame->y, 2, &real_size);
	status = gif->read(gif->handler, &frame->width, 2, &real_size);
	status = gif->read(gif->handler, &frame->height, 2, &real_size);
	status = gif->read(gif->handler, &frame->img_flags, 1, &real_size);
	if (frame->img_flags.local_color_table == 1) {
	  // read the local table
	  frame->palette.size = 1 << (frame->img_flags.lcl_colr_tbl_size + 1);
	  frame->palette.colors = calloc(frame->palette.size, sizeof(color_t));
	  status = gif->read(gif->handler, frame->palette.colors, 3*frame->palette.size, &real_size);
	}
	frame->data = calloc(frame->width * frame->height, 1);
	if (frame->data == NULL) {
	  if (frame->palette.size != 0) {
	    free(frame->palette.colors);
	    frame->palette.size = 0;
	  }
	  return -1;
	}
	// start of image LZW minimum code size
	status = gif->read(gif->handler, &d, 1, &real_size);
	if (d == 0x00) {
	  d = 0x08;
	}
	gif->depth = d;

	status = gif->read(gif->handler, &data_size, 1, &real_size);
	count = 0;
	total_data = 0;
	while (data_size != 0) {
	  uint8_t buf[255] = {0};

	  status = gif->read(gif->handler, buf, data_size, &real_size);
	  memcpy(&frame->data[total_data], buf, data_size);
	  total_data += data_size;

	  status = gif->read(gif->handler, &data_size, 1, &real_size);
	  count ++;
	}
	gif_lzw_decompress(gif, frame, frame->output);
	if (frame->palette.size != 0) {
	  free(frame->palette.colors);
	  frame->palette.size = 0;
	}
	free(frame->data);
	gif->frame_num += 1;
	return 0;
      }
      break;
    case BLOCK_END:
      {
	return 1;
      }
      break;
    }
  } while (1);

  return 0;

}
