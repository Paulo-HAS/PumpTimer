#include <Arduino.h>
// Timer para bomba d'água com ESP32
// Botão: inicia o timer; apertar de novo durante a contagem cancela.

#define BUTTON_PIN 8     // botão entre o pino e GND (usa pull-up interno)
#define RELAY_PIN 12     // sinal do módulo relé

// Módulos relé comuns são ativos em nível baixo (LOW = liga).
// Se o seu for ativo em alto, mude para false.
const bool RELAY_ACTIVE_LOW = true;

const unsigned long PUMP_DURATION_MS = 15UL * 60UL * 1000UL; // 15 minutos
const unsigned long DEBOUNCE_MS = 40;

bool pumpOn = false;
unsigned long pumpStartMs = 0;

bool lastReading = HIGH;
bool stableState = HIGH;
unsigned long lastChangeMs = 0;

void setRelay(bool on) {
  digitalWrite(RELAY_PIN, (on == RELAY_ACTIVE_LOW) ? LOW : HIGH);
}

void startPump() {
  pumpOn = true;
  pumpStartMs = millis();
  setRelay(true);
  Serial.println("Bomba LIGADA");
}

void stopPump(const char *motivo) {
  pumpOn = false;
  setRelay(false);
  Serial.print("Bomba DESLIGADA (");
  Serial.print(motivo);
  Serial.println(")");
}

void setup() {
  Serial.begin(115200);

  // Define o nível de desligado ANTES de configurar como saída,
  // para evitar pulso indesejado no relé.
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  unsigned long now = millis();

  // --- Leitura do botão com debounce ---
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != lastReading) {
    lastChangeMs = now;
    lastReading = reading;
  }

  if ((now - lastChangeMs) > DEBOUNCE_MS && reading != stableState) {
    stableState = reading;

    // Ação na borda de subida (botão pressionado)
    if (stableState == HIGH) {
      if (pumpOn) {
        stopPump("cancelado pelo botão");
      } else {
        startPump();
      }
    }
  }

  // --- Fim da contagem ---
  if (pumpOn && (now - pumpStartMs >= PUMP_DURATION_MS)) {
    stopPump("tempo esgotado");
  }
}