## Smart Apiary

This repository aims to concentrate relevant information and code about the programming of the esp32 microcontroller for the smart apiary project.

##

### Basic information about the project

###### 1. Sensors pinout:

1. PIN 04 - `DHT22`;
2. PIN 32 - `LDR`;
3. PIN 34 - `KY038` and;
4. PIN 05 (SCK) and 18 (DOUT) - `HX711`.

<br>

###### 2. LED's pinout:

1. PIN 15 - `SENT_PIN`;
2. PIN 02 - `FAIL_PIN` and;
3. PIN 19 - `HEART_BEAT_PIN`.

<br>

###### 3. Script changes needed:

Create and configure your credentials for network access on lines 24 and 25, as in the excerpts illustrated below:

```c
#define WIFI_SSID "your_wi_fi_ssid"
#define WIFI_PASSWORD "your_wi_fi_password"
```

Similarly, in lines 11 and 12, make sure to change the firebase cloud database access credentials:

```c
#define FIREBASE_AUTENTICADOR "firebase_auth"
#define FIREBASE_HOST "firebase_url"
```

##

### Remider

The project is still under development, and there may be some bugs or inconsistencies to be corrected. If you have any questions or problems, please contact me via email: lucas.and.rib@gmail.com. Thank you for understanding.

