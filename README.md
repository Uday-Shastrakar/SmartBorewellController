# Smart Borewell Controller

<p align="center">
  <img src="images/project_overview.png" alt="Smart Borewell Controller" width="400">
</p>

## Overview
The <strong>Smart Borewell Controller</strong> is an automated pump control system designed to manage submersible pumps efficiently while ensuring safety and monitoring AC power conditions. It uses an Arduino Nano, relays, optocouplers, and timer-based logic to automatically control pump operation, prevent electrical hazards, and provide alerts for abnormal conditions.


## Features
<ul>
  <li>Automatic detection of <strong>GF (Ground Floor)</strong> and <strong>FF (First Floor)</strong> AC power.</li>
  <li>Timer-based pump operation (15, 30, 60 minutes).</li>
  <li>Emergency stop using a long-press button.</li>
  <li>Motor running safety check using an optocoupler.</li>
  <li>Alerts for abnormal conditions:
    <ul>
      <li>Both AC lines live</li>
      <li>Motor not running after relay activation</li>
      <li>Timer expired</li>
    </ul>
  </li>
  <li>Visual indicators using LEDs:
    <ul>
      <li>Yellow LED → GF AC detected</li>
      <li>Blue LED → FF AC detected</li>
      <li>White LED → Pump running pulse</li>
      <li>Red LED → Emergency / alert</li>
    </ul>
  </li>
  <li>Buzzer alerts with customizable melodies.</li>
  <li>EEPROM storage of last timer selection.</li>
</ul>

---

## Hardware Requirements
<ul>
  <li>Arduino Nano (C-Type)</li>
  <li>2-Channel 10A Relay Module</li>
  <li>2-Channel PC817 Optocoupler Module</li>
  <li>3 AC detection channels using PC817</li>
  <li>Resistors: 150KΩ, 1W (one per AC detection channel)</li>
  <li>Bridge Rectifiers: DB107 or MB6M (one per AC detection channel)</li>
  <li>Submersible Pump</li>
</ul>

---

## Circuit Overview

### AC Detection Circuit (per channel)

AC Line (Live) → 150KΩ 1W resistor → Bridge Rectifier (DB107) → PC817 LED → AC Neutral
PC817 Output → Arduino Input (active-low)
PC817 Output LOW → AC present

PC817 Output HIGH → AC not present

Relay and Pump
<ul> <li>RelayGF → D8 → Controls GF pump line</li> <li>RelayFF → D7 → Controls FF pump line</li> <li>Relays are active-low: LOW = ON, HIGH = OFF</li> </ul>
LEDs
<table> <tr><th>LED</th><th>Pin</th><th>Function</th></tr> <tr><td>Yellow</td><td>A0</td><td>GF AC detection</td></tr> <tr><td>Blue</td><td>12</td><td>FF AC detection</td></tr> <tr><td>White</td><td>13</td><td>Pump running pulse</td></tr> <tr><td>Red</td><td>A1</td><td>Emergency / Both AC live alert</td></tr> <tr><td>Timer LED15</td><td>9</td><td>15 min timer selected</td></tr> <tr><td>Timer LED30</td><td>10</td><td>30 min timer selected</td></tr> <tr><td>Timer LED60</td><td>11</td><td>60 min timer selected</td></tr> </table>
Buttons
<table> <tr><th>Button</th><th>Pin</th><th>Function</th></tr> <tr><td>Button15</td><td>4</td><td>Select 15-min timer / Emergency stop long press</td></tr> <tr><td>Button30</td><td>5</td><td>Select 30-min timer</td></tr> <tr><td>Button60</td><td>6</td><td>Select 60-min timer</td></tr> </table>
Motor Detection
PC817 detects whether the pump motor is running.

Safety logic disables relays if motor does not start after 5 seconds.

Code Behavior
<ul> <li><strong>AC Detection:</strong> Reads optocoupler states with averaging for stability.</li> <li><strong>Relay Control:</strong> Based on confirmed AC power and timer logic.</li> <li><strong>Timer Logic:</strong> Starts when pump runs, turns pump off on expiry.</li> <li><strong>Emergency Stop:</strong> Button 15 long press triggers immediate shutdown.</li> <li><strong>Motor Safety:</strong> Stops relays if motor not detected after 5 seconds.</li> <li><strong>Buzzer / Melody:</strong> Alerts for pump start, timer expiry, motor missing, and both AC live.</li> <li><strong>White LED Pulse:</strong> Indicates pump is running.</li> <li><strong>EEPROM Storage:</strong> Saves last timer selection.</li> <li><strong>Serial Monitor:</strong> Prints detailed status every 1 second for debugging.</li> </ul>
Buzzer / Melody Behavior
<table> <tr><th>Event</th><th>Melody / Beeps</th></tr> <tr><td>Pump Start</td><td>1 short beep</td></tr> <tr><td>Timer Expiry</td><td>3 short beeps</td></tr> <tr><td>Motor Missing</td><td>3 short beeps</td></tr> <tr><td>Both AC Live</td><td>Continuous 2kHz alert</td></tr> <tr><td>GF Relay Start</td><td>440, 554, 659 Hz</td></tr> <tr><td>FF Relay Start</td><td>523, 659, 784 Hz</td></tr> <tr><td>Default Melody</td><td>392, 392, 330, 330 Hz</td></tr> </table>
Installation
<ol> <li>Connect hardware according to the <strong>Circuit Overview</strong>.</li> <li>Upload <code>borewellController.ino</code> to Arduino Nano using Arduino IDE.</li> <li>Open Serial Monitor (9600 baud) for debug information.</li> <li>Verify LEDs and buzzer respond correctly before connecting the pump.</li> </ol>
Safety Notes
<ul> <li>Ensure all AC connections are <strong>insulated and safe</strong>.</li> <li>Use a <strong>fuse or circuit breaker</strong> on pump AC line.</li> <li>Resistors may get warm in prolonged AC operation; use 1W rating.</li> <li>Always test with low-power loads before connecting the pump.</li> </ul>
Future Enhancements
<ul> <li>LCD/OLED display for timer and status.</li> <li>IoT integration for remote monitoring.</li> <li>Adjustable motor check duration and timer.</li> <li>Battery backup with solar integration.</li> </ul>
Author
<p> <strong>Uday Shastrakar</strong><br> Email: <a href="mailto:uday.shastrakar@gmail.com">uday.shastrakar@gmail.com</a><br> GitHub: <a href="https://github.com/Uday-Shastrakar">https://github.com/Uday-Shastrakar</a><br> Location: Pune, Maharashtra, India </p>
License
This project is open-source and free to use.


