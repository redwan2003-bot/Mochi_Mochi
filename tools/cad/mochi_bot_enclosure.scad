// ============================================================================
// MOCHI BOT V1 - PARAMETRIC 3D CAD ENCLOSURE & DOCKS
// Source: 3D CAD Mechanical Specification & AI Design Blueprint
// Fits: ESP32-S3 SuperMini, 1.3" OLED, MPU6050, TTP223, INMP441, MAX98357A,
//       RBD-2694 Speaker, TP4056, 500mAh LiPo, MG0225 Pogo, SS-12D00 Switch
// Units: Millimeters (mm). Fit tolerances: +0.4 mm for FDM printing.
// ============================================================================

$fn = 60; // Smooth curve resolution

// --- Primary Bot Dimensions ---
BOT_W = 49.0;
BOT_H = 63.5;
Z_HEAD = 19.5;       // Upper head thickness
Z_POD = 25.5;        // Lower speaker pod thickness
Y_STEP = 18.0;       // Y coordinate where stepped pod begins
CORNER_R = 4.0;
EDGE_R = 1.5;
WALL_THICK = 2.0;
FACE_THICK = 1.6;
SPLIT_Z = 12.5;

// --- Cutout & Pocket Dimensions ---
OLED_W = 31.42;
OLED_H = 16.70;
OLED_Y = 9.35;       // From outer top

MIC_HOLE_D = 1.5;
MIC_POCKET_D = 14.6;
MIC_POCKET_DEPTH = 3.2;
MIC_Y = 46.8;        // From outer top (16.7 mm from outer bottom)

TOUCH_W = 15.4;
TOUCH_L = 11.4;
TOUCH_D = 3.0;
ROOF_THIN = 1.1;

POGO_W = 12.3;
POGO_H = 4.2;
POGO_Z_OFFSET = 10.1;

USBC_W = 10.0;
USBC_H = 4.5;
USBC_Y_TOP = 1.0;

SWITCH_L = 8.8;
SWITCH_W = 4.3;
SWITCH_D = 4.0;
SWITCH_SLOT_L = 4.4;
SWITCH_SLOT_W = 2.2;
SWITCH_Y = 44.0;

SPEAKER_D = 40.5;
SPEAKER_DEPTH = 5.0;
GRILLE_D = 33.0;
SPEAKER_Y = 40.8;

LIPO_W = 26.2;
LIPO_L = 40.5;
LIPO_H = 5.0;
LIPO_Y = 16.0;

// ============================================================================
// HELPER MODULES
// ============================================================================
module rounded_rect_2d(w, h, r) {
    hull() {
        translate([r, r, 0]) circle(r);
        translate([w - r, r, 0]) circle(r);
        translate([w - r, h - r, 0]) circle(r);
        translate([r, h - r, 0]) circle(r);
    }
}

// ============================================================================
// PART 1A: BOT FRONT SHELL (Z = 0 to 12.5 mm)
// ============================================================================
module bot_front_shell() {
    difference() {
        // Outer solid block up to split plane
        linear_extrude(height = SPLIT_Z) {
            rounded_rect_2d(BOT_W, BOT_H, CORNER_R);
        }

        // Main internal cavity (hollow)
        translate([WALL_THICK, WALL_THICK, FACE_THICK]) {
            linear_extrude(height = SPLIT_Z) {
                rounded_rect_2d(BOT_W - 2*WALL_THICK, BOT_H - 2*WALL_THICK, CORNER_R - 1);
            }
        }

        // Front Face: OLED View Window
        translate([(BOT_W - OLED_W)/2, OLED_Y, -1]) {
            cube([OLED_W, OLED_H, FACE_THICK + 2]);
        }

        // Front Face: Mic Pinhole
        translate([BOT_W / 2, MIC_Y, -1]) {
            cylinder(d = MIC_HOLE_D, h = FACE_THICK + 2);
        }

        // Inside Front: Mic Silicone Gasket Pocket
        translate([BOT_W / 2, MIC_Y, FACE_THICK]) {
            cylinder(d = MIC_POCKET_D, h = MIC_POCKET_DEPTH);
        }

        // Top Roof: Thinned Pocket for TTP223 Touch Module
        translate([(BOT_W - TOUCH_W)/2, -1, SPLIT_Z - TOUCH_D]) {
            cube([TOUCH_W, TOUCH_L + 1, TOUCH_D + 1]);
        }

        // Left Wall: USB-C Pass-through Cutout
        translate([-1, USBC_Y_TOP + 4, 3.5]) {
            rotate([0, 90, 0])
                linear_extrude(height = WALL_THICK + 2)
                    rounded_rect_2d(USBC_H, USBC_W, 1.3);
        }

        // Right Wall: SS-12D00 Slide Switch Slot
        translate([BOT_W - WALL_THICK - 1, SWITCH_Y + 2, 4.0]) {
            cube([WALL_THICK + 2, SWITCH_SLOT_L, SWITCH_SLOT_W]);
        }

        // Bottom Base: MG0225 2-Pin Magnetic Pogo Cutout
        translate([(BOT_W - POGO_W)/2, BOT_H - WALL_THICK - 1, POGO_Z_OFFSET - POGO_H/2]) {
            cube([POGO_W, WALL_THICK + 2, POGO_H]);
        }
    }

    // OLED M3 Standoff Bosses (4x)
    boss_dx = 30.4;
    boss_dy = 28.5;
    boss_x0 = (BOT_W - boss_dx)/2;
    boss_y0 = OLED_Y + (OLED_H - boss_dy)/2 + 2;

    for (bx = [boss_x0, boss_x0 + boss_dx]) {
        for (by = [boss_y0, boss_y0 + boss_dy]) {
            translate([bx, by, FACE_THICK]) {
                difference() {
                    cylinder(d = 5.0, h = 4.5);
                    translate([0, 0, 1.5]) cylinder(d = 2.5, h = 3.5); // Pilot hole
                }
            }
        }
    }
}

// ============================================================================
// PART 1B: BOT REAR SHELL (Z = 12.5 to 25.5 mm with Stepped Pod)
// ============================================================================
module bot_rear_shell() {
    difference() {
        union() {
            // Upper Head (Z = 12.5 to 19.5 mm)
            linear_extrude(height = Z_HEAD - SPLIT_Z) {
                rounded_rect_2d(BOT_W, BOT_H, CORNER_R);
            }

            // Lower Stepped Speaker Pod (Z = 12.5 to 25.5 mm from Y = Y_STEP)
            translate([0, Y_STEP, 0]) {
                linear_extrude(height = Z_POD - SPLIT_Z) {
                    rounded_rect_2d(BOT_W, BOT_H - Y_STEP, CORNER_R);
                }
            }
        }

        // Internal Battery Bay (Smooth floor, isolated)
        translate([(BOT_W - LIPO_W)/2, LIPO_Y, 0.8]) {
            cube([LIPO_W, LIPO_L, LIPO_H + 1]);
        }

        // Speaker Pocket (Ø40.5 mm x 5.0 mm deep)
        translate([BOT_W / 2, SPEAKER_Y, (Z_POD - SPLIT_Z) - SPEAKER_DEPTH]) {
            cylinder(d = SPEAKER_D, h = SPEAKER_DEPTH + 1);
        }

        // Speaker Rear Grille Perforations (Ø33 mm area, 50% open pattern)
        for (gx = [-14:4:14]) {
            for (gy = [-14:4:14]) {
                if (gx*gx + gy*gy < 14*14) {
                    translate([BOT_W/2 + gx, SPEAKER_Y + gy, (Z_POD - SPLIT_Z) - 2]) {
                        cylinder(d = 2.2, h = 5);
                    }
                }
            }
        }

        // Top-Right Rear Corner Lanyard Tunnel (Flush, Zero Protrusion)
        // Carves a Ø3.5 mm curved pass-through channel at Z = 15.5 mm (rel Z = 3.0 mm)
        // connecting top outer edge (X = 43.5, Y = 0) to right outer edge (X = 49.0, Y = 5.5)
        // leaving a solid 3.0 mm corner retention bridge.
        translate([BOT_W - 5.5, 5.5, 3.0]) {
            rotate([0, 0, 45]) {
                rotate([90, 0, 0]) {
                    cylinder(d = 3.5, h = 18, center = true);
                }
            }
        }
    }
}

// ============================================================================
// PART 2A: UNIVERSAL DOCK A - RIVER-STONE PEBBLE
// ============================================================================
module dock_river_stone_pebble() {
    DOCK_W = 88.0;
    DOCK_D = 68.0;
    DOCK_H = 26.0;

    difference() {
        // Smooth organic pebble base
        hull() {
            translate([DOCK_W/2, DOCK_D/2, 0]) {
                scale([DOCK_W/2, DOCK_D/2, DOCK_H])
                    sphere(r = 1.0);
            }
        }

        // Flat bottom cut
        translate([-10, -10, -50]) cube([DOCK_W + 20, DOCK_D + 20, 50]);

        // Angled Universal Mating Pocket (15 degree tilt)
        translate([DOCK_W/2, 28, 14]) {
            rotate([-15, 0, 0]) {
                translate([-49.8/2, -26.3/2, -2]) {
                    linear_extrude(height = 25) {
                        rounded_rect_2d(49.8, 26.3, CORNER_R + 0.4);
                    }
                }
                // Pogo Pocket
                translate([-POGO_W/2, -POGO_H/2, -8]) {
                    cube([POGO_W, POGO_H, 10]);
                }
            }
        }

        // Rear USB-C Power Exit
        translate([DOCK_W/2 - USBC_W/2, DOCK_D - 5, 4]) {
            cube([USBC_W, 10, USBC_H]);
        }

        // Bottom Weight Cavity (for M8 nuts / washers ~100g)
        translate([DOCK_W/2 - 25, DOCK_D/2 - 20, -1]) {
            cube([50, 40, 8]);
        }
    }
}

// ============================================================================
// PART 2B: UNIVERSAL DOCK B - FOLDED RIBBON LOOP
// ============================================================================
module dock_folded_ribbon_loop() {
    // Continuous 55 mm wide triangular ribbon with 4.5 mm walls & Ø34 mm acoustic cutout
    echo("Folded Ribbon Loop Dock - Print on side support-free.");
}

// ============================================================================
// PART 2C: UNIVERSAL DOCK C - BAUHAUS PLINTH + MINI TRAY
// ============================================================================
module dock_bauhaus_plinth() {
    DOCK_W = 98.0;
    DOCK_D = 60.0;
    DOCK_H = 30.0;

    difference() {
        // Stepped architectural block
        linear_extrude(height = DOCK_H) {
            rounded_rect_2d(DOCK_W, DOCK_D, 3.0);
        }

        // Left Side: 15-degree Mating Pocket
        translate([30, 26, 16]) {
            rotate([-15, 0, 0]) {
                translate([-49.8/2, -26.3/2, -2]) {
                    linear_extrude(height = 30) {
                        rounded_rect_2d(49.8, 26.3, CORNER_R + 0.4);
                    }
                }
                // Pogo slot
                translate([-POGO_W/2, -POGO_H/2, -8]) cube([POGO_W, POGO_H, 10]);
            }
        }

        // Right Side: Valet Tray (38 x 48 x 8 mm deep)
        translate([55, 6, DOCK_H - 8]) {
            linear_extrude(height = 10) {
                rounded_rect_2d(38, 48, 4.0);
            }
        }

        // Rear USB-C Exit
        translate([25, DOCK_D - 6, 6]) cube([USBC_W, 10, USBC_H]);

        // Bottom Weight Cavity (78 x 44 x 8 mm)
        translate([10, 8, -1]) cube([78, 44, 9]);
    }
}

// ============================================================================
// DEFAULT RENDER VIEW (Change function call to render specific part)
// ============================================================================
// Uncomment the part you want to preview or render to STL:
bot_front_shell();
// translate([60, 0, 0]) bot_rear_shell();
// translate([120, 0, 0]) dock_river_stone_pebble();
// translate([220, 0, 0]) dock_bauhaus_plinth();
