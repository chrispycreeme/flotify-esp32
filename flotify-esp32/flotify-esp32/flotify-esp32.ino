// Heres what you need to do po coach;
// I'm having trouble connecting my ESP32 sa phpmyadmin ko:
// See code lines: 21, 42-47, 64-89, 
// I need help sa pag send ng data sa php database ko, or if u have another way of doing it, kahit sa local host, you can do it naman po. Anything to give me an idea on how to do it. ehehe.
// thanksss

#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

#define RX_Ultrasonic 16
#define TX_Ultrasonic 17
#define WATERSENSOR 36
#define DHTPIN 21
#define DHTTYPE DHT11
#define rainSensor 39
#define floatSwitchPin 22

const char* ssid = "PLDTHOMEFIBER_A+";
const char* password = "Aplus_SOLUTIONS_2023";
const char* serverName = "http://192.168.1.135/flotify-php/dataHandler.php"; 

DHT dht(DHTPIN, DHTTYPE);
const int ledPin = 2;
// const int relay = 8;

const float coeff1 = 4.01187481e-6;
const float coeff2 = -7.17971039e-3;
const float coeff3 = 3.13373787;

void setup() {
  pinMode(floatSwitchPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  // pinMode(relay, OUTPUT);
  pinMode(WATERSENSOR, INPUT);
  Serial.begin(115200);
  // digitalWrite(relay, LOW);
  pinMode(TX_Ultrasonic, OUTPUT);
  pinMode(RX_Ultrasonic, INPUT);
  pinMode(rainSensor, INPUT);
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float distance = measureDistance();
  int waterLevel = analogRead(WATERSENSOR);
  int floatSwitchState = digitalRead(floatSwitchPin);
  int rainVal = map(analogRead(rainSensor), 0, 4095, 0, 1023);
  float rainAnalog = coeff1 * rainVal * rainVal + coeff2 * rainVal + coeff3;
  String waterLevelIndicator = determineWaterIndicator(floatSwitchState, waterLevel, distance);
  int critPrcnt = calculateCriticalPercentage(waterLevel);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String serverPath = String(serverName) + "?rainAnalog=" + String(rainAnalog)
                       + "&rainIntensity=" + String(rainAnalog)
                       + "&waterlevel_ultrasonic=" + String(distance)
                       + "&waterLevel_indicator=" + waterLevelIndicator
                       + "&temperature=" + String(temperature)
                       + "&humidity=" + String(humidity);

    http.begin(serverPath.c_str());
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

  Serial.println(waterLevelIndicator);
  printReadings(distance, critPrcnt, waterLevel);
  readDHTSensor();
  readRainSensor();

  delay(2000);
}

void handleFloatSwitchState(int state) {
  if (state == LOW) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Float switch triggered: Liquid level is high");
    // digitalWrite(relay, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
    // digitalWrite(relay, LOW);
  }
}

float measureDistance() {
  digitalWrite(TX_Ultrasonic, LOW);
  delayMicroseconds(5);
  digitalWrite(TX_Ultrasonic, HIGH);
  delayMicroseconds(20);
  digitalWrite(TX_Ultrasonic, LOW);

  float duration = pulseIn(RX_Ultrasonic, HIGH);
  return duration * 0.034 / 2;
}

int calculateCriticalPercentage(int waterLevel) {
  return map(waterLevel, 0, 4095, 100, 0);
}

String determineWaterIndicator(int floatSwitchState, int calculateCriticalPercentage, float distance) {
  String waterIndicator = "Undefined";
  switch (floatSwitchState) {
    case HIGH:
      if (calculateCriticalPercentage <= 85 && (distance * 0.01) >= 1.00) {
        waterIndicator = "Safe";
      }
      break;

    case LOW:
      if (calculateCriticalPercentage <= 85 && (distance * 0.01) > 0.50 && (distance * 0.01) <= 1.00) {
        waterIndicator = "Moderate";
      } else if (calculateCriticalPercentage > 85 && (distance * 0.01) < 0.50) {
        waterIndicator = "Critical";
      }
      break;
  }

  return waterIndicator;
}

void readRainSensor() {
  int rainVal = map(analogRead(rainSensor), 0, 4095, 0, 1023);
  float rainAnalog = coeff1 * rainVal * rainVal + coeff2 * rainVal + coeff3;
  Serial.print("Rain Analog Value: ");
  Serial.println(rainVal);
  Serial.print("Rain Calculated MM: ");
  Serial.print(rainAnalog);
  Serial.println("mm");
}

void printReadings(float distance, int calculateCriticalPercentage, int waterLevel) {
  Serial.print("Distance: ");
  Serial.print(distance * 0.01);
  Serial.println(" m");
  Serial.print("Critical Levels ");
  Serial.print(calculateCriticalPercentage);
  Serial.println("%");
  Serial.println(waterLevel);
}

void readDHTSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");
}
