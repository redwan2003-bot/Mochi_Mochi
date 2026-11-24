# Monday Hardware Arrival & Caliper Verification Checklist

> **Purpose**: Physical bench verification protocol for Monday when your parcel arrives.
> **Tools Required**: Digital Vernier Caliper, Digital Multimeter (DMM), Soldering Station, 28-30 AWG Silicone Wire, Heatshrink ($2.5\text{ mm}$).

---

## 1. Package Unboxing & BOM Audit

Tick off each physical component from your parcel against this locked list:

- [ ] **1. ESP32-S3 SuperMini Development Board** (Native USB-C on GPIO19/20, 4MB Flash)
- [ ] **2. 1.3" Monochrome OLED Display** (SH1106, 4-Pin I²C: VCC, GND, SCL, SDA)
- [ ] **3. MPU6050 6-Axis Motion Sensor Module** (GY-521, 8 pins, INT on pin 8)
- [ ] **4. TTP223 Red Mini Capacitive Touch Module** (3 pins: VCC, GND, I/O)
- [ ] **5. INMP441 Omnidirectional I²S MEMS Microphone** (6 pins: VDD, GND, SD, L/R, WS, SCK)
- [ ] **6. MAX98357A I²S Mono Class-D Audio Amplifier** (VIN, GND, DIN, BCLK, LRC, GAIN, SD)
- [ ] **7. 4cm Thin Speaker** (RBD-2694, 8Ω, 0.25–0.5W, slim metal frame)
- [ ] **8. TP4056 USB-C Li-Ion Charger Board** (IN+, IN-, BAT+, BAT-, OUT+, OUT-)
- [ ] **9. Protected 1S 3.7V 500mAh LiPo Battery** (HCY 402535P with built-in PCM protection circuit)
- [ ] **10. 2-Pin 2A Magnetic Pogo Connector** (MG0225 / RBD-3143, 2.54mm pitch)
- [ ] **11. SPDT Micro Slide Switch** (SS-12D00 / SS12D10, 3 pins, 2.0mm travel)
- [ ] **12. 1N4148 Switching Diode** (Small glass axial diode with black cathode band)
- [ ] **13. 10 kΩ Resistor** (1/4W through-hole or 0805: Brown-Black-Orange-Gold)
- [ ] **14. 20 kΩ Resistor** (1/4W through-hole or 0805: Red-Black-Orange-Gold)
- [ ] **15. Silicone O-Ring / Foam Gasket** (For INMP441 acoustic port isolation)

---

## 2. Pre-Slice Caliper Verification (Caliper Measurements)

Before running final 3D prints or soldering, measure these batch-dependent dimensions with your digital caliper:

| Item | Critical Feature | Expected Blueprint Spec | Your Caliper Measurement | Tolerance / Fit Verdict |
|---|---|---|---|---|
| **TP4056 Charger** | PCB Length | $26.0\text{ mm}$ or $28.0\text{ mm}$ | `___.___ mm` | Enclosure bay is sized for $28.4\text{ mm}$ (fits both lengths). |
| **TP4056 Charger** | PCB Thickness | $1.0\text{--}1.6\text{ mm}$ | `___.___ mm` | Verify USB-C port aligns with side slot. |
| **GY-521 MPU6050** | M3 Corner Hole Pitch | $17.78\text{ mm}$ (8-pin row) | `___.___ mm` | Verify boss pitch in 3D printed cradle. |
| **HCY 402535P LiPo**| Pouch Dimensions | $4.0 \times 25.0 \times 37.0\text{ mm}$ | `___ x ___ x ___ mm` | Bay is $5.0 \times 26.2 \times 40.5\text{ mm}$ ($+15\%$ Z swelling allowance). |
| **RBD-2694 Speaker** | Outer Diameter & Depth | $\varnothing 40.0 \times 4.2\text{--}4.5\text{ mm}$ | `Ø___.___ x ___.___ mm` | Pocket is $\varnothing 40.5 \times 5.0\text{ mm}$ deep. |
| **MG0225 Pogo** | Flange Dimensions | $11.0\text{--}12.2 \times 3.6\text{--}4.2\text{ mm}$ | `___ x ___ mm` | Cutout is $12.3 \times 4.2\text{ mm}$ (R1.9 corners). |
| **SS-12D00 Switch** | Body & Actuator | Body $8.5 \times 3.6\text{ mm}$; Actuator $1.5\text{ mm}$ | `___ x ___ mm` | Pocket is $8.8 \times 4.3\text{ mm}$; slot $4.4 \times 2.2\text{ mm}$. |

---

## 3. Bench Multimeter Checks (Cold Test Before Power)

> [!CAUTION]
> Always perform these continuity checks with the battery **DISCONNECTED** and power switch **OFF**.

1. **Check 1: System 5V Rail to GND**:
   - DMM in Resistance ($\Omega$) mode. Probe between TP4056 `OUT+` and System `GND`.
   - **PASS**: Resistance $>20\text{ k}\Omega$ (decoupling caps charging).
2. **Check 2: 3.3V Regulated Rail to GND**:
   - Probe between ESP32-S3 `3V3` pin and `GND`.
   - **PASS**: Resistance $>10\text{ k}\Omega$.
3. **Check 3: LiPo Polarity Check**:
   - DMM in DC Voltage mode. Probe raw battery red lead ($+$) and black lead ($-$).
   - **PASS**: Reads between $+3.70\text{ V}$ and $+4.20\text{ V}$ DC. (Red is positive).
4. **Check 4: SPDT Power Switch Continuity**:
   - Switch in **ON** position: Pin 2 (Center) to Pin 3 (ON throw) measures $0.0\ \Omega$ (Short / Beep).
   - Switch in **OFF** position: Pin 2 to Pin 3 measures Open-Loop (`OL`).

---

## 4. Software & Firmware Pre-Flight

1. **Pre-Flashing without LiPo**:
   - Keep LiPo battery disconnected and power switch **OFF**.
   - Plug ESP32-S3 into your PC via USB-C data cable.
   - Run [`Flash_Firmware.bat`](../Flash_Firmware.bat) or open [esp.huhn.me](https://esp.huhn.me) in Chrome/Edge and upload [`tools/binaries/mochi_bot_v1_factory_merged.bin`](../tools/binaries/mochi_bot_v1_factory_merged.bin) at offset `0x0`.
2. **Serial Boot Verification**:
   - Watch the serial monitor at 115200 baud.
   - Confirm diagnostics report:
     - `I2C Bus: Probe OK (OLED at 0x3C, MPU6050 at 0x68)`
     - `I2S Bus: Initialized (Half-Duplex 16 kHz)`
     - `Sensors: Touch calibrated, Clap detector armed`
     - `BLE: Advertising as MochiBot-V1`
