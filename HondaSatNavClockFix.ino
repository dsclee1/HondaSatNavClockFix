char bytes[100];
int count = 0;
bool is_10 = false, is_75 = false;

// see https://content.u-blox.com/sites/default/files/GPS-WeekNumber-Rollover-Workaround_AppNote_(UBX-19016936).pdf
static const uint16_t month_days[2][13] = {
  { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
  { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

uint16_t day_number_1980(uint16_t year, uint16_t month, uint16_t day)
{
  uint16_t gps_years = year - 1980;
  uint16_t leap_year = (gps_years % 4 == 0) ? 1 : 0;
  uint16_t day_of_year = month_days[leap_year][month - 1] + day;
  if (gps_years == 0) return day_of_year;
  return gps_years * 365 + ((gps_years - 1) / 4) + 1 + day_of_year;
}

void date_1980(uint16_t day_number, uint16_t *year, uint16_t *month, uint16_t *day)
{
  uint16_t gps_years = ((day_number - 1) * 100) / 36525;
  uint16_t leap_year = (gps_years % 4 == 0) ? 1 : 0;
  uint16_t day_of_year = day_number;
  if (gps_years > 0)
  day_of_year = day_number - (gps_years * 365 + ((gps_years - 1) / 4) + 1);
  uint16_t month_of_year = (day_of_year - 1) / 31 + 1;
  if (day_of_year > month_days[leap_year][month_of_year])
  month_of_year++;
  *day = day_of_year - month_days[leap_year][month_of_year - 1];
  *month = month_of_year;
  *year = 1980 + gps_years;
}

uint8_t dec_to_bcd(uint8_t data)
{
  return (uint8_t)(((data/10)<<4)|(data%10));
}

void process_date(char *yearbyte, char *monthbyte, char *daybyte)
{
  uint16_t year = (((*yearbyte >> 4) + 200) * 10) + (*yearbyte & 0xf);
  uint16_t month = (*monthbyte & 0xf) + ((*monthbyte >> 4) * 10);
  uint16_t day = (*daybyte & 0xf) + ((*daybyte >> 4) * 10);
  // get number of days since 1980
  uint16_t days_since_1980 = day_number_1980(year, month, day);
  // correct the number of days - add our 1024 weeks - this will be wrong again in 2038
  days_since_1980 = days_since_1980 + (1024 * 7);
  // change our year, month, day values
  date_1980(days_since_1980, &year, &month, &day);
  // convert back to bytes
  *yearbyte = dec_to_bcd(year - 2000); // convert to 2 digit year - this will be wrong in 2100
  *monthbyte = dec_to_bcd(month);
  *daybyte = dec_to_bcd(day);
}

// byte packet indexes
#define YEARBYTE 21
#define MONTHBYTE 22
#define DAYBYTE 23
#define PREVYEARBYTE 27
#define PREVMONTHBYTE 28
#define PREVDAYBYTE 29
#define CSUMBYTE 46

void setup() {
  Serial.begin(9600, SERIAL_8O1);
}

void loop() {
  if (Serial.available()) {
    bytes[count] = Serial.read();
    // 0x10 will be duplicated in the packet
    if (bytes[count] == 0x10 && is_10 == false) is_10 = true;
    else
    {
      // last byte was a 0x10, check if we've started a 0x75 or are at the end with a 0x03
      if (is_10)
      {
        is_10 = false;
        if (bytes[count] == 0x75)
        {
          Serial.write(0x10);
          Serial.write(0x75);
          count = 2;
          is_75 = true;
          return;
        }
        else if (bytes[count] == 0x03)
        {
          Serial.write(0x10);
          Serial.write(0x03);
          is_75 = false;
          count = 0;
          return;
        }
        else if (bytes[count] != 0x10)
        {
          // not a packet of interest - write it
          Serial.write(0x10);
          Serial.write(bytes[count]);
          count = 0;
          return;
        }
      }
      // only want to change 0x75 packets
      if (is_75)
      {
        // current - year, month or day byte
        if (count == YEARBYTE || count == MONTHBYTE || count == DAYBYTE)
        {
          // take action on the day byte
          if (count == DAYBYTE)
          {
            // check for zeroed month
            if (bytes[MONTHBYTE] != 0) process_date(&bytes[YEARBYTE], &bytes[MONTHBYTE], &bytes[DAYBYTE]);
            for (int i = YEARBYTE; i <= DAYBYTE; i++)
            {
              Serial.write(bytes[i]);
              // repeat 0x10
              if (bytes[i] == 0x10) Serial.write(0x10);
            }
          }
        }
        // previous GPS fix - year, month or day byte
        else if (count == PREVYEARBYTE || count == PREVMONTHBYTE || count == PREVDAYBYTE)
        {
          if (count == PREVDAYBYTE)
          {
            if (bytes[PREVMONTHBYTE] != 0) process_date(&bytes[PREVYEARBYTE], &bytes[PREVMONTHBYTE], &bytes[PREVDAYBYTE]);
            for (int i = PREVYEARBYTE; i <= PREVDAYBYTE; i++)
            {
              Serial.write(bytes[i]);
              if (bytes[i] == 0x10) Serial.write(0x10);
            }
          }
        }
        // calculate checksum
        else if (count == CSUMBYTE)
        {
          char csum = 0;
          for (int i = 2; i < CSUMBYTE; i++) csum ^= bytes[i];
          Serial.write(csum);
          if (csum == 0x10) Serial.write(0x10);
        }
        // write the bytes of no interest
        else
        {
          Serial.write(bytes[count]);
          if (bytes[count] == 0x10) Serial.write(0x10);
        }
        count++;
      }
      // write the bytes of no interest
      else
      {
        Serial.write(bytes[count]);
        if (bytes[count] == 0x10) Serial.write(0x10);
      }
    }
  }
}
