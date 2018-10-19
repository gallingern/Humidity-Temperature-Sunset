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
}


// Warning, contains a hack
bool isDST() {
  int dayOfMonth = Time.day();
  int month = Time.month();
  int dayOfWeek = Time.weekday() - 1; // make Sunday 0 .. Saturday 6
  const int MARCH = 3;
  const int APRIL = 4;
  const int OCTOBER = 10;
  const int NOVEMBER = 11;
  bool isDaylightSavings = false;

  // April to October is DST
  if ((month >= APRIL) && (month <= OCTOBER)) {
    isDaylightSavings = true;
  }

  // March after second Sunday is DST
  if (month == MARCH) {
    // voodoo magic
    if ((dayOfMonth - dayOfWeek) > 8) {
      isDaylightSavings = true;
    }
  }

  // November before first Sunday is DST
  if (month == NOVEMBER) {
    // voodoo magic
    if (!((dayOfMonth - dayOfWeek) > 1)) {
      isDaylightSavings = true;
    }
  }

  return isDaylightSavings;
}


void getSunset() {

  Time.zone(isDST() ? PDT_OFFSET : PST_OFFSET);

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
