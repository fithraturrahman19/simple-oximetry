#include <Wire.h>
#include <avr/io.h>
#define MCP4725_ADDR 0x60
#define SAMPLEFILTER_TAP_NUM 11

typedef struct {
  double history[SAMPLEFILTER_TAP_NUM];
  unsigned int last_index;
} SampleFilter;

void SampleFilter_init(SampleFilter* f);
void SampleFilter_put(SampleFilter* f, double input);
double SampleFilter_get(SampleFilter* f);
void adc_init();
int adc_read(int ch);
void sendAddress();

double temp=0.0;
SampleFilter X;
unsigned int Y;
byte buffera[2];
double filter_taps[SAMPLEFILTER_TAP_NUM] = {
  0.0145973292612419,
  0.0306285267463775,
  0.0725994062012431,
  0.124479701492389,
  0.166452568056866,
  0.182484936483765,
  0.166452568056866,
  0.124479701492389,
  0.0725994062012431,
  0.0306285267463775,
  0.0145973292612419
};

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  SampleFilter_init(&X);
  adc_init();
}

void loop()
{
  PORTB = 0x01//sample
  PORTB = 0x00//hold
  temp = (double) adc_read(0);
  SampleFilter_put(&X,temp);
  Y = (unsigned int) SampleFilter_get(&X);
  buffera[0] = Y>>2; // MSB 11-4 shift right 4 places
  buffera[1] = ((Y<<2)&15)<<4; // LSB 3-0 shift left 4 places
  Wire.beginTransmission(MCP4725_ADDR);
  Wire.write(64);
  Wire.write(buffera[0]);  // 8 MSB
  Wire.write(buffera[1]);  // 4 LSB
  Wire.endTransmission();
  Serial.println(adc_read(1));
}

void SampleFilter_init(SampleFilter* f) {
  int i;
  for(i = 0; i < SAMPLEFILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void SampleFilter_put(SampleFilter* f, double input) {
  f->history[f->last_index++] = input;
  if(f->last_index == SAMPLEFILTER_TAP_NUM)
    f->last_index = 0;
}

double SampleFilter_get(SampleFilter* f) {
  double acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < SAMPLEFILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : SAMPLEFILTER_TAP_NUM-1;
    acc += f->history[index] * filter_taps[i];
  };
  return acc;
}

void adc_init()
{
    // AREF = AVcc
    ADMUX = (1<<REFS0);
    // ADC Enable and prescaler of 128
    // 16000000/128 = 125000
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}
int adc_read(int ch)
{
  // select the corresponding channel 0~7
  // ANDing with ’7′ will always keep the value
  // of ‘ch’ between 0 and 7
  ch &= 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
 
  // start single convertion
  // write ’1′ to ADSC
  ADCSRA |= (1<<ADSC);
  // wait for conversion to complete
  // ADSC becomes ’0′ again
  // till then, run loop continuously
  while(ADCSRA & (1<<ADSC));
  return (ADCW);
}

