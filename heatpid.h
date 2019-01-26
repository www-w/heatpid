#include <stc12.h>
#include <stdint.h>

// 定义按键IO口 必须在同一端口
#define BUTTON_MODE P1_7
#define BUTTON_UP P1_6
#define BUTTON_DOWN P1_5
#define BUTTON_MASK 0xD0	// 11100000
