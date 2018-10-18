#include "Sunrise.h"
#include "PietteTech_DHT.h"
#define DHTTYPE  AM2302         // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   3              // Digital pin for communications

// Berkeley
const int LATITUDE   =  37.8716;
const int LONGITUDE  = -122.2727;
const int PST_OFFSET = -8;
const int PDT_OFFSET = -7;

PietteTech_DHT DHT(DHTPIN, DHTTYPE);
Sunrise sunsetBerkeley(LATITUDE,LONGITUDE);

int temp_f = 0;
int humidity = 0;
int head_index_f = 0;
int sunsetHour = 0;
int sunsetMinute = 0;
int UTCOffset = 0;
int DSTStatus = 0;
char temp_string[64];
char hour_string[64];
char minute_string[64];
int nextPublish = millis();
int publishInterval = 10000; // 10 seconds


void setup() {
  Serial.begin(9600);

  Particle.variable("temp_f", &temp_f, INT);
  Particle.variable("head_index_f", &head_index_f, INT);
  Particle.variable("humidity_%", &humidity, INT);
  Particle.variable("sunsetHour", &sunsetHour, INT);
  Particle.variable("sunsetMinute", &sunsetMinute, INT);
  Particle.variable("isDST", &DSTStatus, INT);
}


bool isDST()
{ // (Central) European Summer Timer calculation (last Sunday in March/October)
  int dayOfMonth = Time.day();
  int month = Time.month();
  int dayOfWeek = Time.weekday() - 1; // make Sunday 0 .. Saturday 6

  if (month >= 4 && month <= 9)
  { // April to September definetly DST
    return true;
  }
  else if (month < 3 || month > 10)
  { // before March or after October is definetly standard time
    return false;
  }

  // March and October need deeper examination
  boolean lastSundayOrAfter = (dayOfMonth - dayOfWeek > 24);
  if (!lastSundayOrAfter)
  { // before switching Sunday
    return (month == 10); // October DST will be true, March not
  }

  if (dayOfWeek)
  { // AFTER the switching Sunday
    return (month == 3); // for March DST is true, for October not
  }

  int secSinceMidnightUTC = Time.now() % 86400;
  boolean dayStartedAs = (month == 10); // DST in October, in March not
  // on switching Sunday we need to consider the time
  if (secSinceMidnightUTC >= 1*3600)
  { // 1:00 UTC (=1:00 GMT/2:00 BST or 2:00 CET/3:00 CEST)
    return !dayStartedAs;
  }

  return dayStartedAs;
}


void getSunset() {

  DSTStatus = isDST();

  if (DSTStatus) {
    UTCOffset = PDT_OFFSET;
  }
  else {
    UTCOffset = PST_OFFSET;
  }

  Time.zone(UTCOffset);

  sunsetBerkeley.updateSolarTimes();
  sunsetHour = sunsetBerkeley.sunSetHour;
  sunsetMinute = sunsetBerkeley.sunSetMinute;

  sprintf(hour_string, "%d", sunsetHour);
  sprintf(minute_string, "%d", sunsetMinute);
}


//http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
void heatIndex() {
  double T = DHT.getFahrenheit();
  double RH = DHT.getHumidity();
  head_index_f = (int)(0.5 * (T + 61.0 + ((T-68.0)*1.2) + (RH*0.094)));
  sprintf(temp_string, "%d", head_index_f);
}


void publish() {
  Particle.publish("temp", temp_string, 60, PRIVATE);
  Particle.publish("hour", hour_string, 60, PRIVATE);
  Particle.publish("minute", minute_string, 60, PRIVATE);
  Particle.publish("time", Time.timeStr(), 60, PRIVATE);
}


void loop() {
  int result = DHT.acquireAndWait();
  temp_f = (int)DHT.getFahrenheit();
  humidity = (int)DHT.getHumidity();
  heatIndex();
  getSunset();

  if (millis() > nextPublish) {
    publish();
    nextPublish = millis() + publishInterval;
  }
}
