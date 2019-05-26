/**
 * This file is part of Portmino.
 * 
 * Portmino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Portmino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Portmino.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "input.h"
#include "render.h"

/**
 * What kind of screen is this?
 */
typedef enum {
    SCREEN_NONE,
    SCREEN_MAINMENU,
    SCREEN_RULESETMENU,
    SCREEN_PLAYMENU,
    SCREEN_INGAME,
    SCREEN_PAUSEMENU,
} screentype_t;

typedef struct ingame_s ingame_t;
typedef struct mainmenu_s mainmenu_t;
typedef struct pausemenu_s pausemenu_t;
typedef struct playmenu_s playmenu_t;
typedef struct rulesetmenu_s rulesetmenu_t;
typedef struct screen_s screen_t;
typedef struct screens_s screens_t;

/**
 * Screen configuration
 */
typedef struct {
    /**
     * Screen type.
     */
    screentype_t type;

    /**
     * Run one frame's worth of screen logic
     * 
     * Return an integer >0 to be routed to the navigation function below.
     */
    int (*frame)(screen_t* screen, const gameinputs_t* inputs);

    /**
     * Navigate to the proper screen
     * 
     * Here, you are expected to either push a new screen onto the screen
     * stack or pop the current screen from the stack.
     */
    void (*navigate)(screens_t* screens, int result);

    /**
     * Screen renderer
     * 
     * Just what it says on the tin.  Call your draw functions here.
     */
    void (*render)(screen_t* screen);

    /**
     * Screen destructor
     */
    void (*destruct)(screen_t* screen);
} screen_config_t;

/**
 * The complete screen data structure.
 */
struct screen_s {
    /**
     * Screen configuration
     */
    screen_config_t config;

    /**
     * Screen data union
     * 
     * Do not touch this member when initializing.
     */
    union screen_u {
        mainmenu_t* mainmenu;
        pausemenu_t* pausemenu;
        playmenu_t* playmenu;
        rulesetmenu_t* rulesetmenu;
        ingame_t* ingame;
    } screen;
};

#define MAX_SCREENS 8

/**
 * A stack of screens.
 */
typedef struct screens_s {
    /**
     * The actual stack of screen structs.
     */
    screen_t screens[MAX_SCREENS];

    /**
     * Number of active screens.
     */
    uint8_t screen_count;
} screens_t;

void screens_init(screens_t* screens);
void screens_deinit(screens_t* screens);
screen_t* screens_top(screens_t* screens);
bool screens_push(screens_t* screens, screen_t screen);
bool screens_pop(screens_t* screens);
bool screens_pop_until(screens_t* screens, screentype_t type);
void screens_frame(screens_t* screens, const gameinputs_t* inputs);
void screens_render(screens_t* screens);
