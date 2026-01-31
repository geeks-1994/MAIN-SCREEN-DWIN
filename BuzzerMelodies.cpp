#include "BuzzerMelodies.h"

BuzzerMelodies::BuzzerMelodies() {
}

void BuzzerMelodies::begin() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

// Función interna que genera la onda cuadrada manualmente
void BuzzerMelodies::playRaw(uint16_t frequency, uint16_t durationMs) {
    if (frequency == 0) {
        delay(durationMs);
        return;
    }

    // Periodo de la onda en microsegundos
    // periodo = 1 / f  -> en us = 1,000,000 / f
    unsigned long periodUs = 1000000UL / frequency;
    unsigned long halfPeriodUs = periodUs / 2;

    // Número de ciclos = tiempo_total / periodo
    unsigned long totalTimeUs = (unsigned long)durationMs * 1000UL;
    unsigned long cycles = totalTimeUs / periodUs;

    for (unsigned long i = 0; i < cycles; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(halfPeriodUs);
        digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(halfPeriodUs);
    }
}

void BuzzerMelodies::playTone(uint16_t frequency, uint16_t durationMs) {
    playRaw(frequency, durationMs);
    delay(20); // pequeña pausa entre notas
}

// ==========================
//   Melodía de inicio
// ==========================
void BuzzerMelodies::playStartup() {
    // Melodía ORIGINAL estilo juego retro / plataforma
    // Saltarina, alegre, pensada para buzzer pasivo
    // =========================
    //  G-Station Modern Startup
    //  (Original, elegante y moderno)
    // =========================

    // Suave fade-in ascendente
    playTone(440, 120);    // A4
    playTone(523, 120);    // C5
    playTone(587, 150);    // D5

    // Transición brillante
    playTone(784, 180);    // G5
    playTone(988, 200);    // B5

    // Firma moderna (tono agudo fuerte)
    playTone(1318, 230);   // E6

    // Cierre suave y elegante
    playTone(659, 220);    // E5
};

// ==========================
//   Tono fuerte de error
// ==========================
void BuzzerMelodies::playError() {
    // ================================
    // ⚠️ ERROR INDUSTRIAL ULTRA FUERTE
    // ================================
    // Pausa corta
    delay(80);

    // Doble pulso agudo intenso (alerta real)
    playTone(1500, 120);
    delay(40);
    playTone(1500, 200);

    // Remate grave vibrante
    playTone(180, 250);
};



void BuzzerMelodies::playOk() {
     // ======================================
    //  OK FUERTE - Confirmación Industrial
    //  Sonido moderno, potente y positivo
    // ======================================

    // Golpe inicial brillante
    playTone(880, 100);    // A5

    // Ascenso marcado
    playTone(1200, 120);   // Fuerte

    // Firma corta potente
    playTone(1600, 180);   // Muy fuerte y claro
}

void BuzzerMelodies::dwinImpactTone(){
     // =============================
    //  TONO IMPACTO TIPO DISPARO
    //  "POP" + "CRACK" super rápido
    // =============================

    // POP grave y muy corto
    playTone(150, 40);   // Explosión inicial

    // CRACK agudo
    playTone(1800, 30);  // Pico agudo corto

    // Eco corto (simulado)
    playTone(600, 50);

};

