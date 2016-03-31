/* 
   ADF435x.cpp - ADF435x PLL Communication Library
   Created by Neal Pisenti, 2013.
   JQI - Strontium - UMD
 Substantially modified by Steve Pettitt, SEC Ltd 2016

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   aunsigned long with this program.  If not, see <http://www.gnu.org/licenses/>.


 */

#include "Energia.h"
#include "SPI.h"
#include <ADF435x.h>



/* CONSTRUCTOR */

// Constructor function; initializes communication pinouts

ADF435x::ADF435x(byte ssPin) {
    _ssPin = ssPin;
    pinMode(_ssPin, OUTPUT);
    digitalWrite(_ssPin, HIGH);
}





// Initializes a new ADF435x object refClk (in Mhz), and initial frequency/10 to fit in 2^32
void ADF435x::initialize(uint32_t freq, uint32_t refClk){
    _freq = freq; // one tenth of real value REMEMBER!
    _refClk = refClk;
    _phase = 1;

    _feedbackType = 0;  //0=fractional, 1=integer
    if (_feedbackType) _ABP = 1;
    _doubler = 1; //
    _halve = 0; //
    _channelSpacing = 100; //100 Hz
    _powerdown = 0;
    _auxOnOff = 0;
    _rfEnabled = 1;
    _prescaler = 1;  // 4/5 or 8/9...

    // default power = 5dbm or -4 in this case... (0-3)
    _auxPower = 0;
    _rfPower = 0;

    // sets register values which don't have dynamic values...
    ADF435x::setR1();
    ADF435x::setR3();
    ADF435x::setR5();

    ADF435x::setFreq(_freq);
//    ADF435x::update();
}

// gets current frequency setting
int ADF435x::getFreq(){
    return _freq;
}



void ADF435x::setFreq(uint32_t freq){
// frequency passed is one tenth real value to stay within 32-bit value
    _freq = freq;
    int multiplier;
    // selects the right divider range (ie, output divider is 2^(_divider))
    // the multiplier is required if you're using fundamental feedback to N-counter
    if (_freq > 220000000) {
        _divider = 0;
        multiplier = 1;
        R_counter = 50;
    }
    else if (_freq > 110000000) {
        _divider = 1;
        multiplier = 2;
        R_counter = 25;
        }
        else if (_freq > 55000000) {
            _divider = 2;
            multiplier = 4;
            R_counter = 16;
            }
            else if (_freq > 27500000) {
                    _divider = 3;
                    multiplier = 8;
                    R_counter = 8;
                }
                else if (_freq >13750000) {
                        _divider = 4;
                        multiplier = 16;
                        R_counter = 4;
                    }
                    else if (_freq >6875000) {
                            _divider = 5;
                            multiplier = 32;
                            R_counter = 2;
                        }
                        else {  //_freq lies between 34.375MHz to 68.75MHz
                            _divider = 6;
                            multiplier = 64;
                            R_counter = 1;
                        }

  

        _mod = _refClk/_channelSpacing; // 10000000/100 = 1000000
        _mod *= _doubler; //_doubler should always be set
        _mod >>= _divider;
        _mod /= R_counter;
//Serial.print("\nRaw Modulus = ");Serial.println(_mod);

        while (_mod < 4096) _mod *=2; // set mod as close to 4095 as possible...
        _mod /= 2; //restore to less than 4096
    
    uint64_t test = _freq*10;
    test <<= _divider;
    test *= R_counter;
    uint32_t tast = _refClk << _doubler;
    tast >>= _halve;
    _integ = (uint64_t)(test/tast);
    _frac = ((uint64_t)(test % tast)*_mod)/(int32_t)tast;
    
    // reduce the frac/mod fraction....
    uint32_t x=1, y, z;
    y = _mod; z = _frac;
    while (x != 0) {
        if (y>z) {
            x = y - z;
        }
        else x = z - y;
        y = z;
        z = x;
    }
    _mod /= y; _frac /= y;
    
/* debugging only
    Serial.print("\nFrequency(KHz) = ");Serial.println(_freq);
    Serial.print("fPFD = ");Serial.println(tast);
    Serial.print("Integer = ");Serial.println(_integ);
    Serial.print("Modulus = ");Serial.println(_mod);
    Serial.print("Fraction = ");Serial.println(_frac);
    Serial.print("R Counter = ");Serial.println(R_counter);
    Serial.print("Divider = ");Serial.println(_divider);
    Serial.print("Multiplier = ");Serial.println(multiplier);

    Serial.print("R0 = ");Serial.println(R0.reg0,HEX);
    Serial.print("R1 = ");Serial.println(R1.reg1,HEX);
    Serial.print("R2 = ");Serial.println(R2.reg2,HEX);
    Serial.print("R3 = ");Serial.println(R3.reg3,HEX);
    Serial.print("R4 = ");Serial.println(R4.reg4,HEX);
    Serial.print("R5 = ");Serial.println(R5.reg5,HEX);
*/
    
    ADF435x::update();
}



// updates dynamic registers, and writes values to PLL board
void ADF435x::update(){
    // updates registers with dynamic values...
    ADF435x::setR0();
    ADF435x::setR1();
    ADF435x::setR2();
    ADF435x::setR3();
    ADF435x::setR4();
    ADF435x::setR5();


    // writes registers to device
    ADF435x::write4Bytes(R5.reg5);
    ADF435x::write4Bytes(R4.reg4);
    ADF435x::write4Bytes(R3.reg3);
    ADF435x::write4Bytes(R2.reg2);
    ADF435x::write4Bytes(R1.reg1);
    ADF435x::write4Bytes(R0.reg0);
}

void ADF435x::setFeedbackType(bool feedback){
    _feedbackType = feedback;
}

void ADF435x::powerDown(bool pd){
    _powerdown = pd;
    ADF435x::setR2();
    ADF435x::update();

}

void ADF435x::rfEnable(bool rf){
    _rfEnabled = rf;
    ADF435x::setR4();
    ADF435x::update();
}

// CAREFUL!!!! pow must be 0, 1, 2, or 3... corresponding to -4, -1, 3, 5 dbm.
void ADF435x::setRfPower(int pow){
    _rfPower = pow;
    ADF435x::setR4();
    ADF435x::update();
}

void ADF435x::auxEnable(void){
    _auxOnOff = 1;
    ADF435x::setR4();
    ADF435x::write4Bytes(R4.reg4);
}

void ADF435x::auxDisable(void){
    _auxOnOff = 0;
    ADF435x::setR4();
    ADF435x::write4Bytes(R4.reg4);
//    ADF435x::update();
}

// CAREFUL!!!! pow must be 0, 1, 2, or 3... corresponding to -4, -1, 3, 5 dbm.
void ADF435x::setAuxPower(int pow){
    _auxPower = pow;
    ADF435x::setR4();
    ADF435x::update();
}

// REGISTER UPDATE FUNCTIONS

void ADF435x::setR0(){
    R0.d0.reserved = 0;
    R0.d0.integer = _integ; // not less than 23 decimal
    R0.d0.frac = _frac;
    R0.d0.ctrl = 0;
}

void ADF435x::setR1(){
    R1.d1.reserved = 0;
    R1.d1.phaseAdjust = 0; // disabled
    R1.d1.prescaler = 1; //  4/5 or 8/9?
    R1.d1.phaseValue = 1; //recommended value
    R1.d1.modulus = _mod;
    R1.d1.ctrl = 1;
}

void ADF435x::setR2(){
    R2.d2.reserved = 0;
    R2.d2.noiseMode = 3;
    R2.d2.Muxout = 6;
    R2.d2.refDoubler = _doubler;
    R2.d2.refDivBy2 = _halve;
    R2.d2.rCounter = R_counter;  //default value = 1
    R2.d2.doubleBuffer = 0;
    R2.d2.chargePump = 7;
    R2.d2.LDF = _feedbackType; //fractional(0) or integer(1)?
    R2.d2.LDP = 0; //10nS (or 6nS if 1)
    R2.d2.PD_Polarity = 1;
    R2.d2.PD = 0;
    R2.d2.CP3state = 0;
    R2.d2.counterReset = 0;
    R2.d2.ctrl = 2;
}

void ADF435x::setR3(){
    R3.d3.reserved1 = 0;
    R3.d3.bandSelCkMode = 0;
    R3.d3.ABP = _ABP;
    R3.d3.chargeCancel = 0;
    R3.d3.reserved2 = 0;
    R3.d3.CSR = 0;
    R3.d3.reserved3 = 0;
    R3.d3.clockDivMode = 0;
    R3.d3.clockDivider = 150; //divide by 150
    R3.d3.ctrl = 3;
}

void ADF435x::setR4(){
    R4.d4.reserved = 0;
    R4.d4.feedbackSel = 1; //0= divided, 1= fundamental
    R4.d4.RF_div = _divider;
    R4.d4.bandSelCkDiv = 160;
    R4.d4.VCO_PD = 0;
    R4.d4.MTLD = 0;
    R4.d4.auxOutEnable = _auxOnOff;
    R4.d4.auxOutSel = 0; //disable auxilliary output
    R4.d4.auxOutPower = _auxPower;
    R4.d4.RF_OutEn = 1;
    R4.d4.OutputPower = _rfPower;
    R4.d4.ctrl = 4;

 }

void ADF435x::setR5(){
    R5.d5.reserved1 = 0;
    R5.d5.LD_Mode = 1; //lock detect mode
    R5.d5.reserved2 = 0;
    R5.d5.reserved3 = 3;
    R5.d5.ctrl = 5;
}

// Writes SPI to particular register.
//      registerInfo is a 2-element array which contains [register, number of bytes]
// This is bollocks as it is ALWAYS 4 bytes.
/*
void ADF435x::writeRegister(byte data[]){


    digitalWrite(_ssPin, LOW);

    // Writes the data
    for(int i = 0; i < 4 ; i++){
        SPI.transfer(data[i]);
    }
    digitalWrite(_ssPin, HIGH);

}
*/
void ADF435x::write4Bytes(uint32_t d)
{
    uint8_t h;
    
    digitalWrite(_ssPin, LOW);
    h = (d >> 24) & 0xff;
    SPI.transfer(h);
    h =(d >> 16) & 0xff;
    SPI.transfer(h);
    h =(d >> 8) & 0xff;
    SPI.transfer(h);
    h =d & 0xff;
    SPI.transfer(h);
    
    digitalWrite(_ssPin, HIGH);
}

