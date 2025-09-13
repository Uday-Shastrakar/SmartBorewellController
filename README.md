SmartBorewellController
Smart Borewell Pump Controller using Arduino Nano with USB-C. Features multi-floor AC voltage detection with PC817 optocouplers, relay-based pump control, and timer selection (15/30/60 min) via push buttons. Includes LEDs and buzzer for status and alerts. Safe, reliable, and easy to use for automated pump management.

<p align="center"> Automated borewell pump controller with multi-floor AC monitoring and timer control using Arduino Nano. </p>
Project Description
This project uses an Arduino Nano with USB Type-C to control a borewell pump safely and efficiently. It features multi-floor AC voltage detection using PC817 optocouplers, relay-based pump switching, and selectable timer durations (15, 30, 60 minutes). Status and alerts are indicated by LEDs and a buzzer.

Features
AC voltage detection on Ground Floor and First Floor using PC817 optocouplers

Relay control for pump switching on multi-floor power lines

Timer selection via push buttons with on-screen LED status

Audible alert (buzzer) and visual status indicators (LEDs)

Safe isolation between mains AC and Arduino using optocouplers

Compact design using Arduino Nano with USB Type-C

Circuit Overview
The system monitors two AC lines via PC817 optocouplers with 470kΩ current-limiting resistors to safely detect AC presence. Relays switch the Live (Phase) wire to the pump on/off. Push buttons select timer duration controlling pump run time.

Wiring Summary
Component	Pin (Arduino Nano)	Purpose
PC817 Optocouplers	D2 (FF AC), D3 (GF AC)	AC voltage detection inputs
Relay Module INs	D7 (Relay 2), D8 (Relay 1)	Control relays for pumps
Push Buttons	D4 (60min), D5 (30min), D6 (15min)	Timer selection inputs
LEDs	D9, D10, D11 (Timer LEDs Green)	Timer status indicators
D12 (Blue LED, FF AC)	First Floor AC presence
A0 (Yellow LED, GF AC)	Ground Floor AC presence
A2 (Red LED, Alert)	Alert notifications
D13 (White LED, Pump ON)	Pump running status
Buzzer	A1	Audible alert
How to Use
Connect all components following the wiring diagram.

Upload the Arduino sketch to the Nano via USB-C.

Ensure AC mains connections are properly insulated and comply with safety standards.

Use push buttons to select pump run timer duration (15, 30, or 60 minutes).

The system detects AC power presence on floors and runs pumps accordingly with relay control.

LEDs and buzzer provide real-time status feedback and fault alerts.

Screenshots / Images
Include wiring schematics and photos here (optional)

License
This project is licensed under the MIT License - see the LICENSE file for details.

Author
Uday Shastrakar

uday.shastrakar@gmail.com

Support
Contributions, issues, and feature requests are welcome!
If you find this project useful, please give it a ⭐️ on GitHub!
