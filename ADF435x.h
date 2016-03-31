/* 
   ADF435x.h - ADF435x PLL Communication Library
   Created by Neal Pisenti, 2013.
   JQI - Strontium - UMD

    Massively modified for Energia - Steve Pettitt, SEC 2016
 
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

#ifndef ADF435x_h
#define ADF435x_h

#include "Arduino.h"

class ADF435x
{
    public: 
        // Constructor function. 
        // Creates PLL object, with given SS pin
        ADF435x(byte);

        // Initialize with initial frequency, refClk (defaults to 10Mhz); 
        void initialize(uint32_t, uint32_t );


        // powers down the PLL/VCO
        void powerDown(bool);
        void setRfPower(int);
        void setAuxPower(int);
        void auxEnable(void);
        void auxDisable(void);
        void rfEnable(bool);

        // Gets current frequency
        int getFreq();

        // Sets frequency
        void setFreq(uint32_t);

        void setInt(int);

        void setFeedbackType(bool);

        void update();
        uint32_t _freq, fPFD, _refClk, _integ;
        int _phase;
    unsigned int _divider, _auxPower, _rfPower;
    uint32_t _mod;
        unsigned int _frac, modulus, _channelSpacing, _outputDivider, fPDF, R_counter;
    
    
    
    
    // function to write data to register.
    void setR0();
    void setR1();
    void setR2();
    void setR3();
    void setR4();
    void setR5();
    void writeRegister(byte[4]);
    void write4Bytes(uint32_t);
    
    private:
        // Instance variables that hold pinout mapping
        byte _ssPin;

        // Instance variables for ...
    bool _powerdown, _auxOnOff, _rfEnabled, _feedbackType, _prescaler, _ABP;
    unsigned int _doubler, _halve;

    
    typedef struct R0bitfield{
        unsigned int ctrl:3;
        unsigned int frac:12;
        unsigned int integer:16;
        unsigned int reserved:1;
    } d0;
    union {
        uint32_t reg0;
        R0bitfield d0;
    } R0;
    
    
    typedef struct R1bitfield{
        unsigned int ctrl:3;
        unsigned int modulus:12;
        unsigned int phaseValue:12;
        unsigned int prescaler:1;
        unsigned int phaseAdjust:1;
        unsigned int reserved:3;
    } d1;
    union {
        uint32_t reg1;
        R1bitfield d1;
    } R1;
    
    typedef struct R2bitfield{
        unsigned int ctrl:3;
        unsigned int counterReset:1;
        unsigned int CP3state:1;
        unsigned int PD:1;
        unsigned int PD_Polarity:1;
        unsigned int LDP:1;
        unsigned int LDF:1;
        unsigned int chargePump:4;
        unsigned int doubleBuffer:1;
        unsigned int rCounter:10;
        unsigned int refDivBy2:1;
        unsigned int refDoubler:1;
        unsigned int Muxout:3;
        unsigned int noiseMode:2;
        unsigned int reserved:1;
    } d2;
    union {
        uint32_t reg2;
        R2bitfield d2;
    } R2;
    
    
    typedef struct R3bitfield{
        unsigned int ctrl:3;
        unsigned int clockDivider:12;
        unsigned int clockDivMode:2;
        unsigned int reserved3:1;
        unsigned int CSR:1;
        unsigned int reserved2:2;
        unsigned int chargeCancel:1;
        unsigned int ABP:1;
        unsigned int bandSelCkMode:1;
        unsigned int reserved1:8;
    } d3;
    union {
        uint32_t reg3;
        R3bitfield d3;
    } R3;
    
    typedef struct R4bitfield{
        unsigned int ctrl:3;
        unsigned int OutputPower:2;
        unsigned int RF_OutEn:1;
        unsigned int auxOutPower:2;
        unsigned int auxOutEnable:1;
        unsigned int auxOutSel:1;
        unsigned int MTLD:1;
        unsigned int VCO_PD:1;
        unsigned int bandSelCkDiv:8;
        unsigned int RF_div:3;
        unsigned int feedbackSel:1;
        unsigned int reserved:8;
    } d4;
    union {
        uint32_t reg4;
        R4bitfield d4;
    } R4;
    
    typedef struct R5bitfield{
        unsigned int ctrl:3;
        unsigned int reserverd4:16;
        unsigned int reserved3:2;
        unsigned int reserved2:1;
        unsigned int LD_Mode:2;
        unsigned int reserved1:8;
    } d5;
    union {
        uint32_t reg5;
        R5bitfield d5;
    } R5;
    
        // register values

};
 

#endif
