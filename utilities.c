/*
    (c) 2009 by Leon Winter
    (c) 2009, 2010 by Hannes Schueller
    (c) 2009, 2010 by Matto Fransen
    (c) 2010 by Hans-Peter Deifel
    (c) 2010 by Thomas Adam
    see LICENSE file
*/

#include "includes.h"
#include "vimprobable.h"
#include "main.h"
#include "utilities.h"

extern char commandhistory[COMMANDHISTSIZE][255];
extern Command commands[COMMANDSIZE];
extern int lastcommand, maxcommands, commandpointer;
extern KeyList *keylistroot;
extern Key keys[];
extern char *error_msg;
extern gboolean complete_case_sensitive;

gboolean read_rcfile(const char *config)
{
	int t;
	char s[255];
	FILE *fpin;
	gboolean returnval = TRUE;

	if ((fpin = fopen(config, "r")) == NULL)
		return FALSE;
	while (fgets(s, 254, fpin)) {
		/*
		 * ignore lines that begin with #, / and such 
		 */
		if (!isalpha(s[0]))
			continue;
		t = strlen(s);
		s[t - 1] = '\0';
		if (!process_line(s))
			returnval = FALSE;
	}
	fclose(fpin);
	return returnval;
}

void save_command_history(char *line)
{
	char *c;

	c = line;
	while (isspace(*c) && *c)
		c++;
	if (!strlen(c))
		return;
	strncpy(commandhistory[lastcommand], c, 254);
	lastcommand++;
	if (maxcommands < COMMANDHISTSIZE - 1)
		maxcommands++;
	if (lastcommand == COMMANDHISTSIZE)
		lastcommand = 0;
	commandpointer = lastcommand;
}

gboolean
process_save_qmark(const char *bm, WebKitWebView *webview)
{
    FILE *fp;
    const char *filename;
    const char *uri = webkit_web_view_get_uri(webview);
    char qmarks[10][101];
    char buf[100];
    int  i, mark, l=0;
    Arg a;
    mark = -1;
    mark = atoi(bm);
    if ( mark < 1 || mark > 9 ) 
    {
	    a.i = Error;
	    a.s = g_strdup_printf("Invalid quickmark, only 1-9");
	    echo(&a);
	    return TRUE;
    }	    
    if ( uri == NULL ) return FALSE;
    for( i=0; i < 9; ++i ) strcpy( qmarks[i], "");

    filename = g_strdup_printf(QUICKMARK_FILE);

    /* get current quickmarks */
    
    fp = fopen(filename, "r");
    if (fp != NULL){
       for( i=0; i < 10; ++i ) {
           if (feof(fp)) {
               break;
           }
           fgets(buf, 100, fp);
	   l = 0;
	   while (buf[l] && l < 100 && buf[l] != '\n') {
		   qmarks[i][l]=buf[l]; 
		   l++;
	  }	   
          qmarks[i][l]='\0';
       }
       fclose(fp);
    }

    /* save quickmarks */
    strcpy( qmarks[mark-1], uri );
    fp = fopen(filename, "w");
    if (fp == NULL) return FALSE;
    for( i=0; i < 10; ++i ) 
        fprintf(fp, "%s\n", qmarks[i]);
    fclose(fp);
    a.i = Error;
    a.s = g_strdup_printf("Saved as quickmark %d: %s", mark, uri);
    echo(&a);

    return TRUE;
}

void
make_keyslist(void) 
{
    int i;
    KeyList *ptr, *current;

    ptr     = NULL;
    current = NULL;
    i       = 0;
    while ( keys[i].key != 0 )
    {
        current = malloc(sizeof(KeyList));
        if (current == NULL) {
            printf("Not enough memory\n");
            exit(-1);
        }
        current->Element = keys[i];
        current->next = NULL;
        if (keylistroot == NULL) keylistroot = current;
        if (ptr != NULL) ptr->next = current;
        ptr = current;
        i++;
    }
}

gboolean
parse_colour(char *color) {
    char goodcolor[8];
    int colorlen;

    colorlen = (int)strlen(color);

    goodcolor[0] = '#';
    goodcolor[7] = '\0';

    /* help the user a bit by making string like
       #a10 and strings like ffffff full 6digit
       strings with # in front :)
     */

    if (color[0] == '#') {
        switch (colorlen) {
            case 7:
                strncpy(goodcolor, color, 7);
            break;
            case 4:
                goodcolor[1] = color[1];
                goodcolor[2] = color[1];
                goodcolor[3] = color[2];
                goodcolor[4] = color[2];
                goodcolor[5] = color[3];
                goodcolor[6] = color[3];
            break;
            case 2:
                goodcolor[1] = color[1];
                goodcolor[2] = color[1];
                goodcolor[3] = color[1];
                goodcolor[4] = color[1];
                goodcolor[5] = color[1];
                goodcolor[6] = color[1];
            break;
        }
    } else {
        switch (colorlen) {
            case 6:
                strncpy(&goodcolor[1], color, 6);
            break;
            case 3:
                goodcolor[1] = color[0];
                goodcolor[2] = color[0];
                goodcolor[3] = color[1];
                goodcolor[4] = color[1];
                goodcolor[5] = color[2];
                goodcolor[6] = color[2];
            break;
            case 1:
                goodcolor[1] = color[0];
                goodcolor[2] = color[0];
                goodcolor[3] = color[0];
                goodcolor[4] = color[0];
                goodcolor[5] = color[0];
                goodcolor[6] = color[0];
            break;
        }
    }

    if (strlen (goodcolor) != 7) {
        return FALSE;
    } else {
        strncpy(color, goodcolor, 8);
        return TRUE;
    }
}

gboolean
changemapping(Key * search_key, int maprecord) {
    KeyList *current, *newkey;

    current = keylistroot;

    if (current)
        while (current->next != NULL) {
            if (
                current->Element.mask   == search_key->mask &&
                current->Element.modkey == search_key->modkey &&
                current->Element.key    == search_key->key
               ) {
                current->Element.func = commands[maprecord].func;
                current->Element.arg  = commands[maprecord].arg;
                return TRUE;
            }
            current = current->next;
        }
    newkey = malloc(sizeof(KeyList));
    if (newkey == NULL) {
        printf("Not enough memory\n");
        exit (-1);
    }
    newkey->Element.mask   = search_key->mask;
    newkey->Element.modkey = search_key->modkey;
    newkey->Element.key    = search_key->key;
    newkey->Element.func   = commands[maprecord].func;
    newkey->Element.arg    = commands[maprecord].arg;
    newkey->next           = NULL;

    if (keylistroot == NULL) keylistroot = newkey;

    if (current != NULL) current->next = newkey;

    return TRUE;
}

gboolean
mappings(const Arg *arg) {
    char line[255];

    if ( !arg->s ) {
        set_error("Missing argument.");
        return FALSE;
    }
    strncpy(line, arg->s, 254);
    if (process_map_line(line))
        return TRUE;
    else {
        set_error("Invalid mapping.");
        return FALSE;
    }
}

gboolean
process_mapping(char * keystring, int maprecord) {
    Key search_key;

    search_key.mask   = 0;
    search_key.modkey = 0;
    search_key.key    = 0;

    if (strlen(keystring) == 1) {
        search_key.key = keystring[0];
    }

    if (strlen(keystring) == 2) {
        search_key.modkey= keystring[0];
        search_key.key = keystring[1];
    }

    /* process stuff like <S-v> for Shift-v or <C-v> for Ctrl-v
       or stuff like <S-v>a for Shift-v,a or <C-v>a for Ctrl-v,a
    */
    if ((strlen(keystring) == 5 ||  strlen(keystring) == 6)  && keystring[0] == '<'  && keystring[4] == '>') {
        switch (toupper(keystring[1])) {
            case 'S':
                search_key.mask = GDK_SHIFT_MASK;
                if (strlen(keystring) == 5) {
                    keystring[3] = toupper(keystring[3]);
                } else {
                    keystring[3] = tolower(keystring[3]);
                    keystring[5] = toupper(keystring[5]);
                }
            break;
            case 'C':
                search_key.mask = GDK_CONTROL_MASK;
            break;
        }
        if (!search_key.mask)
            return FALSE;
        if (strlen(keystring) == 5) {
            search_key.key = keystring[3];
        } else {
            search_key.modkey= keystring[3];
            search_key.key = keystring[5];
        }
    }

    /* process stuff like <S-v> for Shift-v or <C-v> for Ctrl-v
       or  stuff like a<S-v> for a,Shift-v or a<C-v> for a,Ctrl-v
    */
    if (strlen(keystring) == 6 && keystring[1] == '<' && keystring[5] == '>') {
        switch (toupper(keystring[2])) {
            case 'S':
                search_key.mask = GDK_SHIFT_MASK;
                keystring[4] = toupper(keystring[4]);
            break;
            case 'C':
                search_key.mask = GDK_CONTROL_MASK;
            break;
        }
        if (!search_key.mask)
            return FALSE;
        search_key.modkey= keystring[0];
        search_key.key = keystring[4];
    }
    return (changemapping(&search_key, maprecord));
}

gboolean
process_map_line(char *line) {
    int listlen, i;
    char *c;
    my_pair.line = line;
    c = search_word (0);

    if (!strlen (my_pair.what))
        return FALSE;
    while (isspace (*c) && *c)
        c++;

    if (*c == ':' || *c == '=')
        c++;
    my_pair.line = c;
    c = search_word (1);
    if (!strlen (my_pair.value))
        return FALSE;
    listlen = LENGTH(commands);
    for (i = 0; i < listlen; i++) {
        if (strlen(commands[i].cmd) == strlen(my_pair.value) && strncmp(commands[i].cmd, my_pair.value, strlen(my_pair.value)) == 0)
            return process_mapping(my_pair.what, i);
    }
    return FALSE;
}

gboolean
build_taglist(const Arg *arg, FILE *f) {
    int k = 0, in_tag = 0;
    int t = 0, marker = 0;
    char foundtab[MAXTAGSIZE+1];
    while (arg->s[k]) {
        if (!isspace(arg->s[k]) && !in_tag) {
            in_tag = 1;
            marker = k;
        }
        if (isspace(arg->s[k]) && in_tag) {
            /* found a tag */
            t = 0;
            while (marker < k && t < MAXTAGSIZE) foundtab[t++] = arg->s[marker++];
            foundtab[t] = '\0';
            fprintf(f, " [%s]", foundtab);
            in_tag = 0;
        }
        k++;
    }
    if (in_tag) {
        t = 0;
        while (marker < strlen(arg->s) && t < MAXTAGSIZE) foundtab[t++] = arg->s[marker++];
        foundtab[t] = '\0';
        fprintf(f, " [%s]", foundtab );
    }
    return TRUE;
}

void
set_error(const char *error) {
    /* it should never happen that set_error is called more than once, 
     * but to avoid any potential memory leaks, we ignore any subsequent 
     * error if the current one has not been shown */
    if (error_msg == NULL) {
        error_msg = g_strdup_printf("%s", error);
    }
}

void 
give_feedback(const char *feedback) 
{ 
    Arg a = { .i = Info };

    a.s = g_strdup_printf(feedback);
    echo(&a);
}

Listelement *
complete_list(const char *searchfor, const int mode, Listelement *elementlist)
{
    FILE *f;
    const char *filename;
    Listelement *candidatelist = NULL, *candidatepointer = NULL;
    char s[255] = "", readelement[MAXTAGSIZE + 1] = "";
    int i, t, n = 0;

    if (mode == 2) {
        /* open in history file */
        filename = g_strdup_printf(HISTORY_STORAGE_FILENAME);
    } else {
        /* open in bookmark file (for tags and bookmarks) */
        filename = g_strdup_printf(BOOKMARKS_STORAGE_FILENAME);
    }
    f = fopen(filename, "r");
    if (f == NULL) {
        g_free((gpointer)filename);
        return (elementlist);
    }

    while (fgets(s, 254, f)) {
        if (mode == 1) {
            /* just tags (could be more than one per line) */
            i = 0;
            while (s[i] && i < 254) {
                while (s[i] != '[' && s[i])
                    i++;
                if (s[i] != '[')
                    continue;
                i++;
                t = 0;
                while (s[i] != ']' && s[i] && t < MAXTAGSIZE)
                    readelement[t++] = s[i++];
                readelement[t] = '\0';
                candidatelist = add_list(readelement, candidatelist);
                i++;
            }
        } else {
            /* complete string (bookmarks & history) */
            candidatelist = add_list(s, candidatelist);
        }
        candidatepointer = candidatelist;
        while (candidatepointer != NULL) {
            if (!complete_case_sensitive) {
               g_strdown(candidatepointer->element);
            }
            if (!strlen(searchfor) || strstr(candidatepointer->element, searchfor) != NULL) {
                /* only use string up to the first space */
                memset(readelement, 0, MAXTAGSIZE + 1);
                if (strchr(candidatepointer->element, ' ') != NULL) {
                    i = strcspn(candidatepointer->element, " ");
                    strncpy(readelement, candidatepointer->element, i);
                } else {
                    strncpy(readelement, candidatepointer->element, MAXTAGSIZE);
                }
                /* in the case of URLs without title, remove the line break */
                if (readelement[strlen(readelement) - 1] == '\n') {
                    readelement[strlen(readelement) - 1] = '\0';
                }
                elementlist = add_list(readelement, elementlist);
                n = count_list(elementlist);
            }
            if (n >= MAX_LIST_SIZE)
                break;
            candidatepointer = candidatepointer->next;
        }
        free_list(candidatelist);
        candidatelist = NULL;
        if (n >= MAX_LIST_SIZE)
            break;
    }
    g_free((gpointer)filename);
    return (elementlist);
}

Listelement *
add_list(const char *element, Listelement *elementlist)
{
    int n, i = 0;
    Listelement *newelement, *elementpointer, *lastelement;

    if (elementlist == NULL) { /* first element */
        newelement = malloc(sizeof(Listelement));
        if (newelement == NULL) 
            return (elementlist);
        strncpy(newelement->element, element, 254);
        newelement->next = NULL;
        return newelement;
    }
    elementpointer = elementlist;
    n = strlen(element);

    /* check if element is already in list */
    while (elementpointer != NULL) {
        if (strlen(elementpointer->element) == n && 
                strncmp(elementpointer->element, element, n) == 0)
            return (elementlist);
        lastelement = elementpointer;
        elementpointer = elementpointer->next;
        i++;
    }
    /* add to list */
    newelement = malloc(sizeof(Listelement));
    if (newelement == NULL)
        return (elementlist);
    lastelement->next = newelement;
    strncpy(newelement->element, element, 254);
    newelement->next = NULL;
    return elementlist;
}

void
free_list(Listelement *elementlist)
{
    Listelement *elementpointer;

    while (elementlist != NULL) {
        elementpointer = elementlist->next;
        free(elementlist);
        elementlist = elementpointer;
    }
}

int
count_list(Listelement *elementlist)
{
    Listelement *elementpointer = elementlist;
    int n = 0;

    while (elementpointer != NULL) {
        n++;
        elementpointer = elementpointer->next;
    }
    
    return n;
}
