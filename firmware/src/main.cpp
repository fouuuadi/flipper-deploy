#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <WiFi.h>

#include "Button.h"
#include "Config.h"

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

// Un bouton par GPIO. L'id (2e arg du bind) est ce qui part sur MQTT.
Button flipperLeft(PIN_FLIPPER_LEFT);
Button flipperRight(PIN_FLIPPER_RIGHT);
Button navLeft(PIN_NAV_LEFT);
Button navRight(PIN_NAV_RIGHT);
Button frontTop(PIN_FRONT_TOP);
Button frontMiddle(PIN_FRONT_MIDDLE);
Button frontBottom(PIN_FRONT_BOTTOM);
Button underPlunger(PIN_UNDER_PLUNGER);
Button plunger(PIN_PLUNGER);

// --- Publication MQTT -------------------------------------------------------

void publishButton(const char *id, int state) {
  JsonDocument doc;
  doc["id"] = id;
  doc["state"] = state;
  doc["ts"] = millis();
  char buf[96];
  const size_t n = serializeJson(doc, buf);
  mqtt.publish(TOPIC_BUTTON, reinterpret_cast<const uint8_t *>(buf), n);
  Serial.printf("[input] button %s state=%d\n", id, state);
}

void publishPlunger(int state) {
  JsonDocument doc;
  doc["state"] = state;
  doc["ts"] = millis();
  char buf[64];
  const size_t n = serializeJson(doc, buf);
  mqtt.publish(TOPIC_PLUNGER, reinterpret_cast<const uint8_t *>(buf), n);
  Serial.printf("[input] plunger state=%d\n", state);
}

// press → state 1, release → state 0 : c'est *le* point clé (un flipper doit
// pouvoir rester levé tant que le bouton est maintenu).
void bindButton(Button &button, const char *id) {
  button.onPress([id]() { publishButton(id, 1); });
  button.onRelease([id]() { publishButton(id, 0); });
}

// --- Connectivité -----------------------------------------------------------

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("[wifi] connecting");
  unsigned int retry = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (WIFI_MAX_CONNECTION_RETRY != 0 && ++retry >= WIFI_MAX_CONNECTION_RETRY) {
      Serial.println(" giving up");
      return;
    }
  }
  Serial.printf("\n[wifi] connected, ip=%s\n", WiFi.localIP().toString().c_str());
}

void connectMqtt() {
  mqtt.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
  while (!mqtt.connected()) {
    Serial.printf("[mqtt] connecting to %s:%d ...", MQTT_BROKER_HOST, MQTT_BROKER_PORT);
    if (mqtt.connect(DEVICE_ID)) {
      Serial.println(" ok");
    } else {
      Serial.printf(" failed rc=%d, retry in 2s\n", mqtt.state());
      delay(2000);
    }
  }
}

// --- Setup / loop -----------------------------------------------------------

// Fail-fast : si la config d'environnement n'a pas été injectée (variables
// vides), on bloque avec un message clair plutôt que de boucler en silence.
void requireConfig() {
  if (strlen(WIFI_SSID) == 0 || strlen(MQTT_BROKER_HOST) == 0) {
    while (true) {
      Serial.println(
          "[FATAL] config manquante : definir WIFI_SSID / MQTT_BROKER_HOST "
          "(variables d'environnement, cf. README)");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  requireConfig();
  connectWifi();
  connectMqtt();

  flipperLeft.begin();
  flipperRight.begin();
  navLeft.begin();
  navRight.begin();
  frontTop.begin();
  frontMiddle.begin();
  frontBottom.begin();
  underPlunger.begin();
  plunger.begin();

  bindButton(flipperLeft, "L1");
  bindButton(flipperRight, "R1");
  bindButton(navLeft, "L2");
  bindButton(navRight, "R2");
  bindButton(frontTop, "top");
  bindButton(frontMiddle, "middle");
  bindButton(frontBottom, "bottom");
  bindButton(underPlunger, "under_plunger");

  plunger.onPress([]() { publishPlunger(1); });
  plunger.onRelease([]() { publishPlunger(0); });

  Serial.println("[setup] ready");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWifi();
  if (!mqtt.connected()) connectMqtt();
  mqtt.loop();

  flipperLeft.update();
  flipperRight.update();
  navLeft.update();
  navRight.update();
  frontTop.update();
  frontMiddle.update();
  frontBottom.update();
  underPlunger.update();
  plunger.update();
}
