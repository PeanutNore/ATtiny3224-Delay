# ATtiny3224-Delay
A Digital Delay pedal for guitar based on the ATtiny3224 microcontroller and 23LC512 SRAM

This program, PCB design, and associated schematics are distributed in the hope that they will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

![ATtinyDelay](https://github.com/user-attachments/assets/106b8345-1de5-4a50-a64b-f96a5cb06bf4)

To use this software you will need:

- A complete and working ATtiny Delay circuit
    - Refer to the included schematic
    - Gerber files for ordering PCBs are included in ATTiny_Delay_Pedal.zip. I have had success using JLCPCB. This is not an endorsement, use whatever PCB vendor you prefer.
- Arduino IDE version 1.8.19 or greater
- megaTinyCore (https://github.com/SpenceKonde/megaTinyCore) for ATtiny3224 support
- A UPDI programmer (I recommend the UPDI Friend from Adafruit)

This is a basic digital delay pedal for guitar that aims achieve high audio quality at minimal cost compared to other DIY delay options such as using an FV-1 or PT2399.
It provides 12 bit sample depth at greater than 24khz sample rate, which results in no loss of quality when used with typical guitar and amp rigs.

 * Copyright 2025 Samuel Brown. All Rights Reservered.
 * 
 * Licensed under Creative Commons CC-BY-NC-SA 4.0
 * https://creativecommons.org/licenses/by-nc-sa/4.0/
 * 
 * You are free to share and adapt this software for non-
 * commercial purposes provided that you give appropriate 
 * credit and attribution, and distribute your contributions 
 * under the same CC-BY-NC-SA license.
 * 
 * For commercial licensing contact sam.brown.rit08@gmail.com
