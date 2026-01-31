#pragma once
#include <Arduino.h>

// =========================
// CONFIGURACIÓN DE PIN
// =========================
#define BUZZER_PIN  18   // 🔴 CAMBIA ESTE GPIO AL QUE REALMENTE USAS
// =========================

class BuzzerMelodies {
public:
    BuzzerMelodies();

    // Inicialización del pin
    void begin();

    // Tono genérico
    void playTone(uint16_t frequency, uint16_t durationMs);

    // Melodía de inicio (tipo “inicio Windows”)
    void playStartup();

    // Tono de error fuerte
    void playError();

    void  playOk();

  void dwinImpactTone();

private:
    void playRaw(uint16_t frequency, uint16_t durationMs);
};
