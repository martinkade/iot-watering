#include <Arduino.h>
#include <Matter.h>
#include <MatterOutlet.h>

// ==============================================================================
// Hardware Pin Configuration
// ==============================================================================
// GPIO 23 controls the relay driving the watering pump or solenoid valve.
// Most relay breakout modules use Active-LOW logic:
//   - LOW  = relay energized (contacts closed -> Pump ON)
//   - HIGH = relay de-energized (contacts open -> Pump OFF)
const int RELAY_PIN = 23;

// GPIO 22 is connected to the water tank float switch (dry-run protection).
// Configured with the internal pull-up resistor (INPUT_PULLUP):
//   - Float switch closed (sufficient water level): pulled down to GND (reads LOW)
//   - Float switch open (tank empty / water level low): pulled up (reads HIGH)
const int FLOAT_SWITCH_PIN = 22;

// ==============================================================================
// Matter Device Endpoint
// ==============================================================================
// Declare a Matter On/Off Outlet device cluster.
// In smart home ecosystems such as Google Home, Apple Home, or Home Assistant,
// this device is exposed as a controllable smart plug/switch for the water pump.
MatterOutlet matterPump;

// ==============================================================================
// Callback Function: Matter State Change Handler
// ==============================================================================
// This callback is invoked asynchronously whenever a Matter controller
// (e.g., Google Home app, voice command, or routine) requests a state change.
//
// Parameters:
//   targetState: true = turn ON requested, false = turn OFF requested.
//
// Returns:
//   true  = state transition was accepted and executed.
//   false = state transition was rejected (e.g., due to safety trip).
bool onPumpStateChange(bool targetState) {
  // Check the float switch to confirm if water is available in the reservoir.
  // Switch connects to GND when water level is sufficient (Active LOW).
  bool waterAvailable = (digitalRead(FLOAT_SWITCH_PIN) == LOW);

  if (targetState == true) {
    if (waterAvailable) {
      Serial.println("Matter command received: Pump ON");
      // Energize the relay (Active-LOW: write LOW to close circuit and run pump)
      digitalWrite(RELAY_PIN, LOW);
      // Synchronize the internal Matter cluster attribute with the actual state
      matterPump.setState(true);
      return true;
    } else {
      // Dry-run protection: reject the command because the reservoir is empty!
      Serial.println("Matter command rejected: Water reservoir is empty!");
      // Force the state back to OFF in the Matter ecosystem (e.g., Google Home app)
      matterPump.setState(false);
      return false;
    }
  } else {
    Serial.println("Matter command received: Pump OFF");
    // De-energize the relay (Active-LOW: write HIGH to open circuit and stop pump)
    digitalWrite(RELAY_PIN, HIGH);
    // Synchronize the internal Matter cluster attribute with the actual state
    matterPump.setState(false);
    return true;
  }
}

// ==============================================================================
// Arduino Setup: Initialization & Matter Commissioning
// ==============================================================================
void setup() {
  // Initialize serial communication for debugging and commissioning instructions
  Serial.begin(115200);

  // Configure relay control pin as an OUTPUT
  pinMode(RELAY_PIN, OUTPUT);
  // Ensure the relay is safely de-energized (OFF) at startup to prevent unintended watering
  digitalWrite(RELAY_PIN, HIGH);

  // Configure float switch pin with the internal pull-up resistor.
  // This eliminates the need for an external physical pull-up resistor.
  pinMode(FLOAT_SWITCH_PIN, INPUT_PULLUP);

  // Initialize the core ESP32 Matter stack
  Matter.begin();

  // Initialize the Matter Outlet endpoint and register with the Matter node
  matterPump.begin();

  // Register the callback function to handle incoming ON/OFF control commands
  matterPump.onChange(onPumpStateChange);

  Serial.println("Waiting for Matter commissioning / network ready...");

  // Check whether the device has already been paired with a Matter fabric.
  // If not, print setup instructions and the 11-digit manual pairing code.
  if (!Matter.isCommissioned()) {
    Serial.println("Device is not yet commissioned into a Matter fabric.");
    Serial.println(
        "Open Google Home -> 'Devices' -> '+ Add' -> 'Matter-enabled device'");
    Serial.printf("Manual Setup Code: %s\n",
                  Matter.getManualPairingCode().c_str());
  }
}

// ==============================================================================
// Arduino Main Loop: Continuous Safety Watchdog
// ==============================================================================
void loop() {
  // Real-time emergency shut-off watchdog:
  // Continuously monitors if the water runs out while the pump is actively running.
  // Condition: Relay is active (LOW) AND float switch signals low water level (HIGH).
  if (digitalRead(RELAY_PIN) == LOW && digitalRead(FLOAT_SWITCH_PIN) == HIGH) {
    Serial.println("EMERGENCY SHUT-OFF: Water level dropped below minimum threshold!");
    // Immediately turn off the pump to prevent pump damage from running dry
    digitalWrite(RELAY_PIN, HIGH);
    // Update the Matter endpoint state so Google Home reflects the emergency shutdown
    matterPump.setState(false);
  }

  // Short delay to avoid tight polling loop and yield CPU time to background tasks
  // (such as FreeRTOS Wi-Fi, BLE, and Matter protocol handling threads)
  delay(200);
}