/*
fht_adc.pde
guest openmusiclabs.com 9.5.12
example sketch for testing the fht library.
it takes in data on ADC0 (Analog0) and processes them
with the fht. the data is sent out over the serial
port at 115.2kb.  there is a pure data patch for
visualizing the data.
*/

#define OCTAVE 1 // use the log output function
#define FHT_N 16 // set to 16 point fht

#include <FHT.h> // include FHT for signal filtering
#include <QueueList.h> // include Queue's for noise detection
//#include <pnew.cpp>
//#include <vector>

int bluePin = 3;
int redPin = 6;
int greenPin = 5; // "Green" pin ;)

int blueBand = 4;
int redBand = 8;
int greenBand = 16;

int blueNorm = 254;
int redNorm = 41;
int greenNorm = 11;

int blueVal;
int redVal;
int greenVal;

// Accumulator for rgb mean check
int blueCount = 0;
int redCount = 0;
int greenCount = 0;

// Counter checking if rgb is at mean
int blueCheck;
int redCheck;
int greenCheck;

// Threshold for all check values
int checkThresh = 100;

//std::vector<int> blueDetect;
//std::vector<int>::const_iterator it;

void setup() {
  pinMode(bluePin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS22);
  TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00); 
  TCCR0B = _BV(CS00); 
  OCR2A = 180;
  OCR2B = 50;
  Serial.begin(57600); // use the serial port
  //TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void loop() {
  while(1) { // reduces jitter
    cli();  // UDRE interrupt slows this way down on arduino1.0
    for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fht_input[i] = k; // put real data into bins
    }
    fht_window(); // window the data for better frequency response
    fht_reorder(); // reorder the data before doing the fht
    fht_run(); // process the data in the fht
    fht_mag_octave(); // take the output of the fht
    sei();
//    Serial.print("blueBand: ");
//    Serial.print(fht_oct_out[blueBand]);
//    Serial.print(" redBand: ");
//    Serial.print(fht_oct_out[redBand]);
//    Serial.print(" greenBand: ");
//    Serial.println(fht_oct_out[greenBand]);
    setLED(blueNorm, blueBand, bluePin, fht_oct_out); // Pin 3
    setLED(greenNorm, greenBand, greenPin, fht_oct_out); // Pin 6
    setLED(redNorm, redBand, redPin, fht_oct_out); // Pin 5
  }
}

void setLED(int norm, int band, int pinNum, uint8_t* fht) {
 if (fht[band] != norm) {
  analogWrite(pinNum, fht[band]);
  //return 1;
 }
 else {
  analogWrite(pinNum, 0);
  //return 0;
 }
}
