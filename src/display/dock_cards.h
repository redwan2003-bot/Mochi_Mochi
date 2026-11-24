#pragma once

#include <U8g2lib.h>
#include "app_state.h"

// ============================================================================
// Mochi Bot V1 - Milestone 4 Dock Glance Cards
// Provides interactive glanceable cards on the 1.3" OLED when docked:
// Card 0: Big Smart Clock & Date Glance
// Card 1: Expressive Sleepy Face with breathing animation
// Card 2: Hardware Telemetry (5V Pogo, TP4056 charge, Heap, Uptime, Mic)
// ============================================================================

enum DockCardType {
    DOCK_CARD_CLOCK = 0,
    DOCK_CARD_SLEEPY_FACE = 1,
    DOCK_CARD_TELEMETRY = 2,
    DOCK_CARD_PC_STATS = 3,
    DOCK_CARD_COUNT = 4
};

// Cycles to next dock card (triggered by capacitive touch while docked)
void dock_cards_next();

// Sets active dock glance card
void dock_cards_set(DockCardType card);

// Returns active dock glance card
DockCardType dock_cards_get_current();

// Renders the currently active dock glance card
void dock_cards_render(U8G2& u8g2, const AppContext& ctx);
