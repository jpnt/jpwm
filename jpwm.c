/* see LICENSE for copyright and license */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <xcb/randr.h>
#include <xcb/xcb_keysyms.h>
//#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
//#include <xcb/xcb_icccm.h>
//#include <xcb/xcb_ewmh.h>
#include <X11/keysym.h>

#ifdef DEBUG
#define debug_print(...)	fprintf(stderr, __VA_ARGS__);
#else
#define debug_print(...)	;
#endif /* DEBUG */

#define True		1
#define False		0
#define Mod1Mask	XCB_MOD_MASK_1
#define Mod4Mask	XCB_MOD_MASK_4
#define ShiftMask	XCB_MOD_MASK_SHIFT
#define ControlMask	XCB_MOD_MASK_CONTROL
#define Button1		XCB_BUTTON_INDEX_1
#define Button2		XCB_BUTTON_INDEX_2
#define Button3		XCB_BUTTON_INDEX_3
#define XCB_MOVE_RESIZE	XCB_CONFIG_WINDOW_X | \
	XCB_CONFIG_WINDOW_Y | XCB_CONFIG_WINDOW_WIDTH | \
	XCB_CONFIG_WINDOW_HEIGHT
#define XCB_MOVE	XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y
#define XCB_RESIZE	XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT

static char *WM_ATOM_NAME[] = { "WM_PROTOCOLS", "WM_DELETE_WINDOW" };
static char *NET_ATOM_NAME[] = { "_NET_SUPPORTED", "_NET_WM_STATE_FULLSCREEN",
	"_NET_WM_STATE", "_NET_ACTIVE_WINDOW" };

#define LENGTH(x)	(sizeof(x)/sizeof(*x))
#define CLEANMASK(mask)	(mask & ~(numlockmask | XCB_MOD_MASK_LOCK))
#define BUTTONMASK	XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE

enum { RESIZE, MOVE };
enum { TILE, MONOCLE, BSTACK, GRID, MODES };
enum { WM_PROTOCOLS, WM_DELETE_WINDOW, WM_COUNT };
enum { NET_SUPPORTED, NET_FULLSCREEN, NET_WM_STATE, NET_ACTIVE, NET_COUNT };

typedef union {
	const char **com; /* command to run */
	const uint32_t i; /* integer to indicate state */
} Arg;

/* structures */
typedef struct {
	uint32_t mod; /* modifier mask */
	xcb_keysym_t keysym; /* key pressed */
	void (*func)(const Arg *); /* function to be triggered */
	const Arg arg; /* argument to the function */
} Key;

typedef struct {
	uint32_t mask; /* modifier mask */
	uint32_t button; /* mouse button pressed */
	void (*func)(const Arg *); /* function to be triggered */
	const Arg arg; /* argument to the function */
} Button;

typedef struct Client {
	struct Client *next; /* client after this one, NULL if last one */
	xcb_window_t win; /* the window this client represents */
	uint8_t flags; /* bitmask for window state flags */
} Client;

#define CLIENT_F_URGENT		0x1
#define CLIENT_F_TRANSIENT	0x2
#define CLIENT_F_FULLSCRN	0x4
#define CLIENT_F_FLOATING	0x8
#define CLIENT_F_STICKY		0x10

typedef struct {
	int8_t mode; /* tiling layout mode */
	float master_size; /* size of master window */
	Client *head; /* head of the client list */
	Client *current; /* current focused client */
	Client *prevfocus; /* previously focused client */
	uint8_t showpanel; /* visibility status of the panel */
} Workspace;

//typedef struct {
//	const char *class; /* the class name of the instance */
//	const int8_t workspace; /* what workspace it should be spawned at */
//	const uint8_t floating; /* spawn in floating mode */
//} AppRule;

// TODO: reduce amount of functions and complexity
/* function prototypes */
static void switch_mode(const Arg *arg);
static void toggle_bar(void);
static void kill_client(void);
static void force_kill_client(void);
static void next_win(void);
static void prev_win(void);
static void resize_master(const Arg *arg);
static void mouse_aside(void);
static void rotate(const Arg *arg);
static void swap_master(void);
static void last_workspace(void);
static void spawn(const Arg *arg);
static void change_workspace(const Arg *arg);
static void client_to_workspace(const Arg *arg);
//static client* addwindow(xcb_window_t w);
//static void buttonpress(xcb_generic_event_t *e);
//static void change_desktop(const Arg *arg);
//static void cleanup(void);
//static void client_to_desktop(const Arg *arg);
//static void clientmessage(xcb_generic_event_t *e);
//static void configurerequest(xcb_generic_event_t *e);
//static void deletewindow(xcb_window_t w);
//static void desktopinfo(void);
//static void destroynotify(xcb_generic_event_t *e);
//static void die(const char *fmt, ...);
//static void enternotify(xcb_generic_event_t *e);
//static void focusurgent();
//static unsigned int getcolor(char* color);
//static void grabbuttons(client *c);
//static void grabkeys(void);
//static void grid(int h, int y);
//static void keypress(xcb_generic_event_t *e);
//static void kill_client(void);
//static void last_desktop(void);
//static void maprequest(xcb_generic_event_t *e);
//static void monocle(int h, int y);
//static void move_down(void);
//static void move_up(void);
//static void mouse_aside(void);
//static void mousemotion(const Arg *arg);
//static void next_win(void);
//static client* prev_client(void);
//static void prev_win(void);
//static void propertynotify(xcb_generic_event_t *e);
//static void quit(const Arg *arg);
//static void removeclient(client *c);
//static void resize_master(const Arg *arg);
//static void resize_stack(const Arg *arg);
//static void rotate(const Arg *arg);
//static void rotate_filled(const Arg *arg);
//static void run(void);
//static void save_desktop(int i);
//static void select_desktop(int i);
//static void setfullscreen(client *c, uint8_t fullscrn);
//static int setup(int default_screen);
///* static void sigchld(); */
//static void spawn(const Arg *arg);
//static void stack(int h, int y);
//static void swap_master();
//static void switch_mode(const Arg *arg);
//static void tile(void);
//static void togglepanel();
//static void update_current(client *c);
//static void unmapnotify(xcb_generic_event_t *e);
//static client* wintoclient(xcb_window_t w);
//
#include "config.h"

/* global variables */
//static int retval;

//static uint8_t running = True, showpanel = SHOW_PANEL;
//static int previous_desktop = 0, current_desktop = 0, retval = 0;
//static int wh, ww, mode = DEFAULT_MODE, master_size = 0, growth = 0;
//static unsigned int numlockmask = 0, win_unfocus, win_focus;
//static xcb_connection_t *dis;
//static xcb_screen_t *screen;
//static client *head, *prevfocus, *current;
//static xcb_atom_t wmatoms[WM_COUNT], netatoms[NET_COUNT];
//static desktop desktops[DESKTOPS];
//
///* events array
// * on receival of a new event, call the appropriate function to handle it
// */
//static void (*events[XCB_NO_OPERATION])(xcb_generic_event_t *e);
//
///* layout array - given the current layout mode, tile the windows
// * h (or hh) - avaible height that windows have to expand
// * y (or cy) - offset from top to place the windows (reserved by the panel) */
//static void (*layout[MODES])(int h, int y) = {
//	[TILE] = stack, [BSTACK] = stack, [GRID] = grid, [MONOCLE] = monocle,
//};
//
///* get screen of display */
//static xcb_screen_t *xcb_screen_of_display(xcb_connection_t *con, int screen)
//{
//	xcb_screen_iterator_t iter;
//	iter = xcb_setup_roots_iterator(xcb_get_setup(con));
//	for (; iter.rem; --screen, xcb_screen_next(&iter)) if (screen == 0) return iter.data;
//	return NULL;
//}
//
///* wrapper to move and resize window */
//static inline void xcb_move_resize(xcb_connection_t *con, xcb_window_t win, int x, int y, int w, int h)
//{
//	unsigned int pos[4] = { x, y, w, h };
//	xcb_configure_window(con, win, XCB_MOVE_RESIZE, pos);
//}
//
///* wrapper to move window */
//static inline void xcb_move(xcb_connection_t *con, xcb_window_t win, int x, int y)
//{
//	unsigned int pos[2] = { x, y };
//	xcb_configure_window(con, win, XCB_MOVE, pos);
//}
//
///* wrapper to resize window */
//static inline void xcb_resize(xcb_connection_t *con, xcb_window_t win, int w, int h)
//{
//	unsigned int pos[2] = { w, h };
//	xcb_configure_window(con, win, XCB_RESIZE, pos);
//}
//
///* wrapper to raise window */
//static inline void xcb_raise_window(xcb_connection_t *con, xcb_window_t win)
//{
//	unsigned int arg[1] = { XCB_STACK_MODE_ABOVE };
//	xcb_configure_window(con, win, XCB_CONFIG_WINDOW_STACK_MODE, arg);
//}
//
///* wrapper to set xcb border width */
//static inline void xcb_border_width(xcb_connection_t *con, xcb_window_t win, int w)
//{
//	unsigned int arg[1] = { w };
//	xcb_configure_window(con, win, XCB_CONFIG_WINDOW_BORDER_WIDTH, arg);
//}
//
///* wrapper to get xcb keysymbol from keycode */
//static xcb_keysym_t xcb_get_keysym(xcb_keycode_t keycode)
//{
//	xcb_key_symbols_t *keysyms;
//	xcb_keysym_t	   keysym;
//
//	if (!(keysyms = xcb_key_symbols_alloc(dis))) return 0;
//	keysym = xcb_key_symbols_get_keysym(keysyms, keycode, 0);
//	xcb_key_symbols_free(keysyms);
//
//	return keysym;
//}
//
///* wrapper to get xcb keycodes from keysymbol */
//static xcb_keycode_t* xcb_get_keycodes(xcb_keysym_t keysym)
//{
//	xcb_key_symbols_t *keysyms;
//	xcb_keycode_t	 *keycode;
//
//	if (!(keysyms = xcb_key_symbols_alloc(dis))) return NULL;
//	keycode = xcb_key_symbols_get_keycode(keysyms, keysym);
//	xcb_key_symbols_free(keysyms);
//
//	return keycode;
//}
//
///* retieve RGB color from hex (think of html) */
//static unsigned int xcb_get_colorpixel(char *hex)
//{
//	char strgroups[3][3]  = {{hex[1], hex[2], '\0'}, {hex[3], hex[4], '\0'}, {hex[5], hex[6], '\0'}};
//	unsigned int rgb16[3] = {(strtol(strgroups[0], NULL, 16)), (strtol(strgroups[1], NULL, 16)),
//		(strtol(strgroups[2], NULL, 16))};
//	return (rgb16[0] << 16) + (rgb16[1] << 8) + rgb16[2];
//}
//
///* wrapper to get atoms using xcb */
//static void xcb_get_atoms(char **names, xcb_atom_t *atoms, unsigned int count)
//{
//	xcb_intern_atom_cookie_t cookies[count];
//	xcb_intern_atom_reply_t  *reply;
//
//	for (unsigned int i = 0; i < count; i++) cookies[i] = xcb_intern_atom(dis, 0, strlen(names[i]), names[i]);
//	for (unsigned int i = 0; i < count; i++) {
//		reply = xcb_intern_atom_reply(dis, cookies[i], NULL); /* TODO: Handle error */
//		if (reply) {
//		debug_print("xcb_get_atoms: %s: %d\n", names[i], reply->atom);
//			atoms[i] = reply->atom; free(reply);
//		} else puts("WARN: jpwm failed to register %s atom\n");
//	}
//}
//
///* wrapper to window get attributes using xcb */
//static void xcb_get_attributes(xcb_window_t *windows, xcb_get_window_attributes_reply_t **reply, unsigned int count)
//{
//	xcb_get_window_attributes_cookie_t cookies[count];
//	for (unsigned int i = 0; i < count; i++) cookies[i] = xcb_get_window_attributes(dis, windows[i]);
//	for (unsigned int i = 0; i < count; i++) reply[i]   = xcb_get_window_attributes_reply(dis, cookies[i], NULL); /* TODO: Handle error */
//}
//
///* check if other wm exists */
//static int xcb_checkotherwm(void)
//{
//	xcb_generic_error_t *error;
//	unsigned int values[1] = {XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT|XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY|
//							  XCB_EVENT_MASK_PROPERTY_CHANGE|XCB_EVENT_MASK_BUTTON_PRESS};
//	error = xcb_request_check(dis, xcb_change_window_attributes_checked(dis, screen->root, XCB_CW_EVENT_MASK, values));
//	xcb_flush(dis);
//	if (error) return 1;
//	return 0;
//}
//
///* create a new client and add the new window
// * window should notify of property change events
// */
//client* addwindow(xcb_window_t w) {
//	client *c, *t = prev_client(head);
//	if (!(c = (client *)calloc(1, sizeof(client)))) err(EXIT_FAILURE, "cannot allocate client");
//
//	if (!head) head = c;
//	else if (!ATTACH_ASIDE) { c->next = head; head = c; }
//	else if (t) t->next = c; else head->next = c;
//
//	unsigned int values[1] = { XCB_EVENT_MASK_PROPERTY_CHANGE|(FOLLOW_MOUSE?XCB_EVENT_MASK_ENTER_WINDOW:0) };
//	xcb_change_window_attributes_checked(dis, (c->win = w), XCB_CW_EVENT_MASK, values);
//	return c;
//}
//
///* on the press of a button check to see if there's a binded function to call */
//void buttonpress(xcb_generic_event_t *e) {
//	xcb_button_press_event_t *ev = (xcb_button_press_event_t*)e;
//	debug_print("buttonpress: %d state: %d\n", ev->detail, ev->state);
//
//	client *c = wintoclient(ev->event);
//	if (!c) return;
//	if (CLICK_TO_FOCUS && current != c && ev->detail == XCB_BUTTON_INDEX_1) update_current(c);
//
//	for (unsigned int i=0; i<LENGTH(buttons); i++)
//		if (buttons[i].func && buttons[i].button == ev->detail &&
//			CLEANMASK(buttons[i].mask) == CLEANMASK(ev->state)) {
//			if (current != c) update_current(c);
//			buttons[i].func(&(buttons[i].arg));
//		}
//
//	if (CLICK_TO_FOCUS) {
//		xcb_allow_events(dis, XCB_ALLOW_REPLAY_POINTER, ev->time);
//		xcb_flush(dis);
//	}
//}
//
///* focus another desktop
// *
// * to avoid flickering
// * first map the new windows
// * first the current window and then all other
// * then unmap the old windows
// * first all others then the current */
//void change_desktop(const Arg *arg) {
//	if (arg->i == current_desktop) return;
//	previous_desktop = current_desktop;
//	select_desktop(arg->i);
//	if (current) xcb_map_window(dis, current->win);
//	for (client *c=head; c; c=c->next) xcb_map_window(dis, c->win);
//	select_desktop(previous_desktop);
//	for (client *c=head; c; c=c->next) if (c != current) xcb_unmap_window(dis, c->win);
//	if (current) xcb_unmap_window(dis, current->win);
//	select_desktop(arg->i);
//	tile(); update_current(current);
//	desktopinfo();
//}
//
///* remove all windows in all desktops by sending a delete message */
//void cleanup(void) {
//	xcb_query_tree_reply_t  *query;
//	xcb_window_t *c;
//
//	xcb_ungrab_key(dis, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
//	if ((query = xcb_query_tree_reply(dis,xcb_query_tree(dis,screen->root),0))) {
//		c = xcb_query_tree_children(query);
//		for (unsigned int i = 0; i != query->children_len; ++i) deletewindow(c[i]);
//		free(query);
//	}
//	xcb_set_input_focus(dis, XCB_INPUT_FOCUS_POINTER_ROOT, screen->root, XCB_CURRENT_TIME);
//}
//
///* move a client to another desktop
// *
// * remove the current client from the current desktop's client list
// * and add it as last client of the new desktop's client list */
//void client_to_desktop(const Arg *arg) {
//	if (!current || arg->i == current_desktop) return;
//	int cd = current_desktop;
//	client *p = prev_client(current), *c = current;
//
//	select_desktop(arg->i);
//	client *l = prev_client(head);
//	update_current(l ? (l->next = c):head ? (head->next = c):(head = c));
//
//	select_desktop(cd);
//	if (c == head || !p) head = c->next; else p->next = c->next;
//	c->next = NULL;
//	xcb_unmap_window(dis, c->win);
//	update_current(prevfocus);
//
//	if (FOLLOW_WINDOW) change_desktop(arg); else tile();
//	desktopinfo();
//}
//
///* To change the state of a mapped window, a client MUST
// * send a _NET_WM_STATE client message to the root window
// * message_type must be _NET_WM_STATE
// *   data.l[0] is the action to be taken
// *   data.l[1] is the property to alter three actions:
// *   - remove/unset _NET_WM_STATE_REMOVE=0
// *   - add/set _NET_WM_STATE_ADD=1,
// *   - toggle _NET_WM_STATE_TOGGLE=2
// *
// * check if window requested fullscreen or activation */
//void clientmessage(xcb_generic_event_t *e) {
//	xcb_client_message_event_t *ev = (xcb_client_message_event_t*)e;
//	client *t = NULL, *c = wintoclient(ev->window);
//	if (c && ev->type					  == netatoms[NET_WM_STATE]
//		  && ((unsigned)ev->data.data32[1] == netatoms[NET_FULLSCREEN]
//		  ||  (unsigned)ev->data.data32[2] == netatoms[NET_FULLSCREEN]))
//		setfullscreen(c, (ev->data.data32[0] == 1 || (ev->data.data32[0] == 2 && !c->isfullscrn)));
//	else if (c && ev->type == netatoms[NET_ACTIVE]) for (t=head; t && t!=c; t=t->next);
//	if (t) update_current(c);
//	tile();
//}
//
///* a configure request means that the window requested changes in its geometry
// * state. if the window is fullscreen discard and fill the screen else set the
// * appropriate values as requested, and tile the window again so that it fills
// * the gaps that otherwise could have been created
// */
//void configurerequest(xcb_generic_event_t *e) {
//	xcb_configure_request_event_t *ev = (xcb_configure_request_event_t*)e;
//	client *c = wintoclient(ev->window);
//	if (c && c->isfullscrn) setfullscreen(c, True);
//	else {
//		unsigned int v[7];
//		unsigned int i = 0;
//		if (ev->value_mask & XCB_CONFIG_WINDOW_X)			  v[i++] = ev->x;
//		if (ev->value_mask & XCB_CONFIG_WINDOW_Y)			  v[i++] = (ev->y + (showpanel && TOP_PANEL)) ? PANEL_HEIGHT : 0;
//		if (ev->value_mask & XCB_CONFIG_WINDOW_WIDTH)		  v[i++] = (ev->width  < ww - BORDER_WIDTH) ? ev->width  : ww + BORDER_WIDTH;
//		if (ev->value_mask & XCB_CONFIG_WINDOW_HEIGHT)		 v[i++] = (ev->height < wh - BORDER_WIDTH) ? ev->height : wh + BORDER_WIDTH;
//		if (ev->value_mask & XCB_CONFIG_WINDOW_BORDER_WIDTH)   v[i++] = ev->border_width;
//		if (ev->value_mask & XCB_CONFIG_WINDOW_SIBLING)		v[i++] = ev->sibling;
//		if (ev->value_mask & XCB_CONFIG_WINDOW_STACK_MODE)	 v[i++] = ev->stack_mode;
//		xcb_configure_window(dis, ev->window, ev->value_mask, v);
//	}
//	tile();
//}
//
///* close the window */
//void deletewindow(xcb_window_t w) {
//	xcb_client_message_event_t ev;
//	ev.response_type = XCB_CLIENT_MESSAGE;
//	ev.window = w;
//	ev.format = 32;
//	ev.sequence = 0;
//	ev.type = wmatoms[WM_PROTOCOLS];
//	ev.data.data32[0] = wmatoms[WM_DELETE_WINDOW];
//	ev.data.data32[1] = XCB_CURRENT_TIME;
//	xcb_send_event(dis, 0, w, XCB_EVENT_MASK_NO_EVENT, (char*)&ev);
//}
//
///* output info about the desktops on standard output stream
// *
// * the info is a list of ':' separated values for each desktop
// * desktop to desktop info is separated by ' ' single spaces
// * the info values are
// *   the desktop number/id
// *   the desktop's client count
// *   the desktop's tiling layout mode/id
// *   whether the desktop is the current focused (1) or not (0)
// *   whether any client in that desktop has received an urgent hint
// *
// * once the info is collected, immediately flush the stream */
//void desktopinfo(void) {
//	uint8_t urgent = False;
//	int cd = current_desktop, n=0, d=0;
//	for (client *c; d<DESKTOPS; d++) {
//		for (select_desktop(d), c=head, n=0, urgent=False; c; c=c->next, ++n) if (c->isurgent) urgent = True;
//		fprintf(stdout, "%d:%d:%d:%d:%d%c", d, n, mode, current_desktop == cd, urgent, d+1==DESKTOPS?'\n':' ');
//	}
//	fflush(stdout);
//	if (cd != d-1) select_desktop(cd);
//}
//
///* a destroy notification is received when a window is being closed
// * on receival, remove the appropriate client that held that window
// */
//void destroynotify(xcb_generic_event_t *e) {
//	debug_print("destroynotify\n");
//	xcb_destroy_notify_event_t *ev = (xcb_destroy_notify_event_t*)e;
//	client *c = wintoclient(ev->window);
//	if (c) removeclient(c);
//	desktopinfo();
//}
//
///* when the mouse enters a window's borders
// * the window, if notifying of such events (EnterWindowMask)
// * will notify the wm and will get focus */
//void enternotify(xcb_generic_event_t *e) {
//	xcb_enter_notify_event_t *ev = (xcb_enter_notify_event_t*)e;
//	if (!FOLLOW_MOUSE) return;
//	debug_print("enternotify\n");
//	client *c = wintoclient(ev->event);
//	if (c && ev->mode == XCB_NOTIFY_MODE_NORMAL && ev->detail != XCB_NOTIFY_DETAIL_INFERIOR) update_current(c);
//}
//
///* find and focus the client which received
// * the urgent hint in the current desktop */
//void focusurgent() {
//	client *c;
//	int cd = current_desktop, d = 0;
//	for (c=head; c && !c->isurgent; c=c->next);
//	if (c) { update_current(c); return; }
//	else for (uint8_t f=False; d<DESKTOPS && !f; d++) for (select_desktop(d), c=head; c && !(f=c->isurgent); c=c->next);
//	select_desktop(cd);
//	if (c) { change_desktop(&(Arg){.i = --d}); update_current(c); }
//}
//
///* get a pixel with the requested color
// * to fill some window area - borders */
//unsigned int getcolor(char* color)
//{
//	xcb_colormap_t map = screen->default_colormap;
//	xcb_alloc_color_reply_t *c;
//	unsigned int r, g, b, rgb, pixel;
//
//	rgb = xcb_get_colorpixel(color);
//	r = rgb >> 16; g = rgb >> 8 & 0xFF; b = rgb & 0xFF;
//	c = xcb_alloc_color_reply(dis, xcb_alloc_color(dis, map, r * 257, g * 257, b * 257), NULL);
//	if (!c) {
//		die("getcolor: cannot allocate color '%s'\n", color);
//	}
//	pixel = c->pixel; free(c);
//	return pixel;
//}
//
///* set the given client to listen to button events (presses / releases) */
//void grabbuttons(client *c) {
//	unsigned int modifiers[] = { 0, XCB_MOD_MASK_LOCK, numlockmask, numlockmask|XCB_MOD_MASK_LOCK };
//	xcb_ungrab_button(dis, XCB_BUTTON_INDEX_ANY, c->win, XCB_GRAB_ANY);
//	for (unsigned int b=0; b<LENGTH(buttons); b++)
//		for (unsigned int m=0; m<LENGTH(modifiers); m++)
//			if (CLICK_TO_FOCUS)
//				xcb_grab_button(dis, 1, c->win, XCB_EVENT_MASK_BUTTON_PRESS, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
//						XCB_WINDOW_NONE, XCB_CURSOR_NONE, XCB_BUTTON_INDEX_ANY, XCB_BUTTON_MASK_ANY);
//			else
//				xcb_grab_button(dis, 1, c->win, XCB_EVENT_MASK_BUTTON_PRESS, XCB_GRAB_MODE_SYNC, XCB_GRAB_MODE_ASYNC,
//						XCB_WINDOW_NONE, XCB_CURSOR_NONE, buttons[b].button, buttons[b].mask|modifiers[m]);
//}
//
///* the wm should listen to key presses */
//void grabkeys(void) {
//	xcb_keycode_t *keycode;
//	unsigned int modifiers[] = { 0, XCB_MOD_MASK_LOCK, numlockmask, numlockmask|XCB_MOD_MASK_LOCK };
//	xcb_ungrab_key(dis, XCB_GRAB_ANY, screen->root, XCB_MOD_MASK_ANY);
//	for (unsigned int i=0; i<LENGTH(keys); i++) {
//		keycode = xcb_get_keycodes(keys[i].keysym);
//		for (unsigned int k=0; keycode[k] != XCB_NO_SYMBOL; k++)
//			for (unsigned int m=0; m<LENGTH(modifiers); m++)
//				xcb_grab_key(dis, 1, screen->root, keys[i].mod | modifiers[m], keycode[k], XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
//	}
//}
//
///* arrange windows in a grid */
//void grid(int hh, int cy) {
//	int n = 0, cols = 0, cn = 0, rn = 0, i = -1;
//	for (client *c = head; c; c=c->next) if (!ISFFT(c)) ++n;
//	for (cols=0; cols <= n/2; cols++) if (cols*cols >= n) break; /* emulate square root */
//	if (n == 5) cols = 2;
//
//	int rows = n/cols, ch = hh - BORDER_WIDTH, cw = (ww - BORDER_WIDTH)/(cols?cols:1);
//	for (client *c=head; c; c=c->next) {
//		if (ISFFT(c)) continue; else ++i;
//		if (i/rows + 1 > cols - n%cols) rows = n/cols + 1;
//		xcb_move_resize(dis, c->win, cn*cw, cy + rn*ch/rows, cw - BORDER_WIDTH, ch/rows - BORDER_WIDTH);
//		if (++rn >= rows) { rn = 0; cn++; }
//	}
//}
//
///* on the press of a key check to see if there's a binded function to call */
//void keypress(xcb_generic_event_t *e) {
//	xcb_key_press_event_t *ev	   = (xcb_key_press_event_t *)e;
//	xcb_keysym_t		   keysym   = xcb_get_keysym(ev->detail);
//	debug_print("keypress: code: %d mod: %d\n", ev->detail, ev->state);
//	for (unsigned int i=0; i<LENGTH(keys); i++)
//		if (keysym == keys[i].keysym && CLEANMASK(keys[i].mod) == CLEANMASK(ev->state) && keys[i].func)
//				keys[i].func(&keys[i].arg);
//}
//
///* explicitly kill a client - close the highlighted window
// * send a delete message and remove the client */
//void killclient() {
//	if (!current) return;
//	xcb_icccm_get_wm_protocols_reply_t reply; unsigned int n = 0; uint8_t got = False;
//	if (xcb_icccm_get_wm_protocols_reply(dis,
//		xcb_icccm_get_wm_protocols(dis, current->win, wmatoms[WM_PROTOCOLS]),
//		&reply, NULL)) { /* TODO: Handle error? */
//		for(; n != reply.atoms_len; ++n) if ((got = reply.atoms[n] == wmatoms[WM_DELETE_WINDOW])) break;
//		xcb_icccm_get_wm_protocols_reply_wipe(&reply);
//	}
//	if (got) deletewindow(current->win);
//	else xcb_kill_client(dis, current->win);
//	removeclient(current);
//}
//
///* focus the previously focused desktop */
//void last_desktop() {
//	change_desktop(&(Arg){.i = previous_desktop});
//}
//
///* a map request is received when a window wants to display itself
// * if the window has override_redirect flag set then it should not be handled
// * by the wm. if the window already has a client then there is nothing to do.
// *
// * get the window class and name instance and try to match against an app rule.
// * create a client for the window, that client will always be current.
// * check for transient state, and fullscreen state and the appropriate values.
// * if the desktop in which the window was spawned is the current desktop then
// * display the window, else, if set, focus the new desktop.
// */
//void maprequest(xcb_generic_event_t *e) {
//	xcb_map_request_event_t			*ev = (xcb_map_request_event_t*)e;
//	xcb_window_t					   windows[] = { ev->window }, transient = 0;
//	xcb_get_window_attributes_reply_t  *attr[1];
//	xcb_icccm_get_wm_class_reply_t	 ch;
//	xcb_get_geometry_reply_t		   *geometry;
//	xcb_get_property_reply_t		   *prop_reply;
//
//	xcb_get_attributes(windows, attr, 1);
//	if (!attr[0] || attr[0]->override_redirect) return;
//	if (wintoclient(ev->window)) return;
//	debug_print("maprequest\n");
//
//	uint8_t follow = False, floating = False;
//	int cd = current_desktop, newdsk = current_desktop;
//	if (xcb_icccm_get_wm_class_reply(dis, xcb_icccm_get_wm_class(dis, ev->window), &ch, NULL)) { /* TODO: error handling */
//	debug_print("maprequest: class: %s instance %s\n", ch.class_name, ch.instance_name);
//		for (unsigned int i=0; i<LENGTH(rules); i++)
//			if (strstr(ch.class_name, rules[i].class) || strstr(ch.instance_name, rules[i].class)) {
//				follow = rules[i].follow;
//				newdsk = (rules[i].desktop < 0) ? current_desktop:rules[i].desktop;
//				floating = rules[i].floating;
//				break;
//			}
//		xcb_icccm_get_wm_class_reply_wipe(&ch);
//	}
//
//	/* might be useful in future */
//	if ((geometry = xcb_get_geometry_reply(dis, xcb_get_geometry(dis, ev->window), NULL))) { /* TODO: error handling */
//	debug_print("maprequest: geom: %ux%u+%d+%d\n", 
//		geometry->width, geometry->height, geometry->x, geometry->y);
//		free(geometry);
//	}
//
//	if (cd != newdsk) select_desktop(newdsk);
//	client *c = addwindow(ev->window);
//
//	xcb_icccm_get_wm_transient_for_reply(dis, xcb_icccm_get_wm_transient_for_unchecked(dis, ev->window), &transient, NULL); /* TODO: error handling */
//	c->istransient = transient?True:False;
//	c->isfloating  = floating || c->istransient;
//
//	prop_reply  = xcb_get_property_reply(dis, xcb_get_property_unchecked(dis, 0, ev->window, netatoms[NET_WM_STATE], XCB_ATOM_ATOM, 0, 1), NULL); /* TODO: error handling */
//	if (prop_reply) {
//		if (prop_reply->format == 32) {
//			xcb_atom_t *v = xcb_get_property_value(prop_reply);
//			for (unsigned int i=0; i<prop_reply->value_len; i++)
//		debug_print("maprequest: %d : %d\n", i, v[0]);
//			setfullscreen(c, (v[0] == netatoms[NET_FULLSCREEN]));
//		}
//		free(prop_reply);
//	}
//
//	/** information for stdout **/
//	debug_print("maprequest: transient: %d\n", c->istransient);
//	debug_print("maprequest: floating: %d\n", c->isfloating);
//
//	if (cd != newdsk) select_desktop(cd);
//	if (cd == newdsk) { tile(); xcb_map_window(dis, c->win); update_current(c); }
//	else if (follow) { change_desktop(&(Arg){.i = newdsk}); update_current(c); }
//	grabbuttons(c);
//
//	desktopinfo();
//}
//
///* move the mouse pointer to the rightmost screen edge */
//void mouse_aside() {
//	xcb_query_pointer_reply_t	*reply = NULL;
//	int16_t					 rel_x = 0, rel_y = 0;
//
//	reply = xcb_query_pointer_reply(dis,
//									xcb_query_pointer(dis, screen->root),
//									NULL);
//	if (reply) {
//		rel_x = ww - reply->root_x;
//	debug_print("mouse_aside: %s: warp relative pos (%d,%d)\n",
//			__func__, rel_x, rel_y);
//		xcb_warp_pointer(dis,
//						 XCB_NONE, XCB_NONE,
//						 0, 0, 0, 0,
//						 rel_x, rel_y);
//	} else {
//	debug_print("mouse_aside: %s: no mouse query info\n", __func__);
//	}
//}
//
///* grab the pointer and get it's current position
// * all pointer movement events will be reported until it's ungrabbed
// * until the mouse button has not been released,
// * grab the interesting events - button press/release and pointer motion
// * and on on pointer movement resize or move the window under the curson.
// * if the received event is a map request or a configure request call the
// * appropriate handler, and stop listening for other events.
// * Ungrab the poitner and event handling is passed back to run() function.
// * Once a window has been moved or resized, it's marked as floating. */
//void mousemotion(const Arg *arg) {
//	xcb_get_geometry_reply_t  *geometry;
//	xcb_query_pointer_reply_t *pointer;
//	xcb_grab_pointer_reply_t  *grab_reply;
//	int mx, my, winx, winy, winw, winh, xw, yh;
//
//	if (!current) return;
//	geometry = xcb_get_geometry_reply(dis, xcb_get_geometry(dis, current->win), NULL); // TODO: error handling
//	if (geometry) {
//		winx = geometry->x;	 winy = geometry->y;
//		winw = geometry->width; winh = geometry->height;
//		free(geometry);
//	} else return;
//
//	pointer = xcb_query_pointer_reply(dis, xcb_query_pointer(dis, screen->root), 0);
//	if (!pointer) return;
//	mx = pointer->root_x; my = pointer->root_y;
//
//	grab_reply = xcb_grab_pointer_reply(dis, xcb_grab_pointer(dis, 0, screen->root,
//				BUTTONMASK|XCB_EVENT_MASK_BUTTON_MOTION|XCB_EVENT_MASK_POINTER_MOTION,
//			XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, XCB_NONE, XCB_NONE, XCB_CURRENT_TIME), NULL);
//	if (!grab_reply || grab_reply->status != XCB_GRAB_STATUS_SUCCESS) return;
//
//	if (current->isfullscrn) setfullscreen(current, False);
//	if (!current->isfloating) current->isfloating = True;
//	tile(); update_current(current);
//
//	xcb_generic_event_t *e = NULL;
//	xcb_motion_notify_event_t *ev = NULL;
//	uint8_t ungrab = False;
//	do {
//		if (e) free(e);
//		xcb_flush(dis);
//		while(!(e = xcb_wait_for_event(dis))) xcb_flush(dis);
//		switch (e->response_type & ~0x80) {
//			case XCB_CONFIGURE_REQUEST: case XCB_MAP_REQUEST:
//				events[e->response_type & ~0x80](e);
//				break;
//			case XCB_MOTION_NOTIFY:
//				ev = (xcb_motion_notify_event_t*)e;
//				xw = (arg->i == MOVE ? winx : winw) + ev->root_x - mx;
//				yh = (arg->i == MOVE ? winy : winh) + ev->root_y - my;
//				if (arg->i == RESIZE) xcb_resize(dis, current->win,
//				xw>MIN_WIN_SIZE?xw:winw, yh>MIN_WIN_SIZE?yh:winh);
//				else if (arg->i == MOVE) xcb_move(dis, current->win, xw, yh);
//				xcb_flush(dis);
//				break;
//			case XCB_KEY_PRESS:
//			case XCB_KEY_RELEASE:
//			case XCB_BUTTON_PRESS:
//			case XCB_BUTTON_RELEASE:
//				ungrab = True;
//		}
//	} while(!ungrab && current);
//	debug_print("mousemotion: ungrab\n");
//	xcb_ungrab_pointer(dis, XCB_CURRENT_TIME);
//}
//
///* each window should cover all the available screen space */
//void monocle(int hh, int cy) {
//	for (client *c=head; c; c=c->next) if (!ISFFT(c)) xcb_move_resize(dis, c->win, 0, cy, ww, hh);
//}
//
///* move the current client, to current->next
// * and current->next to current client's position */
//void move_down() {
//	/* p is previous, c is current, n is next, if current is head n is last */
//	client *p = NULL, *n = (current->next) ? current->next:head;
//	if (!(p = prev_client(current))) return;
//	/*
//	 * if c is head, swapping with n should update head to n
//	 * [c]->[n]->..  ==>  [n]->[c]->..
//	 *  ^head			  ^head
//	 *
//	 * else there is a previous client and p->next should be what's after c
//	 * ..->[p]->[c]->[n]->..  ==>  ..->[p]->[n]->[c]->..
//	 */
//	if (current == head) head = n; else p->next = current->next;
//	/*
//	 * if c is the last client, c will be the current head
//	 * [n]->..->[p]->[c]->NULL  ==>  [c]->[n]->..->[p]->NULL
//	 *  ^head						 ^head
//	 * else c will take the place of n, so c-next will be n->next
//	 * ..->[p]->[c]->[n]->..  ==>  ..->[p]->[n]->[c]->..
//	 */
//	current->next = (current->next) ? n->next:n;
//	/*
//	 * if c was swapped with n then they now point to the same ->next. n->next should be c
//	 * ..->[p]->[c]->[n]->..  ==>  ..->[p]->[n]->..  ==>  ..->[p]->[n]->[c]->..
//	 *										[c]-^
//	 *
//	 * else c is the last client and n is head,
//	 * so c will be move to be head, no need to update n->next
//	 * [n]->..->[p]->[c]->NULL  ==>  [c]->[n]->..->[p]->NULL
//	 *  ^head						 ^head
//	 */
//	if (current->next == n->next) n->next = current; else head = current;
//	tile();
//}
//
///* move the current client, to the previous from current and
// * the previous from  current to current client's position */
//void move_up() {
//	client *pp = NULL, *p;
//	/* p is previous from current or last if current is head */
//	if (!(p = prev_client(current))) return;
//	/* pp is previous from p, or null if current is head and thus p is last */
//	if (p->next) for (pp=head; pp && pp->next != p; pp=pp->next);
//	/*
//	 * if p has a previous client then the next client should be current (current is c)
//	 * ..->[pp]->[p]->[c]->..  ==>  ..->[pp]->[c]->[p]->..
//	 *
//	 * if p doesn't have a previous client, then p might be head, so head must change to c
//	 * [p]->[c]->..  ==>  [c]->[p]->..
//	 *  ^head			  ^head
//	 * if p is not head, then c is head (and p is last), so the new head is next of c
//	 * [c]->[n]->..->[p]->NULL  ==>  [n]->..->[p]->[c]->NULL
//	 *  ^head		 ^last		   ^head		 ^last
//	 */
//	if (pp) pp->next = current; else head = (current == head) ? current->next:current;
//	/*
//	 * next of p should be next of c
//	 * ..->[pp]->[p]->[c]->[n]->..  ==>  ..->[pp]->[c]->[p]->[n]->..
//	 * except if c was head (now c->next is head), so next of p should be c
//	 * [c]->[n]->..->[p]->NULL  ==>  [n]->..->[p]->[c]->NULL
//	 *  ^head		 ^last		   ^head		 ^last
//	 */
//	p->next = (current->next == head) ? current:current->next;
//	/*
//	 * next of c should be p
//	 * ..->[pp]->[p]->[c]->[n]->..  ==>  ..->[pp]->[c]->[p]->[n]->..
//	 * except if c was head (now c->next is head), so c is must be last
//	 * [c]->[n]->..->[p]->NULL  ==>  [n]->..->[p]->[c]->NULL
//	 *  ^head		 ^last		   ^head		 ^last
//	 */
//	current->next = (current->next == head) ? NULL:p;
//	tile();
//}
//
///* cyclic focus the next window
// * if the window is the last on stack, focus head */
//void next_win() {
//	if (!current || !head->next) return;
//	update_current(current->next ? current->next:head);
//}
//
///* get the previous client from the given
// * if no such client, return NULL */
//client* prev_client(client *c) {
//	if (!c || !head->next) return NULL;
//	client *p; for (p=head; p->next && p->next != c; p=p->next);
//	return p;
//}
//
///* cyclic focus the previous window
// * if the window is the head, focus the last stack window */
//void prev_win() {
//	if (!current || !head->next) return;
//	update_current(prev_client(prevfocus = current));
//}
//
///* property notify is called when one of the window's properties
// * is changed, such as an urgent hint is received
// */
//void propertynotify(xcb_generic_event_t *e) {
//	xcb_property_notify_event_t *ev = (xcb_property_notify_event_t*)e;
//	xcb_icccm_wm_hints_t wmh;
//	client *c;
//
//	debug_print("propertynotify\n");
//	c = wintoclient(ev->window);
//	if (!c || ev->atom != XCB_ICCCM_WM_ALL_HINTS) return;
//	debug_print("propertynotify: got hint\n");
//	if (xcb_icccm_get_wm_hints_reply(dis, xcb_icccm_get_wm_hints(dis, ev->window), &wmh, NULL)) /* TODO: error handling */
//		c->isurgent = c != current && (wmh.flags & XCB_ICCCM_WM_HINT_X_URGENCY);
//	desktopinfo();
//}
//
///* to quit just stop receiving events
// * run() is stopped and control is back to main()
// */
//void quit(const Arg *arg) {
//	retval = arg->i;
//	running = False;
//}
//
///* remove the specified client
// *
// * note, the removing client can be on any desktop,
// * we must return back to the current focused desktop.
// * if c was the previously focused, prevfocus must be updated
// * else if c was the current one, current must be updated. */
//void removeclient(client *c) {
//	client **p = NULL;
//	int nd = 0, cd = current_desktop;
//	for (uint8_t found = False; nd<DESKTOPS && !found; nd++)
//		for (select_desktop(nd), p = &head; *p && !(found = *p == c); p = &(*p)->next);
//	*p = c->next;
//	if (c == prevfocus) prevfocus = prev_client(current);
//	if (c == current || !head->next) update_current(prevfocus);
//	free(c); c = NULL;
//	if (cd == nd -1) tile(); else select_desktop(cd);
//}
//
///* resize the master window - check for boundary size limits
// * the size of a window can't be less than MINWSZ
// */
//void resize_master(const Arg *arg) {
//	int msz = (mode == BSTACK ? wh:ww) * MASTER_SIZE + master_size + arg->i;
//	if (msz < MINWSZ || (mode == BSTACK ? wh:ww) - msz < MINWSZ) return;
//	master_size += arg->i;
//	tile();
//}
//
///* resize the first stack window - no boundary checks */
//void resize_stack(const Arg *arg) {
//	growth += arg->i;
//	tile();
//}
//
///* jump and focus the next or previous desktop */
//void rotate(const Arg *arg) {
//	change_desktop(&(Arg){.i = (DESKTOPS + current_desktop + arg->i) % DESKTOPS});
//}
//
///* jump and focus the next or previous desktop that has clients */
//void rotate_filled(const Arg *arg) {
//	int n = arg->i;
//	while (n < DESKTOPS && !desktops[(DESKTOPS + current_desktop + n) % DESKTOPS].head) (n += arg->i);
//	change_desktop(&(Arg){.i = (DESKTOPS + current_desktop + n) % DESKTOPS});
//}
//
///* main event loop - on receival of an event call the appropriate event handler */
//void run(void) {
//	xcb_generic_event_t *ev;
//	while(running) {
//		xcb_flush(dis);
//		if (xcb_connection_has_error(dis)) err(EXIT_FAILURE, "error: X11 connection got interrupted\n");
//		if ((ev = xcb_wait_for_event(dis))) {
//			if (events[ev->response_type & ~0x80]) events[ev->response_type & ~0x80](ev);
//			else { 
//			debug_print("run: unimplented event: %d\n", ev->response_type & ~0x80);
//		}
//			free(ev);
//		}
//	}
//}
//
///* save specified desktop's properties */
//void save_desktop(int i) {
//	if (i < 0 || i >= DESKTOPS) return;
//	desktops[i].master_size = master_size;
//	desktops[i].mode		= mode;
//	desktops[i].growth	  = growth;
//	desktops[i].head		= head;
//	desktops[i].current	 = current;
//	desktops[i].showpanel   = showpanel;
//	desktops[i].prevfocus   = prevfocus;
//}
//
///* set the specified desktop's properties */
//void select_desktop(int i) {
//	if (i < 0 || i >= DESKTOPS) return;
//	save_desktop(current_desktop);
//	master_size	 = desktops[i].master_size;
//	mode			= desktops[i].mode;
//	growth		  = desktops[i].growth;
//	head			= desktops[i].head;
//	current		 = desktops[i].current;
//	showpanel	   = desktops[i].showpanel;
//	prevfocus	   = desktops[i].prevfocus;
//	current_desktop = i;
//}
//
///* set or unset fullscreen state of client */
//void setfullscreen(client *c, uint8_t fullscrn) {
//	debug_print("setfullscreen: %d\n", fullscrn);
//	long data[] = { fullscrn ? netatoms[NET_FULLSCREEN] : XCB_NONE };
//	if (fullscrn != c->isfullscrn) xcb_change_property(dis, XCB_PROP_MODE_REPLACE, c->win, netatoms[NET_WM_STATE], XCB_ATOM_ATOM, 32, fullscrn, data);
//	if ((c->isfullscrn = fullscrn)) xcb_move_resize(dis, c->win, 0, 0, ww, wh + PANEL_HEIGHT);
//	xcb_border_width(dis, c->win, (!head->next || c->isfullscrn
//				|| (mode == MONOCLE && !ISFFT(c))) ? 0:BORDER_WIDTH);
//	update_current(c);
//}
//
///* get numlock modifier using xcb */
//int setup_keyboard(void)
//{
//	xcb_get_modifier_mapping_reply_t *reply;
//	xcb_keycode_t					*modmap;
//	xcb_keycode_t					*numlock;
//
//	reply = xcb_get_modifier_mapping_reply(dis, xcb_get_modifier_mapping_unchecked(dis), NULL); /* TODO: error checking */
//	if (!reply) return -1;
//
//	modmap = xcb_get_modifier_mapping_keycodes(reply);
//	if (!modmap) return -1;
//
//	numlock = xcb_get_keycodes(XK_Num_Lock);
//	for (unsigned int i=0; i<8; i++)
//	   for (unsigned int j=0; j<reply->keycodes_per_modifier; j++) {
//		   xcb_keycode_t keycode = modmap[i * reply->keycodes_per_modifier + j];
//		   if (keycode == XCB_NO_SYMBOL) continue;
//		   for (unsigned int n=0; numlock[n] != XCB_NO_SYMBOL; n++)
//			   if (numlock[n] == keycode) {
//		   debug_print("setup_keyboard: foudn num-lock %d\n", 1 << i);
//				   numlockmask = 1 << i;
//				   break;
//			   }
//	   }
//
//	return 0;
//}
//
///* set initial values
// * root window - screen height/width - atoms - xerror handler
// * set masks for reporting events handled by the wm
// * and propagate the suported net atoms
// */
//int setup(int default_screen) {
//	sigchld();
//	screen = xcb_screen_of_display(dis, default_screen);
//	if (!screen) err(EXIT_FAILURE, "error: cannot aquire screen\n");
//
//	ww = screen->width_in_pixels;
//	wh = screen->height_in_pixels - PANEL_HEIGHT;
//	for (unsigned int i=0; i<DESKTOPS; i++) save_desktop(i);
//
//	win_focus   = getcolor(FOCUS);
//	win_unfocus = getcolor(UNFOCUS);
//
//	/* setup keyboard */
//	if (setup_keyboard() == -1)
//		err(EXIT_FAILURE, "error: failed to setup keyboard\n");
//
//	/* set up atoms for dialog/notification windows */
//	xcb_get_atoms(WM_ATOM_NAME, wmatoms, WM_COUNT);
//	xcb_get_atoms(NET_ATOM_NAME, netatoms, NET_COUNT);
//
//	/* check if another wm is running */
//	if (xcb_checkotherwm())
//		err(EXIT_FAILURE, "error: other wm is running\n");
//
//	xcb_change_property(dis, XCB_PROP_MODE_REPLACE, screen->root, netatoms[NET_SUPPORTED], XCB_ATOM_ATOM, 32, NET_COUNT, netatoms);
//	grabkeys();
//
//	/* set events */
//	for (unsigned int i=0; i<XCB_NO_OPERATION; i++) events[i] = NULL;
//	events[XCB_BUTTON_PRESS]		= buttonpress;
//	events[XCB_CLIENT_MESSAGE]	  = clientmessage;
//	events[XCB_CONFIGURE_REQUEST]   = configurerequest;
//	events[XCB_DESTROY_NOTIFY]	  = destroynotify;
//	events[XCB_ENTER_NOTIFY]		= enternotify;
//	events[XCB_KEY_PRESS]		   = keypress;
//	events[XCB_MAP_REQUEST]		 = maprequest;
//	events[XCB_PROPERTY_NOTIFY]	 = propertynotify;
//	events[XCB_UNMAP_NOTIFY]		= unmapnotify;
//
//	change_desktop(&(Arg){.i = DEFAULT_DESK});
//	return 0;
//}
//
//// TODO: replace with sigaction
//void sigchld() {
//	if (signal(SIGCHLD, sigchld) == SIG_ERR)
//		err(EXIT_FAILURE, "cannot install SIGCHLD handler");
//	while(0 < waitpid(-1, NULL, WNOHANG));
//}
//
///* execute a command */
//void spawn(const Arg *arg) {
//	//if (fork()) return;
//	//if (dis) close(screen->root);
//	//setsid();
//	//execvp((char*)arg->com[0], (char**)arg->com);
//	//fprintf(stderr, "error: execvp %s", (char *)arg->com[0]);
//	//perror(" failed"); /* also prints the err msg */
//	//exit(EXIT_SUCCESS);
//
//	pid_t pid;
//	pid = fork();
//	if (pid == -1) die("spawn: fork:");
//	if (pid) return;
//	if (dis) close(screen->root);
//	setsid();
//	execvp((char*)arg->com[0], (char**)arg->com);
//	die("spawn: execvp %s:", (char *)arg->com[0]);
//}
//
///* arrange windows in normal or bottom stack tile */
//void stack(int hh, int cy) {
//	client *c = NULL, *t = NULL; uint8_t b = mode == BSTACK;
//	int n = 0, d = 0, z = b ? ww:hh, ma = (mode == BSTACK ? wh:ww) * MASTER_SIZE + master_size;
//
//	/* count stack windows and grab first non-floating, non-fullscreen window */
//	for (t = head; t; t=t->next) if (!ISFFT(t)) { if (c) ++n; else c = t; }
//
//	/* if there is only one window, it should cover the available screen space
//	 * if there is only one stack window (n == 1) then we don't care about growth
//	 * if more than one stack windows (n > 1) on screen then adjustments may be needed
//	 *   - d is the num of pixels than remain when spliting
//	 *   the available width/height to the number of windows
//	 *   - z is the clients' height/width
//	 *
//	 *	  ----------  -.	--------------------.
//	 *	  |   |----| --|--> growth			   `}--> first client will get (z+d) height/width
//	 *	  |   |	|   |						  |
//	 *	  |   |----|   }--> screen height - hh  --'
//	 *	  |   |	| }-|--> client height - z	   :: 2 stack clients on tile mode ..looks like a spaceship
//	 *	  ----------  -'							:: piece of aart by c00kiemon5ter o.O om nom nom nom nom
//	 *
//	 *	 what we do is, remove the growth from the screen height   : (z - growth)
//	 *	 and then divide that space with the windows on the stack  : (z - growth)/n
//	 *	 so all windows have equal height/width (z)				:
//	 *	 growth is left out and will later be added to the first's client height/width
//	 *	 before that, there will be cases when the num of windows is not perfectly
//	 *	 divided with then available screen height/width (ie 100px scr. height, and 3 windows)
//	 *	 so we get that remaining space and merge growth to it (d) : (z - growth) % n + growth
//	 *	 finally we know each client's height, and how many pixels should be added to
//	 *	 the first stack window so that it satisfies growth, and doesn't create gaps
//	 *	 on the bottom of the screen.  */
//	if (!c) return; else if (!n) {
//		xcb_move_resize(dis, c->win, 0, cy, ww - 2*BORDER_WIDTH, hh - 2*BORDER_WIDTH);
//		return;
//	} else if (n > 1) { d = (z - growth)%n + growth; z = (z - growth)/n; }
//
//	/* tile the first non-floating, non-fullscreen window to cover the master area */
//	if (b) xcb_move_resize(dis, c->win, 0, cy, ww - 2*BORDER_WIDTH, ma - BORDER_WIDTH);
//	else   xcb_move_resize(dis, c->win, 0, cy, ma - BORDER_WIDTH, hh - 2*BORDER_WIDTH);
//
//	/* tile the next non-floating, non-fullscreen (first) stack window with growth|d */
//	for (c=c->next; c && ISFFT(c); c=c->next);
//	int cx = b ? 0:ma, cw = (b ? hh:ww) - 2*BORDER_WIDTH - ma, ch = z - BORDER_WIDTH;
//	if (b) xcb_move_resize(dis, c->win, cx, cy += ma, ch - BORDER_WIDTH + d, cw);
//	else   xcb_move_resize(dis, c->win, cx, cy, cw, ch - BORDER_WIDTH + d);
//
//	/* tile the rest of the non-floating, non-fullscreen stack windows */
//	for (b?(cx+=ch+d):(cy+=ch+d), c=c->next; c; c=c->next) {
//		if (ISFFT(c)) continue;
//		if (b) { xcb_move_resize(dis, c->win, cx, cy, ch, cw); cx += z; }
//		else   { xcb_move_resize(dis, c->win, cx, cy, cw, ch); cy += z; }
//	}
//}
//
///* swap master window with current or
// * if current is head swap with next
// * if current is not head, then head
// * is behind us, so move_up until we
// * are the head */
//void swap_master() {
//	if (!current || !head->next) return;
//	if (current == head) move_down();
//	else while (current != head) move_up();
//	update_current(head);
//}
//
///* switch the tiling mode and reset all floating windows */
//void switch_mode(const Arg *arg) {
//	if (mode == arg->i) for (client *c=head; c; c=c->next) c->isfloating = False;
//	mode = arg->i;
//	tile(); update_current(current);
//	desktopinfo();
//}
//
///* tile all windows of current desktop - call the handler tiling function */
//void tile(void) {
//	if (!head) return; /* nothing to arange */
//	layout[head->next ? mode : MONOCLE](wh + (showpanel ? 0:PANEL_HEIGHT),
//				(TOP_PANEL && showpanel ? PANEL_HEIGHT:0));
//}
//
///* toggle visibility state of the panel */
//void togglepanel() {
//	showpanel = !showpanel;
//	tile();
//}
//
///* windows that request to unmap should lose their
// * client, so no invisible windows exist on screen
// */
//void unmapnotify(xcb_generic_event_t *e) {
//	xcb_unmap_notify_event_t *ev = (xcb_unmap_notify_event_t *)e;
//	client *c = wintoclient(ev->window);
//	if (c && ev->event != screen->root) removeclient(c);
//	desktopinfo();
//}
//
///* highlight borders and set active window and input focus
// * if given current is NULL then delete the active window property
// *
// * stack order by client properties, top to bottom:
// *  - current when floating or transient
// *  - floating or trancient windows
// *  - current when tiled
// *  - current when fullscreen
// *  - fullscreen windows
// *  - tiled windows
// *
// * a window should have borders in any case, except if
// *  - the window is the only window on screen
// *  - the window is fullscreen
// *  - the mode is MONOCLE and the window is not floating or transient */
//void update_current(client *c)
//{
//	if (!head) {
//		xcb_delete_property(dis, screen->root, netatoms[NET_ACTIVE]);
//		current = prevfocus = NULL;
//		return;
//	} else if (c == prevfocus) {
//		prevfocus = prev_client(current = prevfocus ? prevfocus:head);
//	} else if (c != current) {
//		prevfocus = current; current = c;
//	}
//	/* num of n:all fl:fullscreen ft:floating/transient windows */
//	int n = 0, fl = 0, ft = 0;
//	for (c = head; c; c = c->next, ++n) if (ISFFT(c)) { fl++; if (!c->isfullscrn) ft++; }
//	xcb_window_t w[n];
//	w[(current->isfloating||current->istransient)?0:ft] = current->win;
//	for (fl += !ISFFT(current)?1:0, c = head; c; c = c->next) {
//		xcb_change_window_attributes(dis, c->win, XCB_CW_BORDER_PIXEL,
//			(c == current ? &win_focus:&win_unfocus));
//		xcb_border_width(dis, c->win, (!head->next || c->isfullscrn
//					|| (mode == MONOCLE && !ISFFT(c))) ? 0:BORDER_WIDTH);
//		//if (CLICK_TO_FOCUS) xcb_grab_button(dis, 1, c->win, XCB_EVENT_MASK_BUTTON_PRESS, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
//		//   screen->root, XCB_NONE, XCB_BUTTON_INDEX_1, XCB_BUTTON_MASK_ANY);
//		if (c != current) w[c->isfullscrn ? --fl : ISFFT(c) ? --ft : --n] = c->win;
//	}
//
//	/* restack */
//	for (ft = 0; ft <= n; ++ft) xcb_raise_window(dis, w[n-ft]);
//
//	xcb_change_property(dis, XCB_PROP_MODE_REPLACE, screen->root, netatoms[NET_ACTIVE],
//			XCB_ATOM_WINDOW, 32, 1, &current->win);
//	xcb_set_input_focus(dis, XCB_INPUT_FOCUS_POINTER_ROOT, current->win, XCB_CURRENT_TIME);
//	//if (CLICK_TO_FOCUS) xcb_ungrab_button(dis, XCB_BUTTON_INDEX_1, XCB_NONE, current->win);
//	tile();
//}
//
///* find to which client the given window belongs to */
//client* wintoclient(xcb_window_t w) {
//	client *c = NULL;
//	int d = 0, cd = current_desktop;
//	for (uint8_t found = False; d<DESKTOPS && !found; ++d)
//		for (select_desktop(d), c=head; c && !(found = (w == c->win)); c=c->next);
//	if (cd != d-1) select_desktop(cd);
//	return c;
//}

/* Helper function to print error message and exit */
void die(int status, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}
	exit(status);
}

/* Helper function to print/log error to a file */
void log_err(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	FILE *log_file_ptr = fopen(LOG_FILE, "a");
	if (!log_file_ptr) return;
	vfprintf(log_file_ptr, fmt, ap);
	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fprintf(log_file_ptr, " %s\n", strerror(errno));
	} else {
		fputc('\n', log_file_ptr);
	}
	fclose(log_file_ptr);
}

void cleanup(void)
{
	//TODO: cleanup everything, dont leave zombie processes
	debug_print("cleanup: done\n");
}

void handle_signal(int signo)
{
	switch(signo) {
	case SIGCHLD:
		while (waitpid(-1, NULL, WNOHANG) > 0);
		break;
	case SIGINT:
	case SIGTERM:
	case SIGHUP:
		cleanup();
		exit(EXIT_SUCCESS);
	default:
		log_err("handle_signal: unexpected signo '%d", signo);
		break;
	}
}

void setup_sigaction(void)
{
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = handle_signal;
	sa.sa_flags = SA_NOCLDSTOP | SA_RESTART;

	if (sigaction(SIGCHLD, &sa, NULL) == -1
		|| sigaction(SIGINT, &sa, NULL) == -1
		|| sigaction(SIGHUP, &sa, NULL) == -1
		|| sigaction(SIGTERM, &sa, NULL) == -1) {
		die(1, "setup_sigaction: sigaction:");
	}
	debug_print("setup_sigaction: done\n");
}

void setup_xcb(void)
{
	//TODO
	debug_print("setup_xcb: done\n");
}

void event_loop(void)
{
	//TODO
}

/* Execute a command */
void spawn(const Arg *arg)
{
	// FOR COMPARISON: MONSTERWM IMPLEMENTATION
	//    if (fork()) return;
	//    if (dis) close(screen->root);
	//    setsid();
	//    execvp((char*)arg->com[0], (char**)arg->com);
	//    fprintf(stderr, "error: execvp %s", (char *)arg->com[0]);
	//    perror(" failed"); /* also prints the err msg */
	//    exit(EXIT_SUCCESS);

	// TODO: JPWM: PRIORITY: ROBUST AND CORRECT
	pid_t pid;

	pid = fork();
	switch(pid) {
	case -1:
		log_err("spawn: fork failed:");
		return;
	case 0:
		// TODO
	default:
		break;
	}
}

/* Switch window manager mode to TILE, BSTACK, MONOCLE or GRID */
void switch_mode(const Arg *arg)
{
	//TODO
}

void toggle_bar(void)
{
	//TODO 
}

int main(void)
{
	setup_sigaction();
	setup_xcb();
	/* from now on must handle all errors and not exit */
	event_loop();
	cleanup();
	die(1, "jpwm: exited abnormally");
}
