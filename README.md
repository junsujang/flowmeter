# flowmeter
Teensyduino code for sampling from a flowmeter and communicating via CAN-bus
Simply have one teensy programmed with a code in can_master, which controls the flowmeter,
and the other teensy with flowmeter_v0 code. If you wire them according to the instruction
in https://www.mikrocontroller.net/attachment/28831/siemens_AP2921.pdf , you should 
be able to see the reset and sampling rate changes through the Serial Monitor of master teensy.
