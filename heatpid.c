#include "heatpid.h"
#define STA_NORMAL 0
#define STA_SETBED 1
#define STA_SETEXT 2
#define BTN_MODE P1_1
#define BTN_UP P1_2
#define BTN_DOWN P1_3
#define BTN_MODE_PRESSED 1
#define BTN_UP_PRESSED 2
#define BTN_DOWN_PRESSED 4
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
__code uint8_t TRANSLATION[] = {	// [0-9be]对应的LED笔段控制码
	0x30, //0
	0x31, //1
	0x32, //2
	0x33, //3
	0x34, //4
	0x35, //5
	0x36, //6
	0x37, //7
	0x38, //8
	0x39, //9
	0x40, //10 b
	0x41 //11 e
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

bool display_enable = false;	// 指示当前循环是否执行显示操作，用以实现闪烁效果

void timer0(void) __interrupt 1    //000BH
{
	ms4++;
	if(ms4==4){
		ms++;
		ms4=0;
		if(btnDown>1 && btnDown<250)btnDown++;
		if(ms==250){
			sec4++;
			ms=0;
		}
	}
}
void displayDelay(){
	// 增加延时可以减小显示频率增加显示亮度
	uint16_t i;
	i=0xFFFF;
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
		if(!BUTTON_UP) btnPressed = BTN_MODE_PRESSED;
		if(!BUTTON_DOWN) btnPressed = BTN_MODE_PRESSED;
		return;
	}
	if(btnDown==0)return;	// not a key press, normal return;
	// 按键已经抬起
	if(btnDown<50){ btnDown = 0; return; }	// 滤除按键抖动
	btnDown=0;	// 重置去抖变量
    switch(btnPressed){
		case BTN_MODE_PRESSED:
			if(STATE==STA_NORMAL)STATE=STA_SETBED;
			else if(STATE==STA_SETBED){
				STATE=STA_SETEXT;
				setBed = bedTmp;
			}
			else if(STATE==STA_SETEXT){
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
	if(STATE==STA_NORMAL){
		display_enable = true;		// 默认状态始终允许显示
		if(sec4 % 4 == 0) {			// 到达一秒钟
			if(strptr == str_e){
				strptr = str_b;
				trans(bedTmp);
			}else{
				strptr = str_e;
				trans(extTmp);
			}
		}

	} else if(sec4 % 2 == 0) {		// 其它模式都需要闪烁显示
		// 500ms 显示 500ms 不显示
		display_enable = !display_enable;
		if(!display_enable)return;	// 不显示时直接返回,直到下一个500ms到来
	}
	if(STATE == STA_SETBED){
		strptr = str_b;
		trans(setBed);
	}
	if(STATE == STA_SETEXT){
		strptr = str_e;
		trans(setExt);
	}
	LED_COM_A = 0;
	displayPort(0);
	displayDelay();
	LED_COM_A = 1;
	LED_COM_B = 0;
	displayPort(1);
	displayDelay();
	LED_COM_B = 1;
	LED_COM_C = 0;
	displayPort(2);
	displayDelay();
	LED_COM_C = 1;
	LED_COM_D = 0;
	displayPort(3);
	displayDelay();
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

	// Init ADC

	while(1){
		display();
		parseButton();
	}
}
