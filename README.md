
# Crane IoT Monitoring System

##  Overview
This project is an **IoT-based monitoring system** for a crane. It tracks:
- Crane running status (ON/OFF)
- Productivity status (working/not working)
- Maintenance flag (when the crane is under maintenance)
- Failure flag (when the operator reports a fault)
- Additional telemetry: oil pressure, temperature, and work hours.

The data is stored locally on the ESP8266 (NodeMCU) and sent to a **Google Sheets** dashboard **on request only** — for accurate KPIs and easy reporting.

---

## ⚙️ How It Works

1. **Inputs**
   - Digital inputs for:
     - Crane Running Status
     - Working Status
     - Maintenance Button
     - Failure Button
     - Manual Save Button
   - Analog inputs (or sensors) for:
     - Oil Pressure
     - Temperature

2. **Processing**
   - ESP calculates time intervals daily: total ON time, working time, failure time, maintenance time.
   - Maintenance tasks tracked:
     - Oil & Filter Change every **250 hours**
     - Air Filter Replacement every **500 hours**
     - Fuel Filter Replacement every **1,000 hours**
   - Warnings appear in the Google Sheet when these limits are reached.

3. **Data Request & Sync**
   - The ESP connects to Wi-Fi via hotspot/router.
   - When the Google Sheet (via Google Apps Script) requests data, the ESP uploads:
     - Daily, weekly, and monthly stats.
     - Live telemetry.
   - Data is **not streamed continuously** — only on request.

4. **Persistence**
   - Data is saved to EEPROM every **hour** or manually by:
     - Pushing the manual save button.
     - Toggling a cell value in the Google Sheet.
   - This prevents data loss on power failure.

5. **Resetting Maintenance**
   - After performing maintenance, the engineer resets maintenance flags via the Google Sheet.

---

##  Components

- **ESP8266 NodeMCU**
- Relay/Contact inputs from crane control panel
- Push buttons for failure & maintenance flags
- Sensors for temperature & oil pressure
- Mobile hotspot or Wi-Fi router

---

##  Google Sheets Web App

- Implemented using **Google Apps Script (`.gs`)**
- Acts as an HTTPS endpoint.
- Receives JSON data from ESP.
- Updates the Google Sheet.
- Can send a **request flag** so the ESP knows when to send data.
- Resets maintenance flags.


---

##  Getting Started

1. **ESP Setup**
   - Flash `CraneMonitor.ino` to your NodeMCU.
   - Set your Wi-Fi SSID & password.
   - Connect input pins & sensors.

2. **Google Sheets**
   - Create a Google Sheet.
   - Open **Extensions > Apps Script**.
   - Paste the content of `GoogleWebApp.gs`.
   - Deploy as **Web App** with access to `Anyone`.
   - Copy the web app URL and update it in `CraneMonitor.ino`.

3. **Test It**
   - Open the Google Sheet.
   - Toggle the request cell to `1` to fetch live data.
   - View live status, KPIs, and maintenance flags.

---

## 📊 KPIs You Can Calculate

- Daily productivity % (working time / running time)
- Mean Time Between Failures (MTBF)
- Mean Time To Repair (MTTR)
- Maintenance compliance

---

##  Future Improvements

- Add MQTT broker for hybrid real-time + on-request updates.
- Add historical charts in Google Data Studio.
- Add OTA firmware updates.
- Integrate email/SMS alerts for maintenance due.

---
