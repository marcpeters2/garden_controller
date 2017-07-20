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

unsigned long long Util::parseULongLongFromString(char* buf) {
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

unsigned long lastMillis = millis();
int overflows = 0;

// TODO: millis() overflows roughly every 50 days.  If this function isn't called for 50 days, it won't detect the overflow properly.
unsigned long long Util::now(unsigned long long timeOffset) {
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
  result = timeOffset + (overflows * ULONG_MAX) + millisNow;
  return result;
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

