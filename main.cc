/*
 *	HT Editor
 *	main.cc
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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
#include "stddata.h"

#include <string.h>

char *htcopyrights[]=
{
	ht_name" "ht_version" ("HT_SYS_NAME")",
	ht_copyright1,
	ht_copyright2,
	0
};

void add_file_history_entry(char *n)
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
	INITDONE(app),
	INITDONE(clipboard),
	INITDONE(obj),
	INITDONE(analyser),
	INITDONE(asm),
	INITDONE(stddata),
	INITDONE(cfg)
};

int htstate;

bool init()
{
	for (htstate=0; htstate<(int)(sizeof initdone / sizeof initdone[0]); htstate++) {
		if (!initdone[htstate].i()) return false;
	}
	return true;
}

void done()
{
	for (htstate--; htstate>=0; htstate--) {
		initdone[htstate].d();
	}
}

void load_file(char *fn, UINT mode)
{
	add_file_history_entry(fn);
	((ht_app*)app)->create_window_file(fn, mode);
}

void show_help()
{
	((ht_app*)app)->create_window_help("hthelp.info", "Top");
}

void params(int argc, char *argv[])
{
	int escaped_params_start = 0;

#define EXPECT_PARAMEND(b) if ((j+1)!=len) { LOG_EX(LOG_ERROR, "syntax error in options"); b;}
#define EXPECT_PARAMS(p, b) if (i+p+1 > argc) { LOG_EX(LOG_ERROR, "syntax error in options"); b;}
#define NOTHING ((void)(0))
	for (int i=1; i<argc; i++) {
		if (argv[i][0]=='-') {
			int len = strlen(argv[i]);
			if (len==1) LOG_EX(LOG_ERROR, "unknown option: %s", argv[i]);
			if (argv[i][1]=='-') {
				if (len==2) {
					// --
					escaped_params_start = i+1;
					break;
				}
				if (strcmp(argv[i], "--bin") == 0) {
					EXPECT_PARAMS(1, NOTHING) else {
						load_file(argv[i+1], FOM_BIN);
						i++;
					}
				} else
				if (strcmp(argv[i], "--help") == 0) {
					show_help();
				} else
				if (strcmp(argv[i], "--text") == 0) {
					EXPECT_PARAMS(1, NOTHING) else {
						load_file(argv[i+1], FOM_TEXT);
						i++;
					}
				} else
				{
					LOG_EX(LOG_ERROR, "unknown option: %s", argv[i]);
				}
			} else {
				for (int j=1; j<len; j++) {
					switch (argv[i][j]) {
						case 'b':
							EXPECT_PARAMEND(break);
							EXPECT_PARAMS(1, break);
							load_file(argv[i+1], FOM_BIN);
							i++;
							break;
						case 'h':
							show_help();
							break;
						case 't':
							EXPECT_PARAMEND(break);
							EXPECT_PARAMS(1, break);
							load_file(argv[i+1], FOM_TEXT);
							i++;
							break;
						default:
							LOG_EX(LOG_ERROR, "unknown option: -%c", argv[i][j]);
							break;
					}
				}
			}
		} else {
//			*search_str_pos++ = argv[i];
			add_file_history_entry(argv[i]);
// FIXME: no auto-detection for now, cause it wont work properly...
//			load_file(argv[i], FOM_AUTO);
			load_file(argv[i], FOM_BIN);
		}
	}
	if (escaped_params_start) {
		char **s = &argv[escaped_params_start];
		while (*s) {
//          	*search_str_pos++ = *s++;
			add_file_history_entry(*s);
			load_file(*s, FOM_BIN);
			s++;
		}
	}
}

int main(int argc, char *argv[])
{
	this_app=argv[0];

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

	int projectconfig_version = 0;   // -1 for older and 1 for newer file found
	int projectconfig_magic = 0;     // !=0 meens wrong magic found

	loadstore_result load;
	int error_info;
	if (!load_projectconfig(&load, &error_info)) {
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
				LOG_EX(LOG_ERROR, "%s %s %s...", "current configuration file ("PROJECT_CONFIG_FILE_NAME") has", "invalid", "magic");
				projectconfig_magic = true;
				break;
			case LS_ERROR_VERSION:
				LOG_EX(LOG_ERROR, "%s %s %s...", "current configuration file ("PROJECT_CONFIG_FILE_NAME") has", "wrong", "version");
				if (error_info < ht_projectconfig_fileversion) {
					projectconfig_version = -1;
				} else {
					projectconfig_version = 1;
				}
				break;
			case LS_ERROR_CORRUPTED:
//				done();
				if (screen) delete screen;
				printf("\n\n\nfatal error loading configuration file (%s)", projectconfig_file);
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

	params(argc, argv);

	try {
		((ht_app*)app)->run(0);
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

	if (projectconfig_magic) {
		if (confirmbox_modal("%s %s %s...\noverwrite it ?", "current configuration file ("PROJECT_CONFIG_FILE_NAME") has", "wrong", "magic")!=button_ok) {
			save_config = false;
		}
	}

	if (projectconfig_version < 0) {
		if (confirmbox_modal("%s %s %s...\noverwrite it ?", "current configuration file ("PROJECT_CONFIG_FILE_NAME") has", "older", "version")!=button_ok) {
			save_config = false;
		}
	} else if (projectconfig_version > 0) {
		if (confirmbox_modal("%s %s %s...\noverwrite it ?", "current configuration file ("PROJECT_CONFIG_FILE_NAME") has", "NEWER", "version")!=button_ok) {
			save_config = false;
		}
	}

	if (save_config) {
		LOG("saving config...");
		save=save_projectconfig();
	}
	LOG("exit.");
	done();
	if (save!=LS_OK) {
		printf("save_projectfile(): error\n");
		return 1;
	}
	return 0;
}

