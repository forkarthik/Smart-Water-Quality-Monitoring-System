# IoT-Based Smart Water Quality Monitoring & Early-Warning System

An end-to-end IoT monitoring and early-warning node for rural and community water sources. Built around an **ESP32 microcontroller**, the system acquires, evaluates, and streams two physical-chemical screening parameters—**Total Dissolved Solids (TDS)** and **Turbidity**—benchmarked against **WHO** and **Bureau of Indian Standards (IS 10500:2012)** potable water guidelines.

Real-time telemetry is published over Wi-Fi to **ThingsBoard Community Edition**, visualized on an interactive web dashboard hosted on **GitHub Pages**, and actuated locally through a **16×2 I2C LCD**, **high-visibility warning LED**, and a **transistor-driven acoustic buzzer**.

---

## 1. System Architecture & End-to-End Data Pipeline

```
[ Water Source / Well / Distribution Tank ]
             │
             ├── Analog TDS Probe (Ion Conductivity - GPIO 34)
             └── Optical Turbidity Probe (Light Scattering - GPIO 35 via 10k/22k divider)
             │
             ▼
         [ ESP32 ]
      • 12-bit Calibrated Multisampling ADC (ADC1)
      • 16×2 I2C Local Screen (Auto-detecting 0x27 / 0x3F)
      • Visual Alert LED (GPIO 25) + 2N2222 Buzzer Alarm (GPIO 19)
             │
             ▼ (Wi-Fi 802.11 b/g/n — HTTP POST JSON)
     [ ThingsBoard Cloud / CE ]
      • Server: http://demo.thingsboard.io
      • Telemetry Ingestion: {"tds": 285.4, "turbidity": 1.2, "status": "NORMAL"}
      • Long-Term Timeseries Database & Rule Engine
             │
             ▼ (Tokenized REST Telemetry API)
     [ Web Dashboard on GitHub Pages ]
      • Interactive Chart.js Graphs (1h, 6h, 24h, 7d, 30d)
      • Threshold Exceedance & Fault Event Logger
      • Client-side localStorage sandboxing
             │
             ▼
     [ Community Operator / Stakeholders ]
```

---

## 2. Hardware Implementation & Pin Allocation

| Component | Pin / Interface | ESP32 GPIO | Rationale & Circuit Protection |
| :--- | :--- | :--- | :--- |
| **ESP32 Dev Module** | Micro-USB | USB 5V Bus | 5V rail (VIN), 3.3V LDO, and 115200 baud Serial Monitor. |
| **Analog TDS Module** | VCC, GND, AOUT | `3V3`, `GND`, `GPIO 34` | Powered from 3.3V rail. Output range $\le 2.3\text{V}$, safely below ESP32 3.3V ADC limit. `GPIO 34` uses ADC1 (no Wi-Fi RF conflict). |
| **Turbidity Module** | VCC, GND, AOUT | `VIN (5V)`, `GND`, `GPIO 35` (via divider) | Requires 5V (VIN) for infrared emission. Scaled down by **10 kΩ / 22 kΩ divider** to $\le 3.09\text{ V}$. |
| **16×2 I2C LCD** | VCC, GND, SDA, SCL | `VIN (5V)`, `GND`, `GPIO 21` (SDA), `GPIO 22` (SCL) | Local real-time display. Firmware auto-scans and initializes address `0x27` or `0x3F`. |
| **Red Warning LED** | Anode (+), Cathode (-) | `GPIO 25` via **220 Ω**, `GND` | Visual alert indicator. 220 Ω limits forward current to $\sim 7\text{ mA}$. |
| **Acoustic Buzzer** | (+), (-) | `VIN (5V)`, 2N2222 Collector | Switched by transistor to prevent drawing excessive current from ESP32 pin. |
| **2N2222 NPN Transistor**| Base, Collector, Emitter | Base via **1 kΩ** to `GPIO 19`, Emitter to `GND`, Collector to Buzzer (-) | Saturated switch ($I_B \approx 2.6\text{ mA}$). |

---

## 3. Circuit Schematics

### A. Turbidity Sensor Voltage Divider
```
Turbidity Module AOUT (0 - 4.5V)
       │
      [ ] R1 (10 kΩ)
       │
       ├───────────────> ESP32 GPIO 35 (ADC1_CH7)  [Max Voltage: 4.5V * (22 / 32) = 3.09V]
       │
      [ ] R2 (22 kΩ)
       │
      GND (Common Ground)
```

### B. Buzzer Transistor Driver
```
ESP32 VIN (5V) ────────(+) Buzzer (-)
                             │
                             │ (Collector)
ESP32 GPIO 19 ──[ 1 kΩ ]──(Base) 2N2222
                             │ (Emitter)
                             └───> Common GND
```

---

## 4. Multi-Tier Early-Warning Logic

| Status | TDS Threshold | Turbidity Threshold | Local Alert State | Dashboard Indication |
| :--- | :--- | :--- | :--- | :--- |
| **SAFE (Normal)** | $\le 300\text{ ppm}$ | $\le 1.0\text{ NTU}$ | LED OFF, Buzzer OFF, LCD: `[SAFE]` | Green Badge, Desirable Baseline |
| **CAUTION (Elevated)** | $300 - 500\text{ ppm}$ | $1.0 - 5.0\text{ NTU}$ | LED Slow Blink, Buzzer OFF, LCD: `[WARN]` | Amber Badge, Filtration Advised |
| **ALERT (Critical)** | $> 500\text{ ppm}$ | $> 5.0\text{ NTU}$ | LED Solid ON, Buzzer Pulsing, LCD: `[ALRT]` | Red Badge, Water Treatment Required |

---

## 5. Repository File Structure

```
.
├── README.md                            # Complete Project Manual & Architectural Overview
├── CIRCUIT_AND_TESTING_GUIDE.md         # Circuit Schematics, Calibration Guide & Viva Tips
├── WaterQualityNode.ino                 # Master Arduino Sketch
├── WaterQualityNode/
│   └── WaterQualityNode.ino             # Arduino-IDE-standard sketch folder
├── tests/                               # Standalone Isolated Unit Tests
│   ├── 00_i2c_scanner/                  # I2C bus auto-detector (0x27 / 0x3F)
│   ├── 01_lcd_test/                     # 16x2 LCD backlight & text test
│   ├── 02_alerts_test/                  # LED and 2N2222 Buzzer driver test
│   ├── 03_tds_test/                     # Raw TDS ADC & voltage monitor
│   └── 04_turbidity_test/               # Raw Turbidity divider monitor
└── water-quality-dashboard/             # Web Application (GitHub Pages)
    ├── index.html                       # Responsive HTML5 layout
    ├── style.css                        # Glassmorphic dark theme & animations
    ├── script.js                        # Telemetry ingestion, Chart.js & auth engine
    └── README.md                        # Dashboard documentation
```

---

## 6. How to Flash the ESP32 via Arduino IDE

1. Open Arduino IDE and open `WaterQualityNode/WaterQualityNode.ino`.
2. Install the **`LiquidCrystal I2C`** library by Frank de Brabander via Library Manager (`Ctrl + Shift + I`).
3. Select your board: **Tools $\to$ Board $\to$ esp32 $\to$ ESP32 Dev Module**.
4. Select your port: **Tools $\to$ Port $\to$ COMx**.
5. Set Wi-Fi credentials in the code:
   ```cpp
   const char* WIFI_SSID     = "forkarthik";
   const char* WIFI_PASSWORD = "YOUR_PASSWORD";
   ```
6. Click **Upload ($\rightarrow$)**.
   *(If `Connecting........_____.....` appears, press and hold the physical **BOOT** button on the ESP32 for 2 seconds).*
7. Open **Serial Monitor** at **115200 baud** to view real-time diagnostics.

---

## 7. Important Scientific Notice (Project Defense / Viva)

> [!IMPORTANT]
> **Physical-Chemical Screening vs. Microbiological Potability:**
> TDS measures dissolved ionic conductivity ($\text{Ca}^{2+}$, $\text{Mg}^{2+}$, salts). Turbidity measures light attenuation caused by suspended particles (silt, mud, clay).
> **Neither sensor can detect microbiological pathogens (*E. coli*, cholera, viruses) or heavy metal toxicity.**
> This system is an **automated first-line early-warning screening tool** to detect pipe bursts, sediment breaches, runoff, and salinization, flagging when immediate laboratory testing or filtration is required.
