# Mochi Bot V1 — Mechanical Assembly, 3D Stacking & Harness Guide

> **Pre-Flight Hardware Assembly & Verification Document**  
> *Target Completion Date: Ready for Physical Assembly on Monday*  
> *Author: Subagent 5 — Hardware Stacking & Mechanical Assembly Verifier*  
> *Electrical Reference: [HARDWARE_AND_PINMAP.md](file:///C:/Users/Redwan%20Ahmmed/.gemini/antigravity/scratch/mochi-bot/docs/HARDWARE_AND_PINMAP.md) | Wiring Layout: [mochi_wiring_diagram.html](file:///C:/Users/Redwan%20Ahmmed/.gemini/antigravity/scratch/mochi-bot/docs/mochi_wiring_diagram.html)*

---

## 1. Executive Summary & Design Constraints

Mochi Bot V1 is an ultra-compact expressive desk pet, portable bag charm, and auxiliary telemetry monitor. Because the complete system integrates an **ESP32-S3 SuperMini**, an **SH1106 1.3" OLED**, a **6-DOF MPU-6050 IMU**, a **TTP223 capacitive touch pad**, an **INMP441 MEMS microphone**, a **MAX98357A Class-D audio amplifier**, an **8Ω miniature speaker**, a **TP4056 LiPo charger**, and a **1S 3.7V 500mAh LiPo battery** into a handheld enclosure (~46mm × 46mm × 36mm), meticulous internal packaging, acoustic separation, and wiring preparation are mandatory before any power is applied.

```
       +-------------------------------------------------------+
       |                  TOP / FRONT FACE                     |
       |  [ 1.3" OLED Display (SH1106) ]  [ TTP223 Touch Pad ] |
       |  [ INMP441 Acoustic Port Pinhole (1.2mm Front Mic) ]  |
       +-------------------------------------------------------+
       |                    INTERNAL MID-FRAME                 |
       |  [ ESP32-S3 SuperMini ]  [ MPU-6050 ]  [ MAX98357A ]  |
       +-------------------------------------------------------+
       |                   BOTTOM / REAR BASE                  |
       |  [ 1S 3.7V 500mAh LiPo ] [ TP4056 Charger Module ]    |
       |  [ 8Ω Sealed Speaker Chamber ] [ 2-Pin Magnetic Dock ]|
       +-------------------------------------------------------+
```

---

## 2. Internal 3D Stacking & Component Arrangement

To eliminate physical interference, electromagnetic noise injection, and thermal runaway, components are arranged in three dedicated physical planes:

```
TOP / FRONT
  ▲
  │   [Tier 1: Front Bezel & User Interface]
  │   ├── SH1106 1.3" OLED (35.4mm x 33.5mm x 4.0mm)
  │   ├── TTP223 Touch Pad (Mounted directly behind top shell)
  │   └── INMP441 MEMS Mic (Front-facing with EVA/silicone gasket)
  │
  │   [Tier 2: Mid-Frame Compute & Signal Carriage]
  │   ├── ESP32-S3 SuperMini (22.5mm x 18.0mm x 3.5mm)
  │   ├── MPU-6050 GY-521 (Rigidly coupled to chassis mid-plane)
  │   └── MAX98357A I2S Class-D Amp (Isolated from mic wiring)
  │
  │   [Tier 3: Base Power & Acoustic Enclosure]
  │   ├── 1S 500mAh LiPo Battery Pouch (30mm x 25mm x 6.5mm)
  │   ├── TP4056 Linear Charger Module (Mounted with heatsink vent)
  │   ├── 8Ω Micro-Speaker inside isolated 2.2cm³ acoustic cup
  │   └── 2-Pin Magnetic Dock Base with polarity-keyed N-S magnets
  ▼
BOTTOM / REAR
```

### 2.1 Tier 1: Front Layer (Display, Touch, and Acoustic Port)
1. **1.3" SH1106 OLED Display (128×64 Monochrome)**:
   - **Mounting**: Front bezel pressure-fit against internal 1.0mm registration ribs, secured with four M1.4 × 3mm self-tapping screws or B-7000 adhesive perimeter.
   - **Clearance**: The glass front sits flush behind a 0.8mm acrylic display window.
   - **Header Clearance**: Pre-desolder vertical pin headers and solder flexible 30 AWG wires directly to the 4 solder pads (`VCC`, `GND`, `SCL`, `SDA`) bent 90° flush to the back of the OLED PCB to save 6mm of Z-depth.
2. **TTP223 Capacitive Touch Sensor**:
   - **Mounting**: Mounted flush against the inside surface of the top enclosure dome ("forehead / pet spot").
   - **Coupling**: Fastened with double-sided 3M 300LSE acrylic conductive-free adhesive tape. No air gaps are allowed between the TTP223 capacitive sensor pad and the 1.2mm plastic shell wall.
   - **Configuration**: Jumper A (Active Level) = OPEN (Active HIGH); Jumper B (Trigger Mode) = OPEN (Momentary switch mode).

### 2.2 Tier 2: Middle Layer (Compute & Sensor Mid-Frame)
1. **ESP32-S3 SuperMini**:
   - **Mounting**: Centered on an internal sliding mid-chassis tray.
   - **USB-C Port Placement**: Directed towards the lower-rear bezel cutout. The USB-C port is accessible for native USB flashing when removed from the dock, but mechanically protected against simultaneous connection while docked.
   - **Antenna Zone**: The 2.4 GHz ceramic chip antenna sits in an overhang cutout in the mid-frame, at least 4.0mm away from any battery foil, speaker magnets, or copper traces.
2. **MPU-6050 (GY-521 6-DOF IMU)**:
   - **Mounting**: Rigid mechanical fastening to the structural mid-frame using two M2 screws and high-durometer standoffs. No soft foam or silicone tape may be used here, as mechanical compliance attenuates sudden motion triggers and dampens wake interrupts.
   - **Orientation**: X-axis lateral (left/right), Y-axis longitudinal (front/rear), Z-axis normal (up/down).
   - **Configuration**: `AD0` pin tied directly to `GND` to select I2C address `0x68`.
3. **MAX98357A I2S Class-D Amplifier**:
   - **Mounting**: Mounted on the mid-frame perimeter, positioned closest to the speaker chamber.
   - **Thermal Clearance**: Class-D amplifier generates localized heat during loud playback. Maintain a 2.0mm air gap around the IC package.

### 2.3 Tier 3: Bottom Layer (Battery, Charging & Acoustic Chamber)
1. **1S 3.7V 500mAh LiPo Battery Pouch**:
   - **Placement**: Low-center-of-gravity placement above the dock base.
   - **Safety Cushioning**: Sandwiched between two 1.0mm fire-retardant EVA foam sheets to prevent puncture from board solder points or structural ribs.
   - **Thermal Separation**: Separated from the TP4056 charging IC by a 1.5mm polycarbonate thermal shield barrier to prevent heat transfer during constant-current charging.
2. **TP4056 Charger Module**:
   - **Placement**: Bottom rear corner. Charging status LED (Red pin 7) aligned with the rear optical light pipe.
   - **Current Limit**: Verify the `BOOST` jumper is open and Rprog is configured for ~100mA charge current (0.2C safe rate for 500mAh LiPo).
3. **SPDT Micro Slide Switch (Main Battery Power Cut-Off)**:
   - **Chassis Cutout**: Positioned on the lower side or bottom shell adjacent to the USB-C port. Slot dimension: 8.6mm × 3.6mm with 1.5mm actuator travel. The slider knob sits slightly recessed (0.5mm) to prevent accidental switching when carried as a bag charm.
   - **Wiring**: Single-pole double-throw switch wired in series with the LiPo battery positive (`BAT+`) line:
     - **Pin 2 (Center Common)**: Solder the red positive lead from the 500mAh LiPo pouch.
     - **Pin 3 (ON Throw)**: Solder the switched red lead to the TP4056 `BAT+` pad.
     - **Pin 1 (OFF / NC Throw)**: Trim flush or leave open.
   - **Insulation**: Each soldered terminal must be individually covered with 2.5mm polyolefin heatshrink tubing (matching the verified build reference photo) to prevent any contact with the LiPo foil or chassis.
   - **Function**: True physical power isolation for zero quiescent drain during long-term storage, and guaranteed zero-power cold isolation when flashing firmware over USB-C.
4. **Miniature 8Ω 1W-2W Speaker**:
   - Located inside a custom sealed enclosure at the lower rear of the chassis, firing downwards/sideways.

### 2.4 Flush Top-Right Lanyard Retention Tunnel (Zero Protrusion)
To enable carrying Mochi Bot as a bag charm, neck pendant, or keychain without adding protruding external eyelets that snag in pockets or collide with desk docks:
- **Geometry**: A curved $\varnothing 3.5\text{ mm}$ internal channel carved through the top-right corner of the rear shell half at $Z = 15.5\text{ mm}$.
- **Coordinates**: Connects the top outer edge ($X = 43.5\text{ mm}, Y = 0$) to the right outer edge ($X = 49.0\text{ mm}, Y = 5.5\text{ mm}$).
- **Load-Bearing Bridge**: Forms an internal $3.0\text{ mm}$ thick corner retention bridge. Slicing with 5 perimeters / 100% infill in this corner zone provides $>15\text{ kg}$ tensile drop-and-pull strength.
- **Universal Dock Clearance**: 100% flush within the perimeter chamfer; drops seamlessly into all three minimalist docks (River-Stone Pebble, Folded Ribbon Loop, and Bauhaus Plinth) with zero mechanical interference.

---

## 3. Acoustic Chamber Isolation (Critical for INMP441 + Speaker)

The Mochi Bot relies on clap detection implemented in firmware via peak energy and transient rise time analysis. If acoustic energy from the internal speaker back-pressure leaks into the microphone chamber, an unrecoverable **acoustic feedback loop** will occur (the bot plays a chirp, hears itself, detects a "clap", and chirps indefinitely).

```
   [ FRONT EXTERIOR ]                         [ REAR / BOTTOM EXTERIOR ]
          │                                               │
          ▼ 1.2mm Front Pinhole                           ▼ Bottom/Side Grille
   +-------------+                                 +---------------+
   | Front Shell |                                 | Speaker Grille|
   +------+------+                                 +-------+-------+
          │                                                │
   +------▼---------------------+                  +-------▼-------+
   | Closed-Cell Silicone O-Ring|                  | Acoustic Cup  |
   | Gasket (Hermetic Seal)     |                  | (Sealed 2.2cc)|
   +------+---------------------+                  | +-----------+ |
          │                                        | | 8Ω Micro- | |
   +------▼---------------------+                  | |  Speaker  | |
   | INMP441 MEMS Microphone    |                  | +-----------+ |
   | (Acoustic Port Inward-Seal)|                  +---------------+
   +----------------------------+                          │
          ▲                                                │
          └─────────── PHYSICAL AIR-TIGHT WALL ────────────┘
                  (NO INTERNAL CHASSIS SOUND PATH)
```

### 3.1 Front Pinhole vs. Rear/Bottom Speaker Grille
1. **Microphone Aperture**:
   - Pinhole diameter: **1.0mm to 1.2mm** drilled through the front-facing shell.
   - Chamfer: 45° outer bevel (0.3mm) to reduce high-frequency air turbulence and port whistling.
2. **Speaker Grille**:
   - Positioned on the **bottom/rear face**, perpendicular (90° to 180° offset) to the microphone axis.
   - Minimum open area: 28% perforation ratio to eliminate acoustic throttling and port compression.

### 3.2 Hermetic MEMS Microphone Isolation Gasket
1. **Gasket Material**:
   - High-density closed-cell silicone rubber or die-cut EVA foam (Shore A durometer 30–40).
   - Dimensions: Outer Diameter 6.0mm, Inner Hole 1.5mm, Uncompressed Thickness 1.5mm.
2. **Installation Procedure**:
   - Place the silicone gasket over the 1.2mm chassis front port.
   - Press the INMP441 PCB acoustic port firmly against the gasket until it compresses by **30% to 35%** (to ~1.0mm thickness).
   - Secure the INMP441 carrier board using two M1.4 retention screws or a rigid mechanical clamp.
   - **Verification**: Visually inspect under magnification that the gasket has not buckled and completely blocks all internal chassis air from reaching the front port.

### 3.3 Dedicated Speaker Back-Chamber (Acoustic Cup)
1. **Acoustic Back-Volume**:
   - The speaker must **never** be mounted naked into the main electronics cavity.
   - A dedicated 3D-printed acoustic cup of **2.0 cm³ to 2.5 cm³** net volume must house the rear of the speaker cone.
2. **Sealing the Perimeter**:
   - Apply a continuous bead of B-7000 flexible adhesive or silicone RTV along the speaker mounting flange before seating it into the cup.
   - Seal the speaker wire exit port: Once the twisted 28 AWG speaker wires pass through the chamber wall, seal the pass-through hole with a drop of UV-cure resin or neutral-cure silicone.
3. **Decoupling from Chassis**:
   - Mount the acoustic cup into the outer shell with thin 0.5mm silicone vibration damping rings to eliminate structure-borne mechanical conduction to the MPU-6050 accelerometer and microphone PCB.

---

## 4. 2-Pin Magnetic Dock Cradle Specifications

The dock cradle serves as Mochi’s ambient home station, providing continuous 5.0V DC regulated power while maintaining reliable electrical and mechanical interlocks.

```
                  FRONT VIEW - CRADLE MATING INTERFACE
         +-----------------------------------------------------+
         |              [ MECHANICAL BEZEL LOCKOUT ]           |
         |         (Blocks USB-C Cable during Dock Seating)    |
         |                                                     |
         |     [ N ]           ( +5V )     ( GND )       [ S ] |
         |   Neodymium        Pogo Pin 1  Pogo Pin 2   Neodymium|
         |    Magnet 1                                 Magnet 2 |
         +-----------------------------------------------------+
```

### 4.1 Magnetic Polarity Pairing (Keyed Anti-Reverse)
To physically prevent reverse (180° rotated) dock seating, a two-magnet polarized arrangement is used:

| Location | Left Magnet Pole (Facing Out) | Right Magnet Pole (Facing Out) | Effect |
|---|---|---|---|
| **Mochi Bot Base** | **NORTH (N)** | **SOUTH (S)** | Normal forward seating: Attraction |
| **Dock Cradle** | **SOUTH (S)** | **NORTH (N)** | Backwards insertion: **Repulsion (>3.0N)** |

> [!IMPORTANT]
> If a user attempts to dock Mochi backwards, the magnetic repulsion immediately pushes the bot out of the cradle. Accidental polarity reversal on the electrical contacts is physically impossible.
> - **Magnet Specification**: N52 Neodymium disc magnets, 6.0mm diameter × 3.0mm thickness, nickel-plated.
> - **Retention**: Press-fit into cylindrical pockets and locked with medium CA (cyanoacrylate) adhesive.

### 4.2 Spring-Loaded Pogo Pin Specifications
- **Contact Pin 1**: Regulated +5.0 V DC (from automotive 5V USB adapter or desktop USB hub).
- **Contact Pin 2**: Common System Ground (GND return).
- **Pitch**: 4.0mm center-to-center.
- **Stroke & Travel**:
  - Full extended uncompressed height: **6.0mm**.
  - Working compressed height: **4.2mm to 4.5mm** (Nominal compression: **1.5mm to 1.8mm**).
  - Maximum compression limit: 4.0mm (2.0mm stroke).
  - Contact force: **65g ± 15g** per pin at working height. This force ensures contact resistance <25 mΩ without overcoming the magnetic hold-down force (estimated at ~400g combined attraction).
- **Mating Contacts on Bot Base**:
  - Gold-plated brass target rivets or flat ENIG PCB pads (3.2mm diameter) slightly recessed (0.3mm) inside guide bezels for self-centering.

### 4.3 Physical USB-C Mechanical Lockout Bezel
The electrical safety architecture strictly prohibits simultaneous connection of USB-C and dock power:
1. **Cradle Enclosure Interlock**:
   - The 3D-printed dock cradle features an elevated perimeter lip (bezel shield, height: 12.0mm) that snugly wraps around the lower rear shell of Mochi.
   - When seated in the cradle, this lip physically covers and completely blocks the USB-C port opening.
2. **Cable Interlock**:
   - If a standard USB-C cable is plugged into Mochi for firmware flashing or debugging, the overmold collar of the cable connector extends 8mm beyond the bot chassis.
   - This prevents Mochi from seating into the dock cradle pocket; the pogo pins remain separated by >6mm.

---

## 5. Soldering & Harness Preparation Guide

Clean, organized wire harnesses prevent pinched insulation, ground loops, and signal cross-talk inside the compact shell.

### 5.1 Recommended Wire Gauges and Color Standards

| Signal Group | Recommended Wire Type & Gauge | Insulation Color | Purpose / Destination |
|---|---|---|---|
| **Primary 5V / Battery** | 24 AWG High-Strand Flexible Silicone | **Red** | LiPo B+, TP4056 OUT+, S3 5V in, MAX98357A VIN |
| **Dock +5V Input** | 24 AWG High-Strand Flexible Silicone | **Orange** | Pogo Pin 1 to TP4056 IN+ & Divider Tap |
| **System Ground (GND)** | 24 AWG High-Strand Flexible Silicone | **Black** | Common Star Ground Bus |
| **Regulated 3.3V Logic** | 28 AWG Flexible Silicone | **Coral / Pink** | S3 3V3 rail to OLED, MPU, Touch, Mic, Amp SD |
| **I2C Bus (SDA / SCL)** | 30 AWG Flexible Silicone | **Cyan / Green** | GPIO8 (SDA: Blue/Cyan), GPIO9 (SCL: Green) |
| **I2S Audio Clocks** | 30 AWG Flexible Silicone | **Violet / Magenta**| GPIO2 (BCLK: Purple), GPIO1 (WS/LRC: Pink) |
| **I2S Mic Data In** | 30 AWG Flexible Silicone | **Yellow** | INMP441 SD -> ESP32-S3 GPIO10 |
| **I2S Amp Data Out** | 30 AWG Flexible Silicone | **Amber** | ESP32-S3 GPIO11 -> MAX98357A DIN |
| **Speaker Bridge Leads**| 28 AWG Twisted Pair Silicone | **Orange / Gray** | MAX98357A OUT+ / OUT- to 8Ω Speaker |
| **Sensor Discrete Logic**| 30 AWG Flexible Silicone | **White / Blue** | GPIO4 (MPU INT), GPIO6 (Touch), GPIO5 (Dock), GPIO7 (Chrg) |

---

### 5.2 Sub-Assembly 1: Pre-Soldering 1N4148 Diode & 10k/20k Voltage Divider

This sub-assembly converts high-voltage dock and charger signals to safe 3.3V levels for ESP32-S3 GPIOs. Assemble this as a modular inline fly-lead harness before mounting boards into the chassis.

```
       [ 10k / 20k DOCK SENSE VOLTAGE DIVIDER ]
       
       Dock Pogo Pin 1 (+5V)
                │
                ├───[ 10 kΩ Resistor ]───┬───[ 28 AWG Pink Wire ]───► To S3 GPIO5 (Dock Sense)
                │                        │
                │                 [ 20 kΩ Resistor ]
                │                        │
                │                        └───[ 28 AWG Black Wire ]──► To Common GND Star
                ▼
       To TP4056 IN+


       [ 1N4148 CHARGE SENSE DIODE HARNESS ]

       TP4056 Pin 7 (CHRG Red LED Cathode)
                │
             [ === ]  Cathode Band (Black Ring)
              \   /   1N4148 Fast Switching Diode
               \ /
                │     Anode Lead
                └───[ 28 AWG Violet Wire ]──────────────────────────► To S3 GPIO7 (Charge Sense)
                                                                       (Internal PULLUP Active)
```

#### Detailed Fabrication Steps:
1. **Voltage Divider Assembly**:
   - Take one 10kΩ 1% 1/8W metal film resistor and one 20kΩ 1% 1/8W resistor. Trim leads to 6mm.
   - Twist one lead of the 10kΩ resistor and one lead of the 20kΩ resistor together to form the center tap junction.
   - Solder a 45mm length of 28 AWG Pink wire to this center tap (runs to S3 GPIO5).
   - Solder a 30mm length of 24 AWG Orange wire to the free end of the 10kΩ resistor (taps into the Dock +5V pogo line).
   - Solder a 30mm length of 26 AWG Black wire to the free end of the 20kΩ resistor (runs to Common GND).
   - Slip 2.5mm polyolefin heat-shrink tubing over each individual resistor and a master 3.5mm heat-shrink sleeve over the entire junction. Shrink with hot air at 250°C.
   - *Transfer function check*: $V_{\text{out}} = 5.0\text{V} \times \frac{20\text{k}\Omega}{10\text{k}\Omega + 20\text{k}\Omega} = 3.333\text{V}$ (Safe for ESP32-S3 3.3V GPIO5).
2. **1N4148 Charge Status Sense Diode**:
   - Locate Pin 7 of the TP4056 SOP-8 IC (or the solder pad on the cathode side of the onboard red `CHRG` LED).
   - Identify the **Cathode** of the 1N4148 diode (indicated by the distinct black painted ring on the glass envelope).
   - Trim the cathode lead to 4mm and solder it directly to TP4056 Pin 7 / LED cathode pad.
   - Solder a 40mm length of 28 AWG Violet wire to the **Anode** lead (runs to S3 GPIO7).
   - Slide 1.5mm heat-shrink tubing over the diode body and shrink.
   - *Operating principle*: When actively charging, TP4056 Pin 7 pulls to ground (~0.15V). The diode conducts, pulling GPIO7 to ~0.75V (Logic LOW). When charging finishes or standby, Pin 7 floats (open-drain). The internal pull-up on GPIO7 holds the pin at 3.3V (Logic HIGH). This diode completely blocks 5V dock voltage from back-feeding into GPIO7.

---

### 5.3 Sub-Assembly 2: Common GND Bus Star-Point Connection

To prevent ground bounce from the Class-D audio switching amplifier (MAX98357A) and Wi-Fi transmission bursts from injecting noise into the I2S microphone (INMP441) and I2C sensors (MPU-6050, OLED), a **Star-Ground Topology** is strictly enforced.

```
                             [ STAR GROUND JUNCTION ]
                               (TP4056 OUT- Pad)
                                      │
         ┌────────────┬───────────────┼───────────────┬────────────┐
         │            │               │               │            │
      S3 GND      MAX98357A       INMP441          MPU6050     OLED GND
      Header         GND            GND            GND & AD0   & TTP223 GND
    (24 AWG)      (24 AWG)       (28 AWG)          (28 AWG)     (28 AWG)
                                      │
                               Dock Pogo Pin 2 &
                             Voltage Divider 20k
                                  (24 AWG)
```

#### Ground Termination Rules:
- **Central Node**: The `OUT−` pad of the TP4056 charger serves as the primary star ground junction.
- **Microphone Grounding**: Connect both `GND` and `L/R` pins of the INMP441 directly to the star ground to select the Left I2S slot.
- **MPU-6050 Grounding**: Connect `GND` and `AD0` together directly at the module header, then run a single 28 AWG wire to the star ground.
- **Audio Grounding**: Solder a direct 24 AWG lead from MAX98357A `GND` to the star ground. **Do not connect speaker terminals to ground!**

---

### 5.4 Sub-Assembly 3: Regulated 3.3V Power Harness

The ESP32-S3 SuperMini onboard high-current low-dropout (LDO) regulator outputs 3.3V on its `3V3` header pin. A pre-formed 5-branch spliced harness distributes clean 3.3V power:

```
 ESP32-S3 (3V3 Header Pin)
          │ [ 26 AWG Main Trunk - 25mm ]
          ▼
   [ Solder Splice Node ] ──► (Insulated with 2.5mm Dual-Wall Heat-Shrink)
          ├─── [Branch 1: 30 AWG Coral - 40mm] ──► OLED Display (VCC)
          ├─── [Branch 2: 30 AWG Coral - 35mm] ──► MPU-6050 IMU (VCC)
          ├─── [Branch 3: 30 AWG Coral - 30mm] ──► TTP223 Touch Pad (VCC)
          ├─── [Branch 4: 30 AWG Coral - 45mm] ──► INMP441 Mic (VDD)
          └─── [Branch 5: 30 AWG Coral - 25mm] ──► MAX98357A (SD_MODE Pull-Up)
```

> [!NOTE]
> Connecting `SD_MODE` of the MAX98357A to 3.3V sets the amplifier to output the **Left Audio Channel** with standard gain.

---

### 5.5 Sub-Assembly 4: Shared I2C & Shared I2S Audio Harnesses

Both I2C and I2S buses share clock lines across pairs of peripherals. Pre-soldering these harnesses minimizes wire clutter inside the mid-frame.

```
 [ SHARED I2C BUS HARNESS ]
 
 S3 GPIO8 (SDA) ──┬── [ 30 AWG Cyan ] ────► OLED SDA
                  └── [ 30 AWG Cyan ] ────► MPU-6050 SDA
 
 S3 GPIO9 (SCL) ──┬── [ 30 AWG Green ] ───► OLED SCL
                  └── [ 30 AWG Green ] ───► MPU-6050 SCL


 [ SHARED I2S CLOCK HARNESS & INDEPENDENT DATA LINES ]

 S3 GPIO1 (WS)  ──┬── [ 30 AWG Magenta ] ─► INMP441 WS  (Word Select / Frame Sync)
                  └── [ 30 AWG Magenta ] ─► MAX98357A LRC

 S3 GPIO2 (BCLK)──┬── [ 30 AWG Violet ] ──► INMP441 SCK (Continuous Bit Clock)
                  └── [ 30 AWG Violet ] ──► MAX98357A BCLK

 S3 GPIO10 (DIN) ────────────────────────── [ 30 AWG Yellow ] ◄── INMP441 SD (Data Out)
 S3 GPIO11 (DOUT)─────────────────────────► [ 30 AWG Amber ]  ──► MAX98357A DIN (Data In)

 MAX98357A OUT+  ─────────────────────────► [ 28 AWG Orange ] ──► Speaker (+) Terminal
 MAX98357A OUT-  ─────────────────────────► [ 28 AWG Gray ]   ──► Speaker (-) Terminal
                                             (Twisted Pair, BTL Floating - NO GND!)
```

---

## 6. Pre-Power Multimeter Inspection Checklist (Checks 1 to 10)

> [!CAUTION]
> **DO NOT CONNECT THE LIPO BATTERY OR APPLY 5V DOCK POWER UNTIL ALL 10 PRE-POWER CHECKS PASS.**  
> A short circuit on the 3.3V rail or reverse battery connection will permanently destroy the ESP32-S3 SoC and MEMS sensors within milliseconds.

Use a calibrated Digital Multimeter (DMM). Perform each step in order:

```
+-------+------------------------------------------+---------------------+-------------------+------------+
| Check | Description                              | Target Test Points  | Expected Value    | Pass/Fail  |
+-------+------------------------------------------+---------------------+-------------------+------------+
| C01   | Dock Power Rail Isolation                | Dock P1 (+) to P2(-)| > 100 kΩ (or OL)  |    [ ]     |
| C02   | LiPo Battery Pad Isolation               | TP4056 BAT+ to BAT- | > 50 kΩ (or OL)   |    [ ]     |
| C03   | System 5V (SYS) Rail to GND              | S3 5V to Common GND | > 20 kΩ           |    [ ]     |
| C04   | Regulated 3.3V Rail to GND               | S3 3V3 to Common GND| > 2.0 kΩ          |    [ ]     |
| C05   | Star Ground Network Continuity           | All GND nodes to P2 | < 0.3 Ω           |    [ ]     |
| C06   | Dock Sense Divider Functional Ratio      | 5V in to GPIO5 out  | 3.30V - 3.35V     |    [ ]     |
| C07   | 1N4148 Charge Sense Diode Polarity       | GPIO7 to TP4056 pin7| 0.55V - 0.65V drop|    [ ]     |
| C08   | I2C Bus Pull-Up Resistance & Isolation   | SDA/SCL to 3V3 & GND| 2.0 kΩ - 4.7 kΩ   |    [ ]     |
| C09   | I2S Clocks & Data Bus Isolation          | GPIO1,2,10,11 to GND| > 50 kΩ (or OL)   |    [ ]     |
| C10   | Speaker BTL Bridge Isolation             | OUT+/OUT- to GND/Spk| 7.2 Ω - 8.5 Ω spk |    [ ]     |
+-------+------------------------------------------+---------------------+-------------------+------------+
```

### Check 1: Dock Power Rail Isolation (Unpowered)
- **DMM Mode**: Resistance ($\Omega$) or Continuity.
- **Probes**: Red Probe on **Dock Pogo Pin 1 (+5V)**, Black Probe on **Dock Pogo Pin 2 (GND)**.
- **Pass Criteria**: Resistance must measure **$>100\text{ k}\Omega$** (or open circuit `OL`).
- **Failure Analysis**: If resistance is $<1\text{ k}\Omega$, an immediate dead short exists between the dock pogo contacts or across the 10k/20k voltage divider. Inspect pogo terminal soldering.

### Check 2: LiPo Battery Pad Isolation (Before Connecting Battery)
- **DMM Mode**: Resistance ($\Omega$).
- **Probes**: Red Probe on **TP4056 `BAT+`**, Black Probe on **TP4056 `BAT−`**.
- **Pass Criteria**: Resistance must measure **$>50\text{ k}\Omega$** (or `OL`).
- **Failure Analysis**: If resistance reads near $0\ \Omega$, solder splash or internal TP4056 MOSFET failure exists. Connecting a LiPo battery under this condition will cause immediate battery swelling or fire.

### Check 3: System 5V (SYS) Rail to Common Ground
- **DMM Mode**: Resistance ($\Omega$).
- **Probes**: Red Probe on **ESP32-S3 `5V` Header Pin** (or MAX98357A `VIN`), Black Probe on **Common GND**.
- **Pass Criteria**: Resistance must read **$>20\text{ k}\Omega$** after decoupling capacitors charge.
- **Failure Analysis**: If reading is $<500\ \Omega$, inspect MAX98357A power pins and ESP32-S3 onboard 5V-to-3.3V LDO input.

### Check 4: Regulated 3.3V Rail to Common Ground
- **DMM Mode**: Resistance ($\Omega$).
- **Probes**: Red Probe on **ESP32-S3 `3V3` Header Pin**, Black Probe on **Common GND**.
- **Pass Criteria**: Resistance must settle between **$2.0\text{ k}\Omega$ and $15\text{ k}\Omega$** (charging of bypass caps across OLED, MPU6050, and ESP32).
- **Failure Analysis**: Reading $<100\ \Omega$ indicates a blown LDO, pinched harness wire, or reversed sensor IC.

### Check 5: Star Ground Network Continuity
- **DMM Mode**: Low-Resistance Continuity (Beep mode).
- **Probes**: Keep Black Probe fixed to **Dock Pogo Pin 2 (GND)**. Probe sequentially with Red Probe:
  1. TP4056 `OUT−` pad
  2. ESP32-S3 `GND` pin
  3. MAX98357A `GND` pin
  4. INMP441 `GND` and `L/R` pins
  5. MPU-6050 `GND` and `AD0` pins
  6. OLED `GND` pin
  7. TTP223 `GND` pin
  8. Low side of 20kΩ divider resistor
- **Pass Criteria**: **Every test point must read $<0.3\ \Omega$** with an instantaneous audio continuity beep.
- **Failure Analysis**: Any floating point indicates an open ground return that will cause spurious resets or floating I2C levels.

### Check 6: Dock Sense Voltage Divider Functional Ratio
- **DMM Mode**: DC Voltage ($V_{\text{DC}}$).
- **Procedure**: Connect a bench power supply set to exactly **$+5.00\text{ V DC}$** across Dock Pogo Pin 1 (+) and Pin 2 (GND) with no battery installed.
- **Measurement**: Probe between **ESP32-S3 GPIO5 wire** and **Common GND**.
- **Pass Criteria**: Voltage must read between **$+3.28\text{ V}$ and $+3.36\text{ V DC}$**.
- **Removal Test**: Disconnect the 5V supply. Voltage on GPIO5 must drop immediately to **$0.00\text{ V}$** (pulled down by the 20kΩ resistor).
- **Failure Analysis**: If voltage exceeds $3.6\text{ V}$, the 20kΩ resistor is disconnected or cold-soldered! Do not connect GPIO5 to the ESP32 until fixed.

### Check 7: 1N4148 Charge Sense Diode Polarity & Forward Drop
- **DMM Mode**: Diode Test Mode ($\rightarrow\vert-$).
- **Measurement A (Forward Bias)**: Red Probe on **ESP32-S3 GPIO7 wire** (Anode), Black Probe on **TP4056 Pin 7 / LED Cathode Pad**.
  - **Pass Criteria**: Reads **$0.55\text{ V}$ to $0.68\text{ V}$** forward junction drop.
- **Measurement B (Reverse Bias)**: Red Probe on **TP4056 Pin 7**, Black Probe on **GPIO7 wire**.
  - **Pass Criteria**: Reads **`OL` (Open Loop / Over Range)**.
- **Failure Analysis**: If forward voltage reads in reverse, the diode was soldered backwards. If reading is $0.00\text{ V}$, the diode is shorted.

### Check 8: I2C Bus Pull-Up Resistance & Signal Isolation
- **DMM Mode**: Resistance ($\Omega$).
- **Measurements**:
  1. **GPIO8 (SDA)** to **3V3 Rail**: Expected **$2.0\text{ k}\Omega$ to $4.7\text{ k}\Omega$** (Parallel pullups from GY-521 and OLED).
  2. **GPIO9 (SCL)** to **3V3 Rail**: Expected **$2.0\text{ k}\Omega$ to $4.7\text{ k}\Omega$**.
  3. **GPIO8 (SDA)** to **GPIO9 (SCL)**: Expected **$>10\text{ k}\Omega$** (No bridge between clock and data).
  4. **GPIO8 / GPIO9** to **GND**: Expected **$>10\text{ k}\Omega$** (Neither line grounded).
- **Failure Analysis**: If resistance to 3V3 is infinite, external pull-ups are missing. If reading between SDA and SCL is $<10\ \Omega$, a microscopic solder bridge exists on the 0.1" header pins.

### Check 9: I2S Clocks & Audio Data Bus Isolation
- **DMM Mode**: Resistance ($\Omega$).
- **Measurements**:
  - Probe between each I2S line (**GPIO1 WS**, **GPIO2 BCLK**, **GPIO10 DIN**, **GPIO11 DOUT**) and **Common GND**.
  - Probe between each adjacent pair (GPIO1 to GPIO2, GPIO10 to GPIO11).
- **Pass Criteria**: All readings must measure **$>50\text{ k}\Omega$ or `OL`**.
- **Failure Analysis**: Any low resistance indicates a short or bridged trace on the SuperMini header.

### Check 10: Speaker BTL Terminal Isolation
- **DMM Mode**: Resistance ($\Omega$).
- **Measurements**:
  1. Probe between **MAX98357A `OUT+`** and **Common GND**: Must read **`OL` / $>1\text{ M}\Omega$**.
  2. Probe between **MAX98357A `OUT−`** and **Common GND**: Must read **`OL` / $>1\text{ M}\Omega$**.
  3. Probe between **`OUT+`** and **`OUT−`** across speaker terminals: Must read the exact DC voice-coil resistance of the speaker: **$7.2\ \Omega$ to $8.5\ \Omega$** (for a nominal 8Ω speaker).
- **Failure Analysis**:
  > [!CAUTION]
  > The MAX98357A uses a Bridge-Tied Load (BTL) output topology. Neither `OUT+` nor `OUT−` is ground! Connecting either speaker terminal to GND will permanently destroy the MAX98357A output FET stage upon power-up.

---

## 7. Final Assembly & Enclosure Integration Sequence

Once Checks 1 through 10 have received verified checkmarks, proceed with mechanical closure:

1. **Step 1 — Mid-Frame Sub-Assembly**:
   - Snap ESP32-S3 SuperMini, MPU-6050, and MAX98357A into the mid-frame tray.
   - Dress and route the 3.3V power, I2C, and I2S wire harnesses through the mid-frame wire channels.
2. **Step 2 — Front Shell Integration**:
   - Seat the silicone MEMS gasket into the front shell port.
   - Fasten the INMP441 microphone carrier board over the gasket.
   - Mount the SH1106 OLED display behind the front acrylic lens.
   - Adhere the TTP223 capacitive touch pad with 3M 300LSE tape to the inner top dome.
3. **Step 3 — Acoustic Chamber Closure**:
   - Seat the miniature 8Ω speaker into the acoustic cup with a perimeter bead of B-7000.
   - Route speaker wires through the rear exit hole; seal the hole with silicone.
4. **Step 4 — Base & Power Sub-Assembly**:
   - Press-fit and glue the keyed N52 magnets and pogo pins into the dock base.
   - Solder the dock pogo leads to the TP4056 input and 10k/20k voltage divider.
   - Apply protective EVA foam padding to the 500mAh LiPo pouch.
   - Mount the SPDT micro slide switch into the chassis slot.
   - Solder LiPo red (+) lead to Switch Pin 2 (Common); solder Switch Pin 3 (ON) to TP4056 `BAT+`; slide and heat 2.5mm heatshrink over both terminals (leave Pin 1 NC open).
   - Solder LiPo black (−) lead directly to TP4056 `BAT−`.
5. **Step 5 — Final Shell Fastening**:
   - Mate the front, mid-frame, and rear base shell sections.
   - Fasten using four perimeter M1.6 × 8mm stainless steel machine screws into brass heat-set inserts.
   - Ensure no wires are pinched along the perimeter parting lines.

---

*Document Status: Certified Ready for Hardware Arrival.*
