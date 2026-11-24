# Mochi Bot V1 — 3D CAD Mechanical Specification & Enclosure Blueprint

> **Engineering Reference**: Extracted and verified from the official **3D CAD Mechanical Specification & AI Design Blueprint** (`mochi-cad-blueprint.pdf`).
> **Locked Envelope**: $49.0\text{ mm (W)} \times 63.5\text{ mm (H)} \times 19.5\text{--}25.5\text{ mm (Z)}$ ($1.93" \times 2.50"$)
> **Parametric CAD Source**: [`tools/cad/mochi_bot_enclosure.scad`](../tools/cad/mochi_bot_enclosure.scad)

---

## 1. Locked 11-Component Mechanical Dimensions

| Component | Nominal Dimensions (mm) | FDM Pocket / Cutout (mm) | Mounting & Z-Stack Rules |
|---|---|---|---|
| **1. ESP32-S3 SuperMini** | $23.5 \times 18.0 \times 4.92$ ($24.55$ L w/ USB-C) | Slot $23.9 \times 18.4$; USB-C $10.0 \times 4.5$ | 0 holes; $15.24\text{ mm}$ pin row pitch. Index USB-C off PCB top face (CL $1.58\text{ mm}$ up). Keep metal clear of ceramic antenna end. |
| **2. 1.3" SH1106 OLED (4-pin I2C)** | $35.4 \times 33.5 \times 1.2$ ($4.3$ bare / $11.3$ w/ pins) | Bezel $31.42 \times 16.70$ (active $29.42 \times 14.70$) | $4\times \varnothing 3.0\text{ mm}$ holes at $30.4 \times 28.5\text{ mm}$ pitch ($2.5\text{ mm}$ inset). Active offset $7.35\text{ mm}$ top / $2.99\text{ mm}$ sides; leave $14.0\text{ mm}$ wide bottom FPC recess. |
| **3. MPU6050 IMU (GY-521)** | $20.3\text{--}21.2 \times 15.0\text{--}16.4 \times 3.4$ | Cradle $21.6 \times 16.8$ | $2\times \varnothing 3.0\text{ mm}$ M3 corner holes; 8-pin $2.54\text{ mm}$ row ($17.78\text{ mm}$ span). |
| **4. TTP223 Red Mini Touch** | $15.0 \times 11.0 \times 2.5$ ($1.6$ PCB) | Pocket $15.4 \times 11.4$, depth $\ge 3.0$ | 0 holes; $11.0 \times 10.5\text{ mm}$ front pad. Thin enclosure roof to $1.0\text{--}1.2\text{ mm}$, **zero air gap**, non-metallic PLA/PETG only. |
| **5. INMP441 MEMS Mic** | $\varnothing 14.0\text{--}14.2 \times 2.0\text{--}2.6$ | Pocket $\varnothing 14.4\text{--}14.6$ + ring gasket | 0 holes; $2\times 3$ pins ($7.62\text{ mm}$ pitch). Compress silicone/foam gasket on flat bottom face aligned to $\varnothing 1.5\text{ mm}$ front chin port. |
| **6. MAX98357A Class-D Amp** | $19.4 \times 17.8 \times 3.0$ ($11.8$ w/ terminal) | Cradle $19.8 \times 18.2$ | $2\times$ M2.5 corner holes; direct-solder speaker wires to save $8.8\text{ mm}$ Z-height. Capped digital amplitude in firmware strictly protects $0.25\text{--}0.5\text{ W}$ speaker. |
| **7. 4cm Thin Speaker (RBD-2694)**| $\varnothing 40.0 \times 4.2\text{--}4.5$ ($8\ \Omega, 0.25\text{--}0.5\text{ W}$) | Pocket $\varnothing 40.4\text{--}40.6$, depth $4.5\text{--}5.0 + 3\text{ mm}$ notch | Front grille $\varnothing 32\text{--}34\text{ mm}$ ($40\text{--}60\%$ open area), $\ge 0.5\text{--}1.0\text{ mm}$ cone clearance, rear foam ring. |
| **8. TP4056 USB-C Charger** | $26.0\text{--}28.0 \times 17.0\text{--}17.35 \times 4.9$ | Slot $17.6\text{ W} \times 28.4\text{ L}$; USB-C $10.0 \times 4.5$ | 0 holes; $\sim 1.0\text{ mm}$ USB-C overhang. Internal mount ok if charging solely via pogo IN+/IN-; leave thermal air gap over IC. |
| **9. HCY 402535P 500 mAh LiPo** | $4.0 \times 25.0 \times 37.0$ (incl. PCM & Kapton) | Bay $5.0 \times 26.2 \times 40.5$ | Includes $+15\%$ Z swelling, side foil crimps, and top-left lead bend; zero sharp edges. |
| **10. 2-Pin Pogo (MG0225 / RBD-3143)** | $11.0\text{--}12.2 \times 3.6\text{--}4.2$ ($2.54\text{ mm}$ pitch, 2A) | Cutout $12.3 \times 4.2$, corners R1.9 | Mating face flush ($0.0$ to $-0.3\text{ mm}$), $1.0\text{--}1.4\text{ mm}$ plunger travel, mandatory $1.2\text{ mm}$ rear backstop lip. |
| **11. SPDT Slide Switch (SS-12D00)** | $8.5 \times 3.6\text{--}4.0 \times 3.5\text{--}3.8$ ($3\text{--}4\text{ mm}$ knob) | Pocket $8.8 \times 4.3 \times 4.0$; slot $4.4 \times 2.2$ | $2.0\text{ mm}$ slide travel; thin outer wall to $\le 1.2\text{--}1.5\text{ mm}$ at slot. Rated $0.3\text{--}0.5\text{ A}$ DC. |

---

## 2. Internal 4-Layer Stepped-Back Coordinate Map

* **Cavity**: $45.0\text{ mm (W)} \times 59.5\text{ mm (H)}$ inside $49.0 \times 63.5\text{ mm}$ outer shell ($2.0\text{ mm}$ perimeter walls, $1.6\text{ mm}$ front/rear faces).
* **Split Plane**: 2-piece clamshell parting plane at $Z = 12.5\text{ mm}$.

```
  [ FRONT Z = 0.0 mm ]
        │  Front Face (1.6 mm): OLED window 31.42 x 16.70 mm; Chin mic pinhole Ø1.5 mm
        ▼
  [ Z = 1.6 - 6.1 mm ] Layer 1: Display & Mic
        ├── 1.3" SH1106 OLED (X = 4.8 - 40.2, Y = 2.0 - 35.5 mm)
        └── INMP441 MEMS Mic (X = 15.2 - 29.8, Y = 38.0 - 52.6 mm, centered chin)
        ▼
  [ Z = 6.1 - 6.9 mm ] Mid-Frame Printed Support Shelf (0.8 mm deck)
        ▼
  [ Z = 6.9 - 12.1 mm ] Layer 2: 4-Board Logic Quad
        ├── Top-Left: ESP32-S3 SuperMini (X = 0 - 24.55, Y = 1.0 - 19.4 mm, Left USB-C)
        ├── Top-Right: MAX98357A Amp (X = 26.5 - 44.7, Y = 1.0 - 20.8 mm)
        ├── Bottom-Left: TP4056 Charger (X = 0.5 - 18.1, Y = 22.0 - 50.4 mm)
        ├── Mid-Right: MPU6050 GY-521 (X = 21.0 - 42.6, Y = 22.0 - 38.8 mm)
        └── Right Wall: SS-12D00 Slide Switch (Y = 42.0 - 50.8 mm)
        ▼  [ CLAMSHELL PARTING PLANE AT Z = 12.5 mm ]
  [ Z = 12.1 - 12.9 mm ] Smooth Isolation Floor (0.8 mm)
        ▼
  [ Z = 12.9 - 17.9 mm ] Layer 3: LiPo Bay & Flush Lanyard Tunnel
        ├── HCY 402535P 500mAh LiPo (X = 2.2 - 42.7, Y = 14.0 - 40.2 mm)
        │   *Leaves top 14 mm behind ESP32-S3 ceramic antenna completely foil-free!*
        └── Flush Lanyard Tunnel in Top-Right Rear Corner (Z = 15.5 mm)
            *Ø3.5 mm channel from (X=43.5, Y=0) to (X=49.0, Y=5.5 mm) with solid 3.0 mm bridge*
        ▼
  [ Z = 17.9 - 18.7 mm ] Airtight Acoustic Bulkhead (0.8 mm divider wall)
        ▼
  [ Z = 18.7 - 23.9 mm ] Layer 4: Stepped Speaker Pod
        └── 4cm Thin Speaker Ø40.6 x 5.2 mm (Centered X = 22.5, Y = 38.8 mm)
            *Pod extends to 25.5 mm total thickness from Y = 18.0 mm down*
```

---

## 3. Universal Dock Pocket Datum

All three swappable minimalist docks share **one unified pocket datum**:
* **Pocket Mouth**: $49.8 \times 26.3\text{ mm}$ ($+0.4\text{ mm}$ FDM clearance around $49.0 \times 25.5\text{ mm}$ pod).
* **Corner Radius**: Bot corner fillet $R + 0.4\text{ mm} = 4.4\text{ mm}$.
* **Lead-in Chamfer**: $2.0\text{ mm} \times 45^\circ$ top rim lead-in chamfer.
* **Front Lip Height**: Exactly **$12.0\text{ mm}$** above pocket floor (clears the front-chin INMP441 mic port at $16.7\text{ mm}$).
* **MG0225 Pogo Cutout**: $12.3 \times 4.2\text{ mm}$ (R1.9 ends), centered laterally at $X = 24.9\text{ mm}$ and positioned **$10.5\text{ mm}$ back** from inner front wall.
* **Pass-through**: $\varnothing 4.5\text{ mm}$ internal wire tunnel to a rear $10.0 \times 4.5\text{ mm}$ USB-C female port.

### The 3 Swappable Minimalist Docks

1. **River-Stone Pebble**:
   - Outer dimensions: $88 \times 68 \times 26\text{ mm}$ ($3.46" \times 2.68" \times 1.02"$).
   - Tilt: $15^\circ$ backward lean.
   - Rear acoustic feature: $12\text{ mm}$ front lip / $20\text{ mm}$ rear rim (leaves $40\text{ mm}$ rear speaker unobstructed).
   - Weight cavity: $50 \times 40 \times 8\text{ mm}$ ($\sim 100\text{ g}$ M8 nuts/washers).
2. **Folded Ribbon Loop**:
   - Outer dimensions: $55 \times 58 \times 54\text{ mm}$ ($2.17" \times 2.28" \times 2.13"$).
   - Tilt: $18^\circ$ backward lean.
   - Rear acoustic feature: $4.5\text{ mm}$ ribbon wall with **$\varnothing 34\text{ mm}$ circular acoustic pass-through cutout**.
   - Weight slot: $42 \times 42 \times 4\text{ mm}$ ($\sim 70\text{ g}$ pause-print washers).
3. **Bauhaus Plinth + Mini Valet Tray**:
   - Outer dimensions: $98 \times 60 \times 30\text{ mm}$ ($3.86" \times 2.36" \times 1.18"$).
   - Tilt: $15^\circ$ backward lean.
   - Valet tray: Integrated right pocket $38 \times 48 \times 8\text{ mm}$ (R4 fillets) for SD cards, dongles, or rings.
   - Weight cavity: $78 \times 44 \times 8\text{ mm}$ ($\sim 120\text{ g}$ steel nuts/coins).
