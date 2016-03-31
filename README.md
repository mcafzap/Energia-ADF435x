# Energia-ADF435x
Energia library for a fractional-N implementation of the ADF435x PLL.
It allows frequency resolution to 100Hz between 34MHz and 4,400MHz (the extents of the IC).
To stay within 32-bit bounds, frequency is passed as f/10 see example below

Heavily based on
  ADF435x.cpp - an ADF435x PLL Communication Library
   Created by Neal Pisenti, 2013.
   JQI - Strontium - UMD
 but this is integer PLL division only. 
 
 Refer to the Analog Devices datasheet because there are trade-offs in terms of phase noise and spurs.
Communication library to interface with Analog Devices ADF435x PLL IC. Designed
with the ADF4350 evaluation board in mind or any number of Ebay offerings... 

## Getting started

In your sketch, you'll want to include the SPI library in addition to this code:

    #include <SPI.h>
    #include <ADF435x.h>

    #define COM_PIN 32 // sets pin 32 to be the slave-select pin for PLL
    ADF435x PLL(COM_PIN); // declares object PLL of type ADF435x. Will initialize it below.

    void setup(){
        SPI.begin();          // for communicating with DDS/PLLs
        SPI.setClockDivider(4);
        SPI.setDataMode(SPI_MODE0);
        delay(500); // give it a sec to warm up


        PLL.initialize(40, 10); // initialize the PLL to output 400 Mhz, using an
                                // onboard reference of 10Mhz

    }


## Implemented features

Self-explanatory functions...

* `ADF435x::initialize(int frequency, int refClk)` -- initializes PLL with given frequency (Mhz) and reference clock frequency (also in Mhz).
* `ADF435x::getFreq()` -- returns current frequency
* `ADF435x::setFreq(int freq)` -- sets PLL to output new frequency `freq` (in MHz).

Functions you should use after consulting datasheet:

* `ADF435x::setFeedbackType(bool feedback)` -- fundamental vs. divided feedback
* `ADF435x::powerDown(bool pd)` -- power down the VCO (or not).
* `ADF435x::rfEnable(bool rf)` -- enable/disable output on the main RF output.
* `ADF435x::setRfPower(int pow)` -- `pow` should be 0, 1, 2, or 3, corresponding to -4, -1, 3, or 5 dBm.
* `ADF435x::auxEnable(void)` -- enable output on the auxilary output.
* `ADF435x::auxDisable(void)` -- disable output on the auxilary output.
* `ADF435x::setAuxPower(int pow)` -- set auxiliary power output. Again, `pow` should be 0, 1, 2, or 3, corresponding to -4, -1, 3, or 5 dBm. 
