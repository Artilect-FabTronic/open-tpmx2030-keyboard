# 🛠️ Guide de Conception KiCad — Projet Open-TPMX2030

Ce guide est destiné aux membres et hackers du FabLab Artilect participant au développement de la carte électronique (PCB) du clavier ergonomique Open-TPMX2030.

Il regroupe les règles de l'art en ingénierie électronique pour concevoir un PCB propre, robuste, facile à fabriquer (JLCPCB / PCBWay) et à déboguer.

---

## 📌 1. Structure du Projet & Conventions de Nommage

Editer le fichier `.kicad_pro` avec un éditeur de texte est modifier la section `text_variables` en fin du fichier avec les éléments suivants :

```plaintext
  "text_variables": {
    "COMPANY": "Copyright (c) 2026 Artilect FabLab Toulouse",
    "CREATION_DATE": "2026-08-16",
    "LICENSE": "Apache-2.0 license",
    "PROJECT_NAME": "Open-TPMX2030 keyboard",
    "REVISION": "1.0.0",
    "SERIAL_NUM": "S/N: 260816-Artilect-P001",
    "USER_EMAIL": "arnauld.biganzoli@gmail.com",
    "USER_NAME": "ArnauldDev"
  },
```

Et depuis l'édition des fichiers `.kicad_sch` et `.kicad_pcb`, compléter la section `title_block` avec les éléments suivants :

```plaintext
	(title_block
		(title "${PROJECT_NAME}")
		(date "${CREATION_DATE}")
		(rev "${REVISION}")
		(company "${COMPANY}")
		(comment 1 "License: ${LICENSE}")
		(comment 2 "${USER_EMAIL}")
		(comment 3 "${USER_NAME}")
		(comment 4 "Electrical CAD Operator:")
	)
```

### 1.1 Organisation du Dépôt Git

Tout le projet Hardware réside dans le dossier `/pcb` du dépôt GitHub `open-tpmx2030-keyboard`.

```plaintext
├── pcb/
│   ├── open_tpmx2030.kicad_pro       # Fichier projet KiCad
│   ├── open_tpmx2030.kicad_sch       # Schématique racine (Feuille Principale)
│   ├── open_tpmx2030.kicad_pcb       # Layout du PCB
│   ├── open_tpmx2030.kicad_dru       # Règles de routage personnalisées (DRC)
│   ├── mcu_rp2040.kicad_sch          # Module Microcontrôleur RP2040-Zero
│   ├── matrix_expanders.kicad_sch    # Module MCP23017 (I2C)
│   ├── key_matrix.kicad_sch          # Matrice de switches (keyboard 102 touches + diodes)
│   ├── user_interface.kicad_sch      # Encodeurs + LEDs
│   └── libraries/                    # Symboles et Empreintes spécifiques
│       ├── symbols/                  # .kicad_sym (.kicad_sym)
│       ├── footprints/               # .kicad_mod (.pretty)
│       └── 3dmodels/                 # Modèles 3D (.3dshapes, .stp)
└── manufacturing/                    # Gerbers, Drill, BOM, CPL (Générés)
```

### 1.2 Règles de nommage des composants

Pour s'y retrouver facilement lors du soudage manuel ou de l'inspection :

* U1, U2, ... : Circuits intégrés (RP2040, MCP23017)
* SW1 à SW102 : Switches de touches (Cherry MX2A)
* SW_ENC1 à SW_ENC3 : Switches d'encodeurs rotatifs
* D1 à D102 : Diodes de matrice (1N4148 / SOD-123)
* LED1 à LED4 : LEDs d'état (0805)
* R1, R2, ... : Résistances
* C1, C2, ... : Condensateurs
* ENC1 à ENC3 : Encodeurs rotatifs (EC11)

---

## 📐 2. La schématique (Eeschema)

### 2.1 Approche par Schémas Hiérarchiques

Ne mettez jamais l'intégralité d'un projet de 102 touches sur une seule page géante !

Utilisez des Feuilles Hiérarchiques (Hierarchical Sheets) :

1. Racine (open_tpmx2030.kicad_sch) : Bloc-diagramme reliant les sous-modules via des labels hiérarchiques (Bus I2C, INT, Power).
2. Feuille mcu_rp2040 : RP2040-Zero, régulateur de tension (si besoin), filtrage d'alimentation, connecteur USB-C.
3. Feuille matrix_expanders : Les 2x MCP23017, adresses I2C (A0, A1, A2), résistances de tirage I2C (Pull-up).
4. Feuille key_matrix : Matrice $8 \times 13$ réparties proprement en sous-ensembles (ex: réplication de blocs).
5. Feuille user_interface : 3 Encodeurs EC11 + 4 LEDs.

### 2.2 Bonnes Pratiques de Schématique

* **Sens de lecture :** Les signaux d'entrée viennent de la **gauche**, les sorties vont vers la **droite**, les alimentations ($V_{CC}$ / $+3.3\text{V}$) sont en **haut**, et les masses ($\text{GND}$) sont en **bas**.
* **Condensateurs de découplage :** Dessinez TOUJOURS les condensateurs de découplage ($100\text{ nF}$) à côté des broches d'alimentation des puces correspondantes (`VCC/VDD` du MCP23017, etc.) sur le schéma, pour rappeler qu'ils doivent être soudés au plus près en layout.
* **Net Labels clairs :** Utilisez des noms de réseaux explicites (`I2C_SDA`, `I2C_SCL`, `ROW_0`, `COL_12`, `ENC_VOL_CLK`).

---

## ⚡ 3. Les Règles de Routage PCB (Pcbnew)

### 3.1 Empilement de la Carte (Layer Stackup - 2 Couches)

Puisqu'il s'agit d'une carte 2 couches classique (1.6 mm FR4, $35\ \mu\text{m}$ Cuivre) :

* Couche Supérieure (F.Cu - Red) : Signaux principaux, bus de données, matrice de touches.
* Couche Inférieure (B.Cu - Blue) : Plan de Masse Continu ($\text{GND}$).
  * _Règle absolue :_ Évitez au maximum de couper le plan de masse face inférieure avec de longues pistes. Si une piste doit passer en `B.Cu`, faites un "pont" court et revenez en `F.Cu`.

### 3.2 Largeur de Pistes (Track Widths)

Configurons les classes de réseaux (Net Classes) dans KiCad :

| Classe de Réseau | Largeur de Piste                   | Isolation (Clearance)              | Via (Perçage / Anneau)          | Usage                           |
| ---------------- | ---------------------------------- | ---------------------------------- | ------------------------------- | ------------------------------- |
| Default          | $0.20\text{ mm}$ ($8\text{ mil}$)  | $0.20\text{ mm}$ ($8\text{ mil}$)  | $0.6\text{ mm} / 0.3\text{ mm}$ | Signaux matrice, LEDs, GPIOs    |
| HighSpeed_I2C    | $0.25\text{ mm}$ ($10\text{ mil}$) | $0.25\text{ mm}$ ($10\text{ mil}$) | $0.6\text{ mm} / 0.3\text{ mm}$ | Bus I2C (SDA / SCL à 1 MHz)     |
| Power (+3.3V)    | $0.50\text{ mm}$ ($20\text{ mil}$) | $0.25\text{ mm}$ ($10\text{ mil}$) | $0.8\text{ mm} / 0.4\text{ mm}$ | Lignes d'alimentation principal |
| Power (+5V USB)  | $0.75\text{ mm}$ ($30\text{ mil}$) | $0.30\text{ mm}$ ($12\text{ mil}$) | $0.8\text{ mm} / 0.4\text{ mm}$ | VBUS d'entrée USB               |

### 3.3 Placement des Composants

1. **Positionnement Ergonomique (Fixe) :** Placez d'abord les switches MX2A (espacement standard $19.05\text{ mm} \times 19.05\text{ mm}$ ou selon le layout spécifique TypeMatrix), les 3 encodeurs et les 4 LEDs.
2. **Zone Microcontrôleur & Expanders :** Placez le RP2040-Zero et les 2x MCP23017 dans la partie supérieure de la carte.
3. **Condensateurs de découplage ($100\text{ nF}$) :** Placer IMPÉRATIVEMENT chaque condensateur de découplage à moins de $2\text{ mm}$ de la broche $V_{DD}$ du MCP23017, relié directement au plan de masse par un via.

---

## 🛡️ 4. Intégrité du Signal & Compatibilité Électromagnétique (CEM)

1. **Bus I2C à 1 MHz :**
  * Gardez les pistes I2C_SDA et I2C_SCL parallèles, de longueur similaire et le plus court possible entre le RP2040-Zero et les deux MCP23017.
  * Placez les résistances de tirage ($R_{pullup} = 2.2\text{ k}\Omega$) près du RP2040-Zero.
2. **Diodes Anti-Ghosting :**
  * Chaque touche contient une diode 1N4148 (boîtier SOD-123). Placez la diode juste à côté du pad du switch correspondant pour garder un routage ultra-propre et compact.
3. **Vias de Masse (Via Stitching) :**
  * Distribuez généreusement des vias de masse autour du microcontrôleur et le long du contour de la carte pour assurer une excellente continuité du plan de masse GND.

---

## 🔍 5. Liste de Contrôle Avant Fabrication (Checklist DRC)

Avant de générer les fichiers Gerber pour le FabLab ou l'usine :

- [ ] Lancer le DRC (Design Rules Check) : 0 erreur, 0 avertissement toléré.
- [ ] Contrôle d'empreintes (Footprints) : Vérifier le sens des diodes (Cathode / Anode) et l'orientation du RP2040-Zero.
- [ ] Trous de Fixation (Mounting Holes) : Placer des trous $M3$ (non métallisés ou reliés à la masse) aux 4 coins et au centre pour la rigidité lors de la frappe.
- [ ] Visualisation 3D (Alt + 3) : Vérifier qu'aucun composant ne se chevauche (notamment les corps des switches MX2A et les encodeurs EC11).
- [ ] Sérigraphie (Silkscreen) :
  * Nom du projet : Open-TPMX2030 (FabLab Artilect)
  * Version du PCB : v1.0.0 - 2026
  * Logos : Logo Open Source Hardware + Logo Artilect.
  * Indication lisible des LEDs (NUM/FN, CAPS, SCROLL, DVORAK) et des fonctions des encodeurs (LUM, MIC, VOL).

---

## 📦 6. Export pour la Fabrication

Exportez depuis KiCad dans le dossier `/production` :

1. **Fichiers Gerber (RS-274X) :** Couches `F.Cu`, `B.Cu`, `F.Silkscreen`, `B.Silkscreen`, `F.Mask`, `B.Mask`, `Edge.Cuts` (Contour).
2. **Fichier de Perçage (Excellon / NC Drill) :** Unités en mm, coordonnées absolues.
3. **Fichiers de Placement SMT (Position / CPL) :** Fichier `.csv` pour l'assemblage automatique des composants de surface (Diodes SOD-123, MCP23017, Résistances 0805).
