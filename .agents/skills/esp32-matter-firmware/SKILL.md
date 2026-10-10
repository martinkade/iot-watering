---
name: esp32-matter-firmware
description: >-
  Use this skill when modifying, compiling, extending, or troubleshooting the ESP32 Matter-over-Wi-Fi firmware or hardware interfaces for the watering project.
---

# ESP32 Matter Firmware Workflow & Checklist

This skill provides step-by-step procedures for safely maintaining, modifying, and debugging the ESP32 Matter-over-Wi-Fi watering firmware.

---

## 1. Safety & Logic Verification Checklist

Before saving or deploying any changes to `sketch/sketch.ino`:

1. **Active-LOW Relay Logic**:
   - Verify `digitalWrite(RELAY_PIN, HIGH);` occurs in `setup()` immediately upon boot.
   - Verify that turning the pump ON writes `LOW` and turning it OFF writes `HIGH`.
2. **Dry-Run Protection (Two-Tier Check)**:
   - **Tier 1 (Callback)**: `onPumpStateChange()` must check `digitalRead(FLOAT_EMPTY_PIN) == LOW` before setting `digitalWrite(RELAY_PIN, LOW)`.
   - **Tier 2 (Loop Watchdog)**: `loop()` must check `if (digitalRead(RELAY_PIN) == LOW && digitalRead(FLOAT_EMPTY_PIN) == HIGH)` and immediately force the relay to `HIGH` and `matterPump.setState(false)`.
3. **Tank Full Monitoring (Upper Float Switch)**:
   - Verify `FLOAT_FULL_PIN` (`GPIO 21`) updates `matterTankFull.setContact()` on state change (`LOW` = full / closed).
4. **Matter State Synchronization**:
   - Any hardware-triggered shutdown must update the Matter cluster state (`matterPump.setState(false)`) so Google Home reflects the actual state.

---

## 2. Firmware Build & Flash Configuration

When building the sketch in Arduino IDE:

- **Board**: `ESP32 Dev Module` (or equivalent ESP32-WROOM-32)
- **Core**: `esp32 by Espressif` version `>= 3.0.0`
- **Flash Size**: `4MB (32Mb)`
- **Partition Scheme**: `Minimal SPIFFS (Large APPS with OTA)` (or dedicated Matter profile)
  - *Note*: Standard partition schemes will fail with a size error because the Matter cryptographic stack and Bluetooth stack require >1.5 MB flash space.
- **Upload Speed**: `921600` or `115200` baud.

---

## 3. Commissioning & Troubleshooting Procedure

If the device does not appear or fails to pair in Google Home:

1. **Serial Monitor**:
   - Open Serial Monitor at `115200` baud.
   - Look for `Manueller Setup-Code: XXXXXXXXXXX` (11 digits).
2. **Matter Fabric Reset**:
   - If previously commissioned to another fabric or Wi-Fi network, erase flash or call `Matter.decommission()` / hold boot button if implemented.
3. **Network Requirements**:
   - Both the Google Home Hub (Nest Hub / Nest Mini) and the smartphone must be on the **same 2.4 GHz Wi-Fi** during Bluetooth Low Energy (BLE) pairing.
