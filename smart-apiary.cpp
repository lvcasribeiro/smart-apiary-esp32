// Libraries import:
#include <Adafruit_Sensor.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>
#include "HX711.h"
#include <WiFi.h>
#include <time.h>
#include <DHT.h>

// Cloud database connection - Firebase, host and auth:
#define FIREBASE_AUTENTICADOR "AIzaSyAW9ZLQhBTjqRVtM-m-oyHLqBTTNqZAOMk"
#define FIREBASE_HOST "https://smart-apiary-default-rtdb.firebaseio.com"

FirebaseData firebaseData;
FirebaseJson json_measured_variables;

void setup() {
    Serial.begin(9600);

    // Cloud database connection - Firebase:
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTENTICADOR);
    Firebase.reconnectWiFi(true);

    // Delay to capture the first measure:
    delay(2500);
}

void loop() {
    
}
