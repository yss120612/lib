#include "YssRTC.h"
#include <driver\i2c.h>

/**************************************************************************/
// utility code, some of this could be exposed in the DateTime API if needed
/**************************************************************************/

/**
  Number of days in each month, from January to November. December is not
  needed. Omitting it avoids an incompatibility with Paul Stoffregen's Time
  library. C.f. https://github.com/adafruit/RTClib/issues/114
*/
const uint8_t daysInMonth[] PROGMEM = {31, 28, 31, 30, 31, 30,
                                       31, 31, 30, 31, 30};

/**************************************************************************/
/*!
    @brief  Given a date, return number of days since 2000/01/01,
            valid for 2000--2099
    @param y Year
    @param m Month
    @param d Day
    @return Number of days
*/
/**************************************************************************/
static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) {
  if (y >= 2000U)
    y -= 2000U;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(daysInMonth + i - 1);
  if (m > 2 && y % 4 == 0)
    ++days;
  return days + 365 * y + (y + 3) / 4 - 1;
}

/**************************************************************************/
/*!
    @brief  Given a number of days, hours, minutes, and seconds, return the
   total seconds
    @param days Days
    @param h Hours
    @param m Minutes
    @param s Seconds
    @return Number of seconds total
*/
/**************************************************************************/
static uint32_t time2ulong(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
  return ((days * 24UL + h) * 60 + m) * 60 + s;
}

/**************************************************************************/
/*!
    @brief  Constructor from
        [Unix time](https://en.wikipedia.org/wiki/Unix_time).

    This builds a DateTime from an integer specifying the number of seconds
    elapsed since the epoch: 1970-01-01 00:00:00. This number is analogous
    to Unix time, with two small differences:

     - The Unix epoch is specified to be at 00:00:00
       [UTC](https://en.wikipedia.org/wiki/Coordinated_Universal_Time),
       whereas this class has no notion of time zones. The epoch used in
       this class is then at 00:00:00 on whatever time zone the user chooses
       to use, ignoring changes in DST.

     - Unix time is conventionally represented with signed numbers, whereas
       this constructor takes an unsigned argument. Because of this, it does
       _not_ suffer from the
       [year 2038 problem](https://en.wikipedia.org/wiki/Year_2038_problem).

    If called without argument, it returns the earliest time representable
    by this class: 2000-01-01 00:00:00.

    @see The `unixtime()` method is the converse of this constructor.

    @param t Time elapsed in seconds since 1970-01-01 00:00:00.
*/
/**************************************************************************/
DateTime::DateTime(uint32_t t) {
  t -= SECONDS_FROM_1970_TO_2000; // bring to 2000 timestamp from 1970

  ss = t % 60;
  t /= 60;
  mm = t % 60;
  t /= 60;
  hh = t % 24;
  uint16_t days = t / 24;
  uint8_t leap;
  for (yOff = 0;; ++yOff) {
    leap = yOff % 4 == 0;
    if (days < 365U + leap)
      break;
    days -= 365 + leap;
  }
  for (m = 1; m < 12; ++m) {
    uint8_t daysPerMonth = pgm_read_byte(daysInMonth + m - 1);
    if (leap && m == 2)
      ++daysPerMonth;
    if (days < daysPerMonth)
      break;
    days -= daysPerMonth;
  }
  d = days + 1;
}

/**************************************************************************/
/*!
    @brief  Constructor from (year, month, day, hour, minute, second).
    @warning If the provided parameters are not valid (e.g. 31 February),
           the constructed DateTime will be invalid.
    @see   The `isValid()` method can be used to test whether the
           constructed DateTime is valid.
    @param year Either the full year (range: 2000--2099) or the offset from
        year 2000 (range: 0--99).
    @param month Month number (1--12).
    @param day Day of the month (1--31).
    @param hour,min,sec Hour (0--23), minute (0--59) and second (0--59).
*/
/**************************************************************************/
DateTime::DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour,
                   uint8_t min, uint8_t sec) {
  if (year >= 2000U)
    year -= 2000U;
  yOff = year;
  m = month;
  d = day;
  hh = hour;
  mm = min;
  ss = sec;
}

/**************************************************************************/
/*!
    @brief  Copy constructor.
    @param copy DateTime to copy.
*/
/**************************************************************************/
DateTime::DateTime(const DateTime &copy)
    : yOff(copy.yOff), m(copy.m), d(copy.d), hh(copy.hh), mm(copy.mm),
      ss(copy.ss) {}

/**************************************************************************/
/*!
    @brief  Convert a string containing two digits to uint8_t, e.g. "09" returns
   9
    @param p Pointer to a string containing two digits
*/
/**************************************************************************/
static uint8_t conv2d(const char *p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9')
    v = *p - '0';
  return 10 * v + *++p - '0';
}

/**************************************************************************/
/*!
    @brief  Constructor for generating the build time.

    This constructor expects its parameters to be strings in the format
    generated by the compiler's preprocessor macros `__DATE__` and
    `__TIME__`. Usage:

    ```
    DateTime buildTime(__DATE__, __TIME__);
    ```

    @note The `F()` macro can be used to reduce the RAM footprint, see
        the next constructor.

    @param date Date string, e.g. "Apr 16 2020".
    @param time Time string, e.g. "18:34:56".
*/
/**************************************************************************/
DateTime::DateTime(const char *date, const char *time) {
  yOff = conv2d(date + 9);
  // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
  switch (date[0]) {
  case 'J':
    m = (date[1] == 'a') ? 1 : ((date[2] == 'n') ? 6 : 7);
    break;
  case 'F':
    m = 2;
    break;
  case 'A':
    m = date[2] == 'r' ? 4 : 8;
    break;
  case 'M':
    m = date[2] == 'r' ? 3 : 5;
    break;
  case 'S':
    m = 9;
    break;
  case 'O':
    m = 10;
    break;
  case 'N':
    m = 11;
    break;
  case 'D':
    m = 12;
    break;
  }
  d = conv2d(date + 4);
  hh = conv2d(time);
  mm = conv2d(time + 3);
  ss = conv2d(time + 6);
}

/**************************************************************************/
/*!
    @brief  Memory friendly constructor for generating the build time.

    This version is intended to save RAM by keeping the date and time
    strings in program memory. Use it with the `F()` macro:

    ```
    DateTime buildTime(F(__DATE__), F(__TIME__));
    ```

    @param date Date PROGMEM string, e.g. F("Apr 16 2020").
    @param time Time PROGMEM string, e.g. F("18:34:56").
*/
/**************************************************************************/
DateTime::DateTime(const __FlashStringHelper *date,
                   const __FlashStringHelper *time) {
  char buff[11];
  memcpy_P(buff, date, 11);
  yOff = conv2d(buff + 9);
  // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
  switch (buff[0]) {
  case 'J':
    m = (buff[1] == 'a') ? 1 : ((buff[2] == 'n') ? 6 : 7);
    break;
  case 'F':
    m = 2;
    break;
  case 'A':
    m = buff[2] == 'r' ? 4 : 8;
    break;
  case 'M':
    m = buff[2] == 'r' ? 3 : 5;
    break;
  case 'S':
    m = 9;
    break;
  case 'O':
    m = 10;
    break;
  case 'N':
    m = 11;
    break;
  case 'D':
    m = 12;
    break;
  }
  d = conv2d(buff + 4);
  memcpy_P(buff, time, 8);
  hh = conv2d(buff);
  mm = conv2d(buff + 3);
  ss = conv2d(buff + 6);
}

/**************************************************************************/
/*!
    @brief  Constructor for creating a DateTime from an ISO8601 date string.

    This constructor expects its parameters to be a string in the
    https://en.wikipedia.org/wiki/ISO_8601 format, e.g:

    "2020-06-25T15:29:37"

    Usage:

    ```
    DateTime dt("2020-06-25T15:29:37");
    ```

    @note The year must be > 2000, as only the yOff is considered.

    @param iso8601dateTime
           A dateTime string in iso8601 format,
           e.g. "2020-06-25T15:29:37".

*/
/**************************************************************************/
DateTime::DateTime(const char *iso8601dateTime) {
  char ref[] = "2000-01-01T00:00:00";
  memcpy(ref, iso8601dateTime, min(strlen(ref), strlen(iso8601dateTime)));
  yOff = conv2d(ref + 2);
  m = conv2d(ref + 5);
  d = conv2d(ref + 8);
  hh = conv2d(ref + 11);
  mm = conv2d(ref + 14);
  ss = conv2d(ref + 17);
}

/**************************************************************************/
/*!
    @brief  Check whether this DateTime is valid.
    @return true if valid, false if not.
*/
/**************************************************************************/
bool DateTime::isValid() const {
  if (yOff >= 100)
    return false;
  DateTime other(unixtime());
  return yOff == other.yOff && m == other.m && d == other.d && hh == other.hh &&
         mm == other.mm && ss == other.ss;
}

/**************************************************************************/
/*!
    @brief  Writes the DateTime as a string in a user-defined format.

    The _buffer_ parameter should be initialized by the caller with a string
    specifying the requested format. This format string may contain any of
    the following specifiers:

    | specifier | output                                                 |
    |-----------|--------------------------------------------------------|
    | YYYY      | the year as a 4-digit number (2000--2099)              |
    | YY        | the year as a 2-digit number (00--99)                  |
    | MM        | the month as a 2-digit number (01--12)                 |
    | MMM       | the abbreviated English month name ("Jan"--"Dec")      |
    | DD        | the day as a 2-digit number (01--31)                   |
    | DDD       | the abbreviated English day of the week ("Mon"--"Sun") |
    | AP        | either "AM" or "PM"                                    |
    | ap        | either "am" or "pm"                                    |
    | hh        | the hour as a 2-digit number (00--23 or 01--12)        |
    | mm        | the minute as a 2-digit number (00--59)                |
    | ss        | the second as a 2-digit number (00--59)                |

    If either "AP" or "ap" is used, the "hh" specifier uses 12-hour mode
    (range: 01--12). Otherwise it works in 24-hour mode (range: 00--23).

    The specifiers within _buffer_ will be overwritten with the appropriate
    values from the DateTime. Any characters not belonging to one of the
    above specifiers are left as-is.

    __Example__: The format "DDD, DD MMM YYYY hh:mm:ss" generates an output
    of the form "Thu, 16 Apr 2020 18:34:56.

    @see The `timestamp()` method provides similar functionnality, but it
        returns a `String` object and supports a limited choice of
        predefined formats.

    @param[in,out] buffer Array of `char` for holding the format description
        and the formatted DateTime. Before calling this method, the buffer
        should be initialized by the user with the format string. The method
        will overwrite the buffer with the formatted date and/or time.

    @return A pointer to the provided buffer. This is returned for
        convenience, in order to enable idioms such as
        `Serial.println(now.toString(buffer));`
*/
/**************************************************************************/

char *DateTime::toString(char *buffer) const {
  uint8_t apTag =
      (strstr(buffer, "ap") != nullptr) || (strstr(buffer, "AP") != nullptr);
  uint8_t hourReformatted = 0, isPM = false;
  if (apTag) {     // 12 Hour Mode
    if (hh == 0) { // midnight
      isPM = false;
      hourReformatted = 12;
    } else if (hh == 12) { // noon
      isPM = true;
      hourReformatted = 12;
    } else if (hh < 12) { // morning
      isPM = false;
      hourReformatted = hh;
    } else { // 1 o'clock or after
      isPM = true;
      hourReformatted = hh - 12;
    }
  }

  for (size_t i = 0; i < strlen(buffer) - 1; i++) {
    if (buffer[i] == 'h' && buffer[i + 1] == 'h') {
      if (!apTag) { // 24 Hour Mode
        buffer[i] = '0' + hh / 10;
        buffer[i + 1] = '0' + hh % 10;
      } else { // 12 Hour Mode
        buffer[i] = '0' + hourReformatted / 10;
        buffer[i + 1] = '0' + hourReformatted % 10;
      }
    }
    if (buffer[i] == 'm' && buffer[i + 1] == 'm') {
      buffer[i] = '0' + mm / 10;
      buffer[i + 1] = '0' + mm % 10;
    }
    if (buffer[i] == 's' && buffer[i + 1] == 's') {
      buffer[i] = '0' + ss / 10;
      buffer[i + 1] = '0' + ss % 10;
    }
    if (buffer[i] == 'D' && buffer[i + 1] == 'D' && buffer[i + 2] == 'D') {
      static PROGMEM const char day_names[] = "SunMonTueWedThuFriSat";
      const char *p = &day_names[3 * dayOfTheWeek()];
      buffer[i] = pgm_read_byte(p);
      buffer[i + 1] = pgm_read_byte(p + 1);
      buffer[i + 2] = pgm_read_byte(p + 2);
    } else if (buffer[i] == 'D' && buffer[i + 1] == 'D') {
      buffer[i] = '0' + d / 10;
      buffer[i + 1] = '0' + d % 10;
    }
    if (buffer[i] == 'M' && buffer[i + 1] == 'M' && buffer[i + 2] == 'M') {
      static PROGMEM const char month_names[] =
          "JanFebMarAprMayJunJulAugSepOctNovDec";
      const char *p = &month_names[3 * (m - 1)];
      buffer[i] = pgm_read_byte(p);
      buffer[i + 1] = pgm_read_byte(p + 1);
      buffer[i + 2] = pgm_read_byte(p + 2);
    } else if (buffer[i] == 'M' && buffer[i + 1] == 'M') {
      buffer[i] = '0' + m / 10;
      buffer[i + 1] = '0' + m % 10;
    }
    if (buffer[i] == 'Y' && buffer[i + 1] == 'Y' && buffer[i + 2] == 'Y' &&
        buffer[i + 3] == 'Y') {
      buffer[i] = '2';
      buffer[i + 1] = '0';
      buffer[i + 2] = '0' + (yOff / 10) % 10;
      buffer[i + 3] = '0' + yOff % 10;
    } else if (buffer[i] == 'Y' && buffer[i + 1] == 'Y') {
      buffer[i] = '0' + (yOff / 10) % 10;
      buffer[i + 1] = '0' + yOff % 10;
    }
    if (buffer[i] == 'A' && buffer[i + 1] == 'P') {
      if (isPM) {
        buffer[i] = 'P';
        buffer[i + 1] = 'M';
      } else {
        buffer[i] = 'A';
        buffer[i + 1] = 'M';
      }
    } else if (buffer[i] == 'a' && buffer[i + 1] == 'p') {
      if (isPM) {
        buffer[i] = 'p';
        buffer[i + 1] = 'm';
      } else {
        buffer[i] = 'a';
        buffer[i + 1] = 'm';
      }
    }
  }
  return buffer;
}

/**************************************************************************/
/*!
      @brief  Return the hour in 12-hour format.
      @return Hour (1--12).
*/
/**************************************************************************/
uint8_t DateTime::twelveHour() const {
  if (hh == 0 || hh == 12) { // midnight or noon
    return 12;
  } else if (hh > 12) { // 1 o'clock or later
    return hh - 12;
  } else { // morning
    return hh;
  }
}

/**************************************************************************/
/*!
    @brief  Return the day of the week.
    @return Day of week as an integer from 0 (Sunday) to 6 (Saturday).
*/
/**************************************************************************/
uint8_t DateTime::dayOfTheWeek() const {
  uint16_t day = date2days(yOff, m, d);
  return (day + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

/**************************************************************************/
/*!
    @brief  Return Unix time: seconds since 1 Jan 1970.

    @see The `DateTime::DateTime(uint32_t)` constructor is the converse of
        this method.

    @return Number of seconds since 1970-01-01 00:00:00.
*/
/**************************************************************************/
uint32_t DateTime::unixtime(void) const {
  uint32_t t;
  uint16_t days = date2days(yOff, m, d);
  t = time2ulong(days, hh, mm, ss);
  t += SECONDS_FROM_1970_TO_2000; // seconds from 1970 to 2000

  return t;
}

/**************************************************************************/
/*!
    @brief  Convert the DateTime to seconds since 1 Jan 2000

    The result can be converted back to a DateTime with:

    ```cpp
    DateTime(SECONDS_FROM_1970_TO_2000 + value)
    ```

    @return Number of seconds since 2000-01-01 00:00:00.
*/
/**************************************************************************/
uint32_t DateTime::secondstime(void) const {
  uint32_t t;
  uint16_t days = date2days(yOff, m, d);
  t = time2ulong(days, hh, mm, ss);
  return t;
}

/**************************************************************************/
/*!
    @brief  Add a TimeSpan to the DateTime object
    @param span TimeSpan object
    @return New DateTime object with span added to it.
*/
/**************************************************************************/
DateTime DateTime::operator+(const TimeSpan &span) const {
  return DateTime(unixtime() + span.totalseconds());
}

/**************************************************************************/
/*!
    @brief  Subtract a TimeSpan from the DateTime object
    @param span TimeSpan object
    @return New DateTime object with span subtracted from it.
*/
/**************************************************************************/
DateTime DateTime::operator-(const TimeSpan &span) const {
  return DateTime(unixtime() - span.totalseconds());
}

/**************************************************************************/
/*!
    @brief  Subtract one DateTime from another

    @note Since a TimeSpan cannot be negative, the subtracted DateTime
        should be less (earlier) than or equal to the one it is
        subtracted from.

    @param right The DateTime object to subtract from self (the left object)
    @return TimeSpan of the difference between DateTimes.
*/
/**************************************************************************/
TimeSpan DateTime::operator-(const DateTime &right) const {
  return TimeSpan(unixtime() - right.unixtime());
}

/**************************************************************************/
/*!
    @author Anton Rieutskyi
    @brief  Test if one DateTime is less (earlier) than another.
    @warning if one or both DateTime objects are invalid, returned value is
        meaningless
    @see use `isValid()` method to check if DateTime object is valid
    @param right Comparison DateTime object
    @return True if the left DateTime is earlier than the right one,
        false otherwise.
*/
/**************************************************************************/
bool DateTime::operator<(const DateTime &right) const {
  return (yOff + 2000U < right.year() ||
          (yOff + 2000U == right.year() &&
           (m < right.month() ||
            (m == right.month() &&
             (d < right.day() ||
              (d == right.day() &&
               (hh < right.hour() ||
                (hh == right.hour() &&
                 (mm < right.minute() ||
                  (mm == right.minute() && ss < right.second()))))))))));
}

/**************************************************************************/
/*!
    @author Anton Rieutskyi
    @brief  Test if two DateTime objects are equal.
    @warning if one or both DateTime objects are invalid, returned value is
        meaningless
    @see use `isValid()` method to check if DateTime object is valid
    @param right Comparison DateTime object
    @return True if both DateTime objects are the same, false otherwise.
*/
/**************************************************************************/
bool DateTime::operator==(const DateTime &right) const {
  return (right.year() == yOff + 2000U && right.month() == m &&
          right.day() == d && right.hour() == hh && right.minute() == mm &&
          right.second() == ss);
}

/**************************************************************************/
/*!
    @brief  Return a ISO 8601 timestamp as a `String` object.

    The generated timestamp conforms to one of the predefined, ISO
    8601-compatible formats for representing the date (if _opt_ is
    `TIMESTAMP_DATE`), the time (`TIMESTAMP_TIME`), or both
    (`TIMESTAMP_FULL`).

    @see The `toString()` method provides more general string formatting.

    @param opt Format of the timestamp
    @return Timestamp string, e.g. "2020-04-16T18:34:56".
*/
/**************************************************************************/
String DateTime::timestamp(timestampOpt opt) const {
  char buffer[25]; // large enough for any DateTime, including invalid ones

  // Generate timestamp according to opt
  switch (opt) {
  case TIMESTAMP_TIME:
    // Only time
    sprintf(buffer, "%02d:%02d:%02d", hh, mm, ss);
    break;
  case TIMESTAMP_DATE:
    // Only date
    sprintf(buffer, "%u-%02d-%02d", 2000U + yOff, m, d);
    break;
  default:
    // Full
    sprintf(buffer, "%u-%02d-%02dT%02d:%02d:%02d", 2000U + yOff, m, d, hh, mm,
            ss);
  }
  return String(buffer);
}

/**************************************************************************/
/*!
    @brief  Create a new TimeSpan object in seconds
    @param seconds Number of seconds
*/
/**************************************************************************/
TimeSpan::TimeSpan(int32_t seconds) : _seconds(seconds) {}

/**************************************************************************/
/*!
    @brief  Create a new TimeSpan object using a number of
   days/hours/minutes/seconds e.g. Make a TimeSpan of 3 hours and 45 minutes:
   new TimeSpan(0, 3, 45, 0);
    @param days Number of days
    @param hours Number of hours
    @param minutes Number of minutes
    @param seconds Number of seconds
*/
/**************************************************************************/
TimeSpan::TimeSpan(int16_t days, int8_t hours, int8_t minutes, int8_t seconds)
    : _seconds((int32_t)days * 86400L + (int32_t)hours * 3600 +
               (int32_t)minutes * 60 + seconds) {}

/**************************************************************************/
/*!
    @brief  Copy constructor, make a new TimeSpan using an existing one
    @param copy The TimeSpan to copy
*/
/**************************************************************************/
TimeSpan::TimeSpan(const TimeSpan &copy) : _seconds(copy._seconds) {}

/**************************************************************************/
/*!
    @brief  Add two TimeSpans
    @param right TimeSpan to add
    @return New TimeSpan object, sum of left and right
*/
/**************************************************************************/
TimeSpan TimeSpan::operator+(const TimeSpan &right) const {
  return TimeSpan(_seconds + right._seconds);
}

/**************************************************************************/
/*!
    @brief  Subtract a TimeSpan
    @param right TimeSpan to subtract
    @return New TimeSpan object, right subtracted from left
*/
/**************************************************************************/
TimeSpan TimeSpan::operator-(const TimeSpan &right) const {
  return TimeSpan(_seconds - right._seconds);
}




void RTC_I2C::write_register(uint8_t reg, uint8_t val) {
uint8_t buffer[2] = {reg, val};
//   i2c_dev->write(buffer, 2);
i2c_master_write_to_device(0,DS3231_ADDRESS,buffer,2,pdMS_TO_TICKS(1000));
}

/**************************************************************************/
/*!
    @brief Read value from register.
    @param reg register address
    @return value of register
*/
/**************************************************************************/
uint8_t RTC_I2C::read_register(uint8_t reg) {
    uint8_t buffer[1];
//   i2c_dev->write(&reg, 1);
//   i2c_dev->read(buffer, 1);
//   return buffer[0];
    i2c_master_write_read_device(0,DS3231_ADDRESS,&reg,1,buffer,1,pdMS_TO_TICKS(1000));
    return buffer[0];

}


/**************************************************************************/
/*!
    @brief  Start I2C for the DS3231 and test succesful connection
    @param  wireInstance pointer to the I2C bus
    @return True if Wire can find DS3231 or false otherwise.
*/
/**************************************************************************/
bool RTC_DS3231::begin() {
  
}

/**************************************************************************/
/*!
    @brief  Check the status register Oscillator Stop Flag to see if the DS3231
   stopped due to power loss
    @return True if the bit is set (oscillator stopped) or false if it is
   running
*/
/**************************************************************************/
bool RTC_DS3231::lostPower(void) {
  return read_register(DS3231_STATUSREG) >> 7;
}

/**************************************************************************/
/*!
    @brief  Set the date and flip the Oscillator Stop Flag
    @param dt DateTime object containing the date/time to set
*/
/**************************************************************************/
void RTC_DS3231::adjust(const DateTime &dt) {
  uint8_t buffer[8] = {DS3231_TIME,
                       bin2bcd(dt.second()),
                       bin2bcd(dt.minute()),
                       bin2bcd(dt.hour()),
                       bin2bcd(dowToDS3231(dt.dayOfTheWeek())),
                       bin2bcd(dt.day()),
                       bin2bcd(dt.month()),
                       bin2bcd(dt.year() - 2000U)};
  //i2c_dev->write(buffer, 8);
  i2c_master_write_to_device(0,DS3231_ADDRESS,buffer,8,pdMS_TO_TICKS(1000));
  uint8_t statreg = read_register(DS3231_STATUSREG);
  statreg &= ~0x80; // flip OSF bit
  write_register(DS3231_STATUSREG, statreg);
}

/**************************************************************************/
/*!
    @brief  Get the current date/time
    @return DateTime object with the current date/time
*/
/**************************************************************************/
DateTime RTC_DS3231::now() {
  uint8_t buffer[7];
  buffer[0] = 0;
  //i2c_dev->write_then_read(buffer, 1, buffer, 7);
  i2c_master_write_read_device(0,DS3231_ADDRESS,buffer,1,buffer,7,pdMS_TO_TICKS(1000));
  return DateTime(bcd2bin(buffer[6]) + 2000U, bcd2bin(buffer[5] & 0x7F),
                  bcd2bin(buffer[4]), bcd2bin(buffer[2]), bcd2bin(buffer[1]),
                  bcd2bin(buffer[0] & 0x7F));
}

/**************************************************************************/
/*!
    @brief  Read the SQW pin mode
    @return Pin mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
Ds3231SqwPinMode RTC_DS3231::readSqwPinMode() {
  int mode;
  mode = read_register(DS3231_CONTROL) & 0x1C;
  if (mode & 0x04)
    mode = DS3231_OFF;
  return static_cast<Ds3231SqwPinMode>(mode);
}

/**************************************************************************/
/*!
    @brief  Set the SQW pin mode
    @param mode Desired mode, see Ds3231SqwPinMode enum
*/
/**************************************************************************/
void RTC_DS3231::writeSqwPinMode(Ds3231SqwPinMode mode) {
  uint8_t ctrl = read_register(DS3231_CONTROL);

  ctrl &= ~0x04; // turn off INTCON
  ctrl &= ~0x18; // set freq bits to 0

  write_register(DS3231_CONTROL, ctrl | mode);
}

/**************************************************************************/
/*!
    @brief  Get the current temperature from the DS3231's temperature sensor
    @return Current temperature (float)
*/
/**************************************************************************/
float RTC_DS3231::getTemperature() {
  uint8_t buffer[2] = {DS3231_TEMPERATUREREG, 0};
  i2c_master_write_read_device(0,DS3231_ADDRESS,buffer,1,buffer,2,pdMS_TO_TICKS(1000));
  //i2c_dev->write_then_read(buffer, 1, buffer, 2);
  return (float)buffer[0] + (buffer[1] >> 6) * 0.25f;
}

/**************************************************************************/
/*!
    @brief  Set alarm 1 for DS3231
        @param 	dt DateTime object
        @param 	alarm_mode Desired mode, see Ds3231Alarm1Mode enum
    @return False if control register is not set, otherwise true
*/
/**************************************************************************/
bool RTC_DS3231::setAlarm1(const DateTime &dt, Ds3231Alarm1Mode alarm_mode) {
  uint8_t ctrl = read_register(DS3231_CONTROL);
  if (!(ctrl & 0x04)) {
    return false;
  }

  uint8_t A1M1 = (alarm_mode & 0x01) << 7; // Seconds bit 7.
  uint8_t A1M2 = (alarm_mode & 0x02) << 6; // Minutes bit 7.
  uint8_t A1M3 = (alarm_mode & 0x04) << 5; // Hour bit 7.
  uint8_t A1M4 = (alarm_mode & 0x08) << 4; // Day/Date bit 7.
  uint8_t DY_DT = (alarm_mode & 0x10)
                  << 2; // Day/Date bit 6. Date when 0, day of week when 1.
  uint8_t day = (DY_DT) ? dowToDS3231(dt.dayOfTheWeek()) : dt.day();

  uint8_t buffer[5] = {DS3231_ALARM1, uint8_t(bin2bcd(dt.second()) | A1M1),
                       uint8_t(bin2bcd(dt.minute()) | A1M2),
                       uint8_t(bin2bcd(dt.hour()) | A1M3),
                       uint8_t(bin2bcd(day) | A1M4 | DY_DT)};
  //i2c_dev->write(buffer, 5);
  i2c_master_write_to_device(0,DS3231_ADDRESS,buffer,5,pdMS_TO_TICKS(1000));  
  write_register(DS3231_CONTROL, ctrl | 0x01); // AI1E

  return true;
}

/**************************************************************************/
/*!
    @brief  Set alarm 2 for DS3231
        @param 	dt DateTime object
        @param 	alarm_mode Desired mode, see Ds3231Alarm2Mode enum
    @return False if control register is not set, otherwise true
*/
/**************************************************************************/
bool RTC_DS3231::setAlarm2(const DateTime &dt, Ds3231Alarm2Mode alarm_mode) {
  uint8_t ctrl = read_register(DS3231_CONTROL);
  if (!(ctrl & 0x04)) {
    return false;
  }

  uint8_t A2M2 = (alarm_mode & 0x01) << 7; // Minutes bit 7.
  uint8_t A2M3 = (alarm_mode & 0x02) << 6; // Hour bit 7.
  uint8_t A2M4 = (alarm_mode & 0x04) << 5; // Day/Date bit 7.
  uint8_t DY_DT = (alarm_mode & 0x08)
                  << 3; // Day/Date bit 6. Date when 0, day of week when 1.
  uint8_t day = (DY_DT) ? dowToDS3231(dt.dayOfTheWeek()) : dt.day();

  uint8_t buffer[4] = {DS3231_ALARM2, uint8_t(bin2bcd(dt.minute()) | A2M2),
                       uint8_t(bin2bcd(dt.hour()) | A2M3),
                       uint8_t(bin2bcd(day) | A2M4 | DY_DT)};
  //i2c_dev->write(buffer, 4);
  i2c_master_write_to_device(0,DS3231_ADDRESS,buffer,4,pdMS_TO_TICKS(1000));
  write_register(DS3231_CONTROL, ctrl | 0x02); // AI2E

  return true;
}

/**************************************************************************/
/*!
    @brief  Get the date/time value of Alarm1
    @return DateTime object with the Alarm1 data set in the
            day, hour, minutes, and seconds fields
*/
/**************************************************************************/
DateTime RTC_DS3231::getAlarm1() {
  uint8_t buffer[5] = {DS3231_ALARM1, 0, 0, 0, 0};
  //i2c_dev->write_then_read(buffer, 1, buffer, 5);
  i2c_master_write_read_device(0,DS3231_ADDRESS,buffer,1,buffer,5,pdMS_TO_TICKS(1000));
  uint8_t seconds = bcd2bin(buffer[0] & 0x7F);
  uint8_t minutes = bcd2bin(buffer[1] & 0x7F);
  // Fetching the hour assumes 24 hour time (never 12)
  // because this library exclusively stores the time
  // in 24 hour format. Note that the DS3231 supports
  // 12 hour storage, and sets bits to indicate the type
  // that is stored.
  uint8_t hour = bcd2bin(buffer[2] & 0x3F);

  // Determine if the alarm is set to fire based on the
  // day of the week, or an explicit date match.
  bool isDayOfWeek = (buffer[3] & 0x40) >> 6;
  uint8_t day;
  if (isDayOfWeek) {
    // Alarm set to match on day of the week
    day = bcd2bin(buffer[3] & 0x0F);
  } else {
    // Alarm set to match on day of the month
    day = bcd2bin(buffer[3] & 0x3F);
  }

  // On the first week of May 2000, the day-of-the-week number
  // matches the date number.
  return DateTime(2000, 5, day, hour, minutes, seconds);
}

/**************************************************************************/
/*!
    @brief  Get the date/time value of Alarm2
    @return DateTime object with the Alarm2 data set in the
            day, hour, and minutes fields
*/
/**************************************************************************/
DateTime RTC_DS3231::getAlarm2() {
  uint8_t buffer[4] = {DS3231_ALARM2, 0, 0, 0};
  //i2c_dev->write_then_read(buffer, 1, buffer, 4);
  i2c_master_write_read_device(0,DS3231_ADDRESS,buffer,1,buffer,4,pdMS_TO_TICKS(1000));  
  uint8_t minutes = bcd2bin(buffer[0] & 0x7F);
  // Fetching the hour assumes 24 hour time (never 12)
  // because this library exclusively stores the time
  // in 24 hour format. Note that the DS3231 supports
  // 12 hour storage, and sets bits to indicate the type
  // that is stored.
  uint8_t hour = bcd2bin(buffer[1] & 0x3F);

  // Determine if the alarm is set to fire based on the
  // day of the week, or an explicit date match.
  bool isDayOfWeek = (buffer[2] & 0x40) >> 6;
  uint8_t day;
  if (isDayOfWeek) {
    // Alarm set to match on day of the week
    day = bcd2bin(buffer[2] & 0x0F);
  } else {
    // Alarm set to match on day of the month
    day = bcd2bin(buffer[2] & 0x3F);
  }

  // On the first week of May 2000, the day-of-the-week number
  // matches the date number.
  return DateTime(2000, 5, day, hour, minutes, 0);
}

/**************************************************************************/
/*!
    @brief  Get the mode for Alarm1
    @return Ds3231Alarm1Mode enum value for the current Alarm1 mode
*/
/**************************************************************************/
Ds3231Alarm1Mode RTC_DS3231::getAlarm1Mode() {
  uint8_t buffer[5] = {DS3231_ALARM1, 0, 0, 0, 0};
  //i2c_dev->write_then_read(buffer, 1, buffer, 5);
  i2c_master_write_read_device(0,DS3231_ADDRESS,buffer,1,buffer,5,pdMS_TO_TICKS(1000));
  uint8_t alarm_mode = (buffer[0] & 0x80) >> 7    // A1M1 - Seconds bit
                       | (buffer[1] & 0x80) >> 6  // A1M2 - Minutes bit
                       | (buffer[2] & 0x80) >> 5  // A1M3 - Hour bit
                       | (buffer[3] & 0x80) >> 4  // A1M4 - Day/Date bit
                       | (buffer[3] & 0x40) >> 2; // DY_DT

  // Determine which mode the fetched alarm bits map to
  switch (alarm_mode) {
  case DS3231_A1_PerSecond:
  case DS3231_A1_Second:
  case DS3231_A1_Minute:
  case DS3231_A1_Hour:
  case DS3231_A1_Date:
  case DS3231_A1_Day:
    return (Ds3231Alarm1Mode)alarm_mode;
  default:
    // Default if the alarm mode cannot be read
    return DS3231_A1_Date;
  }
}

/**************************************************************************/
/*!
    @brief  Get the mode for Alarm2
    @return Ds3231Alarm2Mode enum value for the current Alarm2 mode
*/
/**************************************************************************/
Ds3231Alarm2Mode RTC_DS3231::getAlarm2Mode() {
  uint8_t buffer[4] = {DS3231_ALARM2, 0, 0, 0};
  //i2c_dev->write_then_read(buffer, 1, buffer, 4);
  i2c_master_write_read_device(0,DS3231_ADDRESS,buffer,1,buffer,4,pdMS_TO_TICKS(1000));
  uint8_t alarm_mode = (buffer[0] & 0x80) >> 7    // A2M2 - Minutes bit
                       | (buffer[1] & 0x80) >> 6  // A2M3 - Hour bit
                       | (buffer[2] & 0x80) >> 5  // A2M4 - Day/Date bit
                       | (buffer[2] & 0x40) >> 3; // DY_DT

  // Determine which mode the fetched alarm bits map to
  switch (alarm_mode) {
  case DS3231_A2_PerMinute:
  case DS3231_A2_Minute:
  case DS3231_A2_Hour:
  case DS3231_A2_Date:
  case DS3231_A2_Day:
    return (Ds3231Alarm2Mode)alarm_mode;
  default:
    // Default if the alarm mode cannot be read
    return DS3231_A2_Date;
  }
}

/**************************************************************************/
/*!
    @brief  Disable alarm
        @param 	alarm_num Alarm number to disable
*/
/**************************************************************************/
void RTC_DS3231::disableAlarm(uint8_t alarm_num) {
  uint8_t ctrl = read_register(DS3231_CONTROL);
  ctrl &= ~(1 << (alarm_num - 1));
  write_register(DS3231_CONTROL, ctrl);
}

/**************************************************************************/
/*!
    @brief  Clear status of alarm
        @param 	alarm_num Alarm number to clear
*/
/**************************************************************************/
void RTC_DS3231::clearAlarm(uint8_t alarm_num) {
  uint8_t status = read_register(DS3231_STATUSREG);
  status &= ~(0x1 << (alarm_num - 1));
  write_register(DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Get status of alarm
        @param 	alarm_num Alarm number to check status of
        @return True if alarm has been fired otherwise false
*/
/**************************************************************************/
bool RTC_DS3231::alarmFired(uint8_t alarm_num) {
  return (read_register(DS3231_STATUSREG) >> (alarm_num - 1)) & 0x1;
}

/**************************************************************************/
/*!
    @brief  Enable 32KHz Output
    @details The 32kHz output is enabled by default. It requires an external
    pull-up resistor to function correctly
*/
/**************************************************************************/
void RTC_DS3231::enable32K(void) {
  uint8_t status = read_register(DS3231_STATUSREG);
  status |= (0x1 << 0x03);
  write_register(DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Disable 32KHz Output
*/
/**************************************************************************/
void RTC_DS3231::disable32K(void) {
  uint8_t status = read_register(DS3231_STATUSREG);
  status &= ~(0x1 << 0x03);
  write_register(DS3231_STATUSREG, status);
}

/**************************************************************************/
/*!
    @brief  Get status of 32KHz Output
    @return True if enabled otherwise false
*/
/**************************************************************************/
bool RTC_DS3231::isEnabled32K(void) {
  return (read_register(DS3231_STATUSREG) >> 0x03) & 0x01;
}

