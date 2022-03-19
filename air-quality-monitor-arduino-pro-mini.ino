#include "PMS.h"
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define PIN            9
#define NUMPIXELS      1

LiquidCrystal_I2C lcd(0x3F,20,4);
SoftwareSerial pmsSerial(2, 3);
PMS pms(pmsSerial);
PMS::DATA data;
Adafruit_BME280 bme;
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

int humidity = 0;

int colorLevel = 30;
uint32_t red = pixels.Color(colorLevel, 0, 0);
uint32_t green = pixels.Color(0, colorLevel, 0);
uint32_t blue = pixels.Color(0, 0, colorLevel);
uint32_t yellow = pixels.Color(colorLevel, colorLevel, 0);
uint32_t orange = pixels.Color(colorLevel, ((int)colorLevel/3), 0);

char buffer[30];

int pm10level = 50;
int pm25level = 25;

const int numberOfsamples = 10;
int pm10data[numberOfsamples];
int pm25data[numberOfsamples];
int sampleDataIndex = 0;

int pm10sum = 0;
int pm25sum = 0;
int pm10avg = 0;
int pm25avg = 0;

int lineShift = 0;

void setup()
{
  Serial.begin(9600);
  pmsSerial.begin(9600);

  lcd.init();                      // initialize the lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(2,1);
  lcd.print("Reading Data...");

  if (! bme.begin(0x76, &Wire)) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
  }
 
  pixels.begin();  
  Serial.println("Pixels clear");
  pixels.clear();
  pixels.show();
}

void loop() {
  if (pms.read(data)) {
    pm10data[sampleDataIndex] = data.PM_AE_UG_10_0;
    pm25data[sampleDataIndex] = data.PM_AE_UG_2_5;
    Serial.print("Index:");  
    Serial.print(sampleDataIndex);
    Serial.print("  PM 2.5 (ug/m3): ");  
    Serial.print(data.PM_AE_UG_2_5);
    Serial.print("  PM 10.0 (ug/m3): ");  
    Serial.println(data.PM_AE_UG_10_0);
    sampleDataIndex++;
  }
 
  if (sampleDataIndex >= numberOfsamples) {
    sampleDataIndex = 0;
    pm10sum = 0;
    pm25sum = 0;
   
    for (int i = 0; i < numberOfsamples; i++) {
      pm10sum += pm10data[i];
      pm25sum += pm25data[i];
    }

    pm10avg = (int) (pm10sum / numberOfsamples);
    pm25avg = (int) (pm25sum / numberOfsamples);

    lcd.clear();

    sprintf(buffer, "PM 2.5: %d (%d%%)", pm25avg, ((int) (100 * pm25avg / pm25level)));
    Serial.println(buffer);
    Serial.println(strlen(buffer));

    lineShift = (int)((20 - strlen(buffer))/2);
    if (lineShift < 0 || lineShift > 4) {
      lineShift = 0;
    }
    lcd.setCursor(lineShift, 2);
    lcd.print(buffer);

    sprintf(buffer, "PM 10.0: %d (%d%%)", pm10avg, ((int) (100 * pm10avg / pm10level)));
    Serial.println(buffer);
    Serial.println(strlen(buffer));

    lineShift = (int)((20 - strlen(buffer))/2);
    if (lineShift < 0 || lineShift > 4) {
      lineShift = 0;
    }
    lcd.setCursor(lineShift, 3);
    lcd.print(buffer);

    pixels.setPixelColor(0, getColor(pm10avg));
    pixels.show();

    lcd.setCursor(0,0);

    lcd.print((int)bme.readTemperature());
    lcd.print(" *C  ");
   
    lcd.print((int) (bme.readPressure() / 100.0F));
    lcd.print(" hPa ");

    humidity = (int) bme.readHumidity();
    if (humidity < 100) {
      lcd.print(" ");
    }

    lcd.print((int)bme.readHumidity());
    lcd.print("%");
   
    Serial.println();
  }
}

uint32_t getColor(int pm10) {
  if (pm10 < 21) {
    return blue;
  } else if (pm10 < 61) {
    return green;
  } else if (pm10 < 101) {
    return yellow;
  } else if (pm10 < 141) {
    return orange;
  } else {
    return red;
  }
}
