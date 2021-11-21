/*
 * ESE519_Lab4_Pong_Starter.c
 *
 * Created: 9/21/2021 21:21:21 AM
 * Author : J. Ye
 */ 


#define F_CPU 16000000UL

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "ST7735.h"
#include "LCD_GFX.h"
#include <util/delay.h>
#include "uart.h"

#define BAUD_RATE 9600
#define BAUD_PRESCALER (((F_CPU / (BAUD_RATE * 16UL))) - 1)
#define paddle_w 2
#define paddle_l 30

int p1=0, p2=0;//player score
int restart=0;//restart or not
int xp1, yp1=0;//player1 paddle position
int xp2, yp2=0;//player2 paddle position
int xball, yball=0;//ball position
int r=2;
int start_1, start_2, start_3=0;
int xball_0, yball_0 = 0;
int dxball, dyball=0;
int c =0;
int player1_r=0;
int player2_r=0;
char String[15];

void set_up_ADC()
{
	
	PRR &= ~(1<<PRADC);//clear power reduction for ADC
	
	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);//select Vref = Avcc
	
	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);//set the ADC clock divided by 128, 125KHz
	
	ADMUX &= ~(1<<MUX0);
	ADMUX &= ~(1<<MUX1);
	ADMUX &= ~(1<<MUX2);
	ADMUX &= ~(1<<MUX3);//select channel 0
	
	ADCSRA |= (1<<ADATE);//set to auto trigger
	
	ADCSRB &= ~(1<<ADTS0);
	ADCSRB &= ~(1<<ADTS1);
	ADCSRB &= ~(1<<ADTS2);//set to free running
	
	DIDR0 |= (1<<ADC0D);//disable digital input buffer on ADC pin
	
	ADCSRA |= (1<<ADEN);//enable ADC
	
	ADCSRA |= (1<<ADIE);//enable ADC interrupt
	
	ADCSRA |= (1<<ADSC);//start conversion

	
}

void set_up_timer2()
{
    DDRD |= (1<<DDD3);//Set PD3 as output port
     
    TCCR2B |= (1<<CS20);
    TCCR2B |= (1<<CS21);
    TCCR2B |= (1<<CS22);//Prescale the Timer2 by 1024
     
    TCCR2A |= (1<<WGM20);
    TCCR2A |= (1<<WGM21);
    TCCR2B |= (1<<WGM22);//Set Timer2 as phase correct PWM, TOP = OCRA

     
    OCR2A = 0;
    OCR2B = 0;
     
    TCCR2A |= (1<<COM2B1);
     
}

void background_setup()
{
	LCD_drawString(70, 5, "PONG", RED, GREEN);
}


void paddle()
{
	sprintf(String,"ADC: %d\n", ADC);
	UART_putstring(String);
	if (restart == 0)
	{
		if (ADC<600&&start_1==0)//no joystick movement
		{
			xp1 = 0;
			yp1 = 50;
			LCD_drawBlock(xp1,yp1,xp1+paddle_w,yp1+paddle_l,RED);
			xp2 = 158;
			yp2 = 50;
			LCD_drawBlock(xp2,yp2,xp2+paddle_w,yp2+paddle_l,BLUE);
		    start_1=1;
		}
		if (ADC<50&&start_1==1)//joystick move down
		{
				if (yp2>6)
				{
					LCD_drawBlock(xp1,yp1,xp1+paddle_w,yp1+8,WHITE);
					yp1=yp1+8;
					LCD_drawBlock(xp1,yp1,xp1+paddle_w,yp1+paddle_l,RED);
					yp2 = 98-yp1+2;
					LCD_drawBlock(xp2,yp2+paddle_l,xp2+paddle_w,yp2+paddle_l+8,WHITE);
					LCD_drawBlock(xp2,yp2,xp2+paddle_w,yp2+paddle_l,BLUE);
				}
	
        }
		else if (ADC>600&&start_1==1)//joystick move up
		{
				if (yp1>6&&yp2>0)
				{
						yp1=yp1-8;
						LCD_drawBlock(xp1,yp1+paddle_l,xp1+paddle_w,yp1+paddle_l+8,WHITE);
						LCD_drawBlock(xp1,yp1,xp1+paddle_w,yp1+paddle_l,RED);
						LCD_drawBlock(xp2,yp2,xp2+paddle_w,yp2+7,WHITE);
						yp2 = 98-yp1;
                        LCD_drawBlock(xp2,yp2,xp2+paddle_w,yp2+paddle_l,BLUE);					
				}			
			
		}
	}
}


void ball()
{
    LCD_drawCircle(xball_0,yball_0 ,2,WHITE);
	if (start_3 == 0 && p1<5 && p2<5) // if condition satisfied to start round
	{
		xball = 80;
		yball = 64; // center of the LCD
		LCD_drawCircle(xball,yball,2, BLACK);
		start_3 = 1;
		switch (c)
		{
			case 0:
			dxball = 5;
			dyball = 9;
			c = 1;
			break;
			case 1:
			dxball = -5;
			dyball = 5;
			c = 2;
			break;
			case 2:
			dxball = 5;
			dyball = -7;
			c = 3;
			break;
			case 3:
			dxball = -5;
			dyball = -6;
			c = 4;
			break;
			case 4:
			dxball = -5;
			dyball = 8;
			c = 0;
			break;
		}	
		
	}
	else
	{
        xball_0=xball;
		yball_0=yball;
		xball=xball+dxball;
		yball=yball+dyball;
		LCD_drawCircle(xball_0,yball_0 ,2,WHITE);
		
		// wall interaction
		if (yball >= 125 ||yball <= 5)
		{
			dyball = dyball*(-1); // horizontal wall
			OCR2A = 10;
			OCR2B = 1;
			Delay_ms(200);
			OCR2A = 0;
			OCR2B = 0;
		}
		
		if (xball >= 157) // player 2 loses
		{
			start_3 = 0;
			p1++;
			LCD_drawCircle(xball,yball,2,WHITE);

			PORTD |= (1<<PORTD4);
			OCR2A = 10;
			OCR2B = 1;
			_delay_ms(200);
			PORTD &= ~(1<<PORTD4);
			OCR2A = 0;
			OCR2B = 0;
		}
		
		if (xball <= 3) //player 1 loses
		{
			start_3 = 0;
			p2++;
			LCD_drawCircle(xball,yball ,2,WHITE);
			PORTD |= (1<<PORTD2);
			OCR2A = 10;
			OCR2B = 1;
			_delay_ms(200);
			PORTD &= ~(1<<PORTD2);
			OCR2A = 0;
			OCR2B = 0;
		}
		
		// paddle interaction
		if (xball <= paddle_w+4 && (yball>yp1 && yball<yp1+paddle_l))
		{
			dxball=dxball*(-1);//paddle hit
			OCR2A = 10;
			OCR2B = 1;
			_delay_ms(200);
			OCR2A = 0;
			OCR2B = 0;
		}
		else if (xball >=160-(paddle_w+4)&& (yball>yp2 && yball<yp2+paddle_l))
		{
			dxball=dxball*(-1);//paddle hit
			OCR2A = 10;
			OCR2B = 1;
			_delay_ms(200);
			OCR2A = 0;
			OCR2B = 0;
		}
	}
	//Ball draw
    
	xball_0=xball;
	yball_0=yball;
	LCD_drawCircle(xball,yball,2,BLACK);
	
}

void score_1()
{
	if(p1==0)
	LCD_drawString(5,5,"0",RED, WHITE);
	else if(p1==1)
	LCD_drawString(5,5,"1",RED, WHITE);
	else if(p1==2)
	LCD_drawString(5,5,"2",RED, WHITE);
	else if(p1==3)
	LCD_drawString(5,5,"3",RED, WHITE);
	else if(p1==4)
	LCD_drawString(5,5,"4",RED, WHITE);
	else if(p1==5)
	LCD_drawString(5,5,"5",RED, WHITE);
}

void score_2()	
{
	if(p2==0)
	LCD_drawString(150,5,"0",BLUE, WHITE);
	else if(p2==1)
	LCD_drawString(150,5,"1",BLUE, WHITE);
	else if(p2==2)
	LCD_drawString(150,5,"2",BLUE, WHITE);
	else if(p2==3)
	LCD_drawString(150,5,"3",BLUE, WHITE);
	else if(p2==4)
	LCD_drawString(150,5,"4",BLUE, WHITE);
	else if(p2==5)
	LCD_drawString(150,5,"5",BLUE, WHITE);
}

void restart_game()
{
	if (player1_r==0 && player2_r==0)//round 1 begin
	{
		LCD_drawString(65, 12, "Round1", RED, GREEN);
		LCD_drawString(125,5,"R:0",BLUE, WHITE);
		LCD_drawString(20,5,"R:0",RED, WHITE);
		if (p1==5 && p2!=5)
		{
			player1_r++;
			LCD_drawString(50,60,"Round 1 P1 WINS!",RED, WHITE);
			LCD_brightness(10);
			LCD_setScreen(WHITE);
			background_setup();
			LCD_drawString(65, 12, "Round2", RED, GREEN);
			LCD_brightness(127);
			p1=0;
			p2=0;
			start_1 = 0;
			restart = 0;
		}
		else if(p1!=5 && p2==5)
		{
			player2_r++;
			LCD_drawString(50,60,"Round 1 P2 WINS!",BLUE, WHITE);
			//Delay_ms(3000);
			LCD_brightness(10);
			LCD_setScreen(WHITE);
			background_setup();
			LCD_drawString(65, 12, "Round2", RED, GREEN);
			LCD_brightness(127);
			p1=0;
			p2=0;
			start_1 = 0;
			restart = 0;
		}
	}
	if (player1_r+player2_r==1)//round 2 begin
	{
		if (player1_r==1)
		{
			LCD_drawString(125,5,"R:0",BLUE, WHITE);
			LCD_drawString(20,5,"R:1",RED, WHITE);
		}
		if (player2_r==1)
		{
			LCD_drawString(125,5,"R:1",BLUE, WHITE);
			LCD_drawString(20,5,"R:0",RED, WHITE);
		}
		Delay_ms(300);
		if (p1==5 && p2!=5 && player1_r==0)
		{
			player1_r++;
			LCD_drawString(50,60,"Round 2 P1 WINS!",RED, WHITE);
			//Delay_ms(3000);
			LCD_brightness(10);
			LCD_setScreen(WHITE);
			background_setup();
			LCD_drawString(65, 12, "Round3", RED, GREEN);
			LCD_brightness(127);
			p1=0;
			p2=0;
			start_1 = 0;
			restart = 0;
		}
		else if(p1!=5 && p2==5&& player2_r==0)
		{
			player2_r++;
			LCD_drawString(50,60,"Round 2 P2 WINS!",BLUE, WHITE);
			//Delay_ms(3000);
			LCD_brightness(10);
			LCD_setScreen(WHITE);
			background_setup();
			LCD_drawString(65, 12, "Round3", RED, GREEN);
			LCD_brightness(127);
			p1=0;
			p2=0;
			start_1 = 0;
			restart = 0;
		}
		else if(p1==5 && p2!=5 && player1_r==1)
		{
			player1_r=0;
			player2_r=0;
			LCD_drawString(50,60,"The End! P1 WINS!",RED, WHITE);
			//Delay_ms(3000);
			LCD_brightness(10);
			LCD_setScreen(WHITE);
			background_setup();
			LCD_brightness(127);
			p1=0;
			p2=0;
			start_1 = 0;
			restart = 0;
		}
		else if(p1!=5 && p2==5 && player2_r==1)
		{
			player1_r=0;
			player2_r=0;
			LCD_drawString(50,60,"The End! P2 WINS!",BLUE, WHITE);
			//Delay_ms(3000);
			LCD_brightness(10);
			LCD_setScreen(WHITE);
			background_setup();
			LCD_brightness(127);
			p1=0;
			p2=0;
			start_1 = 0;
			restart = 0;
		}
	}
	if (player1_r+player2_r==2)//round 3 begin
	{
		LCD_drawString(125,5,"R:1",BLUE, WHITE);
		LCD_drawString(20,5,"R:1",RED, WHITE);
		if (p1==5 && p2!=5)
		{
			player1_r=0;
			player2_r=0;
			LCD_drawString(45,60,"The End! P1 WINS!",RED, WHITE);
			//Delay_ms(3000);
			LCD_brightness(10);
			LCD_setScreen(WHITE);
			background_setup();
			LCD_brightness(127);
			p1=0;
			p2=0;
			start_1 = 0;
			restart = 0;
		}
		else if(p1!=5 && p2==5)
		{
			player1_r=0;
			player2_r=0;
			LCD_drawString(45,60,"The End! P2 WINS!",BLUE, WHITE);
			//Delay_ms(3000);
			LCD_brightness(10);
			LCD_setScreen(WHITE);
			background_setup();
			LCD_brightness(127);
			p1=0;
			p2=0;
			start_1 = 0;
			restart = 0;
		}
	}
    
}
void Initialize()
{

	DDRD |= (1<<DDD4);
	DDRD |= (1<<DDD2);
	//DDRB &= ~(1<<DDB0);
	//DDRB &= ~(1<<DDB2);//set PD4, PD2 as output and PB0, PB2 as input
	lcd_init();
	background_setup();
	set_up_ADC();
	set_up_timer2();
	
}

int main(void)
{
	Initialize();
    LCD_setScreen(WHITE);
	UART_initialize(BAUD_PRESCALER);
	//LCD_drawBlock(0,127,160,128,GREEN);
	//int i=9; //testing
    while (1) 
    {
		background_setup();
		paddle();
		ball();
		score_1();
		score_2();
		restart_game();
		/*if (i==1)
		{
			LCD_drawPixel(100,120,RED);
			_delay_ms(5000);
		}
		if (i==2)
		{
			LCD_drawChar(100, 30, displayChar,BLACK, WHITE);
			_delay_ms(500);
			displayChar++;
		}
		if (i==3)
		{
			LCD_drawCircle(50, 20, 10, GREEN);
			_delay_ms(5000);
		}
		if (i==4)
		{
			LCD_drawBlock(0,0,50,60,RED);
			_delay_ms(5000);
		}
		if (i==5)
		{
			LCD_drawLine(0,0,100,120,BLACK);
			_delay_ms(5000);
		}
		if (i==6)
		{
			LCD_setScreen(WHITE);
			_delay_ms(5000);
		}
		if (i==7)
		{
			LCD_drawString(50,50, "You Win!", RED, BLUE);
			_delay_ms(5000);
		}
		if (i==8)
		{
			LCD_drawrect(50,40,80,90,GREEN);
			_delay_ms(5000);
		}*/
    }
}