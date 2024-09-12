#include <ESP8266WiFi.h>

#define LED_PIN 2

void blink(int gpio, bool reverse = false) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, (reverse) ? LOW : HIGH);
    delay(500);
    digitalWrite(LED_PIN, (reverse) ? HIGH : LOW);
    delay(500);
}

void setup() {
    Serial.begin(115200);

    blink(LED_PIN, true);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_NAME, WIFI_PASSWORD);

    if (WiFi.status() != WL_CONNECTED) {
        Serial.printf("SSID '%s' is not connected\n", WIFI_NAME);
        blink(LED_PIN, true);
    }

    Serial.printf("Connected SSID '%s', channel %u\n", WIFI_NAME, WiFi.channel());
}

void loop() {

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("WiFi RSSI:\t%d dBm\n", WiFi.RSSI());
    }

    blink(LED_PIN, true);

    delay(2000);
}
