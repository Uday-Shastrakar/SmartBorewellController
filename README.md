SmartBorewellController

Smart Borewell Pump Controller using Arduino Nano with USB-C. Features multi-floor AC voltage detection with PC817 optocouplers, relay-based pump control, motor safety, and timer selection (15/30/60 min) via push buttons. Includes LEDs and buzzer for status, alerts, and emergency stop. Safe, reliable, and easy to use for automated pump management.

<p align="center"> Automated borewell pump controller with multi-floor AC monitoring, motor protection, and timer control using Arduino Nano. </p>
Project Description

This project uses an Arduino Nano with USB Type-C to control a borewell pump safely and efficiently.
It detects AC presence from multiple floors via PC817 optocouplers, manages pump switching through relays, and allows the user to run the pump on a selectable timer (15, 30, 60 minutes).

The system includes motor safety detection, buzzer alerts, and an emergency stop feature for reliable and safe operation.

Features

AC voltage detection on Ground Floor and First Floor using PC817 optocouplers

Relay control for pump operation on both GF and FF lines

Timer selection (15 / 30 / 60 minutes) via push buttons

Timer LEDs show active countdown

Audible alerts:

Short tone on button press

Pump ON buzzer notification

Emergency stop buzzer alert

Motor safety: If motor AC is not detected, pump relay is turned off automatically

Emergency Stop: Hold 15 min button for 10 sec → Pump OFF, red LED pulses, buzzer alert

Safe isolation between mains AC and Arduino via optocouplers

Compact design using Arduino Nano with USB-C

Circuit Overview

AC lines are monitored with PC817 optocouplers and high-value current limiting resistors.

Relay modules switch the Live (Phase) supply to the borewell pump.

Buttons allow user to select run-time.

LEDs and buzzer provide visual and audible feedback.

Motor safety optocoupler ensures the relay shuts off if motor is not detected running.

Wiring Summary
Component	Arduino Nano Pin	Purpose
PC817 Optocouplers	D2 (FF), D3 (GF), A3 (Motor)	AC detection inputs
Relay Module	D7 (FF Relay), D8 (GF Relay)	Pump control relays
Push Buttons	D4 (15 min), D5 (30 min), D6 (60 min)	Timer selection
Timer LEDs	D9 (15 min), D10 (30 min), D11 (60 min)	Show active timer
Status LEDs	D12 (Blue = FF AC), A0 (Yellow = GF AC), A1 (Red = Alert), D13 (White = Pump ON)	System status
Buzzer	A2	Alerts and notifications
How to Use

Connect all components as per the wiring summary.

Upload the Arduino sketch to the Nano via USB-C.

Ensure all AC connections are safely insulated.

Press a button (15/30/60 min) → corresponding timer LED turns ON, pump starts.

Pump will automatically switch OFF after timer expires (relay OFF, LED OFF).

Pump ON triggers a buzzer notification and white LED indicator.

If motor AC is not detected, relay shuts off to protect the system.

For emergency stop → hold 15 min button for 10 seconds. Red LED pulses, buzzer sounds, pump turns OFF.

Safety Notes

Always use high-value resistors (470kΩ or more, ≥1W) with optocouplers for mains AC detection.

Keep Arduino and low-voltage electronics fully isolated from mains wiring.

Ensure relays are rated for the pump load (voltage/current).

Proper earthing and insulation is mandatory for safe operation.

Screenshots / Images

(Add wiring diagrams and actual setup photos here)

License

This project is licensed under the MIT License - see the LICENSE
 file for details.

Author

Uday Shastrakar, Software Engineer from Maharashtra, India

GitHub Profile

Email

Support

Contributions, issues, and feature requests are welcome!
If you find this project useful, please give it a ⭐️ on GitHub!
