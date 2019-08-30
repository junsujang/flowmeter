// -------------------------------------------------------------
// based on CANtest for Teensy 3.6 dual CAN bus
// modified to simulate a master node that controls the flowmeter
// Aug 29th, 2019
// Author: JunSu Jang
// At every 5 seconds it either resets the flowmeter or changes the 
// sampling period between 0.5sec and 1sec (ish)

#include <FlexCAN.h>

#ifndef __MK66FX1M0__
  #error "Teensy 3.6 with dual CAN bus is required to run this example"
#endif

static CAN_message_t msg;
static uint8_t hex[17] = "0123456789abcdef";

IntervalTimer resetTimer;
uint8_t sampling_rate_counter = 0;
volatile uint8_t is_reset = 0;
volatile uint8_t sr_change = 0;
volatile uint32_t cur_sr = 1000000;
#define SR 500000 // 0.5sec

// -------------------------------------------------------------
static void hexDump(uint8_t dumpLen, uint8_t *bytePtr)
{
  uint8_t working;
  while( dumpLen-- ) {
    working = *bytePtr++;
    Serial.write( hex[ working>>4 ] );
    Serial.write( hex[ working&15 ] );
  }
  Serial.write('\r');
  Serial.write('\n');
}

static void comm()
{
  is_reset = 1;
  sampling_rate_counter++;
  // Alter the sampling rate every 10 seconds
  if (sampling_rate_counter % 2 == 0)
  {
    cur_sr = (cur_sr + SR) % 1000001;
    sr_change = 1;
  }
}

// -------------------------------------------------------------
void setup(void)
{
  delay(1000);
  Serial.println(F("Hello Teensy 3.6 dual CAN Test."));

  Can0.begin();  

  //if using enable pins on a transceiver they need to be set on
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);

  msg.ext = 0;
  msg.id = 0x100;
  msg.len = 8;
  resetTimer.begin(comm, 5000000);
}


// -------------------------------------------------------------
void loop(void)
{
  CAN_message_t inMsg;
  while (Can0.available()) 
  {
    Can0.read(inMsg);
    Serial.print("CAN bus 0: "); hexDump(8, inMsg.buf);
  }
  if (sr_change) {
    msg.buf[0] = 0x11;
    for (int i = 0; i < 4; i++) {
      msg.buf[4-i] = (cur_sr >> (4*i)) % 0xFF;
    }
    Can0.write(msg);
    sr_change = 0;
    is_reset = 0;
    Serial.println("SR change");

  }
  else if (is_reset) {
    Serial.println("Reset");

    msg.buf[0] = 0x10;
    Can0.write(msg);
    is_reset = 0;
  }
}
