/*
 *	HT Editor
 *	main.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License version 2 as
 *	published by the Free Software Foundation.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "analy_register.h"
#include "asm.h"
#include "htapp.h"
#include "htatom.h"
#include "htcfg.h"
#include "htclipboard.h"
#include "htcurses.h"
#include "htexcept.h"
#include "hthist.h"
#include "htiobox.h"
#include "htkeyb.h"
#include "htidle.h"
#include "htmenu.h"
#include "htpal.h"
#include "htinfo.h"
#include "htreg.h"
#include "htsys.h"
#include "info/infoview.h"
#include "log.h"
#include "stddata.h"

char *htcopyrights[]=
{
	ht_name" "ht_version" ("HT_SYS_NAME") "__TIME__" on "__DATE__,
	ht_copyright1,
	ht_copyright2,
	NULL
};

static void add_file_history_entry(char *n)
{
	ht_clist *hist=(ht_clist*)find_atom(HISTATOM_FILE);
	if (hist) insert_history_entry(hist, n, 0);
}

typedef bool (*initfunc)();
typedef void (*donefunc)();

struct initdonefunc {
	initfunc i;
	donefunc d;
	char *name;
};

#define INITDONE(name) { init_##name, done_##name, #name }

initdonefunc initdone[] = {
	INITDONE(system),
	INITDONE(atom),
	INITDONE(string),
	INITDONE(data),
	INITDONE(pal),
	INITDONE(registry),
	INITDONE(keyb),
	INITDONE(idle),
	INITDONE(menu),
	INITDONE(hist),
	INITDONE(clipboard),
	INITDONE(obj),
	INITDONE(analyser),
	INITDONE(asm),
	INITDONE(stddata),
	INITDONE(cfg),
	INITDONE(app)
};

int htstate;

static bool init()
{
	for (htstate=0; htstate<(int)(sizeof initdone / sizeof initdone[0]); htstate++) {
		if (!initdone[htstate].i()) return false;
	}
	return true;
}

static void done()
{
	for (htstate--; htstate>=0; htstate--) {
		initdone[htstate].d();
	}
}

static void load_file(char *fn, UINT mode)
{
	char cfn[HT_NAME_MAX];
	char cwd[HT_NAME_MAX];

	cwd[0] = 0;
	getcwd(cwd, sizeof cwd);

	if (sys_common_canonicalize(cfn, fn, cwd, sys_is_path_delim)==0) {
		add_file_history_entry(cfn);
		((ht_app*)app)->create_window_file(cfn, mode, true);
	}
}

//#define SPLINE_TEST
#ifdef SPLINE_TEST
//#include "mathfunc.h"
#include "htspline.h"
#endif

static void show_help()
{
#ifdef SPLINE_TEST
	bounds b;
	b.x = 0; b.y = 0; b.w = 80; b.h = 23;
	ht_window *s=new ht_window();
	s->init(&b, "spline", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE, 0);
	b.w = 78;
	b.h = 21;
/*	MathFuncPlotter *sv = new MathFuncPlotter();
	sv->init(&b, "test");
	s->insert(sv);*/
	ht_spline_view *sv = new ht_spline_view();
	sv->init(&b, "test");
	s->insert(sv);
	((ht_app*)app)->insert_window(s, AWT_LOG, 0, false, NULL);
#else
	((ht_app*)app)->create_window_help(MAGIC_HT_HELP, "Top");
#endif
}

static void show_version()
{
	char **copyrights=htcopyrights;
		while (*copyrights) {
		printf("%s\n", *copyrights);
		copyrights++;
	}
	exit(0);
}

static void params(int argc, char *argv[], bool started)
{
	int escaped_params_start = 0;
	// FIXME: FOM_AUTO should be the default
	int load_mode = FOM_BIN;
	bool showhelp = false;
	
#define PARAM_ERROR(a...) {if (started) LOG_EX(LOG_ERROR, a);}
#define EXPECT_PARAMEND(b) if ((j+1)!=len) { PARAM_ERROR("syntax error in options"); b;}
#define EXPECT_PARAMS(p, b) if (i+p+1 > argc) { PARAM_ERROR("syntax error in options"); b;}
#define NOTHING ((void)(0))
	for (int i=1; i<argc; i++) {
		if (argv[i][0]=='-') {
			int len = strlen(argv[i]);
			if (len==1) PARAM_ERROR("unknown option: %s", argv[i]);
			if (argv[i][1]=='-') {
				if (len==2) {
					// --
					escaped_params_start = i+1;
					break;
				}
				if (strcmp(argv[i], "--auto") == 0) {
					EXPECT_PARAMS(1, NOTHING) else {
						if (started) load_file(argv[i+1], FOM_AUTO);
						i++;
					}
				} else
				if (strcmp(argv[i], "--bin") == 0) {
					EXPECT_PARAMS(1, NOTHING) else {
						if (started) load_file(argv[i+1], FOM_BIN);
						i++;
					}
				} else
				if (strcmp(argv[i], "--help") == 0) {
					showhelp = true;
				} else
				if (strcmp(argv[i], "--project") == 0) {
					EXPECT_PARAMS(1, NOTHING) else {
						if (started) ((ht_app*)app)->project_opencreate(argv[i+1]);
						i++;
					}
				} else
				if (strcmp(argv[i], "--text") == 0) {
					EXPECT_PARAMS(1, NOTHING) else {
						if (started) load_file(argv[i+1], FOM_TEXT);
						i++;
					}
				} else
				if (strcmp(argv[i], "--version") == 0) {
					show_version();
				} else
				{
					PARAM_ERROR("unknown option: %s", argv[i]);
				}
			} else {
				for (int j=1; j<len; j++) {
					switch (argv[i][j]) {
						case 'A':
							load_mode = FOM_AUTO;
							break;
						case 'B':
							load_mode = FOM_BIN;
							break;
						case 'T':
							load_mode = FOM_TEXT;
							break;
						case 'a':
							EXPECT_PARAMEND(break);
							EXPECT_PARAMS(1, break);
							if (started) load_file(argv[i+1], FOM_AUTO);
							i++;
							break;
						case 'b':
							EXPECT_PARAMEND(break);
							EXPECT_PARAMS(1, break);
							if (started) load_file(argv[i+1], FOM_BIN);
							i++;
							break;
						case 'h':
							showhelp = true;
							break;
						case 'p':
							EXPECT_PARAMEND(break);
							EXPECT_PARAMS(1, break);
							if (started) ((ht_app*)app)->project_opencreate(argv[i+1]);
							i++;
							break;
						case 't':
							EXPECT_PARAMEND(break);
							EXPECT_PARAMS(1, break);
							if (started) load_file(argv[i+1], FOM_TEXT);
							i++;
							break;
						case 'v':
							show_version();
							break;
						default:
							PARAM_ERROR("unknown option: -%c", argv[i][j]);
							break;
					}
				}
			}
		} else {
			if (started) {
				load_file(argv[i], load_mode);
			}
		}
	}
	if (escaped_params_start && started) {
		char **s = &argv[escaped_params_start];
		while (*s) {
			load_file(*s, load_mode);
			s++;
		}
	}
	if (showhelp && started) show_help();
}

#if defined(WIN32) || defined(__WIN32__)
#define LEAN_AND_MEAN
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
#if defined(WIN32) || defined(__WIN32__)
	HMODULE h = GetModuleHandle(NULL);
	GetModuleFileName(h, appname, sizeof appname-1);
#else
	strncpy(appname, argv[0], sizeof appname-1);
#endif

	params(argc, argv, 0);
	
	if (!init()) {
		int init_failed = htstate;
		done();
		printf("init(): fatal error in init_%s\n", initdone[init_failed].name);
		return 1;
	}

	((ht_app*)app)->sendmsg(msg_draw);

	char **copyrights=htcopyrights;
	while (*copyrights) {
		LOG(*copyrights);
		copyrights++;
	}
	LOG("appname = %s", appname);

	int systemconfig_version = 0;   // -1 for older and 1 for newer file found
	int systemconfig_magic = 0;     // !=0 meens wrong magic found

	loadstore_result load;
	int error_info;
	if (!load_systemconfig(&load, &error_info)) {
		switch (load) {
			case LS_OK:
				break;
			case LS_ERROR_NOT_FOUND:
				LOG_EX(LOG_WARN, "couldn't load configuration file...");
				break;
			case LS_ERROR_READ:
				LOG_EX(LOG_ERROR, "couldn't read configuration file...");
				infobox("couldn't read configuration file...");
				break;
			case LS_ERROR_MAGIC:
			case LS_ERROR_FORMAT:
				LOG_EX(LOG_ERROR, "%s %s %s...", "current configuration file ("SYSTEM_CONFIG_FILE_NAME") has", "invalid", "magic");
				systemconfig_magic = true;
				break;
			case LS_ERROR_VERSION:
				LOG_EX(LOG_ERROR, "%s %s %s...", "current configuration file ("SYSTEM_CONFIG_FILE_NAME") has", "wrong", "version");
				if (error_info < ht_systemconfig_fileversion) {
					systemconfig_version = -1;
				} else {
					systemconfig_version = 1;
				}
				break;
			case LS_ERROR_CORRUPTED:
//				done();
				if (screen) delete screen;
				printf("\n\n\nfatal error loading configuration file (%s)", systemconfig_file);
				if (error_info) {
					printf(":\nerror near line %d\n", error_info);
				} else {
					printf(".\n");
				}
				printf("please try to delete it.\n");
				return 1;
			default: {assert(0);}
		}
	}

	params(argc, argv, 1);

	try {
		((ht_app*)app)->run(false);
	} catch (ht_io_exception *x) {
		done();
		fprintf(stderr, "FATAL: %s: %s\n", "unhandled exception", x->what());
		return 1;
	} catch (std::exception *x) {
		done();
		fprintf(stderr, "FATAL: %s: %s\n", "unhandled exception", x->what());
		return 1;
	} catch (...) {
		done();
		fprintf(stderr, "FATAL: unknown %s !?\n", "unhandled exception");
		return 1;
	}

	loadstore_result save=LS_OK;
	bool save_config = true;

	if (systemconfig_magic) {
		if (confirmbox_modal("%s %s %s...\noverwrite it ?", "current configuration file ("SYSTEM_CONFIG_FILE_NAME") has", "wrong", "magic")!=button_ok) {
			save_config = false;
		}
	}

	if (systemconfig_version < 0) {
		if (confirmbox_modal("%s %s %s...\noverwrite it ?", "current configuration file ("SYSTEM_CONFIG_FILE_NAME") has", "older", "version")!=button_ok) {
			save_config = false;
		}
	} else if (systemconfig_version > 0) {
		if (confirmbox_modal("%s %s %s...\noverwrite it ?", "current configuration file ("SYSTEM_CONFIG_FILE_NAME") has", "NEWER", "version")!=button_ok) {
			save_config = false;
		}
	}

	if (save_config) {
		LOG("saving config...");
		save = save_systemconfig();
	}
	LOG("exit.");
	done();
	if (save!=LS_OK) {
		printf("save_systemconfig(): error\n");
		return 1;
	}
	return 0;
}

