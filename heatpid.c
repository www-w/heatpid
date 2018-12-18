#include "heatpid.h"
#define STA_NORMAL 0
#define STA_SETBED 1
#define STA_SETEXT 2
#define BTN_MODE P1_1
#define BTN_UP P1_2
#define BTN_DOWN P1_3
unsigned char bedTmp;
unsigned char extTmp;
unsigned char setBed;
unsigned char setExt;
unsigned char LChr;
unsigned char RChr;
unsigned char ms4=0;
unsigned char ms=0;
unsigned char sec4=0;
unsigned char btnPressed=0;
unsigned char btnDown=0;
unsigned char STATE=STA_NORMAL;
unsigned char disIndex=0;
unsigned char STRING[]="BED   SETBED   EXT   SETEXT   ";
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
void delay(void){
	unsigned int i;
	i=65535;
	while(i--);
}
void displayPort(unsigned char LR){
	unsigned char mask=1;
	
	while(mask){ // while not zero
		P3=(LR&mask)^mask;
		mask<<=1;
	}
}
void display(void){
	if(STATE==STA_NORMAL){
		if(disIndex>=sizeof(STRING)){
			disIndex=0;
		}
		LChr=*(STRING+disIndex);
		RChr=*(STRING+disIndex+1);

	}
	if(sec4%2==0){
		disIndex++;//500ms
	}
	P1_1=0;P1_2=1;
	displayPort(LChr);
	P1_2=0;P1_1=1;
	displayPort(RChr);
}
void parseButton(void){
	if(!BTN_MODE&!BTN_UP&!BTN_DOWN){
		//some key down
		if(btnDown < 1)btnDown=1;//start timer;
		return;
	}
	//key up or not pressed
	if(btnDown==0)return;
	//keyup
	if(btnDown<50)return; //ignore verb
    
	if(BTN_MODE==0){
        if(STATE==STA_NORMAL)STATE=STA_SETBED;
        else if(STATE==STA_SETBED)STATE=STA_SETEXT;
        else if(STATE==STA_SETEXT)STATE=STA_NORMAL;
        btnPressed=1;
    }
	if(BTN_UP==0){
        if(STATE==STA_NORMAL)return;
        if(STATE==STA_SETBED)setBed++;
        if(STATE==STA_SETEXT)setExt++;
        btnPressed=1;
	}
	if(BTN_DOWN==0){
        if(STATE==STA_NORMAL)return;
        if(STATE==STA_SETBED)setBed--;
        if(STATE==STA_SETEXT)setExt--;
        btnPressed=1;
	}
    
}

void main(void){
	//Init Timer0
	TMOD=2;//00000010 8bit autoreload
	TH0=6;//4 cycle equ 1ms
	TL0=0;
	ET0=1;
	EA=1;
	TR0=1;
	while(1){
		display();
		parseButton();
	}
}
