
#include <Arduino.h>

#include "M5CoreInk.h"
#include <M5GFX.h>
#include "esp_adc_cal.h"
//#include <M5EPD.h>

#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson.git
#include <NTPClient.h>   //https://github.com/taranais/NTPClient
#include "Orbitron_Medium_20.h"
#include "Orbitron_Bold_70.h"
#include "Orbitron_Bold_32.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>

M5GFX display;

// Tip: website til at konvertere fede fonts: https://oleddisplay.squix.ch/#/home
// Tip: Font oversigt: https://m5stack.lang-ship.com/howto/m5gfx/font/
// Tip: https://community.element14.com/members-area/b/blog/posts/the-calendar-running-on-m5stack-core-ink

char temStr[10];
char humStr[10];
float temHere;
float humHere;

const char *ssid = "ShadowNets";        /// EDIIIT
const char *password = "Shadow666"; // EDI8IT
String town = "Bagsværd";              // EDDIT
String Country = "DK";              // EDDIT
const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=" + town + "," + Country + "&units=metric&APPID=";
const String key = "806f3abba7ae3b7ae74d0c7adab2b10c"; /*EDIT                      */

String payload = ""; // whole json
String tmp = "";     // temperatur
String hum = "";     // humidity
char text[64];
char bat[64];

StaticJsonDocument<1000> doc;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

void getData()
{
  if ((WiFi.status() == WL_CONNECTED))
  { // Check the current connection status
    HTTPClient http;
    http.begin(endpoint + key); // Specify the URL
    int httpCode = http.GET();  // Make the request

    if (httpCode > 0)
    { // Check for the returning code
      payload = http.getString();
    }

    else
    {
      // Serial.println("Error on HTTP request");
    }
    http.end(); // Free the resources
  }
  char inp[1000];
  payload.toCharArray(inp, 1000);
  deserializeJson(doc, inp);

  String tmp2 = doc["main"]["temp"];
  String hum2 = doc["main"]["humidity"];
  String town2 = doc["name"];
  String pressure = doc["main"]["pressure"];
  String visib = doc["main"]["visibility"];
  String windSpeed = doc["wind"]["speed"];
  tmp = tmp2;
  hum = hum2;
  tmp.toCharArray(text, 5);

  // M5.SHT30.UpdateData();
  // temHere = M5.SHT30.GetTemperature();
  // humHere = M5.SHT30.GetRelHumidity();

  display.setTextColor(TFT_BLACK);
  display.setFont(&Orbitron_Bold_70);
  int32_t o;
  o = display.textWidth(text);
  display.setCursor((display.width()/2 - o/2), 100);
  display.print(text);
  // canvas.drawString(String(temHere).substring(0,4),180,260);
  // canvas.drawString(String((int)humHere),400,580);
  // canvas.drawString(town2,100,784);
  // canvas.drawString(pressure+" hPa",122,846);
  // canvas.drawString(windSpeed+" m/s",122,816);
}

float getBatteryVoltage()
{
  analogSetPinAttenuation(35, ADC_11db);
  esp_adc_cal_characteristics_t *adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3600, adc_chars);
  uint16_t adcValue = analogRead(35);
  return float(esp_adc_cal_raw_to_voltage(adcValue, adc_chars)) * 25.1 / 5.1 / 1000;
}

void drawBatteryStatus()
{
  float batteryVoltage = getBatteryVoltage();
  int batteryPercentage = (batteryVoltage < 3.2) ? 0 : (batteryVoltage - 3.2) * 100;
  char batteryBuff[16] = {0};
  sprintf(batteryBuff, "%d%%", batteryPercentage);
  String strBattery = String(batteryBuff);
  display.setTextColor(TFT_BLACK);
  
  display.setFont(&Orbitron_Bold_32);
  int32_t w;
  w = display.textWidth((char *)strBattery.c_str());
  //display.drawPng(batteryIconPng, ~0u, (display.width() - w- 16), 4);
  display.setCursor((display.width() - w), 4);
  display.print(strBattery);
}

void setup(void)
{

  M5.begin();

  display.init();
  display.setEpdMode(epd_mode_t::epd_fastest);
  display.clear(TFT_WHITE);

  WiFi.begin(ssid, password);
  int nowifi = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    ++nowifi;
    if (nowifi > 200)
    {
      drawBatteryStatus();
      display.display();
      display.waitDisplay();
      display.wakeup();
      M5.shutdown();
    }
  }
  timeClient.begin();
  timeClient.setTimeOffset(7200); /*EDDITTTTTTTTTTTTTTTTTTTTTTTT                      */
  delay(100);
}

String curSeconds = "";

void loop()
{

  getData();

  while (!timeClient.update())
  {
    timeClient.forceUpdate();
  }
  // The formattedDate comes with the following format:
  // 2018-05-28T16:00:13Z
  // We need to extract date and time
  formattedDate = timeClient.getFormattedDate();

  int splitT = formattedDate.indexOf("T");
  dayStamp = formattedDate.substring(0, splitT);
  timeStamp = formattedDate.substring(splitT + 1, formattedDate.length() - 1);
  curSeconds = timeStamp.substring(6, 8);
  String current = timeStamp.substring(0, 5);
  drawBatteryStatus();

  display.setCursor(10, 50);
  display.print(current); // print current time

  display.display();
  display.waitDisplay();
  display.wakeup();
  delay(700);
  M5.shutdown(3600); // 600 seconds = 10 minutes
}
