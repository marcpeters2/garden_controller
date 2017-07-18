#include <math.h>
#include "Arduino.h"
#include "Util.h"

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
            //Serial.print("parse");
            res = (int)(x / y);
            //Serial.print(res);
            if (res > 0)  // Wait for res > 0, then start adding to string.
                flag = true;
            if (flag == true)
                str = str + String(res);
            x = x - (y * (unsigned long long)res);  // Subtract res times * y from x
            y = y / 10;                   // Reducer y with 10    
     }
//     Serial.println();
//     Serial.println("-----------------------");
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

