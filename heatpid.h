#include <stc12.h>
#include <stdint.h>
#include <stdbool.h>

//#define DEBUG
#ifdef DEBUG
#define DEBUG_SIZE 9
#endif

//#define DEBUG_UART	// Debug on UART
#ifdef DEBUG_UART
void uart_tx(uint8_t data);
#endif

// 定义笔段LED IO口
#define LED_COM_A P1_3				//	aaaaaa
#define LED_COM_B P1_1				//	bb  cc
#define LED_COM_C P1_5				//	dddddd
#define LED_COM_D P1_4				//	ee  ff
#define LED_A P3_1				//	gggggg
#define LED_B P3_0
#define LED_C P3_2
#define LED_D P3_7
#define LED_E P1_2
#define LED_F P3_3
#define LED_G P1_0

// 定义按键IO口 复用LED COM弱上拉口
#define BUTTON_MODE LED_COM_B
#define BUTTON_UP LED_COM_C
#define BUTTON_DOWN LED_COM_A

#define PWM_EXT P3_4	// 挤出机温控PWM输出
#define PWM_BED P3_5	// 热床温控PWM输出
