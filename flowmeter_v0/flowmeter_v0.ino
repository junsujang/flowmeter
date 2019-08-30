// -------------------------------------------------------------
// Simple flowmeter system with Teensy and CAN-bus
// Aug 29th 2019
// Initial Author: Junsu Jang
//
// Description: 
// Teensy uses Can0 (pin3(TX) and pin4(RX)) for communication
// It waits for a command from the master (RESET or SET) and
// at every sampling rate transmits a message as its heartbeat

#include <FlexCAN.h>

// Define global variables
#define DEFAULT_SR 1000000      // Default sampling rate at 1Hz
#define DEVICE_ID 0x100         // Arbitrarily chosen device ID for now
#define MSG_LEN 8               // (bytes) 32-bit long counter value
#define RESET 0x10              // Arbitrarily chosen value assignment for reset
#define SET   0x11              // Arbitrarily chosen value assignment for setting sampling rate
#define CAN_BAUDRATE 250000

IntervalTimer fm_timer;         // flowmeter timer instance
static CAN_message_t out_msg;   // message instance for tx

// Initialize shared variables
uint32_t sampling_period = DEFAULT_SR;
volatile uint32_t flow_counter = 0;
volatile uint8_t new_meas = 0;  // flag for new measurement

// -------------------------------------------------------------
static void meas_flow()
{
  // 1. Measure the flow and store the value
  // 2. set the new measurement flag
  
  // Simple increment for now, please replace with 
  // appropriate sampling code
  flow_counter++;
  new_meas = 1;
  Serial.println(flow_counter);
}

// -------------------------------------------------------------
static void handle_cmd(uint8_t *cmdPtr)
{
  // I assume the following:
  // First byte of the message contains the command 
  // next 4bytes of the message contains the relevant parameter values
  // (i.e. sampling rate in microseconds)
  
  uint8_t cmd = cmdPtr[0];
  Serial.println(cmd);
  // Reset counter
  if (cmd == RESET)
  {
    flow_counter = 0;
    new_meas = 0;
    Serial.println("reset the flowmeter");
  }
  else if (cmd == SET)
  {
    uint32_t new_sr = 0;
    for(int i = 1; i < 5; i++) {
      new_sr = (new_sr << 4) | cmdPtr[i];
    }
    fm_timer.update(new_sr);
    Serial.print("Updated the sampling rate to ");
    Serial.println(new_sr);
  }
}

// -------------------------------------------------------------
void setup(void)
{
  delay(1000);
  Serial.println(F("Hello naive Flowmeter"));  
  
  // Initialize the CAN-bus instance for communication
  Can0.begin(CAN_BAUDRATE);
//  //if using enable pins on a transceiver they need to be set on
//  pinMode(2, OUTPUT);
//
//  digitalWrite(2, HIGH);

  // Extension for 11bits or 29bits long ID
  out_msg.ext = 0;
  out_msg.id = DEVICE_ID; 
  // Send out 8-bits long message everytime
  out_msg.len = MSG_LEN;
  // Initialize the msg buffer to all nulls
  for (int i = 0; i < MSG_LEN; i++) {
    out_msg.buf[i] = 0;
  }

  // Initialize the interval timer instance for sampling
  fm_timer.begin(meas_flow, sampling_period);
}

// -------------------------------------------------------------
void loop(void)
{
  // Check if something is in the message
  //  Yes: Handle command
  //  No: If there has been a new sample value, transmit 32bit data
  
  while (Can0.available()) 
  {
    CAN_message_t inMsg;
    Can0.read(inMsg);
    handle_cmd(inMsg.buf);
  }
  
  if (new_meas)
  {
    Serial.println("new mas");
    // the library restricts 8bit max per msg
    // New counter is available, transmit as heartbeat
    for (int i = 0; i < 4; i++)
    {
      out_msg.buf[3-i] = (flow_counter >> (4*i)) & 0xFF;
    }
    Can0.write(out_msg);
    // Reset the new measurement flag
    new_meas = 0;
  }
}
