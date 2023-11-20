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

// Local date and time capture and time zone correction:
#define NTP_SERVER "pool.ntp.br"

const int daylightOffset_sec = 0;
const long gmtOffset_sec = -3600*3;

// Wi-fi connection, network SSID and password:
#define WIFI_SSID "apiario"
#define WIFI_PASSWORD "12345678"

void setup() {    
    Serial.begin(9600);

    // Wi-fi connection:
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("[!] Connecting to the wi-fi network..");

    while(WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(2500);
    }

    // Identification of connected local IP:
    Serial.println();
    Serial.print("[!] Connected to wi-fi network, IP address: ");
    Serial.println(WiFi.localIP());

    // Initialization of the timestamp capture - ntp server:
    configTime(gmtOffset_sec, daylightOffset_sec, NTP_SERVER);

    // Cloud database connection - Firebase:
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTENTICADOR);
    Firebase.reconnectWiFi(true);

    // Delay to capture the first measure:
    delay(2500);
}

void loop() {
    // NTP server variables capture:
    char current_date[15];
    char current_time[15];

    // Time and date capture:
    struct tm time_info;

    if (!getLocalTime(&time_info)) {
        Serial.println("[!] Failed to capture NTP server data.");
        return;
    }

    // Formatted date and time:
    strftime(current_date, 15, "%e/%m/%Y", &time_info);
    strftime(current_time, 15, "%H:%M:%S", &time_info);
}
