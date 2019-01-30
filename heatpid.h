#include <stc12.h>
#include <stdint.h>
#include <stdbool.h>

// 定义笔段LED IO口
#define LED_COM_A P1_4				//	aaaaaa
#define LED_COM_B P1_3				//	bb  cc
#define LED_COM_C P1_2				//	dddddd
#define LED_COM_D P1_1				//	ee  ff
#define LED_A P1_0					//	gggggg
#define LED_B P3_7
#define LED_C P3_5
#define LED_D P3_4
#define LED_E P3_3
#define LED_F P3_2
#define LED_G P3_1

// 定义按键IO口 复用LED COM弱上拉口
#define BUTTON_MODE LED_COM_A
#define BUTTON_UP LED_COM_B
#define BUTTON_DOWN LED_COM_C

#define PWM_EXT P1_7	// 挤出机温控PWM输出
#define PWM_BED P1_6	// 热床温控PWM输出
