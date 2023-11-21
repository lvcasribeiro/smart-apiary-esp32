// Libraries import:
#include <Adafruit_Sensor.h>
#include <FirebaseESP32.h>
#include <ArduinoJson.h>
#include "HX711.h"
#include <WiFi.h>
#include <time.h>
#include <DHT.h>

// Cloud database connection - Firebase, host and auth:
#define FIREBASE_AUTENTICADOR "firebase_auth"
#define FIREBASE_HOST "firebase_url"

FirebaseData firebaseData;
FirebaseJson json_measured_variables;

// Local date and time capture and time zone correction:
#define NTP_SERVER "pool.ntp.br"

const int daylightOffset_sec = 0;
const long gmtOffset_sec = -3600*3;

// Wi-fi connection, network SSID and password:
#define WIFI_SSID "your_wi_fi_ssid"
#define WIFI_PASSWORD "your_wi_fi_password"

// Global variables:
unsigned long time_now = 0;
const int debug = 1;

// Digital pinout:
#define DHT_PIN 4
#define SENT_PIN 15
#define FAIL_PIN 2
#define HEART_BEAT_PIN 19
#define LOADCELL_DOUT_PIN 18
#define LOADCELL_SCK_PIN 5

// Analog pinout:
#define KY038_PIN 34
#define LDR_PIN 32

// DHT type:
#define DHT_TYPE DHT22

// Instantiate DHT and HX711 sensors:
DHT dht(DHT_PIN, DHT_TYPE);
HX711 scale;

void setup() {
    // Input:
    pinMode(DHT_PIN, INPUT);
    pinMode(KY038_PIN, INPUT);
    pinMode(LDR_PIN, INPUT);

    // Outputs:
    pinMode(SENT_PIN, OUTPUT);
    pinMode(FAIL_PIN, OUTPUT);
    pinMode(HEART_BEAT_PIN, OUTPUT);
    
    Serial.begin(9600);

    // Wi-fi connection:
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    if(debug >= 1) {
        Serial.print("[!] Connecting to the wi-fi network..");
    }

    while(WiFi.status() != WL_CONNECTED) {
        if(debug >= 1) {
            Serial.print(".");
        }
        delay(2500);
    }

    // Identification of connected local IP:
    if(debug >= 1) {
        Serial.println();
        Serial.print("[!] Connected to wi-fi network, IP address: ");
        Serial.println(WiFi.localIP());
    }
    
    // Initialization of the timestamp capture - ntp server:
    configTime(gmtOffset_sec, daylightOffset_sec, NTP_SERVER);

    // Cloud database connection - Firebase:
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTENTICADOR);
    Firebase.reconnectWiFi(true);

    dht.begin();
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.tare();
    scale.set_scale(-940);

    // Delay to capture the first measure:
    delay(2500);
}

void loop() {
    // Reading variables:
    float humidity = dht.readHumidity();
    float humidity_rounded = round(humidity * 100.0) / 100.0;

    float temperature = dht.readTemperature();
    float temperature_rounded = round(temperature * 100.0) / 100.0;

    // Sound measurement variables:
    int sound_decibels = analogRead(KY038_PIN);
    // int sound_measurement = analogRead(KY038_PIN);
    // float sound_voltage = sound_measurement * (3.3 / 4095.0);
    // float sound_decibels = map(sound_voltage, 0.0, 3.3, 48.0, 66.0);

    // Check if any reads failed and exit early to try again:
    if (isnan(humidity) || isnan(temperature)) {
        if(debug >= 1) {
            Serial.println("[!] Failed to read from DHT sensor!");
        }
        return;
    }

    // Compute heat index in Celsius (isFahreheit = false):
    float heat_index = dht.computeHeatIndex(temperature, humidity, false);
    float heat_index_rounded = round(heat_index * 100.0) / 100.0;

    // Brightness measurement variable:
    int luminosity = analogRead(LDR_PIN);
    float luminosity_percentage = map(luminosity, 0.0, 4095.0, 0.0, 100.0);
    float luminosity_percentage_rounded = round(luminosity_percentage * 100.0) / 100.0;

    // Load:
    float load = round((scale.get_value(10)/-1000.0) * 100.0) / 100.0;

    // NTP server variables capture:
    char current_date[15];
    char current_time[15];

    // Time and date capture:
    struct tm time_info;

    if (!getLocalTime(&time_info)) {
        if(debug >= 1) {
            Serial.println("[!] Failed to capture NTP server data.");
        }
        return;
    }

    // Formatted date and time:
    strftime(current_date, 15, "%e/%m/%Y", &time_info);
    strftime(current_time, 15, "%H:%M:%S", &time_info);

    // Debug:
    if(debug >= 2) {
        Serial.print("Temperature: ");
        Serial.print(temperature_rounded);
        Serial.print(" *C | Humidity: ");
        Serial.print(humidity_rounded);
        Serial.print("% | Heat index: ");
        Serial.print(heat_index_rounded);
        Serial.print(" *C | Sound intensity: ");
        Serial.print(sound_decibels);
        Serial.print(" dB | Luminosity: ");
        Serial.print(luminosity_percentage_rounded);
        Serial.print("% | Load: ");
        Serial.print(load);
        Serial.println(" kg.");
    }

    // Values update, DHT --> cloud database:
    if(Firebase.get(firebaseData, "/measured_variables")) {
        Firebase.set(firebaseData, "measured_variables/temperature", temperature_rounded);
        Firebase.set(firebaseData, "measured_variables/humidity", humidity_rounded);
        Firebase.set(firebaseData, "measured_variables/heat_index", heat_index_rounded);
        Firebase.set(firebaseData, "measured_variables/luminosity", luminosity_percentage_rounded);
        Firebase.set(firebaseData, "measured_variables/sound_intensity", sound_decibels);
        Firebase.set(firebaseData, "measured_variables/load", load);

        digitalWrite(HEART_BEAT_PIN, HIGH);

        // Blink the fail led:
        unsigned long led_heart_beat_timer = millis();
        bool led_heart_beat_on = true;

        while (led_heart_beat_on) {
            if(millis() - led_heart_beat_timer >= 750) { 
                digitalWrite(HEART_BEAT_PIN, LOW);
                led_heart_beat_on = false;
            }
        }
    } else {
        if(debug >= 1) {
            Serial.print("[!] Error updating cloud data: [FIREBASE LATENCY][ttl] - ");
            Serial.print(firebaseData.errorReason());
            Serial.println(".");
        }
    }

    if(millis() >= time_now + 60000) {
        time_now += 60000;
        // Incrementing a new json object:
        json_measured_variables.set("temperature", temperature_rounded);
        json_measured_variables.set("humidity", humidity_rounded);
        json_measured_variables.set("heat_index", heat_index_rounded);
        json_measured_variables.set("luminosity", luminosity_percentage_rounded);
        json_measured_variables.set("sound_intensity", sound_decibels);
        json_measured_variables.set("load", load);
        json_measured_variables.set("timestamp", current_time);
        json_measured_variables.set("date", current_date);
    
        if (Firebase.pushJSON(firebaseData, "/json_storage", json_measured_variables)) {
            digitalWrite(SENT_PIN, HIGH);

            // Blink the successful led:
            unsigned long led_sent_timer = millis();
            bool led_sent_on = true;

            while (led_sent_on) {
                if(millis() - led_sent_timer >= 750) { 
                    digitalWrite(SENT_PIN, LOW);
                    led_sent_on = false;
                }
            }
        } else {
            if(debug >= 1) {
                Serial.print("[!] Error sending json cloud data: [FIREBASE LATENCY][ttl] - ");
                Serial.print(firebaseData.errorReason());
                Serial.println(".");
            }
    
            digitalWrite(FAIL_PIN, HIGH);

            // Blink the fail led:
            unsigned long led_fail_timer = millis();
            bool led_fail_on = true;

            while (led_fail_on) {
                if(millis() - led_fail_timer >= 750) { 
                    digitalWrite(FAIL_PIN, LOW);
                    led_fail_on = false;
                }
            }
        }
    }
}
