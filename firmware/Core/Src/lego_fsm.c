
#include "main.h"
#include "lego_fsm.h"
#include "nhd_1_8_160128B.h"
#include "app_filex.h"

static lego_fsm_t lego = {0};
ALIGN_32BYTES(uint32_t fsm_media_memory[64 * FX_STM32_SD_DEFAULT_SECTOR_SIZE / sizeof(uint32_t)]);

extern const unsigned int all_brand_gif_len;
extern const unsigned char const all_brand_gif[];

void init_lego_fsm(void) {
  lego.state = LEGO_FSM_INIT;
  lego.flags = 0x00;
  lego.sd_card = GPIO_PIN_SET;
  lego.sdio_disk = NULL;
  memset(lego.filename, 0, FX_MAX_LONG_NAME_LEN);

  nhd_160128b_init();
  nhd_160128b_FillScreen(0x0000);
  return;
}

void destroy_lego_fsm(void) {
  return;
}

static uint32_t file_read_pos = 0;

unsigned int lego_memory_read(void *handler, uint8_t* buf, uint32_t buf_size, unsigned long *real_size) {
  memcpy(buf, &all_brand_gif[file_read_pos], buf_size);
  file_read_pos += buf_size;
  *real_size = buf_size;
  return FX_SUCCESS;
}

void lego_store_scanline(unsigned char *line, int line_len, int line_num) {
  uint32_t offset = (160 - (line_len / 3)) / 2;
  for (int i = 0; i < 160; i++) {
    if (i < (line_len / 3)) {
      int r = line[i * 3];
      int g = line[i * 3 + 1];
      int b = line[i * 3 + 2];
      uint16_t rgb = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
      asm("REV16 %0,%1" : "=r"(rgb) : "r"(rgb));
      lego.display_buffer[160 * (128 - line_num) + i + offset] = rgb;
    }
  }
}

void lego_fsm(void) {
  UINT status;
  GPIO_PinState cur = HAL_GPIO_ReadPin(SDMMC1_DETECT_GPIO_Port, SDMMC1_DETECT_Pin);
  // put it on the stack

  if (cur != lego.sd_card) {
    lego.sd_card = cur;
    switch(cur) {
    case GPIO_PIN_RESET:
      // card inserted
      lego.state = LEGO_FSM_INIT_SD;
      break;
    case GPIO_PIN_SET:
      // card removed
      lego.state = LEGO_FSM_CLEAN;
      break;
    }
    HAL_Delay(20);
  }

  switch (lego.state) {
    case LEGO_FSM_INIT:
      lego.state = LEGO_FSM_IDLE;
      break;
    case LEGO_FSM_INIT_SD:
      if ((lego.flags & LEGO_SD_INIT) == 0x00) {
	MX_SDMMC1_SD_Init();
	lego.flags |= LEGO_SD_INIT;
      }
      if (lego.sdio_disk == NULL) {
	lego.sdio_disk = calloc(1, sizeof(FX_MEDIA));
	if (lego.sdio_disk == NULL) {
	  lego.state = LEGO_FSM_CLEAN;
	  return;
	}
      }
      if ((lego.flags & LEGO_MEDIA_OPEN) == 0x00) {
	status =  fx_media_open(lego.sdio_disk, "STM32_SDIO_DISK",
				fx_stm32_sd_driver, 0,
				(VOID *) fsm_media_memory, sizeof(fsm_media_memory));
	if (status != FX_SUCCESS) {
	  lego.state = LEGO_FSM_CLEAN;
	} else {
	  lego.flags |= LEGO_MEDIA_OPEN;
	  lego.state = LEGO_FSM_GET_NEXTFILE;
	}
      }
      break;
    case LEGO_FSM_GET_NEXTFILE:
      status = fx_directory_next_entry_find(lego.sdio_disk, lego.filename);
      if (status == FX_NO_MORE_ENTRIES) {
	status = fx_directory_first_entry_find(lego.sdio_disk, lego.filename);
      }
      if (status != FX_SUCCESS) {
	lego.state = LEGO_FSM_CLEAN;
      } else {
#define JPG	".jpg"
#define GIF	".gif"
	if (strcmp(&lego.filename[strlen(lego.filename) - strlen(JPG)], JPG) == 0) {
	  lego.state = LEGO_FSM_OPEN_JPG;
	} else 	if (strcmp(&lego.filename[strlen(lego.filename) - strlen(GIF)], GIF) == 0) {
	  lego.state = LEGO_FSM_OPEN_GIF;
	} else {
	  lego.state = LEGO_FSM_GET_NEXTFILE;
	}
      }
      break;
    case LEGO_FSM_OPEN_JPG:
      if (lego.fx_file == NULL) {
	lego.fx_file = calloc(1, sizeof(FX_FILE));
	if (lego.fx_file == NULL) {
	  lego.state = LEGO_FSM_CLEAN;
	  return;
	}
      }
      status = fx_file_open(lego.sdio_disk,
			    lego.fx_file,
			    lego.filename,
			    FX_OPEN_FOR_READ);
      if (status != FX_SUCCESS) {
	lego.state = LEGO_FSM_CLEAN;
      } else {
	lego.state = LEGO_FSM_READ_JPG;
      }
      break;
    case LEGO_FSM_READ_JPG:
      memset(lego.display_buffer, 0x00, sizeof(lego.display_buffer));
      status = fx_file_read(lego.fx_file, lego.file_data, sizeof(lego.file_data), &lego.file_size);
      if (status != FX_SUCCESS) {
	lego.state = LEGO_FSM_CLEAN;
      } else {
	struct jpeg_error_mgr jerr;
	struct jpeg_decompress_struct cinfo;
	JSAMPARRAY buffer;		/* Output row buffer */
	int row_stride;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, lego.file_data, lego.file_size);
	(void) jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)
	  ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	while (cinfo.output_scanline < cinfo.output_height) {
	  (void) jpeg_read_scanlines(&cinfo, buffer, 1);
	  lego_store_scanline(buffer[0], row_stride, cinfo.output_scanline);
	}
	cinfo.mem->free_pool((j_common_ptr) &cinfo, JPOOL_IMAGE);

	jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress(&cinfo);

	lego.display_width = 160;
	lego.display_height = 128;
	lego.next = LEGO_FSM_JPG_WAIT;
	lego.state = LEGO_FSM_DISPLAY;
      }
      break;
    case LEGO_FSM_JPG_WAIT:
      {
	static uint32_t jpg_wait_until = 0;
	uint32_t cur = HAL_GetTick();

	if (jpg_wait_until == 0) {
	  jpg_wait_until = cur + 2000;
	}
	if (cur > jpg_wait_until) {
	  lego.state = LEGO_FSM_CLOSE_FILE;
	  jpg_wait_until = 0;
	}
      }
    break;
    case LEGO_FSM_OPEN_GIF:
      if (lego.fx_file == NULL) {
	lego.fx_file = calloc(1, sizeof(FX_FILE));
	if (lego.fx_file == NULL) {
	  lego.state = LEGO_FSM_CLEAN;
	  return;
	}
      }
      status = fx_file_open(lego.sdio_disk,
			    lego.fx_file,
			    lego.filename,
			    FX_OPEN_FOR_READ);
      if (status != FX_SUCCESS) {
	lego.state = LEGO_FSM_CLEAN;
      } else {
	lego.state = LEGO_FSM_READ_GIF_HDR;
      }
      break;
    case LEGO_FSM_READ_GIF_HDR:
      memset(lego.display_buffer, 0, sizeof(lego.display_buffer));
      lego.gif = gif_init(lego.fx_file, fx_file_read);
      gif_get_hdr(lego.gif);
      lego.state = LEGO_FSM_GET_GIF_IMG;
      break;
    case LEGO_FSM_GET_GIF_IMG:
      {
	int res;
	gif_frame_t frame = {0};

	frame.output = lego.display_buffer;
	frame.bit_pos = 0;

	res = gif_get_img(lego.gif, &frame);
	if (res == 0) {
	  lego.display_delay = (frame.gce.delay * 10);
	  lego.display_width = lego.gif->width;
	  lego.display_height = lego.gif->height;
	  lego.next = LEGO_FSM_GET_GIF_IMG;
	  if (lego.display_delay >= 50) {
	    lego.next = LEGO_FSM_GIF_WAIT;
	  }
	  lego.state = LEGO_FSM_DISPLAY;
	} else {
	  lego.state = LEGO_FSM_CLOSE_FILE;
	}
      }
      break;
    case LEGO_FSM_GIF_WAIT:
      {
	static uint32_t	gif_wait_until = 0;
	uint32_t cur = HAL_GetTick();

	if (cur < (lego.display_start + lego.display_delay)) {
	  gif_wait_until = lego.display_start + lego.display_delay;
	}
	if (cur > gif_wait_until) {
	  lego.state = LEGO_FSM_GET_GIF_IMG;
	  gif_wait_until = 0;
	}
      }
    break;
    case LEGO_FSM_DISPLAY:
      lego.display_start = HAL_GetTick();
      nhd_160128b_DisplayImage(lego.display_buffer, lego.display_width, lego.display_height);
      lego.display_delay -= HAL_GetTick() - lego.display_start;
      lego.state = lego.next;
      break;
    case LEGO_FSM_CLOSE_FILE:
      fx_file_close(lego.fx_file);
      lego.flags &= ~LEGO_FILE_OPEN;
      lego.state = LEGO_FSM_GET_NEXTFILE;
      break;
    case LEGO_FSM_CLEAN:
      if ((lego.flags & LEGO_FILE_OPEN) == LEGO_FILE_OPEN) {
	fx_file_close(lego.fx_file);
	lego.flags &= ~LEGO_FILE_OPEN;
      }
      if ((lego.flags & LEGO_MEDIA_OPEN) == LEGO_MEDIA_OPEN) {
	fx_media_close(lego.sdio_disk);
	lego.flags &= ~LEGO_MEDIA_OPEN;
      }
      if ((lego.flags & LEGO_SD_INIT) == LEGO_SD_INIT) {
	MX_SDMMC1_SD_Deinit();
	lego.flags &= ~LEGO_SD_INIT;
      }
      lego.state = LEGO_FSM_IDLE;
      break;
    case LEGO_FSM_IDLE:
      {
	{
	  int res;
	  gif_frame_t frame = {0};
	  
	  memset(lego.display_buffer, 0, sizeof(lego.display_buffer));
	  file_read_pos = 0;
	  lego.gif = gif_init(1, lego_memory_read);
	  gif_get_hdr(lego.gif);
	  res = 0;
	  while (res == 0) {
	    if (HAL_GPIO_ReadPin(SDMMC1_DETECT_GPIO_Port, SDMMC1_DETECT_Pin) == GPIO_PIN_RESET) {
	      break;
	    }
	    frame.output = lego.display_buffer;
	    frame.bit_pos = 0;
	    res = gif_get_img(lego.gif, &frame);
	    if (res == 0) {
	      uint32_t cur = HAL_GetTick();
	      uint32_t now;
	      nhd_160128b_DisplayImage(lego.display_buffer, lego.display_width, lego.display_height);
	      if (HAL_GPIO_ReadPin(SDMMC1_DETECT_GPIO_Port, SDMMC1_DETECT_Pin) == GPIO_PIN_RESET) {
		break;
	      }
	      now = HAL_GetTick();
	      HAL_Delay(100 - (now - cur));
	    }
	  }
	}
      }
      break;
  }
  return;
}
