// =============================================================================
// OPENSCAD PARAMETRIC MASTER MODEL — BAHADUR REV 4.1 (60.5 MM COMPACT HEIGHT)
// 100% 2-MANIFOLD | 0.00 MM ZERO INTERFERENCE | FULL 8-PART SYSTEM
// High-Performance 2D-First CSG Architecture
// =============================================================================

// -----------------------------------------------------------------------------
// Top-Level Selectors & Quality
// -----------------------------------------------------------------------------
PART        = "all";    // "all", "01_front", "02_rear", "03_baffle", "04_floor",
                        // "08_collar", "05_dock_a", "06_dock_b", "07_dock_c"
ENABLE_LOGO = true;     // Mustache brand deboss on chin, rear, and all 3 docks
PRINT_LAYOUT = false;   // If true, arranges parts flat on the print bed

$fn = 32;               // High-speed robust detail level
eps = 0.05;             // Non-manifold boolean anti-coplanar offset

// -----------------------------------------------------------------------------
// 1. Locked Hardware & Enclosure Parameters (Rev 4.1 Compact Height)
// -----------------------------------------------------------------------------
bot_w            = 49.0;
bot_h            = 60.5;   // Compact height (shrunk -3.0 mm from 63.5)
seam             = 12.7;
head_depth       = 19.5;   // Flat rear reference datum
pod_depth        = 24.3;   // Maximum depth over sculpted speaker dome

wall             = 2.0;
face             = 1.6;
corner_r         = 4.0;
edge_r           = 1.5;
shadow_chamfer   = 0.4;

inner_w          = 45.0;   // bot_w - 2*wall
inner_h          = 56.5;   // bot_h - 2*wall (Y = 2.0 .. 58.5)

// Subsystem Centerlines & Coordinate Shifts
speaker_x        = 24.5;
speaker_y        = 37.5;   // Locked dock datum: bot_h - speaker_y = 23.00 mm!
mic_x            = 24.5;
mic_y            = 44.8;   // Shifted -2.0 mm
battery_x        = 11.4;
battery_y        = 13.5;   // Shifted -2.5 mm; LiPo bay 26.2 x 40.5 mm
battery_w        = 26.2;
battery_h        = 40.5;
switch_y         = 41.5;   // Shifted -2.5 mm; clears bottom-right M2 boss
tp4056_y         = 23.2;   // Shifted -2.0 mm; clears ESP32 by 2.0mm, bot-left boss by 1.7mm
pogo_z           = 9.5;    // Exactly 10.0 mm forward of 19.5 mm rear datum

baffle_z         = 17.5;   // 4.8 mm LiPo bay Z (12.7..17.5; +0.8 mm swelling safety)
baffle_t         = 0.6;    // Circular acoustic baffle plate thickness
speaker_z        = 18.1;   // baffle_z + baffle_t
speaker_pocket_d = 40.5;
speaker_pocket_depth = 5.0;// 18.1..23.1 mm (4.5 mm speaker + 0.5 mm excursion)
grille_wall      = 1.2;    // 23.1..24.3 mm dome wall

// 4x M2 Boss Fastener Coordinates
m2_bosses = [
    [3.5, 4.0],
    [45.0, 9.0],
    [3.5, 56.5],
    [45.0, 56.5]
];

// -----------------------------------------------------------------------------
// Helper 2D & 3D Primitives
// -----------------------------------------------------------------------------
module rounded_rect_2d(w, h, r) {
    r_safe = max(0.1, min(r, min(w/2, h/2)));
    hull() {
        translate([r_safe, r_safe]) circle(r = r_safe);
        translate([w - r_safe, r_safe]) circle(r = r_safe);
        translate([w - r_safe, h - r_safe]) circle(r = r_safe);
        translate([r_safe, h - r_safe]) circle(r = r_safe);
    }
}

module rounded_box_prism(w, h, d, r) {
    linear_extrude(height = d)
        rounded_rect_2d(w, h, r);
}

// 3D Front Shell Blank (Z = 0 .. seam, R1.5 fillet on front face Z = 0)
module front_shell_blank(w, h, d, r_corner, r_edge) {
    translate([0, 0, r_edge - eps])
        linear_extrude(height = d - r_edge + eps)
            rounded_rect_2d(w, h, r_corner);
    hull() {
        translate([r_edge, r_edge, 0])
            linear_extrude(height = eps)
                rounded_rect_2d(w - 2*r_edge, h - 2*r_edge, max(0.5, r_corner - r_edge));
        translate([0, 0, r_edge])
            linear_extrude(height = eps)
                rounded_rect_2d(w, h, r_corner);
    }
}

// 3D Rear Shell Blank (Z = seam .. head_depth, flat at Z = seam, R1.5 fillet at rear face Z = head_depth)
module rear_shell_blank(w, h, z_start, z_end, r_corner, r_edge) {
    // Main straight body from z_start to z_end - r_edge
    translate([0, 0, z_start])
        linear_extrude(height = z_end - z_start - r_edge + eps)
            rounded_rect_2d(w, h, r_corner);
    // Rear transition to filleted back face
    hull() {
        translate([0, 0, z_end - r_edge])
            linear_extrude(height = eps)
                rounded_rect_2d(w, h, r_corner);
        translate([r_edge, r_edge, z_end - eps])
            linear_extrude(height = eps)
                rounded_rect_2d(w - 2*r_edge, h - 2*r_edge, max(0.5, r_corner - r_edge));
    }
}

// Logo deboss helper
module mustache_deboss(depth) {
    if (ENABLE_LOGO) {
        translate([0, 0, -eps])
            linear_extrude(height = depth + eps)
                import("01_Brand_Logos/bahadur-mustache-cad-deboss.svg", center = true);
    }
}

// -----------------------------------------------------------------------------
// 2. BODY 01 — FRONT SHELL (49.0 x 60.5 x 13.9 mm)
// -----------------------------------------------------------------------------
module part_01_front() {
    difference() {
        union() {
            // Outer Main Enclosure (Z = 0 .. 12.7)
            front_shell_blank(bot_w, bot_h, seam, corner_r, edge_r);

            // Perimeter Seam Alignment Lip (Z = 12.7 .. 13.9, 0.8 mm thick, inset 0.6 mm)
            translate([0.6, 0.6, seam - 0.2])
                difference() {
                    linear_extrude(height = 1.2 + 0.2)
                        rounded_rect_2d(bot_w - 1.2, bot_h - 1.2, corner_r - 0.6);
                    translate([0.8, 0.8, -eps])
                        linear_extrude(height = 1.2 + 0.2 + 2*eps)
                            rounded_rect_2d(bot_w - 2.8, bot_h - 2.8, corner_r - 1.4);
                }

            // 4x Internal M2 Screw Pilot Bosses (Ø4.2 mm OD, solid from Z = 1.6..12.7)
            for (p = m2_bosses) {
                translate([p[0], p[1], face - eps])
                    cylinder(r = 2.1, h = seam - face + eps, $fn = 24);
            }

            // Internal Cradle Walls (0.6 mm locating walls for PCBs, Z = 6.8..11.9)
            // ESP32-S3 SuperMini (23.5 x 18.0)
            translate([2.4 - 0.6, 3.2 - 0.6, 6.8])
                difference() {
                    cube([23.5 + 1.2, 18.0 + 1.2, 5.1]);
                    translate([0.6, 0.6, -eps])
                        cube([23.5, 18.0, 5.1 + 2*eps]);
                    translate([-eps, 3.8, -eps])
                        cube([1.2, 10.4, 5.1 + 2*eps]);
                }

            // MAX98357A Amp Cradle (17.8 x 19.4)
            translate([27.0 - 0.6, 12.0 - 0.6, 6.8])
                difference() {
                    cube([17.8 + 1.2, 19.4 + 1.2, 4.0]);
                    translate([0.6, 0.6, -eps])
                        cube([17.8, 19.4, 4.0 + 2*eps]);
                }

            // TP4056 Charger Cradle (17.35 x 28.0)
            translate([2.6 - 0.6, tp4056_y - 0.6, 6.8])
                difference() {
                    cube([17.35 + 1.2, 28.0 + 1.2, 4.5]);
                    translate([0.6, 0.6, -eps])
                        cube([17.35, 28.0, 4.5 + 2*eps]);
                }

            // MPU6050 GY-521 Cradle (21.0 x 16.0)
            translate([20.2 - 0.6, 34.7 - 0.6, 6.8])
                difference() {
                    cube([21.0 + 1.2, 16.0 + 1.2, 4.0]);
                    translate([0.6, 0.6, -eps])
                        cube([21.0, 16.0, 4.0 + 2*eps]);
                }

            // SS-12D00 Inward Switch Cradle Wall
            translate([41.4, 40.7, 6.0])
                difference() {
                    cube([49.0 - 41.4, 51.1 - 40.7, 5.65]);
                    translate([0.6, 0.8, -eps])
                        cube([4.0, 8.8, 5.65 + 2*eps]);
                }
        }

        // --- SUBTRACTIONS FROM FRONT SHELL ---

        // 1. Main Interior Cavity (45.0 x 56.5, Z = 1.6..12.7 + lip)
        translate([wall, wall, face])
            rounded_box_prism(inner_w, inner_h, seam - face + 1.5, 2.0);

        // 2. Seam 45-degree Shadow Chamfer (0.4 mm at Z = 12.7)
        translate([-eps, -eps, seam - 0.4])
            difference() {
                cube([bot_w + 2*eps, bot_h + 2*eps, 0.5]);
                translate([0.4, 0.4, -eps])
                    rounded_box_prism(bot_w - 0.8, bot_h - 0.8, 0.6, corner_r - 0.4);
            }

        // 3. 1.3" SH1106 OLED Window & Bevel Chamfer (Single Monolithic Truncated Bevel)
        hull() {
            translate([speaker_x - 31.42/2 - 0.8, 9.35 - 0.8, -eps])
                cube([31.42 + 1.6, 16.70 + 1.6, 0.05]);
            translate([speaker_x - 31.42/2, 9.35, 0.8])
                cube([31.42, 16.70, 0.05]);
        }
        translate([speaker_x - 31.42/2, 9.35, 0.8 - eps])
            cube([31.42, 16.70, face - 0.8 + 2*eps]);

        // Internal OLED PCB Pocket (35.4 x 33.5 x 4.3 mm, Z = 1.6..5.9)
        translate([5.80, 2.00, face - eps])
            cube([35.4, 33.5, 4.3 + eps]);

        // 4. INMP441 Microphone Pocket & Pinhole
        translate([mic_x, mic_y, face - eps])
            cylinder(r = 7.3, h = 3.4 + eps, $fn = 24);
        translate([mic_x, mic_y, -eps])
            cylinder(r = 0.75, h = face + 2*eps, $fn = 20);

        // 5. TTP223 Touch Sensor Pocket & Top Pill Dimple
        translate([speaker_x - 7.5, 1.1, face])
            cube([15.0, 11.0, 2.5]);
        translate([speaker_x - 6.0, -eps, 6.8 - 3.0])
            cube([12.0, 0.25 + eps, 6.0]);

        // 6. Left-Wall USB-C Port Slot (10.0 x 4.5 mm, R1.3, Y = 7.0..17.0, Z = 7.73..12.23)
        translate([-eps, 7.0, 7.73])
            rotate([0, 90, 0])
                linear_extrude(height = wall + 2*eps)
                    rounded_rect_2d(4.5, 10.0, 1.3);

        // 7. Right-Wall Switch Thumb-Well & Actuator Slot
        translate([bot_w - 1.8, 45.9 - 5.2, 9.5 - 2.0])
            cube([1.8 + 2*eps, 10.4, 4.0]);
        translate([bot_w - wall - 1.0, 43.7, 8.4])
            cube([wall + 1.0 + 2*eps, 4.4, 2.2]);

        // 8. Bottom 2-Pin Magnetic Pogo Slot (12.3 x 4.2 mm, centered at X=24.5, Z=9.5)
        translate([speaker_x - 12.3/2, bot_h - wall - eps, pogo_z - 4.2/2])
            cube([12.3, wall + 2*eps, 4.2]);

        // 9. 4x M2 Pilot Bores (Ø1.75 mm, depth 9.0 mm)
        for (p = m2_bosses) {
            translate([p[0], p[1], seam - 9.0])
                cylinder(r = 1.75/2, h = 9.0 + 2*eps, $fn = 20);
        }

        // 10. Side Micro-Flutes (30 horizontal grooves)
        for (i = [0 : 29]) {
            y_flute = 28.0 + i * 0.8;
            translate([-eps, y_flute, 1.6])
                cube([0.3 + eps, 0.4, seam - 1.6 + eps]);
            if (y_flute < 38.9 || y_flute > 52.9) {
                translate([bot_w - 0.3, y_flute, 1.6])
                    cube([0.3 + 2*eps, 0.4, seam - 1.6 + eps]);
            }
        }

        // 11. Front Chin Mustache Deboss (14.4 x 9.0 mm, 0.35 mm deep at X=24.5, Y=36.2, Z=0)
        translate([speaker_x, 36.2, 0])
            mustache_deboss(0.35);
    }
}

// -----------------------------------------------------------------------------
// 3. BODY 02 — REAR SHELL (49.0 x 60.5 x 11.6 mm, Z = 12.7..24.3)
// -----------------------------------------------------------------------------
module speaker_grille_2d_pattern() {
    circle(r = 0.7, $fn = 16);
    for (ring = [1 : 6]) {
        r_ring = 2.6 * ring;
        n_holes = floor(180 / asin(2.4 / (2 * r_ring)));
        for (h_idx = [0 : n_holes - 1]) {
            angle = h_idx * (360 / n_holes);
            translate([r_ring * cos(angle), r_ring * sin(angle)])
                circle(r = 0.7, $fn = 16);
        }
    }
}

module part_02_rear() {
    difference() {
        union() {
            // Flat Rear Body Blank (Z = 12.7 .. 19.5, flat at Z = 12.7 seam face, R1.5 fillet at rear face Z = 19.5)
            rear_shell_blank(bot_w, bot_h, seam, head_depth, corner_r, edge_r);

            // Sculpted +4.8 mm Low-Profile Polished Speaker Dome (Z = 19.5 .. 24.3 mm, Ø45.6 mm flare)
            translate([speaker_x, speaker_y, 0])
                rotate_extrude($fn = 72)
                    polygon(points = [
                        [0.0, pod_depth],
                        [19.2, pod_depth],
                        [19.7, pod_depth - 0.02],
                        [20.2, pod_depth - 0.12],
                        [20.7, pod_depth - 0.35],
                        [21.1, pod_depth - 0.75],
                        [21.5, pod_depth - 1.45],
                        [21.8, pod_depth - 2.50],
                        [22.1, pod_depth - 3.60],
                        [22.4, head_depth + 0.30],
                        [22.6, head_depth + 0.10],
                        [22.8, head_depth],
                        [22.8, 16.0],
                        [0.0, 16.0]
                    ]);
        }

        // --- SUBTRACTIONS FROM REAR SHELL ---

        // 1. Continuous Seam Alignment Groove (Z = 12.7 face, 1.2 wide x 1.5 deep, inset 0.4)
        translate([0.4, 0.4, seam - eps])
            difference() {
                linear_extrude(height = 1.5 + eps)
                    rounded_rect_2d(bot_w - 0.8, bot_h - 0.8, corner_r - 0.4);
                translate([1.2, 1.2, -eps])
                    linear_extrude(height = 1.5 + 3*eps)
                        rounded_rect_2d(bot_w - 3.2, bot_h - 3.2, corner_r - 1.6);
            }

        // 2. Seam 45-degree Shadow Chamfer (0.4 mm at Z = 12.7)
        translate([-eps, -eps, seam - eps])
            difference() {
                cube([bot_w + 2*eps, bot_h + 2*eps, 0.4 + eps]);
                translate([0.4, 0.4, -eps])
                    rounded_box_prism(bot_w - 0.8, bot_h - 0.8, 0.4 + 3*eps, corner_r - 0.4);
            }

        // 3. Open LiPo Cavity (45.0 x 56.5, Z = 12.7 .. 17.5)
        translate([wall, wall, seam - eps])
            rounded_box_prism(inner_w, inner_h, baffle_z - seam + eps, 2.0);

        // 4. Circular Acoustic Baffle Counterbore (Ø42.5 mm, Z = 12.7 .. 18.1)
        translate([speaker_x, speaker_y, seam - eps])
            cylinder(r = 21.25, h = speaker_z - seam + eps, $fn = 48);

        // Annular Sealant Moat (Ø42.1 .. 42.9 mm, Z = 17.5 .. 17.9)
        translate([speaker_x, speaker_y, baffle_z - eps])
            difference() {
                cylinder(r = 21.45, h = 0.4 + eps, $fn = 48);
                translate([0, 0, -eps])
                    cylinder(r = 21.05, h = 0.4 + 3*eps, $fn = 48);
            }

        // 5. Speaker Pocket (Ø40.5 mm, Z = 18.1 .. 23.1)
        translate([speaker_x, speaker_y, speaker_z - eps])
            cylinder(r = speaker_pocket_d / 2, h = speaker_pocket_depth + eps, $fn = 48);

        // Wire-tab notch (1.5 x 3.0 x 4.8 mm at X = 44.25, Y = 37.5)
        translate([44.25 - 1.5/2, speaker_y - 1.5, speaker_z])
            cube([1.5, 3.0, 4.8]);

        // 6. Fast Concentric Speaker Grille Cut (Single 2D Extrusion Operation!)
        translate([speaker_x, speaker_y, pod_depth - grille_wall - eps])
            linear_extrude(height = grille_wall + 2*eps)
                speaker_grille_2d_pattern();

        // 7. 4x M2 Through-Holes (Ø2.2 mm) + Pan-Head Counterbores (Ø4.2 mm, 4.0 mm deep)
        for (p = m2_bosses) {
            translate([p[0], p[1], seam - eps])
                cylinder(r = 1.1, h = head_depth - seam + 2*eps, $fn = 20);
            translate([p[0], p[1], head_depth - 4.0])
                cylinder(r = 2.1, h = 4.0 + 2*eps, $fn = 20);
        }

        // 8. Top-Right Curved Flush Lanyard Tunnel (Ø3.5 mm 90-deg swept tube)
        translate([43.5, 5.5, 15.7])
            rotate_extrude(angle = 90, $fn = 24)
                translate([5.5, 0])
                    circle(r = 1.75, $fn = 20);
        translate([43.5, -eps, 15.7])
            rotate([-90, 0, 0])
                cylinder(r = 1.75, h = 5.5 + 2*eps, $fn = 20);
        translate([43.5, 5.5, 15.7])
            rotate([0, 90, 0])
                cylinder(r = 1.75, h = bot_w - 43.5 + 2*eps, $fn = 20);

        // 9. Side Micro-Flutes matching Body 01 (Z = 13.1 .. 17.9, Y = 28.0 .. 51.6)
        for (i = [0 : 29]) {
            y_flute = 28.0 + i * 0.8;
            translate([-eps, y_flute, seam + 0.4])
                cube([0.3 + eps, 0.4, 4.8]);
            if (y_flute < 38.9 || y_flute > 52.9) {
                translate([bot_w - 0.3, y_flute, seam + 0.4])
                    cube([0.3 + 2*eps, 0.4, 4.8]);
            }
        }

        // 10. Rear Upper Mustache Deboss (14.4 x 9.0 mm, 0.40 mm deep at X = 24.5, Y = 7.2, Z = 19.5)
        translate([speaker_x, 7.2, head_depth - 0.40])
            mustache_deboss(0.40);
    }
}

// -----------------------------------------------------------------------------
// 4. BODY 03 — ACOUSTIC BAFFLE (Ø42.1 x 0.60 mm)
// -----------------------------------------------------------------------------
module part_03_baffle() {
    difference() {
        translate([speaker_x, speaker_y, baffle_z])
            cylinder(r = 21.05, h = baffle_t, $fn = 48);
        translate([44.0, speaker_y, baffle_z - eps])
            cylinder(r = 1.0, h = baffle_t + 2*eps, $fn = 20);
    }
}

// -----------------------------------------------------------------------------
// 5. BODY 04 — BATTERY ISOLATION FLOOR (44.6 x 56.1 x 1.8 mm)
// -----------------------------------------------------------------------------
module part_04_floor() {
    union() {
        // Flat Base Plate (44.6 x 56.1 x 0.8 mm, Z = 11.9 .. 12.7, R1.8 corners)
        translate([2.2, 2.2, 11.9])
            difference() {
                rounded_box_prism(44.6, 56.1, 0.8, 1.8);
                // 4x M2 corner clearance cutouts (R2.4 mm)
                translate([3.5 - 2.2, 4.0 - 2.2, -eps])
                    cylinder(r = 2.4, h = 0.8 + 2*eps, $fn = 20);
                translate([45.0 - 2.2, 9.0 - 2.2, -eps])
                    cylinder(r = 2.4, h = 0.8 + 2*eps, $fn = 20);
                translate([3.5 - 2.2, 56.5 - 2.2, -eps])
                    cylinder(r = 2.4, h = 0.8 + 2*eps, $fn = 20);
                translate([45.0 - 2.2, 56.5 - 2.2, -eps])
                    cylinder(r = 2.4, h = 0.8 + 2*eps, $fn = 20);
            }

        // Locating Rim (1.0 mm tall x 0.8 mm thick, Z = 12.7 .. 13.7, overlaps base by 0.2 to fuse)
        translate([battery_x - 0.8, battery_y - 0.8, seam - 0.2])
            difference() {
                cube([battery_w + 1.6, battery_h + 1.6, 1.0 + 0.2]);
                translate([0.8, 0.8, -eps])
                    cube([battery_w, battery_h, 1.0 + 0.2 + 2*eps]);
                translate([-eps, 2.0, -eps])
                    cube([1.6 + 2*eps, 4.0, 1.0 + 0.2 + 2*eps]);
            }
    }
}

// -----------------------------------------------------------------------------
// 6. BODY 08 — FLUSH SWITCH COLLAR (2.8 x 3.4 x 1.5 mm)
// -----------------------------------------------------------------------------
module part_08_collar() {
    difference() {
        translate([47.5, 44.2, 8.1])
            cube([1.5, 3.4, 2.8]);

        translate([47.5 - eps, 45.9 - 1.65/2, 9.5 - 1.65/2])
            cube([1.5 + 2*eps, 1.65, 1.65]);

        for (y_ridge = [44.9, 45.9, 46.9]) {
            translate([49.0 - 0.25, y_ridge - 0.15, 8.1 - eps])
                cube([0.25 + eps, 0.30, 2.8 + 2*eps]);
        }
    }
}

// -----------------------------------------------------------------------------
// 7. SHARED TILTED DOCK POCKET CUTTER (0.00 mm Nom. Error Interface)
// -----------------------------------------------------------------------------
module dock_pocket_cutter(tilt_deg, floor_offset_z, pocket_center_y) {
    rotate([tilt_deg, 0, 0]) {
        translate([(bot_w - 49.6)/2, pocket_center_y, floor_offset_z]) {
            difference() {
                rounded_box_prism(49.6, 20.3, 70.0, 1.8);
            }
            translate([-1.0, -1.0, 50.0])
                cube([51.6, 22.3, 25.0]);

            translate([49.6/2, 20.3, 23.0])
                rotate([-90, 0, 0])
                    cylinder(r = 22.4, h = 6.8 + eps, $fn = 48);

            translate([49.6/2 - 12.3/2, 20.3 - 10.0 - 4.2/2, -15.0])
                cube([12.3, 4.2, 20.0]);
        }
    }
}

// -----------------------------------------------------------------------------
// 8. BODY 05 — DOCK A: RIVER STONE (88.0 x 68.0 x 26.0 mm, 15 deg Tilt)
// -----------------------------------------------------------------------------
module part_05_dock_a() {
    dock_w = 88.0;
    dock_d = 68.0;
    dock_h = 26.0;

    difference() {
        hull() {
            for (x = [18.0, dock_w - 18.0], y = [18.0, dock_d - 18.0]) {
                translate([x, y, 6.0])
                    cylinder(r = 14.0, h = dock_h - 12.0, $fn = 32);
                translate([x, y, 6.0])
                    sphere(r = 6.0, $fn = 20);
                translate([x, y, dock_h - 6.0])
                    sphere(r = 6.0, $fn = 20);
            }
        }

        translate([(dock_w - bot_w)/2, 10.0, 0])
            dock_pocket_cutter(15.0, 4.5, 12.0);

        translate([(dock_w - 50.0)/2, (dock_d - 40.0)/2, -eps])
            rounded_box_prism(50.0, 40.0, 8.0 + eps, 4.0);

        translate([dock_w/2 - 2.25, dock_d - 25.0, -eps])
            cube([4.5, 25.0 + 2*eps, 5.0]);
        translate([dock_w/2 - 7.0, dock_d - 18.0, -eps])
            cube([14.0, 12.0, 5.0]);

        translate([dock_w/2, 2.5, 10.0])
            rotate([90, 0, 0])
                mustache_deboss(0.40);
    }
}

// -----------------------------------------------------------------------------
// 9. BODY 06 — DOCK B: RIBBON LOOP (55.0 x 58.0 x 54.0 mm, 18 deg Tilt)
// -----------------------------------------------------------------------------
module part_06_dock_b() {
    dock_w = 55.0;
    dock_d = 58.0;
    dock_h = 54.0;

    difference() {
        rotate([90, 0, 90])
            linear_extrude(height = dock_w)
                difference() {
                    polygon(points=[
                        [58.00, 0.00],
                        [0.00, 0.00],
                        [0.00, 4.50],
                        [7.00, 4.50],
                        [7.00, 17.36],
                        [5.30, 17.92],
                        [9.00, 29.33],
                        [32.59, 21.66],
                        [43.10, 54.00],
                        [45.27, 53.29],
                        [45.88, 53.13],
                        [46.50, 53.03],
                        [47.12, 53.00],
                        [47.47, 53.00],
                        [58.00, 4.40]
                    ]);
                    polygon(points=[
                        [44.78, 44.63],
                        [34.96, 14.39],
                        [34.78, 13.69],
                        [34.68, 12.98],
                        [34.67, 12.26],
                        [34.74, 11.55],
                        [34.90, 10.85],
                        [35.15, 10.17],
                        [35.47, 9.53],
                        [35.87, 8.93],
                        [36.33, 8.38],
                        [36.86, 7.89],
                        [37.44, 7.47],
                        [36.48, 4.50],
                        [53.48, 4.50]
                    ]);
                    polygon(points=[
                        [32.02, 4.50],
                        [14.25, 10.27],
                        [13.58, 10.45],
                        [12.89, 10.55],
                        [12.19, 10.56],
                        [11.50, 10.50],
                        [11.54, 9.78],
                        [11.67, 9.06],
                        [11.89, 8.37],
                        [12.19, 7.71],
                        [12.56, 7.09],
                        [13.01, 6.52],
                        [13.52, 6.01],
                        [14.09, 5.56],
                        [14.71, 5.19],
                        [15.37, 4.89],
                        [16.06, 4.67],
                        [16.78, 4.54],
                        [17.50, 4.50]
                    ]);
                };

        translate([(dock_w - bot_w)/2, 6.0, 0])
            dock_pocket_cutter(18.0, 3.5, 10.0);

        translate([dock_w/2, 45.0, 26.0])
            rotate([-18.0, 0, 0])
                cylinder(r = 17.0, h = 30.0, center = true, $fn = 32);

        translate([(dock_w - 42.0)/2, (dock_d - 42.0)/2, -eps])
            cube([42.0, 42.0, 4.0 + eps]);

        translate([dock_w/2, 7.15, 23.63])
            rotate([-18.0, 0, 0])
                rotate([90, 0, 0])
                    mustache_deboss(0.40);
    }
}

// -----------------------------------------------------------------------------
// 10. BODY 07 — DOCK C: PLINTH & TRAY (98.0 x 60.0 x 30.0 mm, 15 deg Tilt)
// -----------------------------------------------------------------------------
module part_07_dock_c() {
    dock_w = 98.0;
    dock_d = 60.0;
    dock_h = 30.0;
    plinth_w = 58.0;

    difference() {
        union() {
            rounded_box_prism(plinth_w, dock_d, dock_h, 3.0);
            translate([plinth_w - 2.0, 0, 0])
                rounded_box_prism(dock_w - plinth_w + 2.0, dock_d, 14.0, 3.0);
        }

        translate([(plinth_w - bot_w)/2, 6.0, 0])
            dock_pocket_cutter(15.0, 6.0, 11.0);

        translate([plinth_w + 1.0, (dock_d - 48.0)/2, 14.0 - 8.0])
            rounded_box_prism(38.0, 48.0, 8.0 + eps, 2.5);

        translate([(dock_w - 78.0)/2, (dock_d - 44.0)/2, -eps])
            rounded_box_prism(78.0, 44.0, 8.0 + eps, 4.0);

        translate([plinth_w/2 - 2.25, dock_d - 22.0, -eps])
            cube([4.5, 22.0 + 2*eps, 5.0]);

        translate([plinth_w/2, -eps, 15.0])
            rotate([90, 0, 0])
                mustache_deboss(0.40);
    }
}

// -----------------------------------------------------------------------------
// Top-Level Assembly / Selector Execution
// -----------------------------------------------------------------------------
if (PART == "all") {
    if (PRINT_LAYOUT) {
        translate([0, 0, 0]) part_01_front();
        translate([60, 0, 0]) part_02_rear();
        translate([120, 0, 0]) part_03_baffle();
        translate([120, 50, 0]) part_04_floor();
        translate([120, 110, 0]) part_08_collar();
        translate([0, 80, 0]) part_05_dock_a();
        translate([95, 80, 0]) part_06_dock_b();
        translate([0, 160, 0]) part_07_dock_c();
    } else {
        part_01_front();
        part_02_rear();
        part_03_baffle();
        part_04_floor();
        part_08_collar();
        translate([70, 0, 0]) part_05_dock_a();
        translate([170, 0, 0]) part_06_dock_b();
        translate([240, 0, 0]) part_07_dock_c();
    }
} else if (PART == "01_front") {
    part_01_front();
} else if (PART == "02_rear") {
    part_02_rear();
} else if (PART == "03_baffle") {
    part_03_baffle();
} else if (PART == "04_floor") {
    part_04_floor();
} else if (PART == "08_collar") {
    part_08_collar();
} else if (PART == "05_dock_a") {
    part_05_dock_a();
} else if (PART == "06_dock_b") {
    part_06_dock_b();
} else if (PART == "07_dock_c") {
    part_07_dock_c();
}
