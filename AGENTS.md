# AGENTS.md - AI Agent Guidelines & Context

This file defines the project architecture, hardware constraints, safety protocols, and coding guidelines for AI coding agents (such as Antigravity, Copilot, Cursor, etc.) working on this repository.

---

## 1. Project Overview

- **Repository**: `iot-watering` (ESP32 Smart Watering with Matter-over-Wi-Fi)
- **Primary Goal**: Autonomous and smart-home-integrated plant/garden watering system using native **Matter-over-Wi-Fi** (Google Home, Apple Home, Home Assistant).
- **Core Controller**: ESP32 Dev Module (NodeMCU / WROOM, min. 4 MB Flash).
- **Key Files**:
  - `sketch/sketch.ino`: Main Arduino firmware with Matter stack and safety logic.
  - `SKETCH.md`: Full hardware schematic, wiring tables, and inductive load protection.
  - `README.md`: End-user documentation and setup instructions.
  - `PROJECT.md`: Initial Matter architecture notes.

---

## 2. Hardware & Pinout Constraints

Any modifications to pin definitions or electrical logic MUST adhere to these exact hardware specifications:

| Pin | Connected Hardware | Electrical Logic | Notes |
| :--- | :--- | :--- | :--- |
| `GPIO 23` | 1-Channel Relay Module (`IN`) | **Active-LOW** | `LOW` = Relay closed / Pump ON<br>`HIGH` = Relay open / Pump OFF |
| `GPIO 22` | Float Switch (Water reservoir) | **INPUT_PULLUP** | `LOW` = Switch closed (Water available)<br>`HIGH` = Switch open (Tank empty / Dry-run) |

> [!CAUTION]
> **Active-LOW Relay Logic**:
> Never invert the relay logic unless explicitly instructed. On ESP32 boot, the relay pin must immediately be set to `HIGH` in `setup()` before `pinMode` or early execution to avoid transient pump activation during startup.

---

## 3. Mandatory Safety Rules (Trockenlaufschutz)

The system protects the water pump from running dry through a **two-tier defense mechanism**. Agents modifying `sketch/sketch.ino` MUST preserve both tiers:

1. **Pre-Check in Command Callback (`onPumpStateChange`)**:
   - Before switching the pump `ON`, the float switch MUST be checked (`digitalRead(FLOAT_SWITCH_PIN) == LOW`).
   - If the tank is empty, the command MUST be rejected (`return false`), the relay MUST remain `HIGH`, and `matterPump.setState(false)` MUST be called to synchronize the UI in Google Home.

2. **Real-Time Watchdog in `loop()`**:
   - If the pump is running (`digitalRead(RELAY_PIN) == LOW`) AND the float switch opens (`digitalRead(FLOAT_SWITCH_PIN) == HIGH`), the pump MUST immediately shut off (`digitalWrite(RELAY_PIN, HIGH)`) and Matter state updated to `false`.

---

## 4. Matter & Firmware Guidelines

- **Arduino Core**: Espressif ESP32 Arduino Core `>= 3.0.0` with integrated Matter support.
- **Partition Scheme**: Requires large application partition (e.g., `Minimal SPIFFS (Large APPS with OTA)` or dedicated Matter partition) due to Matter stack flash requirements.
- **Matter Cluster**: The pump is modeled as an `On/Off Outlet` cluster (`MatterOutlet`), providing native switch functionality and timer routines in Google Home.
- **Language Policy**:
  - Source code comments and identifiers: **English**.
  - User-facing documentation (`README.md`, `SKETCH.md`, `PROJECT.md`): **German** (matching user's preferred docs).

---

## 5. File Referencing & Documentation Standards

- Always use GitHub-style markdown links with the `file://` scheme when referencing workspace files in discussions or documentation (e.g., `[sketch.ino](file:///path/to/sketch.ino)`).
- When changing hardware or pin mappings, ensure `sketch/sketch.ino`, `SKETCH.md`, and `README.md` are kept fully synchronized.
