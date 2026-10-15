// Timer para bomba d'água com modo manual
// Botão TIMER : inicia o timer; apertar de novo durante a contagem cancela.
// Botão MANUAL: liga a bomba até ser apertado de novo.
// Enquanto o modo manual estiver ativo, o botão do timer é ignorado.
// Os botões enviam HIGH quando pressionados (precisam de pull-down).

#define TIMER_BUTTON_PIN  8
#define MANUAL_BUTTON_PIN 7
#define RELAY_PIN         12

// Módulos relé comuns são ativos em nível baixo (LOW = liga).
// Se o seu for ativo em alto, mude para false.
const bool RELAY_ACTIVE_LOW = true;

const unsigned long PUMP_DURATION_MS = 30UL * 60UL * 1000UL; // 30 minutos
const unsigned long DEBOUNCE_MS = 50;

// Modos: 0 = desligado, 1 = timer, 2 = manual
#define MODE_OFF    0
#define MODE_TIMER  1
#define MODE_MANUAL 2

int mode = MODE_OFF;
unsigned long timerStartMs = 0;

// Botões: índice 0 = timer, índice 1 = manual
const int NUM_BUTTONS = 2;
const int BTN_TIMER = 0;
const int BTN_MANUAL = 1;

const int buttonPins[NUM_BUTTONS] = {TIMER_BUTTON_PIN, MANUAL_BUTTON_PIN};
bool lastReading[NUM_BUTTONS] = {LOW, LOW};
bool stableState[NUM_BUTTONS] = {LOW, LOW};
unsigned long lastChangeMs[NUM_BUTTONS] = {0, 0};

// Retorna true uma única vez a cada aperto
bool wasPressed(int i) {
  unsigned long now = millis();
  bool reading = digitalRead(buttonPins[i]);

  if (reading != lastReading[i]) {
    lastChangeMs[i] = now;
    lastReading[i] = reading;
  }

  if ((now - lastChangeMs[i]) > DEBOUNCE_MS && reading != stableState[i]) {
    stableState[i] = reading;
    if (stableState[i] == HIGH) return true;
  }
  return false;
}

void setRelay(bool on) {
  digitalWrite(RELAY_PIN, (on == RELAY_ACTIVE_LOW) ? LOW : HIGH);
}

void setMode(int newMode, const char *msg) {
  mode = newMode;
  setRelay(mode != MODE_OFF);
  if (mode == MODE_TIMER) timerStartMs = millis();
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
  bool timerPressed  = wasPressed(BTN_TIMER);
  bool manualPressed = wasPressed(BTN_MANUAL);

  // --- Botão manual ---
  if (manualPressed) {
    if (mode == MODE_MANUAL) {
      setMode(MODE_OFF, "Manual: bomba DESLIGADA");
    } else {
      setMode(MODE_MANUAL, "Manual: bomba LIGADA");
    }
  }
  // --- Botão do timer (ignorado no modo manual) ---
  else if (timerPressed && mode != MODE_MANUAL) {
    if (mode == MODE_TIMER) {
      setMode(MODE_OFF, "Timer: cancelado, bomba DESLIGADA");
    } else {
      setMode(MODE_TIMER, "Timer: bomba LIGADA");
    }
  }

  // --- Fim da contagem do timer ---
  if (mode == MODE_TIMER && (millis() - timerStartMs >= PUMP_DURATION_MS)) {
    setMode(MODE_OFF, "Timer: tempo esgotado, bomba DESLIGADA");
  }
}