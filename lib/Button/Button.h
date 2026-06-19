#pragma once

#include <Arduino.h>
#include <functional>

/**
 * Bouton poussoir débouncé câblé en INPUT_PULLUP (relâché = HIGH, pressé = LOW).
 *
 * Émet deux callbacks **distincts** : `onPress` à l'appui, `onRelease` au
 * relâché. C'est essentiel pour les flippers (maintenir le flipper levé tant
 * que le bouton est enfoncé) — un appui seul ne suffit pas.
 */
class Button {
 public:
  using Callback = std::function<void()>;

  explicit Button(uint8_t pin, uint16_t debounceMs = 50);

  void begin();   // configure le GPIO (à appeler dans setup())
  void update();  // échantillonne + déclenche les callbacks (à appeler dans loop())

  void onPress(Callback cb);
  void onRelease(Callback cb);

  bool isPressed() const { return _stable; }

 private:
  uint8_t _pin;
  uint16_t _debounceMs;
  bool _stable;        // état stable débouncé (true = pressé)
  bool _lastReading;   // dernière lecture brute (true = pressé)
  unsigned long _lastChange;
  Callback _onPress = nullptr;
  Callback _onRelease = nullptr;
};
