#include<reg52.h>
#include<stdlib.h>
#include<stdio.h>
#include<intrins.h>
#define uint unsigned int
#define uchar unsigned char
 
uint num,A_num,x,y=0;
uint h,m,s,year=2024,month=5,day=23,lcd_x=0,lcd_y=0;
 
uchar code Zifu[]="0123456789";
 
 
sbit LCD_EN = P3^4;
sbit LCD_RS = P3^5;
 
 
sbit key_A = P1^0;
sbit key_B = P1^1;
sbit key_C = P1^2;
sbit key_D = P1^3;

sbit led_1 = P2^0;

void delay_1ms(uchar x){
	uchar i,j;
	for(j=0;j<x;j++)
		for(i=0;i<110;i++);
}
 
void write_command(uchar command){
	LCD_RS = 0;
	LCD_EN = 0;
	P0 = command;
	delay_1ms(2);
	LCD_EN = 1;	 		
	delay_1ms(2);
	LCD_EN = 0;
}
 
void write_data(uchar f){
	LCD_RS = 1;
	LCD_EN = 0;
	P0 = f;
	delay_1ms(2);
	LCD_EN = 1;
	delay_1ms(2);
	LCD_EN = 0;
}
 
void lcd_post(int X,int Y){			
	write_command(0x80+X*(0x40)+Y);
}
 
void init(){
	led_1 = 0;
	delay_1ms(1000);
	led_1 = 1;
	h=m=s=0;
	num=A_num=0;
 
	LCD_EN=0;
	write_command(0x38);
	write_command(0x0c);

	write_command(0x01);
 
	TMOD = 0x02;
	TH0 = 6;
	TL0 = 6;
	EA = 1;
	ET0 = 1;
	TR0 = 1;
 
	
	lcd_post(0,0); write_data(Zifu[h/10]);
	lcd_post(0,1); write_data(Zifu[h%10]);
 
	lcd_post(0,2); write_data(':');
 
	lcd_post(0,3); write_data(Zifu[m/10]);
	lcd_post(0,4); write_data(Zifu[m%10]);
 
	lcd_post(0,5); write_data(':');
 
	lcd_post(0,6); write_data(Zifu[s/10]);
	lcd_post(0,7); write_data(Zifu[s%10]);
 

	lcd_post(1,6); write_data(Zifu[year/1000]);
	lcd_post(1,7); write_data(Zifu[(year%1000)/100]);
	lcd_post(1,8); write_data(Zifu[(year%100)/10]);
	lcd_post(1,9); write_data(Zifu[year%10]);
 
	lcd_post(1,10); write_data('-');
 
	lcd_post(1,11); write_data(Zifu[month/10]);
	lcd_post(1,12); write_data(Zifu[month%10]);
 
	lcd_post(1,13); write_data('-');
 
	lcd_post(1,14); write_data(Zifu[day/10]);
	lcd_post(1,15); write_data(Zifu[day%10]);
}
 
 
void keyscan(){

			if(key_C==0){
			delay_1ms(3);
			if(key_C==0){	

					m=(++m)%60;
					lcd_post(0,3); write_data(Zifu[m/10]);
					lcd_post(0,4); write_data(Zifu[m%10]);

			while(!key_C);
		}
	}
	

	

		if(key_B==0){
			delay_1ms(3);
			if(key_B==0){	


					h=(++h)%24;
					lcd_post(0,0); write_data(Zifu[h/10]);
					lcd_post(0,1); write_data(Zifu[h%10]);

			while(!key_B);
		}
	}		
}
 
void main()
{
	init();
	y = 0;
	while(1){
		
		
if(key_A==0){  
		delay_1ms(3);
		if(key_A==0){
			y = 1;
		}
		while(!key_A);
	}
if(key_D==0){  
		delay_1ms(3);
		if(key_D==0){
			y = 0;
		}
		while(!key_D);
}
		


		
		if(num==3686){
			num=0;
			s++;
			if(s==60){
				s=0;
				m++;
				if(m==60){
					m=0;
					h++;
					if(h==24)h=0;
					lcd_post(0,0); write_data(Zifu[h/10]);
					lcd_post(0,1); write_data(Zifu[h%10]);
				}
				lcd_post(0,3); write_data(Zifu[m/10]);
				lcd_post(0,4); write_data(Zifu[m%10]);
			}
			lcd_post(0,6); write_data(Zifu[s/10]);
			lcd_post(0,7); write_data(Zifu[s%10]);
		}
		
		if(y == 1)
		{
		keyscan();
		}
		
		
		
	}
}
 
void T0_time() interrupt 1
{
	num++;
}
