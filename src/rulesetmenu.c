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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "physfs.h"

#include "screen.h"

/**
 * Number of lines in the selector.
 */
#define RULESET_LINES 25

typedef struct rulesetmenu_s {
    /**
     * Currently selected ruleset.
     */
    int selected;

    /**
     * Ruleset filenames.
     */
    char** rulesets;

    /**
     * Ruleset name count.
     */
    size_t ruleset_count;

    /**
     * Held keys.
     */
    gameholds_t holds;
} rulesetmenu_t;

/**
 * Process events on the ruleset menu
 */
static int rulesetmenu_frame(screen_t* screen, const gameevents_t* events) {
    rulesetmenu_t* menu = screen->screen.rulesetmenu;
    playerevents_t mevents = event_menu_filter(&menu->holds, events);

    if (mevents.events[0] & MEVENT_UP) {
        menu->selected = (menu->selected + menu->ruleset_count - 1) % menu->ruleset_count;
    }
    if (mevents.events[0] & MEVENT_DOWN) {
        menu->selected = (menu->selected + 1) % menu->ruleset_count;
    }
    if (mevents.events[0] & MEVENT_OK) {
        return menu->selected + 1;
    }

    return 0;
}

/**
 * Navigate to the proper destination
 */
static void rulesetmenu_navigate(screens_t* screens, int result) {
    
}

typedef struct {
    size_t pages;
    size_t currentpage;
    size_t startindex;
    size_t endindex;
} pageinfo_t;

/**
 * Paginate a list of items
 *
 * Pass in the page size, current (0-indexed) index, and total number of items.
 */
static pageinfo_t paginator(size_t pagesize, size_t index, size_t total) {
    pageinfo_t info = { 0 };
    if (total == 0) {
        return info;
    }

    info.pages = ((total - 1) / pagesize) + 1;
    info.currentpage = (index / pagesize) + 1;
    info.startindex = pagesize * (info.currentpage - 1);
    info.endindex = info.startindex + pagesize - 1;
    if (info.endindex > total - 1) {
        info.endindex = total - 1;
    }

    return info;
}

static void rulesetmenu_render(screen_t* screen, render_module_t* render) {
    rulesetmenu_t* menu = screen->screen.rulesetmenu;

    render->clear();
    render->draw_font(vec2i(100, 4), "Rulesets");

    // Paginate our ruleset.
    char pagedesc[32] = { 0 };
    pageinfo_t pageinfo = paginator(RULESET_LINES, menu->selected, menu->ruleset_count);
    snprintf(pagedesc, sizeof(pagedesc), "Page: %zu/%zu", pageinfo.currentpage, pageinfo.pages);

    // Draw the paginator.
    render->draw_font(vec2i(50, 228), pagedesc);

    // Draw our rules.
    size_t j = 0;
    for (size_t i = pageinfo.startindex;i <= pageinfo.endindex;i++,j++) {
        size_t y = 16 + (j * 8);
        render->draw_font(vec2i(50, y), menu->rulesets[i]);
        if (menu->selected == i) {
            render->draw_font(vec2i(42, y), ">");
        }
    }
}

static void rulesetmenu_delete(screen_t* screen) {
    if (screen->screen.rulesetmenu != NULL) {
        rulesetmenu_t* menu = screen->screen.rulesetmenu;
        for (size_t i = 0;i < menu->ruleset_count;i++) {
            free(menu->rulesets[i]);
            menu->rulesets[i] = NULL;
        }
        free(screen->screen.rulesetmenu);
        screen->screen.rulesetmenu = NULL;
    }
}

screen_config_t ruleset_screen = {
    SCREEN_RULESETMENU,
    rulesetmenu_frame,
    rulesetmenu_navigate,
    rulesetmenu_render,
    rulesetmenu_delete
};

/**
 * Allocate ruleset selection screen
 */
screen_t rulesetmenu_new(void) {
    screen_t screen;
    screen.config.type = SCREEN_NONE;

    rulesetmenu_t* menu = calloc(1, sizeof(rulesetmenu_t));
    if (menu == NULL) {
        return screen;
    }
    screen.config.type = SCREEN_RULESETMENU;

    // Check our rulesets directory
    char** files = PHYSFS_enumerateFiles("ruleset");
    for (char** i = files;*i != NULL;i++) {
        // Try and find a file called main.lua in every directory
        char* mainpath;
        int ok = asprintf(&mainpath, "ruleset/%s/main.lua", *i);
        if (ok < 0) {
            goto fail;
        }

        PHYSFS_Stat stat;
        ok = PHYSFS_stat(mainpath, &stat);
        free(mainpath);
        if (ok == 0) {
            // File does not exist.
            continue;
        }
        if (stat.filetype != PHYSFS_FILETYPE_REGULAR) {
            // File isn't a file.
            continue;
        }

        // Duplicate the ruleset name, so we can hold onto it.
        char* newruleset = strdup(*i);
        if (newruleset == NULL) {
            goto fail;
        }

        // Resize the rulesets array.
        char** newrulesets = realloc(menu->rulesets, sizeof(char*) * (menu->ruleset_count + 1));
        if (newrulesets == NULL) {
            free(newruleset);
            goto fail;
        }

        // Push our ruleset name to the end of the list.
        menu->rulesets = newrulesets;
        menu->rulesets[menu->ruleset_count] = newruleset;
        menu->ruleset_count += 1;
    }
    PHYSFS_freeList(files);

    menu->selected = 0;
    event_holds_init(&menu->holds);

    screen.config = ruleset_screen;
    screen.screen.rulesetmenu = menu;
    return screen;

fail:
    rulesetmenu_delete(&screen);
    screen.config.type = SCREEN_NONE;
    return screen;
}
