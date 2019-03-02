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

#include "screen.h"

#include <stdlib.h>
#include <string.h>

/**
 * Initialize a screens struct inplace
 */
void screens_init(screens_t* screens) {
    memset(screens, 0, sizeof(*screens));
}

/**
 * Deinitialize a screens struct and all screens contained inside
 */
void screens_deinit(screens_t* screens) {
    while (screens_top(screens) != NULL) {
        screens_pop(screens);
    }
}

/**
 * Get the top screen
 */
screen_t* screens_top(screens_t* screens) {
    if (screens->screen_count == 0) {
        return NULL;
    }

    return &screens->screens[screens->screen_count - 1];
}

/**
 * Push a new screen to the screen stack
 */
bool screens_push(screens_t* screens, screen_t screen) {
    if (screen.config.type == SCREEN_NONE) {
        // Screen didn't allocate.
        return false;
    }

    if (screens->screen_count >= MAX_SCREENS) {
        // We can't push another screen.
        screen.config.destruct(&screen);
        return false;
    }

    // Copy screen data to the screen stack.
    screens->screens[screens->screen_count] = screen;
    screens->screen_count += 1;

    return true;
}

/**
 * Pop the current screen from the stack
 */
bool screens_pop(screens_t* screens) {
    screen_t* screen = screens_top(screens);
    if (screen == NULL) {
        // No screen to pop.
        return false;
    }

    // Delete the screen context.
    screen->config.destruct(screen);
    screen->config.type = SCREEN_NONE;
    screens->screen_count -= 1;

    // We've popped a screen.
    return true;
}

/**
 * Run a single frame of the screen with priority
 */
void screens_frame(screens_t* screens, const gameevents_t* events) {
    screen_t* screen = screens_top(screens);
    if (screen == NULL) {
        // Do nothing.
        return;
    }

    // Run the frame for the given stack entry.
    int result = screen->config.frame(screen, events);
    if (result != 0) {
        // We want to navigate away from the screen.  Obey our wishes.
        screen->config.navigate(screens, result);
    }
}

/**
 * Draw the screen that has priority
 */
void screens_render(screens_t* screens) {
    screen_t* screen = screens_top(screens);
    if (screen == NULL) {
        // Do nothing.
        return;
    }

    // Run the frame for the given stack entry.
    screen->config.render(screen);
}
