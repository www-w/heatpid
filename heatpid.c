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
unsigned char STATE=STA_NORMAL;
unsigned char disIndex=0;
unsigned char STRING[]="BED   SETBED   EXT   SETEXT   ";
void timer0(void) __interrupt 1    //000BH
{
	ms4++;
	if(ms4==4){
		ms++;
		ms4=0;
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
/*
void displayPort(unsigned char LR){
	P3_1=LR&1;delay();
	P3_2=LR&2;delay();
	P3_3=LR&3;delay();
	P3_4=LR&4;delay();
	P3_5=LR&5;delay();
	P3_6=LR&6;delay();
	P3_7=LR&7;delay();

}*/
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
	P3=LChr;
	delay();
	P1_2=0;P1_1=1;
	P3=RChr;
	delay();
}
void parseButton(void){
    if(btnPressed){
        btnPressed=BTN_MODE&BTN_UP&BTN_DOWN;
        btnPressed=!btnPressed;
    }
    if(btnPressed)return;
    
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
