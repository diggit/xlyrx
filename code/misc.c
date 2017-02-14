// Copyright (c) 2017 Patrik Bachan
//
// GNU GENERAL PUBLIC LICENSE
//    Version 3, 29 June 2007
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "misc.h"
#include "config.h"

int32_t linear_aproximation(int32_t x, int32_t x0, int32_t y0, int32_t x1, int32_t y1)
{
	if(x1==x0)//prevent division by 0
		return 0;

	return (x-x0)*(y1-y0)/(x1-x0) + y0;
}

// uint8_t wait_timeout(volatile uint8_t *port, uint8_t mask, uint8_t timeout, uint8_t high)
// {
// 	uint8_t counter;
// 	for(counter=0;
// 		((!(*port & mask) && high ) || //high level stop
// 		((*port & mask) && !high )) //low level stop
// 		&& counter!=timeout ;counter++)
//
// 		NOP;
// 	if(counter==timeout)
// 		return 1;
//
// 	else
// 		return 0;
// }
//
// uint8_t wait_timeout_simple(volatile uint8_t *port, uint8_t mask)
// {
// 	return wait_timeout(port,mask,255,1);
// }

uint32_t reduce(uint32_t value, uint8_t decrease) //correct rounding, not just cropping
{
	uint16_t dec=1;
	for(;decrease>1;decrease--)//calculate divider
		dec*=10;

	if (value/dec%10 > 4)//round
		return value/(10*dec)+1;
	else
		return value/(10*dec);
}

char char_buffer[BUFF_SIZE] = {0};


char* stoa(const char *str)
{
	uint8_t length;
	for(length=0;str[length]!='\0' && length<BUFF_SIZE-1;length++)
		char_buffer[length]=str[length];
	char_buffer[length]='\0';
	return char_buffer;
}

char* ctoa(char c)
{
	char_buffer[0]=c;
	char_buffer[1]='\0';
	return char_buffer;
}

char* itoa_dec_fill(int32_t val,int min,char filler,int decimals)//coverts number to string with defined base and minimal digit, char fillers
{

	uint8_t i = 30; //max characters
	uint8_t negative=0;

	if (val==0)
	{
		char_buffer[i--]='0';
	}

	else
	{
		if(val<0)
		{
			negative=1;
			val=-val;
		}
		for (; (val && i); --i, val /= 10)
		{
			if(30-i==decimals && decimals!=0 )
				char_buffer[i--]='.';

			char_buffer[i] = "0123456789abcdef"[val % 10];
		}

	}

	if(30-i<=decimals)
	{
		for(;30-i<decimals;)
			char_buffer[i--]='0';
		char_buffer[i--]='.';
		char_buffer[i--]='0';
	}

	if(negative)
		char_buffer[i--]='-';

	for (; min+i>30; --i)
		char_buffer[i] = filler;

	return &char_buffer[i+1];
}

char* itoa_fill(int32_t val, int min, char filler)
{
	return itoa_dec_fill(val,min,filler,0);
}

char* itoa(int32_t val, int min)
{
	return itoa_dec_fill(val,min,' ',0);
}

char* itoa_dec(int32_t val,int min,int decimals)//coverts number to string with defined base and minimal digit, char fillers
{
	return itoa_dec_fill(val,min,' ',decimals);
}

//untedsted
// int32_t atoi(char *start)
// {
// 	int32_t num=0;
// 	uint8_t negative=0;
// 	switch(*start)
// 	{
// 		case '-':
// 			negative=1;
// 		case '+':
// 			start++;
// 			break;
// 	}
// 	while(*start>='0' && *start<='9')
// 	{
// 		num*=10;
// 		num+=*start-'0';
// 		start++;
// 	}
// 	if(negative)
// 		return -num;
// 	else
// 		return num;
// }

#if F_CPU != 72000000U
	#warning "delay_ms and delay_us calibrated for 72MHz"
#endif
void delay_us(uint16_t delay)
{
	delay=(delay*8);
	while(delay--) {;NOP;NOP;NOP;}
}//dont try to optimize, it's calibrated! (gcc -O2)

void delay_ms(uint16_t delay) //calibrated using oscope
{
	while(delay--) delay_us(1000);
}