#include <math.h>
#include "Arduino.h"
#include "Util.h"

extern "C" char *sbrk(int i);
size_t Util::freeRAM(void)
{
  char stack_dummy = 0;
  return(&stack_dummy - sbrk(0));
}


String Util::toString(unsigned long long x)
{
  boolean flag = false; // For preventing string return like this 0000123, with a lot of zeros in front.
  String str = "";      // Start with an empty string.
  unsigned long long y = 10000000000000000000LLU;
  int res;
  if (x == 0)  // if x = 0 and this is not testet, then function return a empty string.
  {
    str = "0";
    return str;  // or return "0";
  }    
  while (y > 0)
  {                
    res = (int)(x / y);
    if (res > 0)  // Wait for res > 0, then start adding to string.
        flag = true;
    if (flag == true)
        str = str + String(res);
    x = x - (y * (unsigned long long)res);  // Subtract res times * y from x
    y = y / 10;                   // Reducer y with 10    
  }
  
  return str;
}

int Util::parseIntFromString(char* buf) {
  int index = 0;
  int charAsInt;
  int bufferSize = sizeof(buf) / sizeof(char);
  int result = 0;

  while (index < bufferSize) {
    charAsInt = charToInt(buf[index]);

    if (charAsInt < 0 || charAsInt > 9) {
      break;
    }

    result = result * 10 + charAsInt;
    index++;
  }
  return result;
}

unsigned long long Util::toULL(char* buf) {
  int index = 0;
  int charAsInt;
  //int bufferSize = sizeof(buf) / sizeof(char);
  unsigned long long result = 0;

//  Serial.println("Starting to parse ulonglong");
//  Serial.print("Buffer contents: ");
//  Serial.println(buf);
  //Serial.print("Buffer size: ");
  //Serial.println(bufferSize);

  while (buf[index] != '\0') {
    charAsInt = charToInt(buf[index]);

    if (charAsInt < 0 || charAsInt > 9) {
      break;
    }

    result = (unsigned long long) result * 10 + charAsInt;
    index++;
  }
  return result;
}


int Util::charToInt(char c) {
  return c - '0';
}

bool Util::charIsNumeric(char c) {
  int charAsInt = Util::charToInt(c);
  return (charAsInt >= 0) && (charAsInt <= 9);
}


unsigned long long _timeOffset;

void Util::setTime(unsigned long long _time) {
  Serial.println();
  Serial.print("### Setting time to ");
  Serial.println(toString(_time));

  noT4interrupts();
    _timeOffset = _time - ucNow();
  T4interrupts();
}

unsigned long long _timeSlewStart;
unsigned long long _timeSlewEnd;
int _timeSlewAmount;
bool _timeSlewing = false;

void Util::slewTime(unsigned int duration, int amount) {

  if(duration > MAX_TIME_SLEW_DURATION) {
    Serial.println();
    Serial.print("!!! Util::slewTime called with invalid duration: ");
    Serial.println(duration);
    while(true); // Halt
  }

  noT4interrupts();
    freezeSlew();
    unsigned long long _ucNow = ucNow();
    _timeSlewStart = _ucNow;
    _timeSlewEnd = _ucNow + duration;
    _timeSlewAmount = amount;
    _timeSlewing = true;
  T4interrupts();

  Serial.println();
  Serial.print("### Slewing time by ");
  Serial.print(amount);
  Serial.print("ms over ");
  Serial.print(duration);
  Serial.println("s");
}

int Util::calculateSlew() {
  unsigned long long _ucNow = ucNow();
  
  if(!_timeSlewing || _ucNow <= _timeSlewStart) {
    return 0;
  }
  else if(_ucNow >= _timeSlewEnd) {
    return _timeSlewAmount;
  }
  else { // ucNow falls between _timeSlewStart and _timeSlewEnd
    // The difference between _timeSlewEnd and _timeSlewStart is guaranteed to be less than MAX_TIME_SLEW_DURATION
    // due to the gating check in Util::slewTime().  Due to this, it's safe to perform the (double) casting below.
    return (int) (_timeSlewAmount * (((double)(_ucNow - _timeSlewStart)) / ((double)(_timeSlewEnd - _timeSlewStart))));
  }
}

void Util::freezeSlew() {
  int slew = calculateSlew();
  Serial.println();
  Serial.print("### Permanently adding time slew of ");
  Serial.print(slew);
  Serial.print(" to time offset of ");
  Serial.println(toString(_timeOffset));
  Serial.print("### Current \"Now\": ");
  Serial.println(toString(now()));
  
  _timeOffset += calculateSlew();
  _timeSlewing = false;

  Serial.print("### New time offset is ");
  Serial.println(toString(_timeOffset));
  Serial.print("### \"Now\": ");
  Serial.println(toString(now()));
}

unsigned long lastMillis = millis();
int overflows = 0;

// TODO: millis() overflows roughly every 50 days.  If this function isn't called for 50 days, it won't detect the overflow properly.
unsigned long long Util::ucNow() {
  unsigned long millisNow = millis();
  unsigned long long result;
  
  if(millisNow < lastMillis) {
    overflows++;
    Serial.println("**************************************");
    Serial.print("millis() has overflowed!  Overflowed ");
    Serial.print(overflows);
    Serial.println(" times total so far.");
    Serial.println("**************************************");
  }
  else {
//    Serial.print("lastMillis: ");
//    Serial.println(lastMillis);
  }

  lastMillis = millisNow;
  result = (overflows * ULONG_MAX) + millisNow;
  return result;
}

unsigned long long Util::now() {
  int slew = calculateSlew();
  return ucNow() + _timeOffset + slew;
}

void Util::enableTimer4Interrupts() {
  // Set up the generic clock (GCLK4) used to clock timers
  // TODO: Interrupt frequency math is wrong in the comments below.  Need to figure out how to properly determine the interrupt 
  //       frequency based on the parameters being set.
  
  Serial.println();
  Serial.println(">>> Initializing interrupts for Timer/Counter 4");
  
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(0x0030u) |    // Divide the 8MHz clock source by divisor 48: 8MHz/48=166.6KHz
                    GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_OSC8M |     // Use the 8MHz clock source
                   //GCLK_GENCTRL_SRC_DFLL48M |   // Use the 48MHz clock source
                   //GCLK_GENCTRL_SRC_OSC32K |    // Use the 32KHz clock source
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Feed GCLK4 to TC4 and TC5
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TC4 and TC5
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TC4_TC5;     // Feed the GCLK4 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization
 
  REG_TC4_CTRLA |= TC_CTRLA_MODE_COUNT8;           // Set the counter to 8-bit mode
  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization

  REG_TC4_COUNT8_CC0 = 0x55;                      // Set the TC4 CC0 register to some arbitary value
  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization
  REG_TC4_COUNT8_CC1 = 0xAA;                      // Set the TC4 CC1 register to some arbitary value
  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization
  REG_TC4_COUNT8_PER = 0xFF;                      // Set the PER (period) register to its maximum value
  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization

  //NVIC_DisableIRQ(TC4_IRQn);
  //NVIC_ClearPendingIRQ(TC4_IRQn);
  NVIC_SetPriority(TC4_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for TC4 to 0 (highest)
  NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)

  REG_TC4_INTFLAG |= TC_INTFLAG_MC1 | TC_INTFLAG_MC0 | TC_INTFLAG_OVF;        // Clear the interrupt flags
  REG_TC4_INTENSET = TC_INTENSET_MC1 | TC_INTENSET_MC0 | TC_INTENSET_OVF;     // Enable TC4 interrupts
 
  REG_TC4_CTRLA |= TC_CTRLA_PRESCALER_DIV64 |     // Set prescaler to 64, 166.6KHz/64 = 2.6KHz
                // TC_CTRLA_PRESCALER_DIV256 |
                   TC_CTRLA_ENABLE;               // Enable TC4
  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization
  Serial.println("### Interrupts initialized");
}

void Util::noT4interrupts() {
  //TODO: This function hasn't been tested
  REG_TC4_INTENCLR = TC_INTENCLR_MC1 | TC_INTENCLR_MC0 | TC_INTENCLR_OVF;     // Disable TC4 interrupts
}

void Util::T4interrupts() {
  REG_TC4_INTENSET = TC_INTENSET_MC1 | TC_INTENSET_MC0 | TC_INTENSET_OVF;     // Enable TC4 interrupts
}

void Util::printWelcomeMessage() {
  Serial.println("   ___             _             ___         _           _ _");         
  Serial.println("  / __|__ _ _ _ __| |___ _ _    / __|___ _ _| |_ _ _ ___| | |___ _ _"); 
  Serial.println(" | (_ / _` | '_/ _` / -_) ' \\  | (__/ _ \\ ' \\  _| '_/ _ \\ | / -_) '_|");
  Serial.println("  \\___\\__,_|_| \\__,_\\___|_||_|  \\___\\___/_||_\\__|_| \\___/_|_\\___|_|");  
  Serial.println();
  Serial.println("                   ,");
  Serial.println("                  /.\\");
  Serial.println("                 //_`\\");
  Serial.println("            _.-`| \\ ``._");
  Serial.println("        .-''`-.       _.'`.");
  Serial.println("      .'      / /'\\/`.\\    `.");
  Serial.println("     /   .    |/         `.  \\");
  Serial.println("    '   /                  \\  ;");
  Serial.println("   :   '            \\       : :");
  Serial.println("   ;  ;             ;      /  .");
  Serial.println("    ' :             .     '  /");
  Serial.println("     \\ \\           /       .'");
  Serial.println("      `.`        .'      .'");
  Serial.println("        `-..___....----`");
  Serial.println();
  Serial.println();
}

