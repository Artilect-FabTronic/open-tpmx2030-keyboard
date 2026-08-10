# 05 — Analyse GPIO & Extension I2C (MCP23017)

> **Retour à l'index :** [Documentation](./README.md)

---

## Table des matières

- [Problématique : Contrainte GPIO du RP2040-Zero](#problématique--contrainte-gpio-du-rp2040-zero)
- [Étude comparative des topologies hardware](#étude-comparative-des-topologies-hardware)
- [Analyse critique des idées reçues sur la latence](#analyse-critique-des-idées-reçues-sur-la-latence)
- [Choix technologique retenu](#choix-technologique-retenu)
- [Architecture hardware détaillée](#architecture-hardware-détaillée)
- [Configuration de la matrice (NKRO)](#configuration-de-la-matrice-nkro)
- [Configuration I2C et interruptions](#configuration-i2c-et-interruptions)
- [Code de balayage I2C optimisé](#code-de-balayage-i2c-optimisé)
- [Conclusion](#conclusion)

---

## Problématique : Contrainte GPIO du RP2040-Zero

Le RP2040-Zero n'expose que **20 broches GPIO** accessibles (GP0 à GP15, GP26 à GP29).

### Inventaire des GPIOs consommées

| Périphérique             | Broches utilisées  | Nombre |
| ------------------------ | ------------------ | ------ |
| 4 LEDs d'état            | GP0, GP4, GP5, GP6 | 4      |
| Encodeur 1 (CLK, DT, SW) | GP7, GP8, GP9      | 3      |
| Encodeur 2 (CLK, DT, SW) | GP10, GP11, GP12   | 3      |
| Encodeur 3 (CLK, DT, SW) | GP13, GP14, GP15   | 3      |
| **Sous-total consommé**  |                    | **13** |
| **Restant disponible**   |                    | **7**  |

### Besoin pour la matrice de touches

Une matrice directe 8 lignes × 13 colonnes nécessite au minimum :

```
8 (Lignes OUT) + 13 (Colonnes IN) = 21 GPIO
```

> **Conclusion :** Il ne reste que 7 GPIO disponibles, mais la matrice en exige 21.  
> **L'utilisation directe des broches du RP2040 est physiquement impossible.**  
> Un composant d'extension de GPIO est indispensable.

---

## Étude comparative des topologies hardware

Deux familles de solutions ont été étudiées :

### Option A — MCP23017 (Extenseur I2C)

Le **MCP23017** de Microchip est un extenseur 16 bits d'I/O sur bus I2C. Il fournit 16 broches GPIO supplémentaires via seulement 2 fils (SDA + SCL).

- 📄 [Datasheet MCP23017](https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP23017-MCP23S17-16-Bit-IO-Expander-with-Serial-Interface-DS20001952.pdf)
- 🌐 [Page produit Microchip](https://www.microchip.com/en-us/product/mcp23017)

**Caractéristiques clés :**
- Interface : I2C (jusqu'à 1.7 MHz en Fast-Mode Plus)
- 16 GPIO configurables individuellement (IN / OUT / Pull-up / Open-drain)
- **2 broches d'interruption** (`INTA` / `INTB`) pour la détection de changement d'état sans polling
- Tension : 2.7V à 5.5V (compatible 3.3V RP2040)
- Courant max par broche : 20 mA
- Boîtier : SSOP-28 ou DIP-28
- Adressage : 3 bits hardware (A0, A1, A2) → jusqu'à 8 MCP23017 sur le même bus

### Option B — Registres à décalage (SPI)

Les **registres à décalage 74HC165** (parallèle → série, lecture des colonnes) et **74HC595** (série → parallèle, pilotage des lignes) permettent une extension via SPI.

| Critère                   | MCP23017 (I2C)             | 74HC165/595 (SPI)                 |
| ------------------------- | -------------------------- | --------------------------------- |
| **GPIO RP2040 utilisées** | **3** (SDA, SCL, INT)      | **4** (SCK, MOSI, MISO, LATCH)    |
| **Vitesse bus**           | Jusqu'à 1 MHz (Fast-Mode+) | 10–20 MHz                         |
| **Temps scan matrice**    | ~250–400 µs à 1 MHz        | **~5–10 µs** à 20 MHz             |
| **Latence globale**       | < 0.5 ms                   | < 0.01 ms                         |
| **Veille / Wake-up**      | **Natif** (INTA/INTB)      | Nécessite porte logique OR dédiée |
| **Complexité PCB**        | Faible (1 IC SSOP-28)      | Moyenne (2–3 ICs en cascade)      |
| **Coût**                  | ~0.80 € pièce              | ~0.30 € pièce                     |
| **Anti-ghosting**         | Requis (diodes 1N4148)     | Requis (diodes 1N4148)            |

---

## Analyse critique des idées reçues sur la latence

### Idée reçue n°1 : « L'I2C est trop lent pour un clavier »

Cette affirmation est vraie pour l'I2C standard à 100 kHz, mais **fausse** pour le RP2040 configuré en Fast-Mode Plus :

| Mode I2C           | Fréquence | Temps lecture 1 octet | Temps scan 8 lignes |
| ------------------ | --------- | --------------------- | ------------------- |
| Standard           | 100 kHz   | ~100 µs               | ~3.2 ms ❌           |
| Fast Mode          | 400 kHz   | ~25 µs                | ~800 µs ⚠️           |
| **Fast-Mode Plus** | **1 MHz** | **~10 µs**            | **~300 µs ✅**       |

Le taux de rafraîchissement USB haute performance est de **1000 Hz (1 ms)**. Un scan de 300 µs reste **largement en dessous du seuil de latence perceptible**.

> 💡 **Conclusion :** À 1 MHz, la latence I2C est invisible à la frappe. L'argument de la lenteur I2C ne s'applique qu'aux configurations sous-optimales.

### Idée reçue n°2 : « Le SPI est toujours supérieur »

Le SPI est effectivement plus rapide (~5–10 µs de scan contre ~300 µs pour l'I2C), mais cette différence de **290 µs** est imperceptible pour l'être humain (le seuil de perception est ~10 ms).

**En revanche, le SPI présente un inconvénient majeur :** la gestion de la veille du RP2040 nécessite l'ajout d'une porte logique OR externe pour agréger les signaux de réveil des colonnes. Cela augmente la complexité du PCB et le nombre de composants.

### Idée reçue n°3 : « La veille n'est pas nécessaire pour un clavier »

Pour un clavier USB alimenté, la consommation en veille peut malgré tout être réduite en utilisant le mode `Dormant` du RP2040. Le **MCP23017** intègre nativement les broches `INTA` et `INTB` qui déclenchent une interruption matérielle dès qu'une touche est pressée, réveillant instantanément le microcontrôleur sans aucun composant supplémentaire.

---

## Choix technologique retenu

### ✅ Solution retenue : Double MCP23017 sur Bus I2C à 1 MHz

| Paramètre                  | Valeur                                                              |
| -------------------------- | ------------------------------------------------------------------- |
| **Composants**             | 2× MCP23017-E/SS (SSOP-28)                                          |
| **GPIO RP2040 consommées** | 3 (SDA, SCL, INT)                                                   |
| **Capacité totale**        | 32 E/S (16 × 2)                                                     |
| **Utilisation**            | 8 Lignes (OUT) + 13 Colonnes (IN) = 21 I/O utilisées, 11 en réserve |
| **Vitesse bus I2C**        | 1 MHz (Fast-Mode Plus)                                              |
| **Temps scan estimé**      | ~300 µs                                                             |
| **Veille/réveil**          | Natif via INTA du MCP23017                                          |

### Répartition des deux MCP23017

| MCP23017 | Adresse I2C | Port A             | Port B              | Rôle                     |
| -------- | ----------- | ------------------ | ------------------- | ------------------------ |
| **#1**   | 0x20        | L0–L7 (OUT)        | —                   | Pilotage des **Lignes**  |
| **#2**   | 0x21        | C0–C7 (IN Pull-up) | C8–C12 (IN Pull-up) | Lecture des **Colonnes** |

### Configuration des adresses I2C

Les broches A0, A1, A2 du MCP23017 définissent les 3 bits de l'adresse :

| MCP23017 | A2  | A1  | A0  | Adresse I2C |
| -------- | --- | --- | --- | ----------: |
| #1       | 0   | 0   | 0   |      `0x20` |
| #2       | 0   | 0   | 1   |      `0x21` |

---

## Architecture hardware détaillée

```
                       RP2040-ZERO
                   +-----------------+
                   | GP2 (SDA) ------+-----------> Bus I2C (SDA)
                   |                 |              Pull-up 2.2kΩ vers 3.3V
                   | GP3 (SCL) ------+-----------> Bus I2C (SCL)
                   |                 |              Pull-up 2.2kΩ vers 3.3V
                   | GP1 (INT) ------+<----------- INTA (interruption combinée)
                   +-----------------+
                            |
           +----------------+----------------+
           |  Bus I2C @ 1 MHz                |
           |                                 |
  +-----------------+               +-----------------+
  |   MCP23017 #1   |               |   MCP23017 #2   |
  |  Adresse: 0x20  |               |  Adresse: 0x21  |
  |  A0=0, A1=0, A2=0               |  A0=1, A1=0, A2=0
  +-----------------+               +-----------------+
  | GPA0  → Ligne 0 |               | GPA0 ← Colonne 0|
  | GPA1  → Ligne 1 |               | GPA1 ← Colonne 1|
  | GPA2  → Ligne 2 |               | GPA2 ← Colonne 2|
  | GPA3  → Ligne 3 |               | GPA3 ← Colonne 3|
  | GPA4  → Ligne 4 |               | GPA4 ← Colonne 4|
  | GPA5  → Ligne 5 |               | GPA5 ← Colonne 5|
  | GPA6  → Ligne 6 |               | GPA6 ← Colonne 6|
  | GPA7  → Ligne 7 |               | GPA7 ← Colonne 7|
  +-----------------+               | GPB0 ← Colonne 8|
           |                        | GPB1 ← Colonne 9|
           |                        | GPB2 ← Col. 10  |
           |                        | GPB3 ← Col. 11  |
           |                        | GPB4 ← Col. 12  |
           |                        +-----------------+
           |                                 |
           +------------ MATRICE ------------+
                    8 Lignes × 13 Colonnes
                    ~102 Switches Cherry MX2A
                    + 102 Diodes 1N4148 (SOD-123)
```

---

## Configuration de la matrice (NKRO)

### Dimensions

```
8 Lignes (Output) × 13 Colonnes (Input Pull-up) = 104 emplacements
```

### Anti-ghosting — Full N-Key Rollover

Le **N-Key Rollover (NKRO)** garantit la détection simultanée de toutes les touches pressées, sans ambiguïté. Il est implémenté grâce à une **diode par switch** :

```
Ligne (active LOW)
    │
   [SW] ──── [>|] ──── Colonne (Pull-up)
   Switch     1N4148
   Cherry MX  SOD-123
```

- **Anode** de la diode → connectée à la ligne
- **Cathode** de la diode → connectée à la colonne (avec le switch)

Sans diodes, l'appui simultané de plusieurs touches crée des courts-circuits entre colonnes, générant des touches fantômes (ghosting). Les diodes bloquent le courant inverse et isolent chaque switch.

---

## Configuration I2C et interruptions

### Résistances de pull-up I2C

Le bus I2C nécessite des résistances de pull-up sur SDA et SCL :

| Fréquence I2C | Résistance recommandée |
| ------------- | ---------------------- |
| 100 kHz       | 4.7 kΩ                 |
| 400 kHz       | 2.2 kΩ                 |
| **1 MHz**     | **2.2 kΩ** (ou 1 kΩ)   |

### Configuration de l'interruption (MCP23017)

La broche `INTA` du MCP23017 est configurée en mode **Active-Low / Open-Drain** avec mirroring (`INTB` miroir d'`INTA`) :

```cpp
// Configuration du registre IOCON (Control Register)
// Bit 6 (MIRROR) = 1 : INTA et INTB sont couplées (l'une ou l'autre déclenche l'INT)
// Bit 2 (ODR)    = 1 : Sortie Open-Drain (compatible avec la broche INPUT_PULLUP du RP2040)
// Bit 1 (INTPOL) = 0 : Interruption active LOW

Wire.beginTransmission(MCP_ADDR_ROWS);
Wire.write(0x0A); // Registre IOCON
Wire.write(0b01000100); // MIRROR=1, ODR=1, INTPOL=0
Wire.endTransmission();

// Activer les interruptions sur changement d'état sur toutes les colonnes (MCP2)
Wire.beginTransmission(MCP_ADDR_COLS);
Wire.write(0x04); // Registre GPINTENA (enable interrupt Port A)
Wire.write(0xFF); // Interruption activée sur toutes les colonnes Port A
Wire.endTransmission();
```

### Wake-up depuis le mode Dormant

```cpp
// Activer l'interruption sur la broche GP1 pour le réveil
attachInterrupt(PIN_INT_KEYBOARD, []() {
  // Routine de réveil minimale (ISR)
  // Le RP2040 sort du mode Dormant automatiquement sur front descendant de GP1
}, FALLING);

// Entrer en mode Dormant (ultra-basse consommation)
// rp2040.dormant(); // À activer dans le firmware final
```

---

## Code de balayage I2C optimisé

```cpp
#include <Wire.h>

#define MCP_ADDR_ROWS 0x20
#define MCP_ADDR_COLS 0x21

/* ─── Initialisation de la matrice ─────────────────────────────────────────── */
void setupMatrixHardware() {
  Wire.setSCL(3); // GP3
  Wire.setSDA(2); // GP2
  Wire.begin();
  Wire.setClock(1000000); // I2C Fast-Mode Plus : 1 MHz

  // MCP23017 #1 — Lignes : Port A en SORTIE (toutes HIGH = inactives)
  Wire.beginTransmission(MCP_ADDR_ROWS);
  Wire.write(0x00); // Registre IODIRA
  Wire.write(0x00); // 0x00 = toutes les broches en sortie
  Wire.endTransmission();

  // Mettre toutes les lignes à HIGH (inactif)
  Wire.beginTransmission(MCP_ADDR_ROWS);
  Wire.write(0x12); // Registre GPIOA
  Wire.write(0xFF); // Toutes lignes HIGH
  Wire.endTransmission();

  // MCP23017 #2 — Colonnes : Ports A et B en ENTRÉE avec Pull-up
  Wire.beginTransmission(MCP_ADDR_COLS);
  Wire.write(0x00); // Registre IODIRA
  Wire.write(0xFF); // Port A en entrée
  Wire.endTransmission();

  Wire.beginTransmission(MCP_ADDR_COLS);
  Wire.write(0x0C); // Registre GPPUA
  Wire.write(0xFF); // Pull-up Port A activé
  Wire.write(0xFF); // Pull-up Port B activé
  Wire.endTransmission();
}

/* ─── Balayage ultra-rapide : exécuté toutes les ~1 ms ─────────────────────── */
void scanMatrixI2C() {
  for (uint8_t row = 0; row < 8; row++) {
    
    // Activer une seule ligne (mettre à LOW, toutes les autres HIGH)
    Wire.beginTransmission(MCP_ADDR_ROWS);
    Wire.write(0x12);         // Registre GPIOA
    Wire.write(~(1 << row));  // Bit row à 0, tous les autres à 1
    Wire.endTransmission();

    // Lire l'état des 13 colonnes (Port A + Port B de MCP23017 #2)
    Wire.beginTransmission(MCP_ADDR_COLS);
    Wire.write(0x12); // Registre GPIOA (lecture séquentielle A→B)
    Wire.endTransmission();
    
    Wire.requestFrom(MCP_ADDR_COLS, 2); // 2 octets : Port A + Port B
    uint16_t colsState = Wire.read() | ((uint16_t)Wire.read() << 8);

    // Traitement anti-rebond et gestion des touches de la ligne
    processRowState(row, colsState);
  }
}
```

### Calcul du temps de scan

Avec l'I2C à 1 MHz (Fast-Mode Plus) :

| Opération                       | Nombre | Temps estimé            |
| ------------------------------- | ------ | ----------------------- |
| Écriture ligne active (1 octet) | 8 ×    | 8 × ~10 µs = **80 µs**  |
| Lecture 2 octets colonnes       | 8 ×    | 8 × ~20 µs = **160 µs** |
| Overhead I2C (start/stop/ack)   | ~16 ×  | ~60 µs                  |
| **Total estimé**                |        | **~300 µs**             |

> **300 µs << 1000 µs (période USB @ 1000 Hz)** → Aucune latence perceptible.

---

## Conclusion

La topologie basée sur **2× MCP23017 cadencés à 1 MHz** offre le meilleur compromis pour ce projet :

| Critère            | Résultat                                                       |
| ------------------ | -------------------------------------------------------------- |
| **GPIO libérées**  | 17 GPIO sur 20 disponibles pour LEDs, encodeurs, extensions    |
| **Temps de scan**  | ~300 µs (invisible à la frappe)                                |
| **Gestion veille** | Natif via INTA/INTB sans composant additionnel                 |
| **Complexité PCB** | Minimale (2 × SSOP-28, résistances pull-up, découplage)        |
| **NKRO**           | Garanti par les diodes 1N4148 sur chaque switch                |
| **Évolutivité**    | 11 GPIO MCP en réserve pour rétroéclairage, extensions futures |

---

> **Page précédente :** [04 — Architecture Firmware](./04-firmware-architecture.md)  
> **Page suivante :** [06 — Dispositions Clavier](./06-dispositions-clavier.md)
