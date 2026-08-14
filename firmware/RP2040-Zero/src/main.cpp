#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>

/* ============================================================================
 * CONFIGURATION ET DÉFINITION DES BROCHES (RP2040-ZERO)
 * ============================================================================
 * Le RP2040-Zero expose seulement une partie limitée de GPIO ; la matrice 102
 * touches doit donc être étendue via un expanseur I2C (MCP23017).
 */

// --- 1. LEDS D'ÉTAT ---
#define PIN_LED_NUM_FN    0  // LED 1 : Lock Touches / Mode Fn (Bleu)
#define PIN_LED_CAPS      1  // LED 2 : Caps Lock
#define PIN_LED_SCROLL    2  // LED 3 : Scroll Lock
#define PIN_LED_DVORAK    3  // LED 4 : Mode Dvorak activé

// --- 2. NEOPIXEL DE STATUT ---
#define NEOPIXEL_PIN      16
#define NEOPIXEL_COUNT    1
#define NEOPIXEL_BRIGHTNESS 32

// --- 3. ENCODEURS ROTATIFS (CLK, DT, SW) ---
// Encodeur 1 : Luminosité (Switch = Min / Max)
#define ENC_LUM_CLK       4
#define ENC_LUM_DT        5
#define ENC_LUM_SW        6

// Encodeur 2 : Micro (Switch = Mute Micro)
#define ENC_MIC_CLK       7
#define ENC_MIC_DT        8
#define ENC_MIC_SW        9

// Encodeur 3 : Volume (Switch = Mute Audio)
#define ENC_VOL_CLK       10
#define ENC_VOL_DT        11
#define ENC_VOL_SW        12

// --- 4. MATRICE DE TOUCHES ---
const uint8_t NUM_ROWS = 8;
const uint8_t NUM_COLS = 13;
uint8_t rowPins[NUM_ROWS] = {13, 14, 15, 26, 27, 28, 29, 22};
bool keyState[NUM_ROWS][NUM_COLS] = {false};

// --- 5. ÉTATS ET VARIABLES GLOBALES ---
enum class BoardPowerState : uint8_t {
  BOOT,
  USB_WAIT,
  ACTIVE,
  IDLE,
  SLEEP
};

BoardPowerState boardPowerState = BoardPowerState::BOOT;
bool modeDvorak = false;
bool brightnessMaxToggle = false;
bool micMuted = false;
bool audioMuted = false;
volatile uint8_t keyboard_led_state = 0;

volatile bool lumSwitchFlag = false;
volatile bool micSwitchFlag = false;
volatile bool volSwitchFlag = false;
volatile uint32_t lumSwitchTimestamp = 0;
volatile uint32_t micSwitchTimestamp = 0;
volatile uint32_t volSwitchTimestamp = 0;

const uint32_t debounceIntervalMs = 30;  // Variables Anti-rebond (Debounce)
const uint32_t activityTimeoutMs = 15000;
const uint32_t sleepEntryDelayMs = 30000;
unsigned long lastActivityMs = 0;

Adafruit_NeoPixel boardPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_USBD_HID usbHid;
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1))
};

#define MCP_ADDR_ROWS 0x20
#define MCP_ADDR_COLS 0x21

/* ============================================================================
 * PROTOTYPES DES FONCTIONS
 * ============================================================================ */
void initGPIO();
void initUSB();
void initEncoderInterrupts();
void updateBoardState();
void updateLEDs();
void updateNeoPixelStatus();
void scanMatrix();
void readEncoders();
void handleEncoderClick(uint8_t encoderId);
void handleLumSwitchISR();
void handleMicSwitchISR();
void handleVolSwitchISR();
void processRowState(uint8_t row, uint16_t colsState);
void setupMatrixHardware();
void scanMatrixI2C();

/* ============================================================================
 * SETUP & LOOP
 * ============================================================================ */
void setup() {
  initGPIO();
  initEncoderInterrupts();

  boardPixel.begin();
  boardPixel.setBrightness(NEOPIXEL_BRIGHTNESS);
  boardPixel.clear();
  boardPixel.show();

  initUSB();
  lastActivityMs = millis();
}

void loop() {
#if TinyUSB_Need_Task
  TinyUSBDevice.task();
#endif

  const uint32_t now = millis();

  if (lumSwitchFlag && (now - lumSwitchTimestamp) > debounceIntervalMs) {
    lumSwitchFlag = false;
    handleEncoderClick(1);
  }

  if (micSwitchFlag && (now - micSwitchTimestamp) > debounceIntervalMs) {
    micSwitchFlag = false;
    handleEncoderClick(2);
  }

  if (volSwitchFlag && (now - volSwitchTimestamp) > debounceIntervalMs) {
    volSwitchFlag = false;
    handleEncoderClick(3);
  }

  scanMatrix();
  readEncoders();
  updateLEDs();
  updateBoardState();
  updateNeoPixelStatus();

  if (boardPowerState == BoardPowerState::SLEEP) {
    sleep_ms(1);
  }
}

/* ============================================================================
 * IMPLÉMENTATION DES MODULES
 * ============================================================================ */

void initUSB() {
  usbHid.setPollInterval(2);
  usbHid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usbHid.begin();

  while (!TinyUSBDevice.mounted()) {
    delay(10);
  }

  boardPowerState = BoardPowerState::ACTIVE;
}

void initGPIO() {
  // Configuration des LEDs
  pinMode(PIN_LED_NUM_FN, OUTPUT);
  pinMode(PIN_LED_CAPS, OUTPUT);
  pinMode(PIN_LED_SCROLL, OUTPUT);
  pinMode(PIN_LED_DVORAK, OUTPUT);

  // Configuration des encodeurs rotatifs
  pinMode(ENC_LUM_CLK, INPUT_PULLUP);
  pinMode(ENC_LUM_DT, INPUT_PULLUP);
  pinMode(ENC_LUM_SW, INPUT_PULLUP);

  pinMode(ENC_MIC_CLK, INPUT_PULLUP);
  pinMode(ENC_MIC_DT, INPUT_PULLUP);
  pinMode(ENC_MIC_SW, INPUT_PULLUP);

  pinMode(ENC_VOL_CLK, INPUT_PULLUP);
  pinMode(ENC_VOL_DT, INPUT_PULLUP);
  pinMode(ENC_VOL_SW, INPUT_PULLUP);

  // Configuration de la matrice (Lignes en Sortie, Colonnes en Entrée Pull-Up)
  for (uint8_t r = 0; r < NUM_ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
}

void initEncoderInterrupts() {
  attachInterrupt(digitalPinToInterrupt(ENC_LUM_SW), handleLumSwitchISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENC_MIC_SW), handleMicSwitchISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENC_VOL_SW), handleVolSwitchISR, FALLING);
}

void updateBoardState() {
  const uint32_t now = millis();
  const bool usbConnected = TinyUSBDevice.mounted();

  if (!usbConnected) {
    boardPowerState = BoardPowerState::USB_WAIT;
    return;
  }

  if (boardPowerState == BoardPowerState::SLEEP) {
    if (now - lastActivityMs < activityTimeoutMs) {
      boardPowerState = BoardPowerState::ACTIVE;
    }
    return;
  }

  if (now - lastActivityMs > sleepEntryDelayMs) {
    boardPowerState = BoardPowerState::IDLE;
  } else {
    boardPowerState = BoardPowerState::ACTIVE;
  }
}

void updateLEDs() {
  // LED 1: Verrouillage Num / Fn
  digitalWrite(PIN_LED_NUM_FN, (keyboard_led_state & KEYBOARD_LED_NUMLOCK) ? HIGH : LOW);
  
  // LED 2: Caps Lock
  digitalWrite(PIN_LED_CAPS, (keyboard_led_state & KEYBOARD_LED_CAPSLOCK) ? HIGH : LOW);
  
  // LED 3: Scroll Lock
  digitalWrite(PIN_LED_SCROLL, (keyboard_led_state & KEYBOARD_LED_SCROLLLOCK) ? HIGH : LOW);
  
  // LED 4: Mode Dvorak
  digitalWrite(PIN_LED_DVORAK, modeDvorak ? HIGH : LOW);
}

void updateNeoPixelStatus() {
  uint32_t color = 0x000000;

  switch (boardPowerState) {
    case BoardPowerState::BOOT:
      color = boardPixel.Color(32, 32, 0);
      break;
    case BoardPowerState::USB_WAIT:
      color = boardPixel.Color(32, 0, 0);
      break;
    case BoardPowerState::ACTIVE:
      color = boardPixel.Color(0, 32, 32);
      break;
    case BoardPowerState::IDLE:
      color = boardPixel.Color(0, 18, 20);
      break;
    case BoardPowerState::SLEEP:
      color = boardPixel.Color(0, 0, 20);
      break;
  }

  if (audioMuted || micMuted) {
    color = boardPixel.Color(32, 10, 0);
  }

  boardPixel.setPixelColor(0, color);
  boardPixel.show();
}

void scanMatrix() {
  for (uint8_t r = 0; r < NUM_ROWS; r++) {
    digitalWrite(rowPins[r], LOW); // Activation de la ligne

    for (uint8_t c = 0; c < NUM_COLS; c++) {
      // Lecture de la colonne (A adapter avec MCP23017 ou GPIOs dédiés)
      bool pressed = false; // ex: digitalRead(colPins[c]) == LOW;

      if (pressed != keyState[r][c]) {
        keyState[r][c] = pressed;

        // Traitement de la touche appuyée/relâchée
        if (pressed) {
          // Traiter les touches spéciales (ex: Switch Mode Dvorak)
          // Sinon envoyer le scancode HID standard via la table de translation
          lastActivityMs = millis();
        }
      }
    }

    digitalWrite(rowPins[r], HIGH); // Désactivation de la ligne
  }
}

void readEncoders() {
  static uint8_t lastLumCLK = HIGH;
  static uint8_t lastMicCLK = HIGH;
  static uint8_t lastVolCLK = HIGH;

  // --- Encodeur 1 : Luminosité ---
  uint8_t currentLumCLK = digitalRead(ENC_LUM_CLK);
  if (currentLumCLK != lastLumCLK && currentLumCLK == LOW) {
    if (digitalRead(ENC_LUM_DT) != currentLumCLK) {
      // Augmenter luminosité (signal PWM ou Consumer Code)
      lastActivityMs = millis();
    } else {
       // Diminuer luminosité
      lastActivityMs = millis();
    }
  }
  lastLumCLK = currentLumCLK;

  const uint8_t currentMicCLK = digitalRead(ENC_MIC_CLK);
  if (currentMicCLK != lastMicCLK && currentMicCLK == LOW) {
    lastActivityMs = millis();
  }
  lastMicCLK = currentMicCLK;

  const uint8_t currentVolCLK = digitalRead(ENC_VOL_CLK);
  if (currentVolCLK != lastVolCLK && currentVolCLK == LOW) {
    lastActivityMs = millis();
  }
  lastVolCLK = currentVolCLK;
}

void handleEncoderClick(uint8_t encoderId) {
  switch (encoderId) {
    case 1:
      // Click Luminosité : Bascule direct Min / Max
      brightnessMaxToggle = !brightnessMaxToggle;
      // Appliquer le niveau max ou min au rétroéclairage
      lastActivityMs = millis();
      break;

    case 2:
      // Click Micro : Mute / Unmute Micro
      micMuted = !micMuted;
      // Envoyer la commande HID Telephony / Mute Micro
      lastActivityMs = millis();
      break;

    case 3:
      // Click Volume : Mute / Unmute Audio
      audioMuted = !audioMuted;
      // Envoyer la commande HID Consumer Control Volume Mute
      lastActivityMs = millis();
      break;
  }
}

void handleLumSwitchISR() {
  if ((millis() - lumSwitchTimestamp) < debounceIntervalMs) {
    return;
  }

  lumSwitchTimestamp = millis();
  lumSwitchFlag = true;
}

void handleMicSwitchISR() {
  if ((millis() - micSwitchTimestamp) < debounceIntervalMs) {
    return;
  }

  micSwitchTimestamp = millis();
  micSwitchFlag = true;
}

void handleVolSwitchISR() {
  if ((millis() - volSwitchTimestamp) < debounceIntervalMs) {
    return;
  }

  volSwitchTimestamp = millis();
  volSwitchFlag = true;
}

void processRowState(uint8_t row, uint16_t colsState) {
  (void)row;
  (void)colsState;
}

void setupMatrixHardware() {
  Wire.setSCL(3);
  Wire.setSDA(2);
  Wire.begin();
  Wire.setClock(1000000); // Horloge I2C à 1 MHz (Fast-Mode Plus)

  // Configuration MCP1 (Lignes) : Toutes en sortie
  Wire.beginTransmission(MCP_ADDR_ROWS);
  Wire.write(0x00); // Register IODIRA
  Wire.write(0x00); // Port A en Sortie
  Wire.endTransmission();

  // Configuration MCP2 (Colonnes) : Toutes en entrée avec Pull-up
  Wire.beginTransmission(MCP_ADDR_COLS);
  Wire.write(0x00); // IODIRA
  Wire.write(0xFF); // Port A en Entrée
  Wire.write(0xFF); // Port B en Entrée
  Wire.endTransmission();

  Wire.beginTransmission(MCP_ADDR_COLS);
  Wire.write(0x0C); // GPPUA (Pull-ups Port A)
  Wire.write(0xFF); // Active Pull-up
  Wire.write(0xFF); // Active Pull-up Port B
  Wire.endTransmission();
}

// Balayage ultra-rapide exécuté toutes les 1ms
void scanMatrixI2C() {
  for (uint8_t row = 0; row < 8; row++) {
    // Activer une seule ligne (Mettre à LOW)
    Wire.beginTransmission(MCP_ADDR_ROWS);
    Wire.write(0x12); // GPIOA
    Wire.write(~(1 << row));
    Wire.endTransmission();

    // Lire l'état des 13 colonnes sur MCP2
    Wire.beginTransmission(MCP_ADDR_COLS);
    Wire.write(0x12); // GPIOA
    Wire.endTransmission();

    Wire.requestFrom(MCP_ADDR_COLS, 2); // Lire Port A et Port B
    uint16_t colsState = Wire.read() | (Wire.read() << 8);

    // Traitement anti-rebond et enregistrement de l'état des 13 touches de la ligne
    processRowState(row, colsState);
  }
}
