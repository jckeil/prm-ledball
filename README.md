# prm-ledball
ESP32-C5 controlled WS2813B LED ball with wireless remote control for stage lighting, decorations, and live shows
# 🔮 Светодиодный шар на ESP32-C5 (WS2813B)

Проект печатной платы и прошивки для беспроводного светодиодного шара. Устройство разработано специально для сценических выступлений, артистов и световых шоу. Управляется удаленно оператором, позволяя синхронизировать визуальные эффекты с шоу-программой.

---

## 🚀 Основные возможности

* **Управление адресной лентой:** Полная поддержка светодиодов WS2813B для создания плавных динамических эффектов и смены цветов.
* **Современная беспроводная связь:** Базируется на модуле **ESP32-C5**, что обеспечивает стабильный радиус действия и защиту от помех на сцене.
* **Автономное питание:** Устройство спроектировано для работы от литий-ионного (Li-ion) аккумулятора, обеспечивая полную свободу перемещения артиста с шаром в руках.
* **Сценическая архитектура:** Прошивка оптимизирована для мгновенного приема команд удаленного беспроводного управления в реальном времени.

---

## 🛠️ Технические характеристики

* **Микроконтроллер:** ESP32-C5 (модуль ESPC5-32E-H4).
* **Тип светодиодов:** Адресные RGB-светодиоды WS2813B.
* **Источник питания:** Литий-ионный аккумулятор (Li-ion) 3.7V. На плате разведена схема зарядки аккумулятора до 400мА.
* **Форм-фактор:** Компактная плата круглой формы для удобного размещения внутри шара.

---

## ⚙️ Как запустить и использовать

### 1. Железо (Hardware)
* Схемы и разводка платы находятся по адресу: [labelcom.ru/prm-led-ball](https://labelcom.ru).

### 2. Прошивка (Firmware)
* Среда разработки: Arduino IDE.
* Перед использованием необходимо настроить параметры своей беспроводной сети в конфигурационном файле.

---

## ⚖️ Лицензия

Этот проект распространяется под лицензией **GNU General Public License v3.0**. Вы можете свободно использовать, модифицировать и улучшать этот проект, но с условием, что все ваши доработки также останутся открытыми и бесплатными для сообщества.

==============================================================================

## 🇬🇧 Short English Summary

**Project Overview:** Custom PCB and firmware for a wireless, battery-powered WS2813B LED stage ball based on the **ESPC5-32E-H4** (ESP32-C5) module. Designed specifically for performance artists, stage decorations, and live light shows under an operator's real-time remote control.

### Key Features & Specs:
* **MCU:** ESP32-C5 (ESPC5-32E-H4 module).
* **LED Control:** Support for WS2813B addressable RGB LEDs.
* **Power:** 3.7V Li-ion battery with an onboard charging circuit up to 400mA.
* **Form Factor:** Compact round PCB designed to fit perfectly inside a sphere prop.
* **IDE:** Developed using Arduino IDE.

### Schematic & Documentation:
* PCB layouts and schematics can be found at: [labelcom.ru/prm-led-ball](https://labelcom.ru).

**License:** This project is open-source and licensed under the **GNU GPLv3**.
