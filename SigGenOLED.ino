// Include application, user and local libraries
#include <ADF435x.h>
#include <FontPack.h>
#include <Oled_SSD1306.h>

#define DC_OLED P2_5
#define CS_OLED P1_0
#define RST_OLED P2_6


#define MAIN_BUTTON P2_3
#define AUX_BUTTON P2_7
#define COM_PIN P3_1 // sets pin 32 to be the slave-select pin for PLL
#define PERSIST __attribute__((section(".text")))
ADF435x PLL(COM_PIN); // declares object PLL of type ADF4350. Will initialize it below.
#define SPEED  (20)
#define DOTLEN  (1200/SPEED)
#define DASHLEN  (3*(1200/SPEED))

const int buttonPin     = P1_3;               // connect the pushbutton to this LP pin
const int encoderPin1   = P1_4;               // connect one encoder pin to this LP pin
const int encoderPin2   = P1_5;               // connect the other encoder pin to this LP pin
const uint32_t minVal   =   3437500;         // minimum value /10 that will be allowed as input
const uint32_t maxVal   = 440000000;     // maximum value /10 that will be allowed as input
const uint32_t startInc     = 1;             // values increase/decrease by this value to start
const long minInc       = 1;                // minimum increment/decrement allowed when turning
const long maxInc       = 100000000;          // maximum increment/decrement /10 allowed when turning
const long divInc       = 10;               // factor for decreasing the increment/decrement
const uint32_t addTable[] = {100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10};


uint8_t state = false, mMenu=0, timeout = 0;
uint8_t secs, mins, i, j, k = 0, l=0;
uint16_t count = 0;
volatile boolean encoderLH1 = false;
volatile boolean encoderLH2 = false;
uint8_t m, e, f, g, displayScreen=0, adjPointer = 0;
uint8_t x=0, diggy;
char mark[14];
char freq[14];
char buffer[18];
char buf[12];
uint32_t fAddition = minInc;
uint32_t noddy, oldFrock;
// variables used to track whether or not a change has occurred
uint32_t lastEncoderVal = 1;                         
uint32_t lastEncoderInc = 0;

uint32_t encoderVal PERSIST;
uint8_t encoderInc PERSIST;
uint32_t frock PERSIST; // 0.1KHz freq to be set... 144,100,000Hz 
uint8_t mainLevel PERSIST;
uint8_t auxLevel PERSIST;

// Add setup code
void setup() {
  pinMode(P3_1, OUTPUT);
  digitalWrite(P3_1, HIGH); // enable Tx
  Serial.begin (115200);
  Serial.print("\nStarting...");
  SPI.begin();
  SPI.setDataMode(0);
  SPI.setClockDivider(4); // should be done here, NOT in libraries
 
  SSD1306PinSetup(CS_OLED, DC_OLED, RST_OLED);
  SSD1306Init();
  clearScreen();
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(encoderPin1, INPUT_PULLUP); 
  pinMode(encoderPin2, INPUT_PULLUP); 
  attachInterrupt(encoderPin1, ISR_Encoder1, CHANGE);  // interrupt for encoderPin1
  attachInterrupt(encoderPin2, ISR_Encoder2, CHANGE);  // interrupt for encoderPin2
  attachInterrupt(buttonPin, ISR_Button, FALLING);     // interrupt for encoder button
  if (encoderInc > 6) encoderInc = 0;
  mainLevel = 0x03; //output set to +5dBm
  auxLevel = 0;     //aux set to -4dBm
  if (frock > 440000000) frock = 440000000;
  if (frock < minVal) frock = minVal;
  PLL.setFeedbackType(1
  ); //integer PLL... for lowest spurs
  PLL.initialize(frock, 10000000); // initialize the PLL - reference 10Mhz
  lastEncoderVal = encoderVal;
  clearScreen();
  UlToStr(buffer, frock);
  stringDraw( 0, 0, "PLL SYNTHESISER FracN");
  stringDraw( 1, 10, "35-4400MHz   5dBm");
  stringDraw( 4, 4, buffer);
  oldFrock = frock; // equalise values at start-up
  setupUnderscore();
}

// Add loop code

void loop() {
  // check for change in encoder value
  if (oldFrock != frock) {
    //Serial.print("difference is: "); 
    Serial.println(frock);
    oldFrock = frock;
    UlToStr(buffer, frock);  
    PLL.setFreq(frock);
    stringDraw( 7, 0, "               ");
    stringDraw( 4, 4, buffer);    
    stringDraw( 7, 0, " ");
  }
}  


void ISR_Button() {
  ++encoderInc;
  if (encoderInc > 7) encoderInc = 0;
  setupUnderscore();
}


void ISR_Encoder1(){ //Rising edge... hardware debounce
  if (digitalRead(encoderPin1)==HIGH) {   // Low to High transition?
    encoderLH1 = true; //record this input state
    if (!encoderLH2) { // if other is high, then increment count
      oldFrock = frock;
      frock += addTable[encoderInc];                        // increase the value+
  if (frock > 440000000) frock = 440000000;       
    }
  } // High-to-low transition?        
  if (digitalRead(encoderPin1) == LOW) {
    encoderLH1 = false;
  }
}


void ISR_Encoder2(){
  if (digitalRead(encoderPin2) == HIGH) {   // Low to High transition?  
    encoderLH2 = true;
    if (!encoderLH1) {
      oldFrock = frock;
      frock -= addTable[encoderInc];      // decrease the value+
  if (frock < minVal) frock = minVal;
    }
  }  // High-to-low transition?
  if ((digitalRead(encoderPin2) == LOW)) {
    encoderLH2 = false;
  }
}

void UlToStr(char *s, uint32_t bin)
{ //add another zero...
buffer[10] = ' ';
buffer[11] = ' ';
buffer[12] = ' ';
buffer[13] = ' ';
buffer[14] = ' ';
buffer[15] = 'H';
buffer[16] = 'z';
buffer[17] = '\0';
if (bin > 1000000000) {
  buffer[14] = 'G';
}
else if (bin > 1000000) {
  buffer[14] = 'M';
}
else { 
  buffer[14] = 'K';
}
  char n = 9;
    s += n;
//    *s = '\0';
    while (n--)
    {
        *--s = (bin % 10) + '0';
        bin /= 10;
    }
char *t = &buffer[8];
char *d = &buffer[12];
  *d-- = '0';
  *d-- = *t--; 
  *d-- = *t--; 
  *d = ',';
  if (buffer[14] == 'K')  *d = '.';
  *d--; 
  *d-- = *t--; 
  *d-- = *t--;
  *d-- = *t--;
  *d = ',';  
  if (buffer[14] == 'M')  *d = '.';
  *d--; 
  *d-- = *t--; 
  *d-- = *t--; 
  *d-- = *t--;
  *d = ','; 
//  Serial.println(*--d,HEX);
  if (buffer[14] == 'G')  *d = '.'; 
  *d--; 
  *d = *t; 
// Leading zero blanking...
  if (*d == '0') *d++ = ' ';
  if (*d == ',') *d++ = ' ';
  if (*d == '0') *d++ = ' ';
  if (*d == '0') *d++ = ' ';
  if (*d == '0') *d++ = ' ';
  if (*d == ',') *d++ = ' ';
  if (*d == '0') *d++ = ' ';
  if (*d == '0') *d++ = ' ';
  if (*d == '0') *d++ = ' ';
}

void  setupUnderscore(void)
{
  for (i=0;i<12;i++) {
    buf[i] = ' ';
  }
  buf[11] = '\0';
  j = encoderInc;
  if (encoderInc > 0) j++;
  if (encoderInc > 3) j++;
  if (encoderInc > 6) j++;
  buf[j++] = '-';
  stringDraw(5, 4, buf);
  stringDraw( 7, 0, " "); // frig for no /CS
}































