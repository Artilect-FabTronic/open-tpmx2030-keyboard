# 03 — Interface Homme-Machine (IHM)

> **Retour à l'index :** [Documentation](./README.md)

---

## Table des matières

- [Vue d'ensemble de l'IHM](#vue-densemble-de-lihm)
- [Encodeurs rotatifs EC11](#encodeurs-rotatifs-ec11)
- [LEDs d'état](#leds-détat)
- [Keycaps personnalisées](#keycaps-personnalisées)
- [Switches — Sockets Hotswap](#switches--sockets-hotswap)

---

## Vue d'ensemble de l'IHM

L'interface physique du clavier Open-TPMX2030 va au-delà de la simple frappe de touches. Elle intègre :

```plaintext
┌──────────────────────────────────────────────────────────────┐
│  [Enc 1: Luminosité]  [Enc 2: Micro]  [Enc 3: Volume]        │  ← Barre supérieure
│         [LED Num]  [LED Caps]  [LED Scroll]  [LED Dvorak]    │  ← Centre supérieur
├──────────────────────────────────────────────────────────────┤
│                                                              │
│         Matrice 8×13 — 102 touches Cherry MX2A               │  ← Zone de frappe
│         (Layout orthogonal — Type TypeMatrix)                │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

---

## Encodeurs rotatifs EC11

### Présentation

Les **encodeurs rotatifs EC11** sont des potentiomètres incrémentaux sans fin. Ils produisent deux signaux en quadrature (CLK et DT) permettant de détecter la direction et la vitesse de rotation. Chaque encodeur intègre également un **bouton poussoir** (switch) activé par pression axiale.

### Encodeur 1 — Luminosité du rétroéclairage

| Paramètre                | Valeur                           |
| ------------------------ | -------------------------------- |
| **Broches**              | GP7 (CLK), GP8 (DT), GP9 (SW)    |
| **Rotation horaire**     | Augmente la luminosité           |
| **Rotation antihoraire** | Diminue la luminosité            |
| **Clic (pression)**      | **Bascule instantané Min ↔ Max** |
| **Signal HID**           | Consumer Control / PWM interne   |

> Le clic de l'encodeur de luminosité permet de passer instantanément de l'éclairage minimal à maximal (ou inversement), pratique pour alterner rapidement entre environnements lumineux et sombres.

### Encodeur 2 — Gain du microphone

| Paramètre                | Valeur                               |
| ------------------------ | ------------------------------------ |
| **Broches**              | GP10 (CLK), GP11 (DT), GP12 (SW)     |
| **Rotation horaire**     | Augmente le gain micro               |
| **Rotation antihoraire** | Diminue le gain micro                |
| **Clic (pression)**      | **Mute / Unmute microphone**         |
| **Signal HID**           | USB HID Telephony Page (Usage: Mute) |

> Fonction particulièrement utile en télétravail ou en visioconférence. Le clic coupe immédiatement le micro sans quitter l'application active.

### Encodeur 3 — Volume audio du système

| Paramètre                | Valeur                                            |
| ------------------------ | ------------------------------------------------- |
| **Broches**              | GP13 (CLK), GP14 (DT), GP15 (SW)                  |
| **Rotation horaire**     | Augmente le volume                                |
| **Rotation antihoraire** | Diminue le volume                                 |
| **Clic (pression)**      | **Mute / Unmute audio**                           |
| **Signal HID**           | USB Consumer Control (Usage: Volume Up/Down/Mute) |

### Fonctionnement électrique — Décodage en quadrature

Les encodeurs EC11 produisent deux signaux décalés de 90° en phase :

```plaintext
Rotation horaire (+) :
  CLK : ‾‾|__|‾‾|__|‾‾
  DT  : _|‾‾|__|‾‾|__|‾

Rotation antihoraire (-) :
  CLK : ‾‾|__|‾‾|__|‾‾
  DT  : ‾|__|‾‾|__|‾‾|
```

Le firmware compare le front descendant de CLK avec l'état de DT pour déterminer la direction :

- `DT != CLK` au moment du front → **Rotation +**
- `DT == CLK` au moment du front → **Rotation −**

### Paramétrage anti-rebond

Les encodeurs mécaniques peuvent générer des faux signaux lors des transitions. Une constante de debounce est appliquée :

```cpp
const unsigned long debounceDelay = 20; // ms
```

Pour les boutons poussoirs des encodeurs, un délai simplifié de **150 ms** est appliqué après chaque clic détecté.

---

## LEDs d'état

### Disposition physique

Les 4 LEDs sont placées **au centre supérieur** du clavier, dans cet ordre de gauche à droite :

```plaintext
[ Num/Fn ]  [ Caps Lock ]  [ Scroll Lock ]  [ Dvorak ]
```

### Détail de chaque LED

#### LED 1 — Num Lock / Mode Fn (Bleue)

| Paramètre              | Valeur                                            |
| ---------------------- | ------------------------------------------------- |
| **Broche**             | GP0                                               |
| **Couleur**            | Bleue (SMD 0805)                                  |
| **Signal de contrôle** | `KEYBOARD_LED_NUMLOCK` (rapport HID hôte)         |
| **État ON**            | Pavé numérique verrouillé **OU** couche Fn active |
| **État OFF**           | Mode alphanumérique standard                      |

> La LED bleue signale également l'activation de la couche Fn (touches de fonctions spéciales en bleu sur les keycaps).

#### LED 2 — Caps Lock

| Paramètre              | Valeur                                     |
| ---------------------- | ------------------------------------------ |
| **Broche**             | GP4                                        |
| **Couleur**            | Blanc ou ambre (SMD 0805)                  |
| **Signal de contrôle** | `KEYBOARD_LED_CAPSLOCK` (rapport HID hôte) |
| **État ON**            | Majuscule verrouillée                      |
| **État OFF**           | Mode minuscule normal                      |

#### LED 3 — Scroll Lock

| Paramètre              | Valeur                                       |
| ---------------------- | -------------------------------------------- |
| **Broche**             | GP5                                          |
| **Couleur**            | Vert ou blanc (SMD 0805)                     |
| **Signal de contrôle** | `KEYBOARD_LED_SCROLLLOCK` (rapport HID hôte) |
| **État ON**            | Défilement verrouillé                        |
| **État OFF**           | Défilement normal                            |

#### LED 4 — Mode Dvorak

| Paramètre              | Valeur                                   |
| ---------------------- | ---------------------------------------- |
| **Broche**             | GP6                                      |
| **Couleur**            | Rouge ou ambre (SMD 0805)                |
| **Signal de contrôle** | Variable interne `modeDvorak` (firmware) |
| **État ON**            | Layout Dvorak activé au niveau firmware  |
| **État OFF**           | Layout QWERTY ou Bépo standard           |

> Contrairement aux 3 premières LEDs (contrôlées par l'hôte via le rapport HID), la LED Dvorak est gérée **entièrement par le firmware** — son état reflète une configuration interne du clavier, indépendante du système d'exploitation.

### Code de mise à jour des LEDs

```cpp
void updateLEDs() {
  // LED 1: Verrouillage Num / Fn (signal hôte via USB HID)
  digitalWrite(PIN_LED_NUM_FN, (keyboard_led_state & KEYBOARD_LED_NUMLOCK) ? HIGH : LOW);
  
  // LED 2: Caps Lock (signal hôte)
  digitalWrite(PIN_LED_CAPS, (keyboard_led_state & KEYBOARD_LED_CAPSLOCK) ? HIGH : LOW);
  
  // LED 3: Scroll Lock (signal hôte)
  digitalWrite(PIN_LED_SCROLL, (keyboard_led_state & KEYBOARD_LED_SCROLLLOCK) ? HIGH : LOW);
  
  // LED 4: Mode Dvorak (variable interne firmware)
  digitalWrite(PIN_LED_DVORAK, modeDvorak ? HIGH : LOW);
}
```

---

## Keycaps personnalisées

### Contraintes spécifiques au TypeMatrix

Le layout orthogonal du TypeMatrix impose des keycaps à **profil bas** ou **plat** pour éviter les interférences entre touches adjacentes dans la disposition en grille. Les profils recommandés sont :

| Profil       | Description                  | Compatibilité             |
| ------------ | ---------------------------- | ------------------------- |
| **XDA**      | Sphérique uniforme, très bas | ✅ Idéal                   |
| **DSA**      | Sphérique, uniformément bas  | ✅ Très bon                |
| **MDA**      | Modèle intermédiaire         | ✅ Compatible              |
| **SA row 3** | Cylindrique haut             | ❌ Déconseillé             |
| **OEM**      | Standard incurvé             | ⚠️ Possible mais non idéal |

### Configurateur et fournisseurs

- 🎨 **Design sur-mesure :** [ThockFactory Configurator](https://thockfactory.com/fr/configurator) — permet de définir les légendes (Bépo, Dvorak, QWERTY, couche Fn) et le profil.
- 🏺 **Keycaps en céramique :** [Cerakey](https://www.cerakey.com/fr) — keycaps premium en céramique (haptique unique).
- 🌐 **Tai-Hao** : [shop.tai-hao.com](https://shop.tai-hao.com/) — Large sélection de profils.

### Légendes recommandées

Les keycaps devraient afficher :
- **Face principale :** Légende Bépo (ou QWERTY selon configuration)
- **Sous-légende (en bas)** : Légende Dvorak ou alternative
- **Légende latérale ou côté :** Couche Fn (touches de fonction en bleu)

### Impression 3D de keycaps

Pour des keycaps prototypes ou entièrement personnalisées :

- **Format de fichier :** STL (impression FDM/résine) ou STEP (export pour fraise CNC)
- **Technique recommandée :** **Résine stéréolithographique (SLA)** pour la précision du stem (tige d'ancrage Cherry MX ≈ 4mm)
- **Plateformes de modèles :** [Cults3D](https://cults3d.com), [MakerWorld (Bambu Lab)](https://makerworld.com)
- **Projet de référence :** [KLP Lame Keycaps](https://github.com/braindefender/KLP-Lame-Keycaps) — keycaps sphériques pour switches low profile

> 📹 Tutoriel — Impression résine de keycaps : [YouTube Shorts](https://www.youtube.com/shorts/zBXvIhGU1K0)

---

## Switches — Sockets Hotswap

### Objectif

Rendre les switches **interchangeables sans dessoudage** grâce à des sockets PCB enfichables. Cela permet de tester différents types de switches (Brown, Blue, Linear) sans modifier le PCB.

### Options de sockets

| Référence                                                                                                                                                             | Type       | Compatibilité | Remarque                      |
| --------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | ------------- | ----------------------------- |
| **[Kailh CPG151101S11](https://kbdfans.com/products/mechanical-keyboard-switches-kailh-pcb-socket?srsltid=AfmBOooc7kCLPFzwHLKvR6wD-5Dy37-WQghs_7rGPm9wnVt5nA7Hf1pA)** | SMD        | Cherry MX     | Standard de la communauté DIY |
| **[Mill-Max 0305](https://www.mill-max.com/products/pin-receptacle/receptacle-with-no-tail/0305)**                                                                    | Traversant | Cherry MX     | Plus robuste, cher            |
| **[Mill-Max 7305](https://www.mill-max.com/products/pin-receptacle/receptacle-with-no-tail/7305)**                                                                    | Traversant | Cherry MX     | Version ultra-bas profil      |

### Considérations PCB

- L'utilisation de sockets augmente légèrement l'épaisseur du PCB (prévoir une platine ou boîtier adapté).
- L'empreinte KiCad pour les sockets Kailh est disponible dans la bibliothèque [MX_Alps_Hybrid](https://github.com/ai03-2725/MX_Alps_Hybrid).

---

> **Page précédente :** [02 — Conception Hardware & PCB](./02-conception-hardware.md)  
> **Page suivante :** [04 — Architecture Firmware](./04-firmware-architecture.md)
