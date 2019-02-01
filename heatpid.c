#include "heatpid.h"
#define STA_NORMAL 0x80
#define STA_SETBED 0x40
#define STA_SETEXT 0x20
#define BTN_MODE P1_1
#define BTN_UP P1_2
#define BTN_DOWN P1_3
#define BTN_MODE_PRESSED 1
#define BTN_UP_PRESSED 2
#define BTN_DOWN_PRESSED 4
#ifdef DEBUG
uint8_t debug;
#endif
unsigned char bedTmp;
unsigned char extTmp;
unsigned char setBed;
unsigned char setExt;
unsigned char ms4=0;
unsigned char ms=0;
unsigned char sec4=0;
unsigned char btnPressed=0;
unsigned char btnDown=0;
unsigned char STATE=STA_NORMAL;
uint8_t adc_state = 0;
__code uint8_t TRANSLATION[] = {	// [0-9be]对应的LED笔段控制码
	0x77, //0			aaaa
	0x12, //1			b  c
	0x5D, //2			dddd
	0x5B, //3			e  f
	0x3A, //4			gggg
	0x6B, //5
	0x6F, //6
	0x52, //7
	0x7F, //8
	0x7B, //9
	0x2F, //10 b
	0x7D  //11 e
};

char str_e[] = {
	11,	// e
	0,	// 百位
	0,	// 十位
	0	// 个位
};
char str_b[] = {
	10,	// b
	0,	// 百位
	0,	// 十位
	0	// 个位
};
char* strptr=str_e;	// 指向当前显示的字符串

void timer0(void) __interrupt 1    //000BH
{
	ms4++;
	if(ms4==4){
		ms++;
		ms4=0;
		if(btnDown>0 && btnDown<250)btnDown++;
		if(ms==250){
			sec4++;
			ms=0;
			if(sec4 == 4) {
				// every second
				sec4 = 0;

				// STATE Timer
				if(STATE & STA_NORMAL){
					// Every 2 seconds Change Display
					if((STATE & 0x0F) < 2) STATE ++;
				}else{
					// Setting Modes Time out
					if((STATE & 0x0F) < 30) STATE ++;
					else STATE = STA_NORMAL;
				}


				// ADC Timer
				if(adc_state < 0xF0){
					adc_state += 0xF0;
				}
			}
		}
	}
}

// 增量式PID 部分 
struct PID{
	// PID主要数据结构体
	int16_t Sv;	// 用户设定值 用户设定摄氏度，转储为对应adc值保存于此
	int16_t Ek;	// 本次偏差值 偏差值均基于ADC返回值计算
	int16_t Ek1;	// 前次偏差值
	int16_t Ek2;	// 前前次偏差值

	uint8_t pwm;	// 当前输出pwm值 [0,255]
};
void pidCompute(struct PID* pid){
	int16_t t;
	pid->Ek2 = pid->Ek1;
	pid->Ek1 = pid->Ek;
	pid->Ek = pid->Sv - ADC_RES;
	t = pid->Ek - pid->Ek1 + pid->Ek + pid->Ek - pid->Ek1 - pid->Ek1 + pid->Ek2 + pid->pwm;
	if(t > 255) t = 255;
	if(t < 0) t = 0;
	pid->pwm = t;
}

struct PID pidExt = {200,0,0,0,0};
struct PID pidBed = {100,0,0,0,0};

void EEPROM_read(uint8_t addrh, uint8_t addrl){
	IAP_CONTR = 0x83;	// enable iap
	IAP_ADDRH = addrh;
	IAP_ADDRL = addrl;
	IAP_CMD = 1;	// 字节读
	IAP_TRIG= 0x5A;
	IAP_TRIG= 0xA5;
}
void EEPROM_clear(uint8_t sec){
	IAP_CONTR = 0x83;
	IAP_ADDRH = sec;
	IAP_ADDRL = 0;
	IAP_CMD = 3;	// 扇区擦除
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
}
void EEPROM_write(uint8_t addrh, uint8_t addrl, uint8_t data){
	IAP_CONTR = 0x83;
	IAP_ADDRH = addrh;
	IAP_ADDRL = addrl;
	IAP_DATA = data;
	IAP_CMD = 2;	// Program
	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;
}
#define ADC2TMP(a) EEPROM_read(1,(a))	// adc值转温度值，返回IAP_DATA
#define TMP2ADC(a) EEPROM_read(0,(a))	// 温度值转adc值，返回IAP_DATA
void EEPROM_restore(void) {	// Restore parameter from EEPROM
	// Restore data from EEPROM
	EEPROM_read(2,0);
	setExt = IAP_DATA;
	EEPROM_read(2,1);
	setBed = IAP_DATA;
	TMP2ADC(setExt);
	pidExt.Sv = IAP_DATA;
	TMP2ADC(setBed);
	pidBed.Sv = IAP_DATA;
}


void displayDelay(){
	// 增加延时可以减小显示频率增加显示亮度
	uint16_t i;
	i=0xFF;
	while(i--);
}
// 传入strptr四个元素中的一个元素索引，译码并显示
void displayPort(char LR){
	uint8_t data = TRANSLATION[strptr[LR]];	// 有效数据，低七位
	if(data&0x40){ LED_A = 0; }
	if(data&0x20){ LED_B = 0; }
	if(data&0x10){ LED_C = 0; }
	if(data&0x08){ LED_D = 0; }
	if(data&0x04){ LED_E = 0; }
	if(data&0x02){ LED_F = 0; }
	if(data&0x01){ LED_G = 0; }
	displayDelay();
	LED_A = LED_B = LED_C = LED_D = LED_E = LED_F = LED_G = 1;
}
/** 传入值，按百位十位个位写入strptr指定位置，以待转码显示 **/
void trans(uint8_t a){
	uint8_t i;
	uint8_t bai = 0, shi = 0;
	if(a>199){
		bai = 2;
		a -= 200;
	} else if(a>99){
		bai = 1;
		a -= 100;
	}
	for(i=9,shi=0;i<100;i+=10,shi++){ // i = [9-99] shi = [0-9]
		if(a <= i){
			a -= i-9;
			break;
		}
	}

	strptr[1] = bai;	
	strptr[2] = shi;	
	strptr[3] = a;	

}
void parseButton(void){
	if(!(BUTTON_MODE && BUTTON_UP && BUTTON_DOWN)) {
		// any key down
		if(btnDown < 1)btnDown=1;//start timer;
		if(!BUTTON_MODE) btnPressed = BTN_MODE_PRESSED;
		if(!BUTTON_UP) btnPressed = BTN_UP_PRESSED;
		if(!BUTTON_DOWN) btnPressed = BTN_DOWN_PRESSED;
		return;
	}
	if(btnDown==0)return;	// not a key press, normal return;
	// 按键已经抬起
	if(btnDown<100){ btnDown = 0; return; }	// 滤除按键抖动
	btnDown=0;	// 重置去抖变量
    switch(btnPressed){
		case BTN_MODE_PRESSED:
			if(STATE & STA_NORMAL)STATE=STA_SETBED;
			else if(STATE & STA_SETBED){
				STATE=STA_SETEXT;
				setBed = bedTmp; // read tmp from eeprom
			}
			else if(STATE & STA_SETEXT){
				STATE=STA_NORMAL;
				setExt = extTmp;
				/* 保存设定值 */
				//TODO
			}
			break;
		case BTN_UP_PRESSED:
			if(STATE==STA_NORMAL)return;
			if(STATE==STA_SETBED)setBed++;
			if(STATE==STA_SETEXT)setExt++;
			break;
		case BTN_DOWN_PRESSED:
			if(STATE==STA_NORMAL)return;
			if(STATE==STA_SETBED)setBed--;
			if(STATE==STA_SETEXT)setExt--;
	}
}
void display(void){
	parseButton();
	if(STATE & STA_NORMAL){
		// 默认状态始终允许显示
		if(STATE == STA_NORMAL+2){	// 到达两秒钟
			STATE = STA_NORMAL;
			if(strptr == str_e){
				strptr = str_b;
				trans(bedTmp);
			}else{
				strptr = str_e;
				trans(extTmp);
			}
		}

	} else if(sec4 < 1){
		// 其它模式都需要闪烁显示
		// 500ms 显示 500ms 不显示
		// 700ms 300ms
		return;
	}
	if(STATE == STA_SETBED){
		strptr = str_b;
		trans(setBed);
	}
	if(STATE == STA_SETEXT){
		strptr = str_e;
		trans(setExt);
	}
#ifdef DEBUG
	strptr[0] = 1;
	trans(debug);
#endif
	LED_COM_A = 0;
	displayPort(0);
	LED_COM_A = 1;
	LED_COM_B = 0;
	displayPort(1);
	LED_COM_B = 1;
	LED_COM_C = 0;
	displayPort(2);
	LED_COM_C = 1;
	LED_COM_D = 0;
	displayPort(3);
	LED_COM_D = 1;
}



void main(void){
	//Init Timer0
	TMOD=2;//00000010 8bit autoreload
	TH0=6;//4 cycle equ 1ms
	TL0=0;
	ET0=1;
	EA=1;
	TR0=1;

	EEPROM_restore();

	// Init ADC
	ADC_CONTR = 0x80;	// 10000000;
	P1M1 = 0xC0;	// P1.7 & P1.6 ADC input
	// A Channel P1.7
	// B Channel P1.6

	while(1){
		display();
		parseButton();

		// ADC & PID
		if(adc_state == 0xF1){
			// compute A
			P1ASF = 0x80;	// P1.7 & P1.6 ADC input
			ADC_CONTR = 0x8F;
			NOP();NOP();NOP();NOP();NOP();
			while(ADC_CONTR == 0x8F);
			ADC_CONTR = 0x80;
			// retrun value in ADC_RES 
			ADC2TMP(ADC_RES);
			// return value in IAR_DATA
			extTmp = IAP_DATA;
			// PID start works
			// pv = ADC_RES 当前值
			pidCompute(&pidExt);

			adc_state = 0;	// A ok next B
		}else if(adc_state == 0xF0){
			// compute B
			P1ASF = 0x40;
			ADC_CONTR = 0x8E;
			NOP();NOP();NOP();NOP();NOP();
			while(ADC_CONTR== 0x8E);
			ADC_CONTR = 0x80;
			// retrun value in ADC_RES
			ADC2TMP(ADC_RES);
			bedTmp = IAP_DATA;
			// PID
			pidCompute(&pidBed);

			adc_state = 1;	// B ok next A
		}

		// Soft PWM OUTPUT
		uint8_t i;
		for(i = 1; i != 0; i++){	// i = [1, 255]
			if(pidExt.pwm >= i){
				PWM_EXT = 0;	// Power On
			}else{
				PWM_EXT = 1;
			}
			if(pidBed.pwm >= i){
				PWM_BED = 0;
			}else{
				PWM_EXT = 1;
			}
		}

	}
}
