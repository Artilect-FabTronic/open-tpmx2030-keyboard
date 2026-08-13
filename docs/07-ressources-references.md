# 07 — Ressources & Références

> **Retour à l'index :** [Documentation](./README.md)

---

## Table des matières

- [Datasheets et documentation technique](#datasheets-et-documentation-technique)
- [Dépôts GitHub et bibliothèques](#dépôts-github-et-bibliothèques)
- [Tutoriels vidéo](#tutoriels-vidéo)
- [Fournisseurs de composants](#fournisseurs-de-composants)
- [Fournisseurs de keycaps](#fournisseurs-de-keycaps)
- [Projets DIY inspirants](#projets-diy-inspirants)
- [Communautés et forums](#communautés-et-forums)
- [Ressources institutionnelles](#ressources-institutionnelles)

---

## Datasheets et documentation technique

### Microcontrôleur RP2040

| Document                        | Description                                             | Lien                                                                                                                                 |
| ------------------------------- | ------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------ |
| **RP2040 Datasheet**            | Référence complète du chip (mémoire, PIO, USB, GPIO...) | [raspberrypi.com](https://pip-assets.raspberrypi.com/categories/814-rp2040/documents/RP-008371-DS-1-rp2040-datasheet.pdf)            |
| **Hardware Design with RP2040** | Guide de conception hardware (découplage, USB, flash)   | [raspberrypi.com](https://pip-assets.raspberrypi.com/categories/814-rp2040/documents/RP-008279-DS-1-hardware-design-with-rp2040.pdf) |
| **Waveshare RP2040-Zero Wiki**  | Documentation officielle de la carte Waveshare          | [waveshare.com](https://www.waveshare.com/wiki/RP2040-Zero)                                                                          |
| **TinyGo RP2040-Zero Pinout**   | Référence des constantes de broches                     | [tinygo.org](https://tinygo.org/docs/reference/microcontrollers/machine/waveshare-rp2040-zero/)                                      |
| **Arduino-Pico Guide**          | Guide d'utilisation Arduino pour le RP2040 (Adafruit)   | [adafruit.com](https://cdn-learn.adafruit.com/downloads/pdf/rp2040-arduino-with-the-earlephilhower-core.pdf)                         |

### Extenseur GPIO MCP23017

| Document                  | Description                                        | Lien                                                                                                                                                                                   |
| ------------------------- | -------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **MCP23017 Datasheet**    | Datasheet complète (registres, I2C, interruptions) | [microchip.com](https://ww1.microchip.com/downloads/aemDocuments/documents/APID/ProductDocuments/DataSheets/MCP23017-MCP23S17-16-Bit-IO-Expander-with-Serial-Interface-DS20001952.pdf) |
| **Page produit MCP23017** | Outils de sélection et alternatives                | [microchip.com](https://www.microchip.com/en-us/product/mcp23017)                                                                                                                      |

### Protocole USB HID

| Document                                                      | Description                                        | Lien                                                                                                                                                                                                         |
| ------------------------------------------------------------- | -------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| **The USB keyboard protocol**                                 | Protocole USB clavier HID (Human Interface Device) | [blog.gistre.epita.fr](https://blog.gistre.epita.fr/posts/ivan.imbert-2024-09-09-the_usb_keyboard_protocol/) / [emrecmic.wordpress.com](https://emrecmic.wordpress.com/wp-content/uploads/2016/05/hid-1.pdf) |
| **Device Class Definition for Human Interface Devices (HID)** | Protocole USB clavier HID (Human Interface Device) | [usb.org](https://www.usb.org/sites/default/files/documents/hid1_11.pdf)                                                                                                                                     |
| **Construire des périphériques USB HID**                      | Guide pour créer des périphériques USB HID         | [electroseed.fr](https://www.electroseed.fr/wiki/fr/docs/tutorials/tutorials-nodeblue/nodeblue_99q_hid/)                                                                                                     |
| **Arduino USB HID Reference**                                 | Référence de la bibliothèque clavier Arduino       | [docs.arduino.cc](https://docs.arduino.cc/language-reference/en/functions/usb/Keyboard/)                                                                                                                     |
| **Keyboard Modifiers & Special Keys**                         | Codes des modificateurs et touches spéciales       | [docs.arduino.cc](https://docs.arduino.cc/language-reference/en/functions/usb/Keyboard/keyboardModifiers/)                                                                                                   |

---

## Dépôts GitHub et bibliothèques

### Firmware et bibliothèques Arduino-Pico

| Dépôt                                                                                                                                                | Description                                                     |
| ---------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------- |
| [earlephilhower/arduino-pico](https://github.com/earlephilhower/arduino-pico)                                                                        | Core Arduino non-officiel pour RP2040 — bibliothèques, exemples |
| [arduino-pico/libraries/Keyboard](https://github.com/earlephilhower/arduino-pico/tree/master/libraries/Keyboard)                                     | Bibliothèque Keyboard pour RP2040                               |
| [KeyboardPassword.ino](https://github.com/earlephilhower/arduino-pico/blob/master/libraries/Keyboard/examples/KeyboardPassword/KeyboardPassword.ino) | Exemple d'utilisation de la bibliothèque Keyboard               |
| [earlephilhower/Keyboard](https://github.com/earlephilhower/Keyboard)                                                                                | Bibliothèque Keyboard standalone                                |

### Projets de claviers DIY de référence

| Dépôt                                                                                                                  | Description                                                        |
| ---------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------ |
| [blakesmith/embedded/keebee](https://github.com/blakesmith/embedded/tree/master/keebee)                                | DIY USB Keyboard from Scratch (STM32F042K6T6) — référence complète |
| [Nuclear-Squid/Quacken](https://github.com/Nuclear-Squid/Quacken/tree/main/firmware)                                   | Firmware clavier Pro Micro RP2040                                  |
| [MakerIO-Hub — Macro Pad ESP32-S3](https://github.com/MakerIO-Hub/I-Built-My-Own-Macro-Pad-From-Scratch-with-ESP32-S3) | Macro pad DIY ESP32-S3 avec KiCad                                  |
| [esp32beans/ESP32_USB_Host_HID](https://github.com/esp32beans/ESP32_USB_Host_HID)                                      | USB Host HID sur ESP32                                             |

### Layouts et ergonomie

| Dépôt                                                                                                      | Description                                   |
| ---------------------------------------------------------------------------------------------------------- | --------------------------------------------- |
| [DreymaR/BigBagKbdTrixPKL](https://github.com/DreymaR/BigBagKbdTrixPKL)                                    | Big Bag of Keyboard Tricks — Colemak-DH, EPKL |
| [EPKL_Layouts_Default.ini](https://github.com/DreymaR/BigBagKbdTrixPKL/blob/main/EPKL_Layouts_Default.ini) | Fichier de configuration EPKL                 |
| [Delapouite/awesome-keyboard](https://github.com/Delapouite/awesome-keyboard)                              | Liste curated de ressources claviers          |
| [braindefender/KLP-Lame-Keycaps](https://github.com/braindefender/KLP-Lame-Keycaps)                        | Keycaps sphériques imprimées 3D               |
| [keebio/cepstrum-case](https://github.com/keebio/cepstrum-case)                                            | Boîtier clavier imprimé 3D                    |
| [touchelibre (GitLab)](https://gitlab.com/touchelibre)                                                     | Projet ToucheLibre de Lilian Tribouilloy      |

### Dépôt de ce projet

| Dépôt                                                                                                     | Description               |
| --------------------------------------------------------------------------------------------------------- | ------------------------- |
| [Artilect-FabTronic/open-tpmx2030-keyboard](https://github.com/Artilect-FabTronic/open-tpmx2030-keyboard) | Dépôt principal du projet |

---

## Tutoriels vidéo

### Microcontrôleur RP2040

| Titre                                                           | Chaîne              | Lien                                                   |
| --------------------------------------------------------------- | ------------------- | ------------------------------------------------------ |
| **EB_#698 Flash — Le RP2040-Zero, Le Petit Raspberry Pi Pico!** | Électro-Bidouilleur | [YouTube](https://www.youtube.com/watch?v=h4QJl2mEFyY) |
| **Getting Started with Waveshare RP2040-Zero \| Blink LED**     | —                   | [YouTube](https://www.youtube.com/watch?v=Vx-Y7pCRSJY) |

### Claviers DIY et HID

| Titre                                                            | Chaîne        | Lien                                                                                            |
| ---------------------------------------------------------------- | ------------- | ----------------------------------------------------------------------------------------------- |
| **Making My Own USB Keyboard From Scratch (STM32F042)**          | Blake Smith   | [Article + code](https://blakesmith.me/2019/01/16/making-my-own-usb-keyboard-from-scratch.html) |
| **J'ai construit mon propre macro pad ESP32-S3 (de A à Z)**      | MakerIO       | [YouTube](https://www.youtube.com/watch?v=Mk3VkKXWFn4)                                          |
| **I'm Building My Own Macro Keyboard — ESP32-S3, KiCad, PCB #1** | MakerIO       | [YouTube](https://www.youtube.com/watch?v=A0w_XrWLfuM)                                          |
| **USB Keyboard and Mouse on ESP32-S2/S3**                        | —             | [YouTube](https://www.youtube.com/watch?v=tdTQtg8-1n4)                                          |
| **DIY HID USB Keyboard Using STM32 [HAL]**                       | Instructables | [instructables.com](https://www.instructables.com/STM32-As-HID-USB-Keyboard-STM32-Tutorials/)   |

### Design claviers ergonomiques

| Titre                                                      | Chaîne      | Lien                                                   |
| ---------------------------------------------------------- | ----------- | ------------------------------------------------------ |
| **CMKB SoflePLUS2 Review — Neat New Ideas**                | —           | [YouTube](https://www.youtube.com/watch?v=phtbv2l8srY) |
| **How (Not) To Build An Open Source Keyboard feat. TOTEM** | —           | [YouTube](https://www.youtube.com/watch?v=aVhC3ekbFAs) |
| **Creator Micro 2 body swap (Work Louder)**                | Work Louder | [YouTube](https://www.youtube.com/watch?v=E-NVRhMK8tg) |

### Impression 3D de keycaps

| Titre                                                 | Lien                                                         |
| ----------------------------------------------------- | ------------------------------------------------------------ |
| **Impression de touche de clavier avec de la résine** | [YouTube Shorts](https://www.youtube.com/shorts/zBXvIhGU1K0) |

---

## Fournisseurs de composants

### Switches mécaniques

| Fournisseur   | Produit                       | Lien                                                                             |
| ------------- | ----------------------------- | -------------------------------------------------------------------------------- |
| **Cherry**    | MX2A Brown (V1 — Tactile)     | [cherry.de](https://www.cherry.de/en-gb/product/mx2a-brown)                      |
| **Cherry**    | MX2A Blue (V2 — Clicky)       | [cherry.de](https://www.cherry.de/en-gb/product/mx2a-blue)                       |
| **RS-Online** | Interrupteurs de clavier      | [rs-online.com](https://fr.rs-online.com/web/p/interrupteurs-de-clavier/0664569) |
| **Mouser**    | Cherry Electrical (catalogue) | [mouser.fr](https://www.mouser.fr/fr/manufacturer/cherry-electrical/)            |
| **Gateron**   | Switches alternatifs          | [gateron.co](https://www.gateron.co/)                                            |

### Microcontrôleurs et modules

| Fournisseur    | Produit                         | Lien                                                                         |
| -------------- | ------------------------------- | ---------------------------------------------------------------------------- |
| **Amazon FR**  | RP2040-Zero lot de 6            | [amazon.fr](https://www.amazon.fr/dp/B0H3ZWW6R3/ref=sspa_dk_detail_5)        |
| **OpenElab**   | RP2040-Zero                     | [openelab.io](https://openelab.io/fr/products/rp2040-zero-a-pico)            |
| **Gotronic**   | Raspberry Pi Pico SC0915        | [gotronic.fr](https://www.gotronic.fr/art-carte-raspberry-pi-pico-33027.htm) |
| **AliExpress** | ESP32-C3 SuperMini (alternatif) | [aliexpress.com](https://fr.aliexpress.com/item/1005012143614522.html)       |

---

## Fournisseurs de keycaps

| Fournisseur      | Spécialité                                              | Lien                                                         |
| ---------------- | ------------------------------------------------------- | ------------------------------------------------------------ |
| **ThockFactory** | Configurateur keycaps sur-mesure (légendes Bépo/Dvorak) | [thockfactory.com](https://thockfactory.com/fr/configurator) |
| **Cerakey**      | Keycaps en céramique premium                            | [cerakey.com](https://www.cerakey.com/fr)                    |
| **Tai-Hao**      | Large sélection profils et couleurs                     | [shop.tai-hao.com](https://shop.tai-hao.com/)                |
| **Keeb.io**      | Boutique DIY keyboards                                  | [keeb.io](https://keeb.io/collections/keyboards)             |

---

## Projets DIY inspirants

### ToucheLibre (2020) — Lilian Tribouilloy

Projet fondateur ayant inspiré l'Open-TPMX2030, présenté au FabLab Artilect lors d'une soirée SuperLundi en mars 2020.

- 🌐 [Site ToucheLibre](https://touchelibre.fr/index.php/presentation-projet-en-video/)
- 📦 [GitLab ToucheLibre](https://gitlab.com/touchelibre)

### ThinkMatrix — TypeMatrix mécanique DIY

Réalisation communautaire d'un TypeMatrix mécanique en bois avec un microcontrôleur Teensy, discutée sur le forum Bépo.

- 🌐 [Fil de discussion forum Bépo](https://forum.bepo.fr/d/1232-conception-clavier-typematrix-mechanique-version-2)

### keebee — DIY USB Keyboard from Scratch (Blake Smith)

Projet de référence pour la conception d'un clavier USB HID complet depuis zéro, avec STM32F042K6T6.

- 📝 [Article de blog](https://blakesmith.me/2019/01/16/making-my-own-usb-keyboard-from-scratch.html)
- 📦 [Code source GitHub](https://github.com/blakesmith/embedded/tree/master/keebee)

> La boucle firmware décrite par Blake Smith correspond exactement à l'approche retenue pour le Open-TPMX2030 :
>
> 1. Scanner toutes les touches de la matrice.
> 2. Associer les positions aux symboles (selon le layout actif).
> 3. Générer des paquets USB HID et les envoyer.
> 4. Mettre à jour les LEDs selon l'état des touches.

### Dreymar's Big Bag of Keyboard Tricks

Référence incontournable pour les layouts alternatifs avancés (Colemak-DH, Angle Mod, Wide Mod, etc.) sur Windows (EPKL) et Linux (XKB).

- 🌐 [dreymar.colemak.org](https://dreymar.colemak.org/)
- 📦 [GitHub EPKL](https://github.com/DreymaR/BigBagKbdTrixPKL)

---

## Communautés et forums

| Communauté                | Description                                          | Lien                                                                        |
| ------------------------- | ---------------------------------------------------- | --------------------------------------------------------------------------- |
| **Forum Bépo**            | Référence francophone layouts ergonomiques           | [forum.bepo.fr](https://forum.bepo.fr)                                      |
| **r/MechanicalKeyboards** | Grande communauté internationale claviers mécaniques | [reddit.com](https://reddit.com/r/MechanicalKeyboards)                      |
| **Locoduino**             | Communauté francophone Arduino/RP2040                | [locoduino.org](https://www.locoduino.org/spip.php?article319)              |
| **Framboise 314**         | Blog français Raspberry Pi / RP2040                  | [framboise314.fr](https://www.framboise314.fr/carte-waveshare-rp2040-zero/) |
| **Linagora**              | Communauté open source française                     | [inagora.fr](https://inagora.fr/)                                           |

---

## Ressources

| Ressource                             | Description                            | Lien                                                                                                   |
| ------------------------------------- | -------------------------------------- | ------------------------------------------------------------------------------------------------------ |
| **Raspberry Pi — Pico Series**        | Documentation officielle Pico / RP2040 | [raspberrypi.com](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html)         |
| **Raspberry Pi Pico — Wikipedia FR**  | Article Wikipédia en français          | [wikipedia.org](https://fr.wikipedia.org/wiki/Raspberry_Pi_Pico)                                       |
| **Disposition Dvorak — Wikipedia FR** | Article Wikipédia sur le Dvorak        | [wikipedia.org](https://fr.wikipedia.org/wiki/Disposition_Dvorak)                                      |
| **Commitizen**                        | Guide des bonnes pratiques de commit   | [commitizen-tools.github.io](https://commitizen-tools.github.io/commitizen/tutorials/writing_commits/) |

---

> **Page précédente :** [06 — Dispositions Clavier](./06-dispositions-clavier.md)  
> **Retour à l'index :** [Documentation](./README.md)
