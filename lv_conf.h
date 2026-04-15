#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* Color settings */
#define LV_COLOR_DEPTH 8
#define LV_COLOR_16_SWAP 0
#define LV_COLOR_SCREEN_TRANSP 0
#define LV_COLOR_16_SWAP 0
#define LV_COLOR_SCREEN_TRANSP 0

/* Memory settings */
#define LV_MEM_CUSTOM 1
#define LV_MEM_SIZE (48U * 1024U)
#define LV_MEM_CUSTOM_INCLUDE <stdlib.h>
#define LV_MEM_CUSTOM_ALLOC malloc
#define LV_MEM_CUSTOM_FREE free
#define LV_MEM_CUSTOM_REALLOC realloc
#define LV_MEM_CUSTOM_CALLOC calloc

/* Tick */
#define LV_TICK_CUSTOM 0

/* Logging */
#define LV_USE_LOG 1
#define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

/* Fonts used by SquareLine Studio */
#define LV_FONT_MONTSERRAT_8 1
#define LV_FONT_MONTSERRAT_10 1
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Core widgets used by the generated UI */
#define LV_USE_LABEL 1
#define LV_USE_IMG 1
#define LV_USE_OBJ 1
#define LV_USE_THEME_DEFAULT 1

#endif /*LV_CONF_H*/
