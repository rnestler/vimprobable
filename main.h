/*
    (c) 2009 by Leon Winter
    (c) 2009, 2010 by Hannes Schueller
    (c) 2009, 2010 by Matto Fransen
    (c) 2010 by Hans-Peter Deifel
    (c) 2010 by Thomas Adam
    see LICENSE file
*/
#ifndef VP_MAIN_H
#define VP_MAIN_H

#include <X11/Xlib.h>
#include "includes.h"
#include "vimprobable.h"
#include "utilities.h"
#include "callbacks.h"
#include "javascript.h"

/* the CLEAN_MOD_*_MASK defines have all the bits set that will be stripped from the modifier bit field */
#define CLEAN_MOD_NUMLOCK_MASK (GDK_MOD2_MASK)
#define CLEAN_MOD_BUTTON_MASK (GDK_BUTTON1_MASK|GDK_BUTTON2_MASK|GDK_BUTTON3_MASK|GDK_BUTTON4_MASK|GDK_BUTTON5_MASK)

/* remove unused bits, numlock symbol and buttons from keymask */
#define CLEAN(mask) (mask & (GDK_MODIFIER_MASK) & ~(CLEAN_MOD_NUMLOCK_MASK) & ~(CLEAN_MOD_BUTTON_MASK))

#define IS_ESCAPE(event) (IS_ESCAPE_KEY(CLEAN(event->state), event->keyval))
#define IS_ESCAPE_KEY(s, k) ((s == 0 && k == GDK_Escape) || (s == GDK_CONTROL_MASK && k == GDK_bracketleft))

/* functions */
void fill_suggline(char * suggline, const char * command, const char *fill_with);
void save_command_history(char *line);
void toggle_proxy(gboolean onoff);
void toggle_scrollbars(gboolean onoff);
void update_state(void);
gboolean echo(const Arg *arg);
gboolean echo(const Arg *arg);
gboolean process_keypress(GdkEventKey *event);
gboolean process_line(char *line);
char * search_word(int whichword);
GtkWidget * fill_eventbox(const char * completion_line);

#endif
