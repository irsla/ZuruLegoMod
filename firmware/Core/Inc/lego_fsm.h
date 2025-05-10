#ifndef __LEGO_FSM_H__
#define __LEGO_FSM_H__

#include "app_filex.h"
#include <stdio.h>
#include "jpeglib.h"
#include "gif_decoder.h"

typedef enum lego_state_e {
  LEGO_FSM_INIT = 0,
  LEGO_FSM_INIT_SD,
  LEGO_FSM_GET_NEXTFILE,
  LEGO_FSM_OPEN_JPG,
  LEGO_FSM_READ_JPG,
  LEGO_FSM_JPG_WAIT,
  LEGO_FSM_OPEN_GIF,
  LEGO_FSM_READ_GIF_HDR,
  LEGO_FSM_GET_GIF_IMG,
  LEGO_FSM_GIF_WAIT,
  LEGO_FSM_DISPLAY,
  LEGO_FSM_CLOSE_FILE,
  LEGO_FSM_CLEAN,
  LEGO_FSM_IDLE
} lego_state_t;

typedef struct lego_fsm_s lego_fsm_t;

#define LEGO_SD_INIT	0x01
#define LEGO_MEDIA_OPEN	0x02
#define LEGO_FILE_OPEN	0x04

struct lego_fsm_s {
  lego_state_t	state;
  int8_t	flags;
  GPIO_PinState	sd_card;
  FX_MEDIA      *sdio_disk;
  CHAR          filename[FX_MAX_LONG_NAME_LEN];
  FX_FILE       *fx_file;
  lego_state_t	next;
  uint8_t	file_data[128*160*2];
  ULONG		file_size;
  uint16_t	display_buffer[128*160*2];
  uint8_t	display_width;
  uint8_t	display_height;
  gif_t		*gif;
  int32_t	display_start;
  int32_t	display_delay;
};

#endif /* !__LEGO_FSM_H__ */
