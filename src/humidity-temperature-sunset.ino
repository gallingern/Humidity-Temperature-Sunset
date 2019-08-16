#include "Calendar.h"
#include "Sunrise.h"
#include "PietteTech_DHT.h"
#define DHTTYPE  AM2302         // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   3              // Digital pin for communications

// Berkeley
const int LATITUDE   =  37.8716;
const int LONGITUDE  = -122.2727;

PietteTech_DHT DHT(DHTPIN, DHTTYPE);
Sunrise sunsetBerkeley(LATITUDE,LONGITUDE);

int temp_f = 0;
int humidity = 0;
int head_index_f = 0;
int sunset_time = 0; // in minutes past midnight
const int hour_to_minute = 60;
char temp_string[64];
char time_string[64];
int next_publish = millis();
int publish_interval = 10000; // 10 seconds


void setup() {
  Particle.variable("temp_f", &temp_f, INT);
  Particle.variable("head_index_f", &head_index_f, INT);
  Particle.variable("humidity_%", &humidity, INT);
  Particle.variable("sunset_time", &sunset_time, INT);
}


void getSunset() {

  Time.zone(isDaylightSavingsTime() ? PDT_OFFSET : PST_OFFSET);

  sunsetBerkeley.updateSolarTimes();
  sunset_time = sunsetBerkeley.sunSetHour * hour_to_minute +
                sunsetBerkeley.sunSetMinute;

  sprintf(sunset_string, "%d", sunset_time);
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
  Particle.publish("sunset", sunset_string, 60, PRIVATE);
  Particle.publish("time", Time.timeStr(), 60, PRIVATE);
}


void loop() {
  int result = DHT.acquireAndWait();
  temp_f = (int)DHT.getFahrenheit();
  humidity = (int)DHT.getHumidity();
  heatIndex();
  getSunset();

  if (millis() > next_publish) {
    publish();
    next_publish = millis() + publish_interval;
  }
}
