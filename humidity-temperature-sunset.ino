#include <Sunrise.h>
#include <PietteTech_DHT.h>
#define DHTTYPE  AM2302         // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   3              // Digital pin for communications
// Berkeley
#define LATITUDE        37.8716
#define LONGITUDE       -122.2727
#define PST_OFFSET      -8
#define PDT_OFFSET      -7

PietteTech_DHT DHT(DHTPIN, DHTTYPE);
Sunrise sunsetSummer(LATITUDE,LONGITUDE, PDT_OFFSET);
Sunrise sunsetWinter(LATITUDE,LONGITUDE, PST_OFFSET);

int temp_f = 0;
int humidity = 0;
int head_index_f = 0;
int sunsetHour = 0;
int sunsetMinute = 0;
char temp_string[64];
char hour_string[64];
char minute_string[64];
int nextPublish = millis();


void setup() {
    Serial.begin(9600);
    Particle.syncTime();
    Time.zone(Time.isDST() ? PDT_OFFSET : PST_OFFSET);
    sunsetSummer.Actual(); //Actual, Civil, Nautical, Astronomical
    sunsetWinter.Actual();
    Particle.variable("temp_f", &temp_f, INT);
    Particle.variable("head_index_f", &head_index_f, INT);
    Particle.variable("humidity_%", &humidity, INT);
    Particle.variable("sunsetHour", &sunsetHour, INT);
    Particle.variable("sunsetMinute", &sunsetMinute, INT);
}


//http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
void heatIndex() {
    double T = DHT.getFahrenheit();
    double RH = DHT.getHumidity();
    head_index_f = (int)(0.5 * (T + 61.0 + ((T-68.0)*1.2) + (RH*0.094)));
    sprintf(temp_string, "%d", head_index_f);
}


void getSunset() {
    // t == minutes after midnight of sunset
    int t = (Time.isDST() ? sunsetSummer.Set(Time.month(),Time.day()) : sunsetWinter.Set(Time.month(),Time.day()));
    sunsetHour = (Time.isDST() ? sunsetSummer.sun_Hour() : sunsetWinter.sun_Hour());
    sunsetMinute = (Time.isDST() ? sunsetSummer.sun_Minute() : sunsetWinter.sun_Minute());
    sprintf(hour_string, "%d", sunsetHour);
    sprintf(minute_string, "%d", sunsetMinute);
}


void publish() {
    Particle.publish("temp", temp_string, 60, PRIVATE);
    Particle.publish("hour", hour_string, 60, PRIVATE);
    Particle.publish("minute", minute_string, 60, PRIVATE);
}


void loop() {
    int result = DHT.acquireAndWait();
    temp_f = (int)DHT.getFahrenheit();
    humidity = (int)DHT.getHumidity();
    heatIndex();
    getSunset();
    
    if (millis() > nextPublish) {
        publish();
        nextPublish = millis() + 5000;
    }
}