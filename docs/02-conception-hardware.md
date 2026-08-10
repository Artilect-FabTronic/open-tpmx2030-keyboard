# 02 — Conception Hardware & PCB

> **Retour à l'index :** [Documentation](./README.md)

---

## Table des matières

- [Microcontrôleur — RP2040-Zero](#microcontrôleur--rp2040-zero)
- [Matrice de touches](#matrice-de-touches)
- [Switches Cherry MX2A](#switches-cherry-mx2a)
- [Assignation finale des broches GPIO](#assignation-finale-des-broches-gpio)
- [Architecture hardware globale](#architecture-hardware-globale)
- [Nomenclature BOM](#nomenclature-bom)
- [Outils de conception PCB](#outils-de-conception-pcb)

---

## Microcontrôleur — RP2040-Zero

### Pourquoi le RP2040 ?

Le **RP2040** de Raspberry Pi a été retenu comme cœur du projet pour les raisons suivantes :

- **Dual-core ARM Cortex-M0+** cadencé à 133 MHz — puissance largement suffisante pour la gestion d'un clavier complexe.
- **Support natif USB HID** : pas de circuit USB externe requis, le RP2040 gère lui-même le protocole USB Full Speed.
- **Écosystème open source vaste** : SDK C/C++, MicroPython, CircuitPython, Arduino-Pico.
- **Faible coût** (~3-5 € l'unité en lot).
- **Format castellated** du RP2040-Zero : permet de souder directement la carte sur le PCB principal.

### Caractéristiques techniques du RP2040-Zero (Waveshare)

| Caractéristique      | Valeur                                               |
| -------------------- | ---------------------------------------------------- |
| **Processeur**       | ARM Cortex-M0+ dual-core @ 133 MHz                   |
| **SRAM**             | 264 Ko                                               |
| **Mémoire Flash**    | 2 Mo (W25Q16 SOIC-8)                                 |
| **GPIO accessibles** | 20 broches (GP0–GP15, GP26–GP29)                     |
| **Connectique**      | USB-C                                                |
| **Spécificité**      | Format ultra-compact, castellated (soudable sur PCB) |
| **LED RGB intégrée** | WS2812B sur GPIO 16                                  |

### Ressources

- 📄 [Datasheet RP2040](https://pip-assets.raspberrypi.com/categories/814-rp2040/documents/RP-008371-DS-1-rp2040-datasheet.pdf)
- 📄 [Hardware Design with RP2040](https://pip-assets.raspberrypi.com/categories/814-rp2040/documents/RP-008279-DS-1-hardware-design-with-rp2040.pdf)
- 🌐 [Wiki Waveshare RP2040-Zero](https://www.waveshare.com/wiki/RP2040-Zero)
- 🔌 [Pinout TinyGo Reference](https://tinygo.org/docs/reference/microcontrollers/machine/waveshare-rp2040-zero/)

### Pinout et schématique de la carte RP2040-Zero

![Waveshare RP2040-Zero Pinout](./RP2040-Zero-Board/waveshare-rp2040-zero-pinout.jpg)

![Waveshare RP2040-Zero Schéma](./RP2040-Zero-Board/waveshare-rp2040-zero-schematic.jpg)

### Options de programmation

| Environnement                      | Langage | Remarques                                 |
| ---------------------------------- | ------- | ----------------------------------------- |
| **Arduino-Pico** (Earle Philhower) | C++     | Retenu pour ce projet — support HID natif |
| SDK officiel Raspberry Pi          | C/C++   | Plus bas niveau, plus de contrôle         |
| MicroPython                        | Python  | Moins adapté pour HID temps-réel          |
| CircuitPython (Adafruit)           | Python  | Bonne bibliothèque HID                    |

**Pour ajouter le support RP2040 dans Arduino IDE**

Additional Boards Manager URLs : `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`

---

## Matrice de touches

### Principe de la matrice

La matrice de touches est la solution standard pour connecter un grand nombre de touches avec un minimum de broches. Les touches sont organisées en **lignes** et **colonnes** ([vidéo](https://www.youtube.com/watch?v=dop0SoD2XL8)) :

- Une seule **ligne** est activée à la fois (mise à LOW).
- Toutes les **colonnes** sont lues simultanément.
- Une **diode par touche** (1N4148) évite le phénomène de *ghosting* (touches fantômes) et permet le **N-Key Rollover (NKRO)** — détection simultanée de toutes les touches.

### Dimensions de la matrice

Pour 102 touches, la topologie retenue est :

```
8 Lignes (Output) × 13 Colonnes (Input Pull-up) = 104 emplacements
```

> Les 2 emplacements non utilisés peuvent être réservés pour des touches supplémentaires futures.

### Diodes Anti-Ghosting

- **Référence :** 1N4148 (boîtier SOD-123 pour montage CMS)
- **Câblage :** Anode → Ligne, Cathode → Colonne (via le switch)
- **Effet :** Full N-Key Rollover — appui simultané de toutes les touches sans ambiguïté.

```
    Ligne (LOW)
        |
       [A]→[K]→[1N4148]→ Colonne (IN Pull-up)
       [A] = Anode diode
       [K] = Cathode diode
       [1N4148] = Diode
```

### Pourquoi pas de GPIO directs sur le RP2040 ?

Le RP2040-Zero n'expose que **20 broches GPIO**. Avec les périphériques déjà connectés (LEDs, encodeurs), il ne reste que **7 GPIO libres**, insuffisants pour piloter 21 lignes/colonnes. L'extension via **MCP23017** (I2C) est donc indispensable.

> Pour l'analyse complète de ce choix, voir : [05 — Analyse GPIO & Extension I2C](./05-analyse-gpio-extension.md)

---

## Switches Cherry MX2A

### Version V1 — Brown (Tactile)

- **Référence :** [Cherry MX2A Brown](https://www.cherry.de/en-gb/product/mx2a-brown)
- **Type :** Tactile, pré-lubrifié d'usine
- **Caractéristiques :** Frappe fluide et relativement silencieuse, retour tactile subtil
- **Usage recommandé :** Environnements de bureau, frappe intensive

### Version V2 — Blue (Clicky)

- **Référence :** [Cherry MX2A Blue](https://www.cherry.de/en-gb/product/mx2a-blue)
- **Type :** Clicky, retour tactile accentué avec clic sonore
- **Usage recommandé :** Utilisateurs appréciant le feedback sonore

### Sockets Hotswap

Recherche en cours d'un support de touche **soudable sur PCB** permettant l'échange des switches Cherry MX2A sans dessouder.

- Compatibilité : Cherry MX (standard 5mm stem spacing)
- Références à valider : Kailh PCB Socket, Mill-Max 0305 / 7305

> **Fournisseurs :** [RS-Online — Interrupteurs clavier](https://fr.rs-online.com/web/p/interrupteurs-de-clavier/0664569) | [Mouser — Cherry Electrical](https://www.mouser.fr/fr/manufacturer/cherry-electrical/)

---

## Assignation finale des broches GPIO

Cette table intègre l'ensemble des périphériques connectés au RP2040-Zero :

| Fonction               | Pin RP2040-Zero        | Direction  | Rôle                                   |
| ---------------------- | ---------------------- | ---------- | -------------------------------------- |
| **I2C SDA**            | GP2                    | Bidir      | Bus données I2C (MCP23017 #1 & #2)     |
| **I2C SCL**            | GP3                    | OUT        | Bus horloge I2C @ 1 MHz                |
| **INT_KEYBOARD**       | GP1                    | IN Pull-up | Interruption matrice / Wake-Up sommeil |
| **LED 1 (Num/Fn)**     | GP0                    | OUT        | LED état — Couleur bleue               |
| **LED 2 (Caps)**       | GP4                    | OUT        | LED Caps Lock                          |
| **LED 3 (Scroll)**     | GP5                    | OUT        | LED Scroll Lock                        |
| **LED 4 (Dvorak)**     | GP6                    | OUT        | LED Mode Dvorak                        |
| **Enc 1 CLK (Lum)**    | GP7                    | IN Pull-up | Luminosité — Signal horloge            |
| **Enc 1 DT (Lum)**     | GP8                    | IN Pull-up | Luminosité — Signal direction          |
| **Enc 1 SW (Lum)**     | GP9                    | IN Pull-up | Luminosité — Bouton poussoir           |
| **Enc 2 CLK (Mic)**    | GP10                   | IN Pull-up | Microphone — Signal horloge            |
| **Enc 2 DT (Mic)**     | GP11                   | IN Pull-up | Microphone — Signal direction          |
| **Enc 2 SW (Mic)**     | GP12                   | IN Pull-up | Microphone — Bouton poussoir           |
| **Enc 3 CLK (Vol)**    | GP13                   | IN Pull-up | Volume — Signal horloge                |
| **Enc 3 DT (Vol)**     | GP14                   | IN Pull-up | Volume — Signal direction              |
| **Enc 3 SW (Vol)**     | GP15                   | IN Pull-up | Volume — Bouton poussoir               |
| **LED RGB (NeoPixel)** | GP16                   | OUT        | LED RGB intégrée (WS2812B)             |
| **Disponibles**        | GP26, GP27, GP28, GP29 | Libre      | PWM rétroéclairage, extensions futures |

### Bilan GPIO

| Catégorie            | GPIOs utilisées   |
| -------------------- | ----------------- |
| Bus I2C (SDA + SCL)  | 2                 |
| Interruption Wake-up | 1                 |
| LEDs d'état          | 4                 |
| Encodeurs (3 × 3)    | 9                 |
| **Total utilisé**    | **16 / 20**       |
| **Libres**           | **4 (GP26–GP29)** |

---

## Architecture hardware globale

```
                       RP2040-ZERO
                   +-----------------+
                   | GP2 (SDA) ------+----------> Bus I2C (SDA) + Pull-up 2.2kΩ
                   | GP3 (SCL) ------+----------> Bus I2C (SCL) + Pull-up 2.2kΩ
                   | GP1 (INT_WAKE) -+<---------- INTA (Combined Interrupt)
                   | GP0  (LED Num) -+----------> LED SMD 0805 Bleue
                   | GP4  (LED Caps)-+----------> LED SMD 0805
                   | GP5  (LED Scrl)-+----------> LED SMD 0805
                   | GP6  (LED Dvk) -+----------> LED SMD 0805
                   | GP7-9  (Enc 1) -+----------> Encodeur Luminosité EC11
                   | GP10-12 (Enc 2)-+----------> Encodeur Microphone EC11
                   | GP13-15 (Enc 3)-+----------> Encodeur Volume EC11
                   +-----------------+
                            |
           +----------------+----------------+
           |                                 |
  +-----------------+               +-----------------+
  |   MCP23017 #1   |               |   MCP23017 #2   |
  |  Adresse: 0x20  |               |  Adresse: 0x21  |
  +-----------------+               +-----------------+
  | Port A: Lignes  |               | Port A: Col 0-7 |
  | (L0 - L7) OUT   |               | Port B: Col 8-12|
  +-----------------+               +-----------------+
           |                                 |
           +------------ Matrice ------------+
                    (102 Switches + 1N4148)
                    8 Lignes × 13 Colonnes
```

---

## Nomenclature BOM

*Bill of Materials — Nomenclature préliminaire du projet*

| Désignation            | Référence / Modèle     | Qté   | Boîtier    | Remarques                                   |
| ---------------------- | ---------------------- | ----- | ---------- | ------------------------------------------- |
| **Microcontrôleur**    | Waveshare RP2040-Zero  | 1     | Module     | Format castellated, USB-C intégré           |
| **Extenseur GPIO**     | MCP23017-E/SS          | 2     | SSOP-28    | Bus I2C, adresses 0x20 et 0x21              |
| **Mémoire Flash**      | W25Q16 / W25Q32        | 1     | SOIC-8     | Embarquée sur le RP2040-Zero                |
| **Switches Mec.**      | Cherry MX2A Brown (V1) | ~102  | Traversant | V1 — Tactile                                |
| **Switches Mec.**      | Cherry MX2A Blue (V2)  | ~102  | Traversant | V2 — Clicky                                 |
| **Diodes Anti-Ghost.** | 1N4148W                | ~102  | SOD-123    | 1 par switch — NKRO                         |
| **Encodeurs Rotatifs** | EC11 avec switch       | 3     | Traversant | Luminosité, Micro, Volume                   |
| **LED Signalisation**  | LED SMD 0805 Bleue     | 1     | 0805       | Num/Fn Lock                                 |
| **LED Signalisation**  | LED SMD 0805           | 3     | 0805       | Caps, Scroll, Dvorak                        |
| **Connecteur USB**     | USB Type-C Réceptacle  | 1     | SMD        | Alimentation & Data HID                     |
| **Résistances I2C**    | 2.2kΩ                  | 2     | 0402       | Pull-up SDA et SCL                          |
| **Condensateurs**      | 100nF                  | ~10   | 0402       | Découplage alimentation                     |
| **Condensateurs**      | 10µF                   | 2     | 0805       | Filtrage alimentation principale            |
| **Keycaps Set**        | Custom Ortholinear     | 1 set | —          | Marquages Bépo/Dvorak/QWERTY (ThockFactory) |
| **Sockets Hotswap**    | Kailh PCB Socket (TBC) | ~102  | SMD        | À confirmer — compatibilité Cherry MX       |

---

## Outils de conception PCB

### KiCad

Le PCB est conçu sous [**KiCad**](https://www.kicad.org/) (version 10+), logiciel EDA open source.

- **Schématique (Schéma électrique)** : KiCad Schematic Editor (`.kicad_sch`)
- **PCB Layout** : KiCad PCB Editor (`.kicad_pcb`)
- **Empreintes Cherry MX** : disponibles dans les bibliothèques officielles KiCad ou via [github.com/blakesmith/embedded](https://github.com/blakesmith/embedded/tree/master/keebee/hardware)

```
Empreinte 3D switch Cherry MX :
${KISYS3DMOD}/Buttons_Switches_Keyboard.3dshapes/SW_Cherry_MX1A_1.00u_Plate.wrl
```

### Ergogen (Layout PCB automatisé)

[**Ergogen**](https://ergogen.xyz/new) est un outil web permettant de générer automatiquement la disposition physique des touches et le PCB à partir d'un fichier de configuration YAML. Particulièrement adapté aux claviers ortholinéaires.

---

> **Page précédente :** [01 — Présentation du projet](./01-presentation-projet.md)  
> **Page suivante :** [03 — Interface Homme-Machine (IHM)](./03-interface-ihm.md)
