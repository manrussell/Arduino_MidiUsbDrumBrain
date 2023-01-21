# Arduino_MidiUsbDrumBrain
Arduino project that makes a usb midi interface for 6 inputs.

# Overview
This was made because my guitar hero drum kit does not forward midi off messages so even though the PS's interface box has a physical midi i/o ports they are useless to nearly everybody. I just wanted a quick solution to this problem.

# Hardware
Arduino Leonardo ( the board just has to be able to use the **MIDIUSB** library though)

Some Zener diodes ...
Circuit simulation was done using https://www.falstad.com/circuit/


# Software
Prerequists: Arduino ide, and the **MIDIUSB** library.
To configure edit the macro's between /* Start of Config */ and /* End of Config */
Configurable 
1. up to 6 inputs/ADC's
2. uart printf output... uart can be very slow, so better,
3. LED output on a hit

Outputs just midi on/off, the analogue circuitry may bugger up the velocity sensitivity, it depends on what you do there.
Also there is a minimum velocity before anything counts as a hit see, **HIT_THRESHOLD**


