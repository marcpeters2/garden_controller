#include "TimeService.h"

bool _DEBUG_TIME_CHANGES = true;

void TimeService::debugTimeChanges(bool val) {
  _DEBUG_TIME_CHANGES = val;
}

unsigned long long _timeOffset;

void TimeService::setTime(unsigned long long _time) {
  if (_DEBUG_TIME_CHANGES) {   
    Serial.println();
    Serial.print("### Setting time to ");
    Serial.println(Util::toString(_time));
  }

  Util::noT4interrupts();
    _timeOffset = _time - ucNow();
  Util::T4interrupts();
}

unsigned long long _timeSlewStart;
unsigned long long _timeSlewEnd;
int _timeSlewAmount;
bool _timeSlewing = false;

void TimeService::slewTime(unsigned int duration, int amount) {

  if(duration > MAX_TIME_SLEW_DURATION) {
    Serial.println();
    Serial.print("!!! Util::slewTime called with invalid duration: ");
    Serial.println(duration);
    while(true); // Halt
  }

  Util::noT4interrupts();
    freezeSlew();
    unsigned long long _ucNow = ucNow();
    _timeSlewStart = _ucNow;
    _timeSlewEnd = _ucNow + duration;
    _timeSlewAmount = amount;
    _timeSlewing = true;
  Util::T4interrupts();

  if(_DEBUG_TIME_CHANGES) {
    Serial.println();
    Serial.print("### Slewing time by ");
    Serial.print(amount);
    Serial.print("ms over ");
    Serial.print(duration);
    Serial.println("ms");
  }
}

int TimeService::calculateSlew() {
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

void TimeService::freezeSlew() {
  int slew = calculateSlew();

  if(_DEBUG_TIME_CHANGES) {
    Serial.println();
    Serial.print("### Permanently adding time slew of ");
    Serial.print(slew);
    Serial.print(" to time offset of ");
    Serial.println(Util::toString(_timeOffset));
    Serial.print("### Current \"Now\": ");
    Serial.println(Util::toString(now()));
  }
  
  _timeOffset += calculateSlew();
  _timeSlewing = false;

  if(_DEBUG_TIME_CHANGES) {
    Serial.print("### New time offset is ");
    Serial.println(Util::toString(_timeOffset));
    Serial.print("### \"Now\": ");
    Serial.println(Util::toString(now()));
  }
}

unsigned long lastMillis = millis();
int overflows = 0;

// TODO: millis() overflows roughly every 50 days.  If this function isn't called for 50 days, it won't detect the overflow properly.
unsigned long long TimeService::ucNow() {
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

unsigned long long TimeService::now() {
  int slew = calculateSlew();
  return ucNow() + _timeOffset + slew;
}
