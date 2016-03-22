# Energia-ADF435x
Energia library for a fractional-N implementation of the ADF435x PLL.
It allows frequency resolution to 100Hz between 34MHz and 4,400MHz (the extents of the IC).

Heavily based on
  ADF435x.cpp - an ADF435x PLL Communication Library
   Created by Neal Pisenti, 2013.
   JQI - Strontium - UMD
 but this is integer PLL division only. 
 
 Refer to the Analog Devices datasheet because there are trade-offs in terms of phase noise and spurs.
