#include "SPI.h"


//IO Pins
const uint8_t InputPin = PIN_PA6;
const uint8_t TimePin = PIN_PA7;
const uint8_t ModePin = PIN_PB0;
const uint8_t RAM_CS = PIN_PB2;
const uint8_t DAC_CS = PIN_PB1;

//Audio Samples
volatile uint16_t sampleIn = 2047;
volatile uint16_t sampleOut = 2047;
volatile uint8_t sampleBuf[2] = {7, 255};

//Step Counters
volatile uint16_t sampleStep = 0;
volatile uint16_t delayStep = 0;

//Controls
volatile uint16_t delayTime = 32768;
volatile uint16_t targetTime = 32768;
volatile uint16_t extraClocks = 0;

//SPI
uint32_t _SPIspeed = 20000000;
SPISettings _spi_settings;

ISR(TCA0_OVF_vect) {                                        //runs each time the TCA0 periodic interrupt triggers
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;                 //clear the interrupt flags
  
  //read the input
  sampleIn = analogRead(InputPin);
  ADC0.MUXPOS = ((TimePin & 0x7F) << ADC_MUXPOS_gp);
    
  if (!(digitalReadFast(ModePin))){targetTime = targetTime << 4;} //multiplies by 16 (max 65408)
  else {targetTime = targetTime << 3;}                            //multiplies by 8 (max 32704)
  targetTime += 128;                                             //shift max to 65536
  if (delayTime < targetTime){delayTime += 2;}
  if (delayTime > targetTime){delayTime -= 2;}
  delayStep = sampleStep - delayTime;
  
  //read the delayed sample from RAM
  sampleOut = 0x3000;
  digitalWriteFast(RAM_CS, LOW);
  SPI.beginTransaction(_spi_settings);
  SPI.transfer(0x03);     //command READ
  SPI.transfer((uint8_t)((delayStep >> 8) & 0xFF)); //send top half of address to read
  SPI.transfer((uint8_t)(delayStep & 0xFF));        //send bottom half of address
  sampleOut |= (SPI.transfer(0xFF) << 8);           //send a dummy byte to get back the top half of sample
  sampleOut |= SPI.transfer(0xFF);                  //send a dummy byte to get back the bottom half of sample
  digitalWriteFast(RAM_CS, HIGH);

  //send the delayed sample to the DAC
  digitalWriteFast(DAC_CS, LOW);
  SPI.transfer((uint8_t)((sampleOut >> 8) & 0xFF));
  SPI.transfer((uint8_t)(sampleOut & 0xFF));
  digitalWriteFast(DAC_CS, HIGH);
  
  //store the newest sample in RAM
  digitalWriteFast(RAM_CS, LOW);
  SPI.transfer(0x02);        //command WRITE
  SPI.transfer((uint8_t)((sampleStep >> 8) & 0xFF));  //top half of adddress to write
  SPI.transfer((uint8_t)(sampleStep & 0xFF));         //bottom half of address to write
  SPI.transfer((uint8_t)((sampleIn >> 8) & 0xFF));    //top half of sample to store
  SPI.transfer((uint8_t)(sampleIn & 0xFF));           //bottom half of sample
  SPI.endTransaction();
  digitalWriteFast(RAM_CS, HIGH);

  //increment the step counter by 2 (since we're writing 2 bytes at a time), rolls over on its own at 65536
  sampleStep += 2;
  targetTime = analogRead(TimePin) & 0xFFF8;                     //masks the 3 LSBs for noise reduction (max 4088)
  ADC0.MUXPOS = ((InputPin & 0x7F) << ADC_MUXPOS_gp);
}

void setup() {
  //basic startup
  pinMode(InputPin, INPUT);
  pinMode(TimePin, INPUT);
  pinMode(ModePin, INPUT);
  pinMode(RAM_CS, OUTPUT);
  pinMode(DAC_CS, OUTPUT);
  digitalWriteFast(RAM_CS, HIGH);
  digitalWriteFast(DAC_CS, HIGH);
  //setup SPI and RAM chip
  SPI.begin();
  _spi_settings = SPISettings(_SPIspeed, MSBFIRST, SPI_MODE0);
  SPI.usingInterrupt(TCA0_OVF_vect);
  digitalWriteFast(RAM_CS, LOW);
  SPI.beginTransaction(_spi_settings);
  SPI.transfer(0b00000001);       //command WRMR (write mode register)
  SPI.transfer(0b01000000); //sequential mode
  SPI.endTransaction();
  digitalWriteFast(RAM_CS, HIGH);
  //fill RAM with 2047s
  digitalWriteFast(RAM_CS, LOW);
  SPI.beginTransaction(_spi_settings);
  SPI.transfer(0x02);       //command WRITE
  SPI.transfer((uint8_t) 0x00);
  SPI.transfer((uint8_t) 0x00);
  for (uint32_t i; i < 65536; i++){
    SPI.transfer((uint8_t)((sampleOut >> 8) & 0xFF));
    SPI.transfer((uint8_t)(sampleOut & 0xFF));
  }
  SPI.endTransaction();
  digitalWriteFast(RAM_CS, HIGH);

  //setup ADC
  analogReference(EXTERNAL);
  analogReadResolution(12);
  analogSampleDuration(3);
  analogClockSpeed(3000);
  delayTime = analogRead(TimePin) & 0xFFF8;                     //masks the 3 LSBs for noise reduction (max 4088)
  ADC0.MUXPOS = ((InputPin & 0x7F) << ADC_MUXPOS_gp);
  
  
  //setup interrupt timing for desired sample rate
  takeOverTCA0();
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;       //clears the flags
  TCA0.SINGLE.INTCTRL = 0b00000001;               //overflow interrupt every PER cycles
  TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;  //Clock prescaler / 1 (20 MHz)    
  TCA0.SINGLE.PER = 1000;                          //ADJUST SAMPLE RATE HERE: Currently 625 clocks = 32ksps. 1000 clocks = 20ksps
  TCA0.SINGLE.CTRLA |= 1;                         //Enables the timer
}

void loop() {
  // there is no main loop, after setup everything happens within the ISR

}
