// Timer para bomba d'água com modo manual
// Botão TIMER : inicia o timer; apertar de novo durante a contagem cancela.
// Botão MANUAL: liga a bomba até ser apertado de novo.
// Enquanto o modo manual estiver ativo, o botão do timer é ignorado.
// Os botões enviam HIGH quando pressionados (precisam de pull-down).

#define TIMER_BUTTON_PIN  8
#define MANUAL_BUTTON_PIN 7
#define RELAY_PIN         12

// Módulos relé comuns são ativos em nível baixo (LOW = liga).
// Se for ativo em alto, mude para false.
const bool RELAY_ACTIVE_LOW = true;

const unsigned long PUMP_DURATION_MS = 30UL * 60UL * 1000UL; // 30 minutos
const unsigned long DEBOUNCE_MS = 50;

enum Mode { OFF, TIMER_MODE, MANUAL_MODE };     // Estados do sistema
Mode mode = OFF;                                // Estado atual
unsigned long timerStartMs = 0;

// Debounce genérico: detecta borda de subida (botão pressionado)
struct Button {
  uint8_t pin;
  bool lastReading;
  bool stableState;
  unsigned long lastChangeMs;
};

Button timerBtn  = {TIMER_BUTTON_PIN,  LOW, LOW, 0};
Button manualBtn = {MANUAL_BUTTON_PIN, LOW, LOW, 0};

// Retorna true uma única vez a cada aperto
bool wasPressed(Button &b) {
  unsigned long now = millis();
  bool reading = digitalRead(b.pin);

  if (reading != b.lastReading) {
    b.lastChangeMs = now;
    b.lastReading = reading;
  }

  if ((now - b.lastChangeMs) > DEBOUNCE_MS && reading != b.stableState) {
    b.stableState = reading;
    if (b.stableState == HIGH) return true;
  }
  return false;
}

void setRelay(bool on) {
  digitalWrite(RELAY_PIN, (on == RELAY_ACTIVE_LOW) ? LOW : HIGH);
}

void setMode(Mode newMode, const char *msg) {
  mode = newMode;
  setRelay(mode != OFF);
  if (mode == TIMER_MODE) timerStartMs = millis();
  Serial.println(msg);
}

void setup() {
  Serial.begin(115200);

  // Nível de desligado definido ANTES de configurar a saída
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? HIGH : LOW);
  pinMode(RELAY_PIN, OUTPUT);
  setRelay(false);

  // Pull-down externo (resistor de ~10k entre o pino e o GND)
  pinMode(TIMER_BUTTON_PIN, INPUT);
  pinMode(MANUAL_BUTTON_PIN, INPUT);
}

void loop() {
  bool timerPressed  = wasPressed(timerBtn);
  bool manualPressed = wasPressed(manualBtn);

  // --- Botão manual ---
  if (manualPressed) {
    if (mode == MANUAL_MODE) {
      setMode(OFF, "Manual: bomba DESLIGADA");
    } else {
      setMode(MANUAL_MODE, "Manual: bomba LIGADA");
    }
  }
  // --- Botão do timer (ignorado no modo manual) ---
  else if (timerPressed && mode != MANUAL_MODE) {
    if (mode == TIMER_MODE) {
      setMode(OFF, "Timer: cancelado, bomba DESLIGADA");
    } else {
      setMode(TIMER_MODE, "Timer: bomba LIGADA");
    }
  }

  // --- Fim da contagem do timer ---
  if (mode == TIMER_MODE && (millis() - timerStartMs >= PUMP_DURATION_MS)) {
    setMode(OFF, "Timer: tempo esgotado, bomba DESLIGADA");
  }
}