# Smart Borewell Pump Controller

<div align="center">
  <img src="https://via.placeholder.com/600x200?text=Smart+Borewell+Pump+Controller" alt="Project Banner">
</div>

---

## 🔧 Project Overview

This project is an **Arduino-based borewell pump controller** that automates water pumping with safety features, timers, and AC phase monitoring. It is designed to:

- Control a submersible pump via **2-channel relays**.
- Monitor **AC power sources** (GF and FF) using **PC817 optocouplers**.
- Prevent motor damage using **motor safety checks**.
- Alert the user if **both AC sources are present simultaneously**.
- Provide **custom timer controls** (15, 30, 60 minutes).
- Include **emergency stop** functionality.

---

## 🛠 Hardware Components

| Component                  | Quantity | Notes |
|-----------------------------|---------|-------|
| Arduino Nano (C Type)       | 1       | 5V logic control |
| 2-Channel Relay Module 10A  | 1       | Active LOW control for pump |
| PC817 Optocoupler Module    | 2       | For AC detection |
| AC Rectification Circuit    | 3 ch    | Each channel: 150K 1W → DB107S Bridge → PC817 |
| Push Buttons (15/30/60 min) | 3       | Timer selection |
| LEDs (Red, Yellow, Blue, White) | 4 | Status indicators |
| Buzzer (Active or Passive) | 1       | Alerts & timer sounds |
| Jumper wires, resistors, PCB | -     | For connections |

---

## 📌 Pin Configuration

| Function                  | Pin          |
|---------------------------|-------------|
| Red LED (Alert)           | A1          |
| Buzzer                    | A2          |
| Yellow LED (GF AC)        | A0          |
| Blue LED (FF AC)          | 12          |
| White LED (Pump Pulse)    | 13          |
| Relay GF                  | 8           |
| Relay FF                  | 7           |
| Timer Button 15 min       | 4           |
| Timer Button 30 min       | 5           |
| Timer Button 60 min       | 6           |
| Timer LED 15 min          | 9           |
| Timer LED 30 min          | 10          |
| Timer LED 60 min          | 11          |
| Optocoupler GF AC         | 3           |
| Optocoupler FF AC         | 2           |
| Optocoupler Motor         | A3          |

---

## 🔌 AC Detection Circuit

Each AC source is connected as follows:

```text
AC Line (L) → 150K 1W Resistor → DB107S Bridge Rectifier → PC817 LED → AC Neutral (N)
The PC817 output pin is read by Arduino to detect AC presence (active LOW).

Ensures safe isolation between high voltage AC and Arduino logic.

⚡ Features & Behavior
Timer Control

Start pump for 15, 30, or 60 minutes using buttons.

Timer LEDs indicate the selected duration.

Timer can be restarted any time using the buttons.

Emergency Stop

Press 15-minute button for 7 seconds to trigger emergency stop.

Turns off all relays, stops pump, and pulses red LED.

Resets only after both AC sources are turned off.

Pump Motor Safety

Motor optocoupler checks if the pump is running after relays are activated.

If no motor detected in 5 seconds, relays turn OFF and wait 20 seconds before retry.

AC Power Alerts

If both GF and FF AC sources are present, system triggers alert buzzer and red LED.

Prevents pump operation in unsafe conditions.

White LED Pulse

Pulses while the pump is running to indicate operation visually.

Buzzer / Melody

Beeps on pump start, timer expiry, and emergency events.

Different melodies for GF, FF, and default alerts.

Serial Monitor Output

Provides real-time status of pump, timer, relays, AC detection, motor state, and emergency.

🖥 Software Behavior (Serial Monitor Example)
text
Copy code
Pump: ON | Timer Running: YES | Time Remaining: 14:55 | Selected Timer: 15
GF Relay: ON | FF Relay: OFF | optoGF: ON | optoFF: OFF | Motor Opto: ON | Emergency: NO
...
TIMER EXPIRED - Pump OFF
Pump: OFF | Timer Running: NO | Time Remaining: 0:00 | Selected Timer: 15
GF Relay: ON | FF Relay: OFF | optoGF: ON | optoFF: OFF | Motor Opto: OFF | Emergency: NO
📈 Workflow Diagram
<div align="center"> <img src="https://via.placeholder.com/600x400?text=Workflow+Diagram" alt="Workflow Diagram"> </div>
AC detection → confirm power → relay decision → motor check → pump timer → LED & buzzer updates.

Emergency stop and dual AC alerts override normal operation.

💻 Arduino Code
Written for Arduino Nano (C Type) using digitalRead, digitalWrite, analogWrite, tone, and EEPROM.

Handles all timers, relays, LEDs, motor safety, and buzzer melodies.

Code structure:

setup() → pin initialization & load timer from EEPROM

loop() → main logic, AC detection, relay control, timer, motor safety, alerts

Functions: handleButtons(), runPumpTimer(), handleMotorSafety(), handleWhiteLEDPulse(), handleMelody()

⚙ Usage Instructions
Connect the AC sources via rectification circuit to PC817 modules.

Connect relays to pump and Arduino pins as per table.

Upload Arduino code.

Power Arduino with 5V.

Select desired pump runtime using timer buttons.

Observe LEDs:

White → Pump running

Timer LEDs → Selected duration

Red → Alert / emergency

Stop pump immediately with emergency stop or let timer expire.

⚠ Safety Notes
Ensure isolation from AC mains using PC817 and resistors.

Never touch the AC rectification circuit when powered.

Use proper fuses and protective devices for 10A relays.

📂 File Structure (Recommended)
markdown
Copy code
SmartBorewellController/
│
├── borewellController.ino
├── README.md
├── circuit_diagram.png
├── workflow_diagram.png
└── images/
    └── pin_mapping.png
🎵 Buzzer Melodies
Event	Notes (Hz)
GF Relay Start	440, 554, 659
FF Relay Start	523, 659, 784
Timer Expired / Alert	392, 392, 330, 330

🏆 Conclusion
This Smart Borewell Pump Controller automates water pumping while ensuring:

Motor safety

AC source monitoring

Emergency stops

User-friendly timer controls

It’s modular and can be extended for IoT monitoring or remote notifications in the future.

<div align="center"> Made with ❤️ by Uday Shastrakar </div> ```
