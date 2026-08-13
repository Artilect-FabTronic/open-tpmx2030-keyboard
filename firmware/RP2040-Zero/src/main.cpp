#include <Arduino.h>
#include <Adafruit_TinyUSB.h>

/* ============================================================================
 * CONFIGURATION ET DÉFINITION DES BROCHES (RP2040-ZERO)
 * ============================================================================
 * Note : Le RP2040-Zero disposant de 20 GPIO accessibles (GP0 à GP15, GP26 à GP29),
 * il est conseillé d'utiliser un extenseur de GPIO (ex: MCP23017 I2C) ou des
 * registres à décalage (74HC595 / 74HC165) pour la matrice complète 102 touches.
 */

// --- 1. LEDS D'ÉTAT ---
#define PIN_LED_NUM_FN    0  // LED 1 : Lock Touches / Mode Fn (Bleu)
#define PIN_LED_CAPS      1  // LED 2 : Caps Lock
#define PIN_LED_SCROLL    2  // LED 3 : Scroll Lock
#define PIN_LED_DVORAK    3  // LED 4 : Mode Dvorak activé

// --- 2. ENCODEURS ROTATIFS (CLK, DT, SW) ---
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

// --- 3. MATRICE DE TOUCHES ---
const uint8_t NUM_ROWS = 8;
const uint8_t NUM_COLS = 13; // Ajustable selon le multiplexage retenu
uint8_t rowPins[NUM_ROWS] = {13, 14, 15, 26, 27, 28, 29, 22};
// Pour les colonnes, nous simulons la lecture d'un registre/GPIO
bool keyState[NUM_ROWS][NUM_COLS] = {false};

// --- 4. ÉTATS ET VARIABLES GLOBALES ---
bool modeDvorak = false;
bool brightnessMaxToggle = false;
bool micMuted = false;
bool audioMuted = false;

// Variables Anti-rebond (Debounce)
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 20;

// USB HID Keyboard Report
Adafruit_USBD_HID usbHid;
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1))
};

/* ============================================================================
 * PROTOTYPES DES FONCTIONS
 * ============================================================================ */
void initGPIO();
void updateLEDs();
void scanMatrix();
void readEncoders();
void handleEncoderClick(uint8_t encoderId);

/* ============================================================================
 * SETUP & LOOP
 * ============================================================================ */
void setup() {
  initGPIO();

  // Initialisation USB HID (TinyUSB Core RP2040)
  usbHid.setPollInterval(2);
  usbHid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usbHid.begin();

  // Attente de la connexion USB
  while (!TinyUSBDevice.mounted()) {
    delay(10);
  }
}

void loop() {
  #if TinyUSB_Need_Task
    TinyUSBDevice.task();
  #endif

  // 1. Scan de la matrice de touches
  scanMatrix();

  // 2. Lecture des encodeurs rotatifs et boutons
  readEncoders();

  // 3. Mise à jour de l'affichage des LEDs
  updateLEDs();

  delay(1); // Petit délai pour stabiliser la boucle principal
}

/* ============================================================================
 * IMPLÉMENTATION DES MODULES
 * ============================================================================ */

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
    } else {
      // Diminuer luminosité
    }
  }
  lastLumCLK = currentLumCLK;

  if (digitalRead(ENC_LUM_SW) == LOW) {
    handleEncoderClick(1);
    delay(150); // Anti-rebond simplifié
  }

  // --- Encodeur 2 : Microphone ---
  uint8_t currentMicCLK = digitalRead(ENC_MIC_CLK);
  if (currentMicCLK != lastMicCLK && currentMicCLK == LOW) {
    if (digitalRead(ENC_MIC_DT) != currentMicCLK) {
      // Gain Micro +
    } else {
      // Gain Micro -
    }
  }
  lastMicCLK = currentMicCLK;

  if (digitalRead(ENC_MIC_SW) == LOW) {
    handleEncoderClick(2);
    delay(150);
  }

  // --- Encodeur 3 : Volume ---
  uint8_t currentVolCLK = digitalRead(ENC_VOL_CLK);
  if (currentVolCLK != lastVolCLK && currentVolCLK == LOW) {
    if (digitalRead(ENC_VOL_DT) != currentVolCLK) {
      // Consumer Control: Volume Increment
    } else {
      // Consumer Control: Volume Decrement
    }
  }
  lastVolCLK = currentVolCLK;

  if (digitalRead(ENC_VOL_SW) == LOW) {
    handleEncoderClick(3);
    delay(150);
  }
}

void handleEncoderClick(uint8_t encoderId) {
  switch (encoderId) {
    case 1:
      // Click Luminosité : Bascule direct Min / Max
      brightnessMaxToggle = !brightnessMaxToggle;
      // Appliquer le niveau max ou min au rétroéclairage
      break;

    case 2:
      // Click Micro : Mute / Unmute Micro
      micMuted = !micMuted;
      // Envoyer la commande HID Telephony / Mute Micro
      break;

    case 3:
      // Click Volume : Mute / Unmute Audio
      audioMuted = !audioMuted;
      // Envoyer la commande HID Consumer Control Volume Mute
      break;
  }
}

// Algorithme de Balayage

#include <Wire.h>

#define MCP_ADDR_ROWS 0x20
#define MCP_ADDR_COLS 0x21

void setupMatrixHardware() {
  Wire.setSCL(GP3);
  Wire.setSDA(GP2);
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
