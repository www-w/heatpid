#include "heatpid.h"
#define STA_NORMAL 0
#define STA_SETBED 1
#define STA_SETEXT 2
unsigned char bedTmp;
unsigned char extTmp;
unsigned char LChr;
unsigned char RChr;
unsigned char ms4=0;
unsigned char ms=0;
unsigned char sec4=0;
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
void displayPort(unsigned char LR){
	if(LR=='L')P1_1=0;
	if(LR=='R')
	
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
	}
}
