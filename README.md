# Mockup VCU Specification (Arduino Uno + HW-184)

## Overview

Mock VCU node that reads driver inputs, applies state machine logic gated by BMS state, implements plausibility checks, manages power maps, and respects power limits from the BMS. Sends commands over CAN.

## Hardware

- Arduino Uno
- HW-184 CAN module (MCP2515 + TJA1050)

## Inputs

- 2x 10k potentiometers (gas pedal replicas), 2x buttons, 1x LED
- 1x PWM output (LED + resistor)
- 2x 120 ohm termination resistors, breadboard, wires, USB power

## Wiring Notes

- HW-184 uses SPI: SCK/MISO/MOSI plus CS and INT pins.
- CANH/CANL go to the bus; terminate only at the two ends.

---

## Feature 1: State Machine (Button Cycled, BMS-Gated)

- **States:** IDLE → READY_TO_DRIVE → ERROR (and back to IDLE on button press)
- **Logic:** Button press attempts state transition, but next state allowed only if BMS is in READY state (via CAN). Error state entered if button pressed while BMS in ERROR or if VCU detects implausible input (see below). In ERROR, gas request forced to zero until reset.
- **Behavior:**
  - IDLE: gas pedal request forced to zero.
  - READY_TO_DRIVE: gas pedal request passed to plausibility checker.
  - ERROR: gas pedal request forced to zero; button press resets to IDLE.

---

## Feature 2: Dual Gas Pedal Plausibility Check

- **Inputs:** Two 10k pots representing dual pedals (0–5V each).
- **Check:** If difference > 10%, mark plausible as false; request drops to zero.
- **Output:** Plausible request (0–100 %) or zero if failed.

---

## Feature 3: Power Maps & Switching

- **Maps:** ECO (logarithmic), NORMAL (linear), SPORT (exponential).
- **Button:** Cycles maps; LED indicates current map (different pattern or brightness).
- **Logic:** Plausible request scaled by map factor to produce mapped request.

---

## Feature 4: Power Limiter (BMS-Driven)

- **Input:** Power limit (watts) received from BMS via CAN.
- **Logic:** Clamp mapped request to not exceed the received power limit.
- **Output:** Final power request sent to PWM LED output and CAN.

---

## CAN RX (from BMS)

- **ID 0x100** at 10 Hz
  - Byte0: BMS state (0=IDLE, 1=READY, 2=ERROR)
  - Byte1-2: power limit (uint16, 1 W units)

## CAN TX (to BMS & logger)

- **ID 0x200** at 20 Hz
  - Byte0: VCU state (0/1/2)
  - Byte1-2: plausible gas request (uint16, step 0.1 %)
  - Byte3: map mode (0=ECO, 1=NORMAL, 2=SPORT)
  - Byte4-5: final power request (uint16, 1 W)

---
