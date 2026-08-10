# 01 — Présentation du Projet Open-TPMX2030

> **Retour à l'index :** [Documentation](./README.md)

---

## Table des matières

- [01 — Présentation du Projet Open-TPMX2030](#01--présentation-du-projet-open-tpmx2030)
  - [Table des matières](#table-des-matières)
  - [Genèse du projet](#genèse-du-projet)
  - [Objectifs du projet](#objectifs-du-projet)
  - [Différences avec le TypeMatrix original](#différences-avec-le-typematrix-original)
  - [Organisation du dépôt Git](#organisation-du-dépôt-git)
    - [Structure des branches](#structure-des-branches)
    - [Structure des dossiers](#structure-des-dossiers)
    - [Bonnes pratiques de commit](#bonnes-pratiques-de-commit)
  - [Projets inspirants et références](#projets-inspirants-et-références)
    - [Projets communautaires](#projets-communautaires)
    - [Ressources](#ressources)

---

## Genèse du projet

Le projet **Open-TPMX2030** est né au sein du [FabLab Artilect](https://github.com/Artilect-FabTronic) de Toulouse, inspiré par la présentation du projet [**ToucheLibre**](https://touchelibre.fr/index.php/presentation-projet-en-video/) réalisée par **Lilian Tribouilloy** lors d'une soirée SuperLundi en mars 2020.

L'idée centrale : concevoir un **clavier mécanique open source**, fidèle à l'ergonomie orthogonale du [TypeMatrix 2030](https://typematrix.com/fr/products/2030/), mais en version entièrement DIY — avec des touches mécaniques Cherry MX2A, un microcontrôleur moderne et un firmware sur-mesure.

Le [TypeMatrix 2030](https://bepo.fr/wiki/TypeMatrix) est un clavier ergonomique réputé pour sa disposition en matrice orthogonale (colonnes verticales alignées), plébiscité par les utilisateurs des layouts alternatifs comme le **Bépo** (layout français ergonomique équivalent du Dvorak). La version USB commerciale existe, mais il n'existe pas de version mécanique officielle. Ce projet comble ce manque.

---

## Objectifs du projet

Le projet vise les objectifs suivants :

1. **Reproduire fidèlement la disposition physique orthogonale** du TypeMatrix 2030 en version mécanique.
2. **Intégrer des switches Cherry MX2A** interchangeables (via sockets hotswap si possible).
3. **Concevoir un PCB complet sous KiCad**, entièrement open source et reproductible en FabLab.
4. **Implémenter un firmware sur-mesure** (RP2040-Zero + Arduino-Pico) gérant nativement l'USB HID.
5. **Supporter plusieurs layouts** : QWERTY, Bépo, Dvorak, Colemak-DH — commutables directement depuis le clavier.
6. **Enrichir l'expérience utilisateur** avec 3 encodeurs rotatifs (luminosité, micro, volume) et 4 LEDs d'état.

---

## Différences avec le TypeMatrix original

| Critère                     | TypeMatrix 2030 USB | Open-TPMX2030 (ce projet)                  |
| --------------------------- | ------------------- | ------------------------------------------ |
| **Technologie des touches** | Membrane            | Mécanique (Cherry MX2A)                    |
| **Microcontrôleur**         | Propriétaire        | RP2040-Zero (open source)                  |
| **Firmware**                | Propriétaire        | Arduino-Pico (open source)                 |
| **Encodeurs**               | Absents             | 3× EC11 (Luminosité, Micro, Volume)        |
| **LEDs**                    | Basiques            | 4× SMD 0805 (Num/Fn, Caps, Scroll, Dvorak) |
| **Conception PCB**          | Fermée              | KiCad open source                          |
| **Layouts firmware**        | QWERTY + Dvorak     | QWERTY, Bépo, Dvorak, Colemak-DH           |
| **Source**                  | Commerciale         | Open Source / FabLab                       |

---

## Organisation du dépôt Git

Le dépôt est hébergé sur GitHub :

📌 [https://github.com/Artilect-FabTronic/open-tpmx2030-keyboard](https://github.com/Artilect-FabTronic/open-tpmx2030-keyboard)

### Structure des branches

| Branche            | Rôle                                                           |
| ------------------ | -------------------------------------------------------------- |
| `main`             | Branche principale **protégée** — aucun commit direct autorisé |
| `feature/pcb`      | Fichiers KiCad et conception du PCB                            |
| `feature/firmware` | Code source du firmware                                        |

> **Règle de contribution :** Toute contribution passe par une **Pull Request** depuis une branche feature vers `main`. Les commits directs sur `main` sont bloqués.

### Structure des dossiers

```plaintext
open-tpmx2030-keyboard/
├── README.md                  ← Point d'entrée du projet
├── docs/                      ← Documentation technique complète
│   ├── README.md              ← Index de navigation
│   ├── 01-presentation-projet.md
│   ├── 02-conception-hardware.md
│   ├── 03-interface-ihm.md
│   ├── 04-firmware-architecture.md
│   ├── 05-analyse-gpio-extension.md
│   ├── 06-dispositions-clavier.md
│   ├── 07-ressources-references.md
│   └── RP2040-Zero-Board/     ← Pinout et schéma du RP2040-Zero
├── images/                    ← Images du projet
├── pcb/                       ← Fichiers KiCad (branche feature/pcb)
└── .gitignore
```

### Bonnes pratiques de commit

Le projet suit la convention [Commitizen](https://commitizen-tools.github.io/commitizen/tutorials/writing_commits/) pour les messages de commit :

```
feat: add MX2A hotswap socket footprint in KiCad
fix: correct I2C pull-up resistor value on schematic
docs: update BOM with correct MCP23017 reference
chore: add .gitignore for KiCad temp files
```

---

## Projets inspirants et références

### Projets communautaires

| Projet           | Description                                      | Lien                                                                                             |
| ---------------- | ------------------------------------------------ | ------------------------------------------------------------------------------------------------ |
| **ToucheLibre**  | Clavier open source de Lilian Tribouilloy (2020) | [gitlab.com/touchelibre](https://gitlab.com/touchelibre)                                         |
| **ThinkMatrix**  | TypeMatrix mécanique DIY en bois avec Teensy     | [forum.bepo.fr](https://forum.bepo.fr/d/1232-conception-clavier-typematrix-mechanique-version-2) |
| **Quacken**      | Pro Micro RP2040 keyboard project                | [github.com/Nuclear-Squid/Quacken](https://github.com/Nuclear-Squid/Quacken/tree/main/firmware)  |
| **DreymaR EPKL** | Big Bag Of Keyboard Tricks pour Colemak-DH       | [github.com/DreymaR/BigBagKbdTrixPKL](https://github.com/DreymaR/BigBagKbdTrixPKL)               |
| **keebee**       | DIY USB Keyboard from Scratch (STM32)            | [github.com/blakesmith/embedded](https://github.com/blakesmith/embedded/tree/master/keebee)      |

### Ressources

- 📖 [Wiki Bépo — TypeMatrix](https://bepo.fr/wiki/TypeMatrix)
- 📹 [YouTube — I Built My DREAM Keyboard from SCRATCH](https://www.youtube.com/watch?v=KwFWBdfZKnI) ([Wiki](https://wiki.modhobbyist.com/Projects/0002%20-%20Split%20Keyboard%20-%20Mk.%201/1%20-%20Instructions.html))

---

> **Page suivante :** [02 — Conception Hardware & PCB](./02-conception-hardware.md)
