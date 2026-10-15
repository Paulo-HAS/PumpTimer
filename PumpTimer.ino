#include <Arduino.h>

// Ajuste estes pinos conforme a sua montagem.
const uint8_t BUTTON_PIN = 19;
const uint8_t RELAY_PIN = 18;

// Edite este valor para alterar a duracao do timer.(UL = Unsigned Long)
const unsigned long TIMER_DURATION_MS = 15UL * 60UL * 1000UL;

// Muitos modulos de rele acionam com nivel LOW. Mude para false se o seu
// modulo acionar com nivel HIGH.
const bool RELAY_ACTIVE_LOW = true;

const unsigned long DEBOUNCE_MS = 40;

bool pumpRunning = false;
unsigned long pumpStartedAt = 0;

bool lastRawButtonState = HIGH;
bool stableButtonState = HIGH;
unsigned long lastButtonChangeAt = 0;

void setPump(bool enabled) {
  pumpRunning = enabled;
  const uint8_t onLevel = RELAY_ACTIVE_LOW ? LOW : HIGH;
  const uint8_t offLevel = RELAY_ACTIVE_LOW ? HIGH : LOW;
  digitalWrite(RELAY_PIN, enabled ? onLevel : offLevel);
}

void setup() {
  Serial.begin(115200);

  // Define o estado desligado antes de habilitar a saida do rele.
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  // Configura os pinos de entrada e saida.
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  setPump(false);

  Serial.println("Controlador da bomba pronto.");
}

void loop() {
  const unsigned long now = millis();
  const bool rawButtonState = digitalRead(BUTTON_PIN);

  if (rawButtonState != lastRawButtonState) {
    lastRawButtonState = rawButtonState;
    lastButtonChangeAt = now;
  }

  if ((now - lastButtonChangeAt) >= DEBOUNCE_MS &&
      rawButtonState != stableButtonState) {
    stableButtonState = rawButtonState;

    // O botao usa INPUT_PULLUP: pressionado corresponde a LOW.
    if (stableButtonState == LOW) {
      if (pumpRunning) {
        setPump(false);
        Serial.println("Bomba desligada pelo botao.");
      } else {
        pumpStartedAt = now;
        setPump(true);
        Serial.println("Bomba ligada; timer iniciado.");
      }
    }
  }

  if (pumpRunning && (now - pumpStartedAt) >= TIMER_DURATION_MS) {
    setPump(false);
    Serial.println("Timer encerrado; bomba desligada.");
  }
}