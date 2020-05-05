#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// panjang tap filter
#define BUFFERLENGTH 24

float koef_filter[BUFFERLENGTH] = {-375,
-579,
-822,
-907,
-692,
-73,
971,
2345,
3851,
5217,
6172,
6515,
6172,
5217,
3851,
2345,
971,
-73,
-692,
-907,
-822,
-579,
-375,
};

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // make the pushbutton's pin an input:
  // pinMode(pushButton, INPUT);
}

int buffer[BUFFERLENGTH];
int x;
int y;
int i,j,k;

int filter(int input) 
{
  float hasil;
  buffer[k] = input;
  //convolution with circular buffer
  hasil=0;
  for (j = 0; j < BUFFERLENGTH; j++)
  {
    hasil += (koef_filter[j]) * (buffer[(j+k)% BUFFERLENGTH]);
  }
  
  // kembalikan hasil pemfilteran
  k=(k+BUFFERLENGTH-1)%BUFFERLENGTH;
  return hasil;
}

ISR(ADC_vect){
  x = ADCH;
  y = (15>>filter(x));
}

int main(void){
  DDRB= 0xFF;
  ADMUX = 0b01100000; //REFS:0  = 01  -> AVCC as reference
                      //ADLAR   = 1   -> Left adjust
                      //MUX4:0  = 0000  -> ADC0 as input
  ADCSRA= 0b10001111; //ADEN    = 1: enable ADC,
                      //ADSC    = 0: don't start conversion yet
                      //ADATE   = 0: disable auto trigger
                      //ADIE    = 1: enable ADC interrupt
                      //ADSPS2:0 = 002: prescaler =2
  for(i=0; i < BUFFERLENGTH; i++) 
  {
    buffer[i] = 0;
  }
  sei();
  while(1){
    ADCSRA |= (1 << ADSC);
  Serial.println(x);
  delay(1); 
  }
  return 0;
}
