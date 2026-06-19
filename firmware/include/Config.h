#pragma once

// Constantes matérielles du firmware (pins, timings, topics). WiFi / broker /
// DEVICE_ID sont injectés via build_flags (cf. platformio.ini).

// WIFI_PASSWORD vient de la CI (secret GitHub). Fallback vide pour un build
// local sans secret : ça compile, mais le WiFi ne se connecte pas.
#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

// MQTT_BROKER_HOST optionnel : vide → le firmware vise la passerelle WiFi (la
// borne, qui héberge l'AP FLIPHETIC_CAB0). cf. connectMqtt() dans main.cpp.
#ifndef MQTT_BROKER_HOST
#define MQTT_BROKER_HOST ""
#endif

// --- Mapping boutons → GPIO (ESP32) ---------------------------------------
// L'`id` publié sur MQTT est interprété côté backend (cf. contrat). Le firmware
// reste agnostique du jeu : il dit juste « tel id pressé/relâché ».
//
//   id publié | GPIO | bouton physique        | rôle (décidé par le back)
#define PIN_FLIPPER_LEFT 4    // "L1"  | white left   | flipper gauche
#define PIN_FLIPPER_RIGHT 13  // "R1"  | black right  | flipper droit
#define PIN_NAV_LEFT 16       // "L2"  | black left   | navigation gauche
#define PIN_NAV_RIGHT 25      // "R2"  | white right  | navigation droite
#define PIN_FRONT_TOP 17      // "top"    | vert      | valider / start
#define PIN_FRONT_MIDDLE 18   // "middle" | jaune     | secondaire
#define PIN_FRONT_BOTTOM 19   // "bottom" | rouge     | retour / pause
#define PIN_UNDER_PLUNGER 33  // "under_plunger" | front white
#define PIN_PLUNGER 32        // plunger  | lanceur

// --- Timings --------------------------------------------------------------
#define BUTTON_DEBOUNCE_MS 50

// --- Topics MQTT (contrat back ↔ firmware) --------------------------------
#define TOPIC_BUTTON "pinball/" DEVICE_ID "/input/button"
#define TOPIC_PLUNGER "pinball/" DEVICE_ID "/input/plunger"
#define TOPIC_STATUS "pinball/" DEVICE_ID "/esp32/status"

// Tentatives de (re)connexion WiFi avant d'abandonner (0 = illimité).
#define WIFI_MAX_CONNECTION_RETRY 0
