# 04 — Architecture Firmware

> **Retour à l'index :** [Documentation](./README.md)

---

## Table des matières

- [Choix de l'environnement de développement](#choix-de-lenvironnement-de-développement)
- [Boucle principale du firmware](#boucle-principale-du-firmware)
- [USB HID — Protocole et implémentation](#usb-hid--protocole-et-implémentation)
- [Traduction Scancodes USB HID ↔ PS/2](#traduction-scancodes-usb-hid--ps2)
- [Gestion des layouts alternatifs](#gestion-des-layouts-alternatifs)
- [Code source complet commenté](#code-source-complet-commenté)

---

## Choix de l'environnement de développement

### Arduino-Pico (Earle F. Philhower)

Le firmware est développé sous [**Arduino-Pico**](https://github.com/earlephilhower/arduino-pico) — le core Arduino non-officiel mais très complet pour le RP2040, maintenu par Earle F. Philhower.

**Pourquoi Arduino-Pico plutôt que QMK ou ZMK ?**

| Critère                    | Arduino-Pico               | QMK         | ZMK                  |
| -------------------------- | -------------------------- | ----------- | -------------------- |
| **Flexibilité totale**     | ✅ Accès complet bas niveau | ⚠️ Abstrait  | ⚠️ Abstrait           |
| **Support RP2040-Zero**    | ✅ Natif                    | ⚠️ Limité    | ❌ Peu supporté       |
| **USB HID natif**          | ✅ TinyUSB intégré          | ✅           | ✅                    |
| **Sur-mesure**             | ✅ Total                    | ⚠️ Configuré | ⚠️ Configuré          |
| **Courbe d'apprentissage** | Modérée (C++)              | Modérée     | Élevée (Zephyr RTOS) |
| **Bibliothèques Arduino**  | ✅ Compatibles              | ❌           | ❌                    |

### Installation

1. Ouvrir Arduino IDE
2. Aller dans **Fichier → Préférences → URL de gestionnaire de cartes supplémentaires**
3. Ajouter :
   ```
   https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
   ```
4. **Outils → Type de carte → Gestionnaire de cartes** → Rechercher "RP2040" → Installer

### Dépendances

```cpp
#include <Arduino.h>
#include <Wire.h>           // Bus I2C pour MCP23017
#include <Adafruit_TinyUSB.h> // Stack USB HID (TinyUSB)
```

Bibliothèque de référence :  
📦 [github.com/earlephilhower/arduino-pico/libraries/Keyboard](https://github.com/earlephilhower/arduino-pico/tree/master/libraries/Keyboard)

---

## Boucle principale du firmware

La logique du firmware suit un cycle simple et déterministe, exécuté en continu :

```
┌─────────────────────────────────────────────────────┐
│                   BOUCLE PRINCIPALE                 │
│                    (toutes ~1 ms)                   │
│                                                     │
│  1. TinyUSBDevice.task()  ← Traitement USB          │
│  2. scanMatrixI2C()       ← Lecture matrice MCP     │
│  3. readEncoders()        ← Lecture encodeurs EC11  │
│  4. updateLEDs()          ← Mise à jour LEDs        │
│  5. delay(1)              ← Stabilisation boucle    │
└─────────────────────────────────────────────────────┘
```

### Algorithme de balayage détaillé

```
Pour chaque ligne (0 → 7) :
  1. Écrire sur MCP23017 #1 : activer la ligne courante (LOW)
  2. Lire MCP23017 #2 : état des 13 colonnes (Port A + Port B)
  3. Pour chaque colonne (0 → 12) :
     a. Comparer l'état actuel avec l'état précédent
     b. Si changement détecté → appliquer anti-rebond
     c. Si touche pressée → mapper sur le scancode HID via la table active
     d. Si touche relâchée → retirer le scancode du rapport HID
  4. Désactiver la ligne (HIGH)
  5. Envoyer le rapport HID si modification
```

---

## USB HID — Protocole et implémentation

### Présentation du protocole HID

L'**USB HID** (Human Interface Device) est la norme USB permettant à un clavier de communiquer avec un hôte (ordinateur). Le clavier envoie des **rapports** (reports) à l'hôte à intervalles réguliers.

#### Rapport HID clavier standard (Boot Protocol)

```
Byte 0 : Modificateurs (Shift, Ctrl, Alt, Win...)
Byte 1 : Réservé (0x00)
Bytes 2-7 : Keycodes actifs (jusqu'à 6 touches simultanées)
```

#### Rapport HID NKRO (N-Key Rollover)

En mode NKRO, le rapport HID contient un **bitmap de 256 bits** (32 octets) représentant l'état de chaque touche possible, permettant la détection simultanée de toutes les touches.

### Taux de rapport (Polling Rate)

- **Standard USB FS :** 125 Hz (8 ms de latence max)
- **High-speed (recommandé) :** 1000 Hz (1 ms de latence max)
- **Configuration TinyUSB :**
  ```cpp
  usbHid.setPollInterval(1); // 1 ms = 1000 Hz
  ```

### Consumer Control (encodeurs)

Les fonctions volume et microphone utilisent les **Consumer Controls USB HID** (page 0x0C) :

| Fonction         | Usage ID HID | Page                  |
| ---------------- | ------------ | --------------------- |
| Volume Increment | 0x00E9       | Consumer Page (0x0C)  |
| Volume Decrement | 0x00EA       | Consumer Page (0x0C)  |
| Mute             | 0x00E2       | Consumer Page (0x0C)  |
| Mic Mute         | 0x00CF       | Telephony Page (0x0B) |

### Descripteur HID

```cpp
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
  TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2))
};
```

---

## Traduction Scancodes USB HID ↔ PS/2

### Contexte

Le TypeMatrix USB-2030-B-US-DV-102 Europe utilise des **HID Usage IDs** (scancodes USB HID). Ces codes sont convertis en **scancodes PS/2 (Set 1 Make)** via la *USB HID to PS/2 Scan Code Translation Table* de Microsoft, nécessaire pour la compatibilité avec certains systèmes et firmwares.

### Principe de la table de mapping

```cpp
// Exemple de table de correspondance HID → PS/2
// Index = HID Usage ID, Valeur = PS/2 Scan Code Set 1 Make
const uint8_t hid_to_ps2_table[] = {
  0x00, // 0x00 - No Event
  0x00, // 0x01 - Keyboard ErrorRollOver
  0x00, // 0x02 - Keyboard POSTFail
  0x00, // 0x03 - Keyboard ErrorUndefined
  0x1E, // 0x04 - A
  0x30, // 0x05 - B
  0x2E, // 0x06 - C
  // ... (102 entrées)
};
```

### Tables de mapping par layout

Le firmware maintient **plusieurs tables de scancodes** correspondant aux différents layouts supportés. La commutation entre layouts se fait dynamiquement :

```cpp
// Pointeur sur la table active
const uint8_t* activeKeymap = keymap_qwerty;

// Commutation au runtime
void switchLayout(Layout_t layout) {
  switch (layout) {
    case LAYOUT_QWERTY:  activeKeymap = keymap_qwerty;  break;
    case LAYOUT_BEPO:    activeKeymap = keymap_bepo;    break;
    case LAYOUT_DVORAK:  activeKeymap = keymap_dvorak;  break;
    case LAYOUT_COLEMAK: activeKeymap = keymap_colemak; break;
  }
  modeDvorak = (layout == LAYOUT_DVORAK);
  updateLEDs();
}
```

---

## Gestion des layouts alternatifs

### Layouts supportés

| Layout         | Description                                     | Référence                                                            |
| -------------- | ----------------------------------------------- | -------------------------------------------------------------------- |
| **QWERTY**     | Layout standard international                   | —                                                                    |
| **Bépo**       | Layout français ergonomique (équivalent Dvorak) | [bepo.fr](https://bepo.fr)                                           |
| **Dvorak**     | Layout anglais ergonomique                      | [Wikipedia Dvorak](https://fr.wikipedia.org/wiki/Disposition_Dvorak) |
| **Colemak-DH** | Layout moderne ergonomique                      | [DreymaR EPKL](https://github.com/DreymaR/BigBagKbdTrixPKL)          |

### Implémentation au niveau firmware vs OS

Deux approches existent pour implémenter les layouts alternatifs :

| Approche                          | Avantage                                | Inconvénient                          |
| --------------------------------- | --------------------------------------- | ------------------------------------- |
| **Au niveau firmware** *(retenu)* | Indépendant de l'OS, fonctionne partout | Configuration initiale plus complexe  |
| **Au niveau OS** (XKB, MSKLC…)    | Simple à déployer                       | Dépendant de la configuration de l'OS |

L'approche **firmware** a été retenue car elle rend le clavier autonome — le même layout est actif quel que soit l'ordinateur auquel il est connecté.

### Référence pour Colemak-DH

Le projet [**DreymaR's Big Bag of Keyboard Tricks (EPKL)**](https://github.com/DreymaR/BigBagKbdTrixPKL) est la référence pour l'implémentation du Colemak-DH et de ses variantes. Les fichiers de configuration se trouvent dans :
```
EPKL_Layouts_Default.ini
```

---

## Code source complet commenté

```cpp
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TinyUSB.h>

/* ============================================================================
 * CONFIGURATION ET DÉFINITION DES BROCHES (RP2040-ZERO)
 * ============================================================================
 * Le RP2040-Zero dispose de 20 GPIO accessibles (GP0 à GP15, GP26 à GP29).
 * La matrice 8×13 (102 touches) est gérée via 2 MCP23017 sur le bus I2C.
 * Voir : docs/05-analyse-gpio-extension.md pour le détail du choix.
 */

// --- 1. LEDS D'ÉTAT ---
#define PIN_LED_NUM_FN    0  // LED 1 : Lock Touches / Mode Fn (Bleue)
#define PIN_LED_CAPS      4  // LED 2 : Caps Lock
#define PIN_LED_SCROLL    5  // LED 3 : Scroll Lock
#define PIN_LED_DVORAK    6  // LED 4 : Mode Dvorak activé

// --- 2. ENCODEURS ROTATIFS (CLK, DT, SW) ---
// Encodeur 1 : Luminosité du rétroéclairage
#define ENC_LUM_CLK       7
#define ENC_LUM_DT        8
#define ENC_LUM_SW        9

// Encodeur 2 : Volume / Gain du microphone (Click = Mute Micro)
#define ENC_MIC_CLK       10
#define ENC_MIC_DT        11
#define ENC_MIC_SW        12

// Encodeur 3 : Volume sonore du système (Click = Mute Audio)
#define ENC_VOL_CLK       13
#define ENC_VOL_DT        14
#define ENC_VOL_SW        15

// --- 3. BUS I2C (MCP23017) ---
#define I2C_SDA           2   // GP2 — Bus données SDA
#define I2C_SCL           3   // GP3 — Bus horloge SCL @ 1 MHz
#define PIN_INT_KEYBOARD  1   // GP1 — Interruption wake-up matrice

// --- 4. ADRESSES MCP23017 ---
#define MCP_ADDR_ROWS     0x20  // MCP23017 #1 — Lignes (L0-L7, Port A OUT)
#define MCP_ADDR_COLS     0x21  // MCP23017 #2 — Colonnes (C0-C12, Port A+B IN)

// --- 5. DIMENSIONS MATRICE ---
const uint8_t NUM_ROWS = 8;
const uint8_t NUM_COLS = 13;

// --- 6. ÉTATS ET VARIABLES GLOBALES ---
bool modeDvorak          = false;  // Activation du layout Dvorak
bool brightnessMaxToggle = false;  // Bascule luminosité Min/Max
bool micMuted            = false;  // État Mute microphone
bool audioMuted          = false;  // État Mute audio

// Matrice d'état des touches (pour détection de changement d'état)
bool keyState[NUM_ROWS][NUM_COLS] = {false};

// Variables Anti-rebond (Debounce) pour les encodeurs
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 20; // ms

// Descripteur HID : Keyboard + Consumer Control
Adafruit_USBD_HID usbHid;
uint8_t const desc_hid_report[] = {
  TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
  TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(2))
};

/* ============================================================================
 * PROTOTYPES DES FONCTIONS
 * ============================================================================ */
void initGPIO();
void setupMatrixHardware();
void updateLEDs();
void scanMatrixI2C();
void readEncoders();
void handleEncoderClick(uint8_t encoderId);
void processRowState(uint8_t row, uint16_t colsState);

/* ============================================================================
 * SETUP — Initialisation au démarrage
 * ============================================================================ */
void setup() {
  initGPIO();
  setupMatrixHardware();

  // Initialisation USB HID (TinyUSB Core RP2040)
  usbHid.setPollInterval(1); // Polling Rate 1000 Hz (1 ms)
  usbHid.setReportDescriptor(desc_hid_report, sizeof(desc_hid_report));
  usbHid.begin();

  // Attente de la connexion USB avec l'hôte
  while (!TinyUSBDevice.mounted()) {
    delay(10);
  }
}

/* ============================================================================
 * LOOP — Boucle principale (~1 ms par itération)
 * ============================================================================ */
void loop() {
  // Traitement interne USB TinyUSB (si requis par la plateforme)
  #if TinyUSB_Need_Task
    TinyUSBDevice.task();
  #endif

  // 1. Scan de la matrice de touches via I2C (MCP23017)
  scanMatrixI2C();

  // 2. Lecture des 3 encodeurs rotatifs et leurs boutons
  readEncoders();

  // 3. Mise à jour des 4 LEDs d'état
  updateLEDs();

  delay(1); // Délai de stabilisation de la boucle principale
}

/* ============================================================================
 * INITIALISATION DES GPIO DU RP2040-ZERO
 * ============================================================================ */
void initGPIO() {
  // Configuration des LEDs d'état (sorties numériques)
  pinMode(PIN_LED_NUM_FN, OUTPUT);  digitalWrite(PIN_LED_NUM_FN, LOW);
  pinMode(PIN_LED_CAPS,   OUTPUT);  digitalWrite(PIN_LED_CAPS,   LOW);
  pinMode(PIN_LED_SCROLL, OUTPUT);  digitalWrite(PIN_LED_SCROLL, LOW);
  pinMode(PIN_LED_DVORAK, OUTPUT);  digitalWrite(PIN_LED_DVORAK, LOW);

  // Configuration des encodeurs rotatifs (entrées avec pull-up interne)
  pinMode(ENC_LUM_CLK, INPUT_PULLUP);
  pinMode(ENC_LUM_DT,  INPUT_PULLUP);
  pinMode(ENC_LUM_SW,  INPUT_PULLUP);

  pinMode(ENC_MIC_CLK, INPUT_PULLUP);
  pinMode(ENC_MIC_DT,  INPUT_PULLUP);
  pinMode(ENC_MIC_SW,  INPUT_PULLUP);

  pinMode(ENC_VOL_CLK, INPUT_PULLUP);
  pinMode(ENC_VOL_DT,  INPUT_PULLUP);
  pinMode(ENC_VOL_SW,  INPUT_PULLUP);

  // Broche d'interruption pour le réveil depuis la veille
  pinMode(PIN_INT_KEYBOARD, INPUT_PULLUP);
  // attachInterrupt(PIN_INT_KEYBOARD, wakeFromSleep, FALLING); // À activer pour la veille
}

/* ============================================================================
 * CONFIGURATION HARDWARE DE LA MATRICE (MCP23017 via I2C)
 * Voir docs/05-analyse-gpio-extension.md pour le détail de l'architecture.
 * ============================================================================ */
void setupMatrixHardware() {
  Wire.setSCL(I2C_SCL);
  Wire.setSDA(I2C_SDA);
  Wire.begin();
  Wire.setClock(1000000); // I2C Fast-Mode Plus : 1 MHz

  // Configuration MCP23017 #1 (Lignes L0-L7) : Port A en SORTIE
  Wire.beginTransmission(MCP_ADDR_ROWS);
  Wire.write(0x00); // Registre IODIRA
  Wire.write(0x00); // 0x00 = Toutes les broches en sortie
  Wire.endTransmission();

  // Configuration MCP23017 #2 (Colonnes C0-C12) : Ports A et B en ENTRÉE
  Wire.beginTransmission(MCP_ADDR_COLS);
  Wire.write(0x00); // Registre IODIRA
  Wire.write(0xFF); // 0xFF = Toutes les broches Port A en entrée
  Wire.endTransmission();

  // Activation des résistances pull-up internes sur les colonnes (MCP23017 #2)
  Wire.beginTransmission(MCP_ADDR_COLS);
  Wire.write(0x0C); // Registre GPPUA (Pull-ups Port A)
  Wire.write(0xFF); // Pull-up activé sur Port A
  Wire.write(0xFF); // Pull-up activé sur Port B
  Wire.endTransmission();
}

/* ============================================================================
 * MISE À JOUR DES LEDs D'ÉTAT
 * Les LEDs 1-3 sont contrôlées par le rapport HID de l'hôte.
 * La LED 4 (Dvorak) est contrôlée directement par le firmware.
 * ============================================================================ */
void updateLEDs() {
  // LED 1: Num Lock / Mode Fn (signal reçu de l'hôte via USB HID)
  digitalWrite(PIN_LED_NUM_FN, (keyboard_led_state & KEYBOARD_LED_NUMLOCK)   ? HIGH : LOW);
  
  // LED 2: Caps Lock (signal reçu de l'hôte)
  digitalWrite(PIN_LED_CAPS,   (keyboard_led_state & KEYBOARD_LED_CAPSLOCK)  ? HIGH : LOW);
  
  // LED 3: Scroll Lock (signal reçu de l'hôte)
  digitalWrite(PIN_LED_SCROLL, (keyboard_led_state & KEYBOARD_LED_SCROLLLOCK)? HIGH : LOW);
  
  // LED 4: Mode Dvorak (variable interne firmware — indépendant de l'hôte)
  digitalWrite(PIN_LED_DVORAK, modeDvorak ? HIGH : LOW);
}

/* ============================================================================
 * BALAYAGE DE LA MATRICE VIA I2C (MCP23017)
 * Temps de scan estimé : ~300 µs à 1 MHz → invisible à la frappe (< 1 ms USB)
 * ============================================================================ */
void scanMatrixI2C() {
  for (uint8_t row = 0; row < NUM_ROWS; row++) {
    
    // Étape 1 : Activer UNE SEULE ligne (la mettre à LOW, toutes les autres HIGH)
    Wire.beginTransmission(MCP_ADDR_ROWS);
    Wire.write(0x12);           // Registre GPIOA
    Wire.write(~(1 << row));    // Seul le bit de la ligne courante est à 0
    Wire.endTransmission();

    // Étape 2 : Lire l'état des 13 colonnes sur MCP23017 #2
    // Port A = colonnes 0 à 7, Port B = colonnes 8 à 12
    Wire.beginTransmission(MCP_ADDR_COLS);
    Wire.write(0x12); // Registre GPIOA (lecture séquentielle A puis B)
    Wire.endTransmission();
    
    Wire.requestFrom(MCP_ADDR_COLS, 2); // 2 octets : Port A + Port B
    uint16_t colsState = Wire.read() | ((uint16_t)Wire.read() << 8);

    // Étape 3 : Traiter l'état des touches de cette ligne
    processRowState(row, colsState);
  }
}

/* ============================================================================
 * TRAITEMENT DE L'ÉTAT D'UNE LIGNE
 * Détecte les changements d'état et envoie les scancodes HID.
 * ============================================================================ */
void processRowState(uint8_t row, uint16_t colsState) {
  for (uint8_t col = 0; col < NUM_COLS; col++) {
    // Un bit à 0 signifie que la touche est pressée (pull-up + actif bas)
    bool pressed = !((colsState >> col) & 0x01);

    if (pressed != keyState[row][col]) {
      keyState[row][col] = pressed;
      
      if (pressed) {
        // Mapper la position (row, col) vers le HID Usage ID via la table active
        // uint8_t hidCode = activeKeymap[row * NUM_COLS + col];
        // Traiter les touches spéciales (Dvorak toggle, Fn, etc.)
        // Envoyer le rapport HID
      } else {
        // Retirer le HID Usage ID du rapport HID
      }
    }
  }
}

/* ============================================================================
 * LECTURE DES ENCODEURS ROTATIFS (Décodage en quadrature)
 * ============================================================================ */
void readEncoders() {
  static uint8_t lastLumCLK = HIGH;
  static uint8_t lastMicCLK = HIGH;
  static uint8_t lastVolCLK = HIGH;

  // ─── Encodeur 1 : Luminosité ───────────────────────────────────────────────
  uint8_t currentLumCLK = digitalRead(ENC_LUM_CLK);
  if (currentLumCLK != lastLumCLK && currentLumCLK == LOW) {
    if (digitalRead(ENC_LUM_DT) != currentLumCLK) {
      // Rotation horaire → Augmenter luminosité (PWM ou Consumer Code)
      // setBrightness(brightness + BRIGHTNESS_STEP);
    } else {
      // Rotation antihoraire → Diminuer luminosité
      // setBrightness(brightness - BRIGHTNESS_STEP);
    }
  }
  lastLumCLK = currentLumCLK;

  // Bouton encodeur luminosité — Bascule Min/Max
  if (digitalRead(ENC_LUM_SW) == LOW) {
    handleEncoderClick(1);
    delay(150); // Anti-rebond simplifié pour bouton
  }

  // ─── Encodeur 2 : Microphone ───────────────────────────────────────────────
  uint8_t currentMicCLK = digitalRead(ENC_MIC_CLK);
  if (currentMicCLK != lastMicCLK && currentMicCLK == LOW) {
    if (digitalRead(ENC_MIC_DT) != currentMicCLK) {
      // Rotation horaire → Gain Micro +
    } else {
      // Rotation antihoraire → Gain Micro -
    }
  }
  lastMicCLK = currentMicCLK;

  // Bouton encodeur microphone — Mute/Unmute Micro
  if (digitalRead(ENC_MIC_SW) == LOW) {
    handleEncoderClick(2);
    delay(150);
  }

  // ─── Encodeur 3 : Volume ───────────────────────────────────────────────────
  uint8_t currentVolCLK = digitalRead(ENC_VOL_CLK);
  if (currentVolCLK != lastVolCLK && currentVolCLK == LOW) {
    if (digitalRead(ENC_VOL_DT) != currentVolCLK) {
      // Consumer Control HID: Volume Increment (Usage 0x00E9)
    } else {
      // Consumer Control HID: Volume Decrement (Usage 0x00EA)
    }
  }
  lastVolCLK = currentVolCLK;

  // Bouton encodeur volume — Mute/Unmute Audio
  if (digitalRead(ENC_VOL_SW) == LOW) {
    handleEncoderClick(3);
    delay(150);
  }
}

/* ============================================================================
 * GESTION DES CLICS DES ENCODEURS
 * ============================================================================ */
void handleEncoderClick(uint8_t encoderId) {
  switch (encoderId) {
    case 1:
      // Click Luminosité : Bascule instantanée Min ↔ Max
      brightnessMaxToggle = !brightnessMaxToggle;
      // Appliquer le niveau max ou min au rétroéclairage (PWM)
      // analogWrite(PIN_BACKLIGHT, brightnessMaxToggle ? 255 : 0);
      break;

    case 2:
      // Click Microphone : Mute / Unmute Micro
      micMuted = !micMuted;
      // Envoyer HID Telephony Page - Mute (0x00CF)
      // usbHid.sendReport16(2, micMuted ? HID_USAGE_TELEPHONY_MUTE : 0);
      break;

    case 3:
      // Click Volume : Mute / Unmute Audio
      audioMuted = !audioMuted;
      // Envoyer HID Consumer Page - Mute (0x00E2)
      // usbHid.sendReport16(2, audioMuted ? HID_USAGE_CONSUMER_MUTE : 0);
      break;
  }
}
```

---

> **Page précédente :** [03 — Interface Homme-Machine (IHM)](./03-interface-ihm.md)  
> **Page suivante :** [05 — Analyse GPIO & Extension I2C](./05-analyse-gpio-extension.md)
