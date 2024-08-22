/* see LICENSE for copyright and license */

#ifndef CONFIG_H
#define CONFIG_H

#define LOG_FILE	"/tmp/jpwm.log"

/* Buttons */
#define MOD1		Mod1Mask	/* ALT key */
#define MOD4		Mod4Mask	/* Super/Windows key */
#define CONTROL		ControlMask	/* Control key */
#define SHIFT		ShiftMask	/* Shift key */

/* Generic Settings */
#define DEFAULT_MODE	TILE		/* TILE, MONOCLE, BSTACK, GRID */
#define BAR_PROGRAM	NULL		/* Bar program, NULL for disabled */
#define BAR_HEIGHT	18		/* 0 for no panel */
#define BAR_TOP		True		/* False means bar is on bottom */
#define AUTO_RESIZE	False		/* Auto resize windows when focused */
#define MASTER_SIZE	0.52		/* Master size ratio */
#define NWIN_MASTER	True		/* False means new window is slave */
#define CLICK_FOCUS	True		/* Focus an unfocused window when clicked */
#define HOVER_FOCUS	False		/* Focus the window the mouse just entered */
#define BORDER_WIDTH	2		/* Window border width */
#define FOCUS_COLOR	"#ff950e"	/* Focused window border color */
#define UNFOCUS_COLOR	"#444444"	/* Unfocused window border color */
#define WORKSPACES	22		/* Number of workspaces */
#define DEFAULT_WS	0		/* The workspace to focus on exec */

/* Helper for spawning shell commands */
#define SHCMD(cmd) {.com = (const char*[]){"/bin/sh", "-c", cmd, NULL}}

/* Helper for chaning workspaces */
#define WORKSPACECHANGE(K,N) \
	{ MOD4,			K,	change_workspace,	{.i = N} }, \
	{ MOD4|ShiftMask,	K,	client_to_workspace,	{.i = N} }

/* Commands */
static const char *termcmd[] = {"st", NULL};
static const char *menucmd[] = {"dmenu_run", NULL};

/* Shortcuts */
static Key keys[] = {
	/* modifier		key		function		argument */
	//{  MOD4|SHIFT,		XK_t,		switch_mode,		{.i = TILE}},
	//{  MOD4|SHIFT,		XK_m,		switch_mode,		{.i = MONOCLE}},
	//{  MOD4|SHIFT,		XK_b,		switch_mode,		{.i = BSTACK}},
	//{  MOD4|SHIFT,		XK_g,		switch_mode,		{.i = GRID}},
	{  MOD4,		XK_b,		toggle_bar,		{NULL}},
	{  MOD4,		XK_x,		kill_client,		{NULL}},
	{  MOD4|SHIFT,		XK_x,		force_kill_client,	{NULL}},
	{  MOD4,		XK_j,		next_win,		{NULL}},
	{  MOD4,		XK_k,		prev_win,		{NULL}},
	{  MOD4,		XK_h,		resize_master,		{.i = -20}},
	{  MOD4,		XK_l,		resize_master,		{.i = +20}},
	{  MOD4,		XK_m,		mouse_aside,		{NULL}},
	{  MOD4|SHIFT,		XK_j,		rotate,			{.i = -1}},
	{  MOD4|SHIFT,		XK_k,		rotate,			{.i = +1}},
	{  MOD4,		XK_Return,	swap_master,		{NULL}},
	// TODO: key bind that switches to urgent windows
	{  MOD4,		XK_a,		last_workspace,		{NULL}},
	// TOOD: What does move_{down,up} do? Do I even need this when I have rotate?
	//{  MOD4|SHIFT,		XK_j,		move_down,		{NULL}},
	//{  MOD4|SHIFT,		XK_k,		move_up,		{NULL}},
	// TODO: recommended way of quitting is trough sending SIGNAL (kill)
	//{  MOD4|CONTROL,	XK_r,		quit,			{.i = 0}},
	//{  MOD4|CONTROL,	XK_q,		quit,			{.i = 1}},
	{  MOD4|SHIFT,		XK_Return,	spawn,			{.com = termcmd}},
	{  MOD4,		XK_p,		spawn,			{.com = menucmd}},
	// TODO: functionallity which shows all workspaces in a nice way
	//{  MOD4,		XK_Tab,		spawn,			SHCMD(jpws),
};

/* Switch workspaces */
static Key workspace_keys[] = {
	WORKSPACECHANGE(XK_1, 0),
	WORKSPACECHANGE(XK_2, 1),
	WORKSPACECHANGE(XK_3, 2),
	WORKSPACECHANGE(XK_4, 3),
	WORKSPACECHANGE(XK_5, 4),
	WORKSPACECHANGE(XK_6, 5),
	WORKSPACECHANGE(XK_7, 6),
	WORKSPACECHANGE(XK_8, 7),
	WORKSPACECHANGE(XK_9, 8),
	WORKSPACECHANGE(XK_0, 9),
	WORKSPACECHANGE(XK_F1, 10),
	WORKSPACECHANGE(XK_F2, 11),
	WORKSPACECHANGE(XK_F3, 12),
	WORKSPACECHANGE(XK_F4, 13),
	WORKSPACECHANGE(XK_F5, 14),
	WORKSPACECHANGE(XK_F6, 15),
	WORKSPACECHANGE(XK_F7, 16),
	WORKSPACECHANGE(XK_F8, 17),
	WORKSPACECHANGE(XK_F9, 18),
	WORKSPACECHANGE(XK_F10, 19),
	WORKSPACECHANGE(XK_F11, 20),
	WORKSPACECHANGE(XK_F12, 21),
};

#endif /* !CONFIG_H */
