#include "Button.h"

Button::Button(uint8_t pin, uint16_t debounceMs)
    : _pin(pin),
      _debounceMs(debounceMs),
      _stable(false),
      _lastReading(false),
      _lastChange(0) {}

void Button::begin() {
  pinMode(_pin, INPUT_PULLUP);
  // Pull-up : pressé = LOW. On initialise l'état stable sur la lecture courante
  // pour ne pas émettre un faux event au boot.
  const bool pressed = digitalRead(_pin) == LOW;
  _stable = pressed;
  _lastReading = pressed;
  _lastChange = millis();
}

void Button::onPress(Callback cb) { _onPress = cb; }
void Button::onRelease(Callback cb) { _onRelease = cb; }

void Button::update() {
  const bool reading = digitalRead(_pin) == LOW;

  if (reading != _lastReading) {
    _lastReading = reading;
    _lastChange = millis();
  }

  if ((millis() - _lastChange) >= _debounceMs && reading != _stable) {
    _stable = reading;
    if (_stable) {
      if (_onPress) _onPress();
    } else {
      if (_onRelease) _onRelease();
    }
  }
}
