/*
 *	HT Editor
 *	htapp.cc
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

#include "analy.h"
#include "cmds.h"
#include "log.h"
#include "mfile.h"
#include "htapp.h"
#include "atom.h"
#include "display.h"
#include "htcfg.h"
#include "htclipboard.h"
#include "htdialog.h"
#include "hteval.h"
#include "hthist.h"
#include "htidle.h"
#include "htinfo.h"
#include "htiobox.h"
#include "keyb.h"
#include "htmenu.h"
#include "htpal.h"
#include "htsearch.h"
#include "strtools.h"
#include "sys.h"
#include "httree.h"
#include "infoview.h"
#include "snprintf.h"
#include "stream.h"
#include "textedit.h"
#include "textfile.h"
#include "tools.h"

#include "vfsview.h"

#include "formats.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <exception>

extern "C" {
#include "regex.h"
}

#define ATOM_HT_APP			MAGIC32("APP\x00")
#define ATOM_HT_PROJECT			MAGIC32("APP\x01")
#define ATOM_HT_PROJECT_ITEM		MAGIC32("APP\x02")
#define ATOM_COMPARE_KEYS_PROJECT_ITEM	MAGIC32("APP\x10")

#define HT_PROJECT_CONFIG_SUFFIX     	".htprj"
#define HT_FILE_CONFIG_SUFFIX 		".htcfg"

ht_log *loglines;

/*
 *	CLASS ht_help_window
 */

class ht_help_window : public ht_window {
public:
/* overwritten */
	virtual void handlemsg(htmsg *msg);
};

void ht_help_window::handlemsg(htmsg *msg)
{
	ht_window::handlemsg(msg);
	if (msg->msg == msg_keypressed) {
		switch (msg->data1.integer) {
		case K_Escape: {
			htmsg m;
			m.type = mt_empty;
			m.msg = cmd_window_close;
			((ht_app*)app)->queuemsg(app, m);
			clearmsg(msg);
			return;
		}				
		}				
	}
}

bool file_new_dialog(uint *mode)
{
	Bounds b, c;
	
	app->getbounds(&b);

	b.x = (b.w - 40) / 2,
	b.y = (b.h - 8) / 2;
	b.w = 40;
	b.h = 8;
	
	ht_dialog *d=new ht_dialog();
	d->init(&b, "create new file", FS_KILLER | FS_TITLE | FS_MOVE | FS_RESIZE);
	
	b.x=0;
	b.y=0;
		
	/* mode (input) */
	c=b;
	c.x=0;
	c.y=1;
	c.w=b.w-2-c.x;
	c.h=b.h-2-c.y;

	ht_text_listbox *mode_input = new ht_text_listbox();
	mode_input->init(&c);
	
	mode_input->insert_str(FOM_TEXT, "text");
	mode_input->insert_str(FOM_BIN, "binary");
	mode_input->update();

	d->insert(mode_input);
	
	/* mode (text) */
	c=b;
	c.x=0;
	c.y=0;
	c.w=12;
	c.h=1;

	ht_label *mode_text=new ht_label();
	mode_text->init(&c, "choose ~type", mode_input);

	d->insert(mode_text);

	bool retval = false;
	if (d->run(false)) {
		ht_listbox_data type;

		ViewDataBuf vdb(mode_input, &type, sizeof type);

		*mode = mode_input->getID(type.data->cursor_ptr);
		
		retval = true;
	}

	d->done();
	delete d;
	return retval;
}

/*
 *	class FileBrowserVfsListbox
 */
 
class FileBrowser;

#define FileBrowserVfsListboxData VfsListboxData

class FileBrowserVfsListbox: public VfsListbox {
protected:
	FileBrowser *file_browser;
public:
		void init(Bounds *b, List *vfs_list, ht_text *show_pos, FileBrowser *file_browser);
/* overwritten */
	virtual	void stateChanged();
};

/*
 *	class FileBrowser
 */
 
struct FileBrowserData {
	ht_strinputfield_data name;
	ht_listbox_data listbox;
};

class FileBrowser: public ht_dialog {
protected:
	ht_strinputfield *name_input;
	FileBrowserVfsListbox *listbox;
public:
	virtual	void init(Bounds *b, Bounds *clientarea, const char *title, const char *starturl);
	virtual void setstate(int state, int return_val);
	virtual	bool extract_url(char *buf);
	virtual	void listbox_changed();
};

/**/
void	FileBrowserVfsListbox::init(Bounds *b, List *vfs_list, ht_text *show_pos, FileBrowser *fb)
{
	file_browser = NULL;
	VfsListbox::init(b, vfs_list, show_pos);
	file_browser = fb;
}

void FileBrowserVfsListbox::stateChanged()
{
	if (file_browser) file_browser->listbox_changed();
	VfsListbox::stateChanged();
}

/**/
void FileBrowser::init(Bounds *n, Bounds *clientarea, const char *title, const char *starturl)
{
	ht_dialog::init(n, title, FS_KILLER | FS_TITLE | FS_MOVE | FS_RESIZE);
	Bounds b = *clientarea, c;

	/* name (input) */
	c = b;
	c.x = 1;
	c.y = 1;
	c.w -= 2;
	c.h = 1;

	List *hist = (List*)getAtomValue(HISTATOM_FILE);
	
	name_input = new ht_strinputfield();
	name_input->init(&c, 128, hist);

	insert(name_input);
	
	/* name (text) */
	c = b;
	c.x = 1;
	c.y = 0;
	c.w = 9;
	c.h = 1;

	ht_label *name_text = new ht_label();
	name_text->init(&c, "~name", name_input);

	insert(name_text);

	/* vfslistbox */
	c = b;
	c.x = 1;
	c.y = 4;
	c.w -= 2;
	c.h -= 4;
	listbox = new FileBrowserVfsListbox();
	listbox->init(&c, virtual_fs_list, NULL, this);
	listbox->changeURL(starturl);

	insert(listbox);

	/* vfslistbox (text) */
	c.assign(1, 3, 9, 1);

	ht_label *name_listbox = new ht_label();
	name_listbox->init(&c, "~files", listbox);

	insert(name_listbox);
}

bool FileBrowser::extract_url(char *buf)
{
	ht_strinputfield_data i;
	ViewDataBuf vdb(name_input, &i, sizeof i);
/*	ht_text_listbox_item *t = (ht_text_listbox_item*)listbox->getbyid(d.listbox.cursor_id);
	vfs_extra *x = (vfs_extra*)t->extra_data;*/
	Vfs *vfs = listbox->getCurVfs();
	if (vfs) {
		int buflen = ht_snprintf(buf, VFS_URL_MAX, "%s:", listbox->getCurProto());
		char fname[VFS_URL_MAX];
		bin2str(fname, i.text, i.textlen);
		vfs->canonicalize(buf+buflen, fname, listbox->getCurDir());
		return true;
	}
	return false;
}

void FileBrowser::listbox_changed()
{
	FileBrowserVfsListboxData l;
	ViewDataBuf vdb(listbox, &l, sizeof l);
	ht_text_listbox_item *t = (ht_text_listbox_item*)l.data->cursor_ptr;
	if (t) {
		vfs_extra *x = (vfs_extra*)t->extra_data;
		ht_strinputfield_data i;
		i.textlen = strlen(x->name);
		i.text = (byte*)x->name;
		name_input->databuf_set(&i, sizeof i);
	}
}

void FileBrowser::setstate(int state, int return_val)
{
	if (state == ds_term_ok) {
		ht_strinputfield_data i;
		ViewDataBuf vdb(name_input, &i, sizeof i);
		pstat_t s;
		String fn(i.text, i.textlen);
		int e = sys_pstat(s, fn.contentChar());
		if (e == 0 && (s.caps & pstat_mode_type) && (s.mode & HT_S_IFDIR)) {
			fn.prepend("local:");
			listbox->changeURL(fn.contentChar());
			return;
		}
	} 
	ht_dialog::setstate(state, return_val);
}

/**/

bool file_chooser(const char *title, char *buf, int bufsize)
{
	Bounds b, c;
	
	app->getbounds(&b);

	c = b;
	b.w = 60;
	b.h = 20;
	b.x = (c.w - b.w) / 2,
	b.y = (c.h - b.h) / 2;

	c = b;
	c.x = 0;
	c.y = 0;
	c.w -= 2;
	c.h -= 3;

	// FIXME: hacked!
	char cwd[HT_NAME_MAX];
	strcpy(cwd, "local:");
	if (!getcwd(cwd+6, (sizeof cwd)-6)) {
		cwd[6] = 0;
	}

	FileBrowser *d = new FileBrowser();
	d->init(&b, &c, title, cwd);

	List *hist = (List*)getAtomValue(HISTATOM_FILE);

	/* go! */
	if (d->run(false)) {
		char b[VFS_URL_MAX];
		d->extract_url(b);

		// FIXME: urls not fully supported...
		if (ht_strncmp(b, "local:", 6) == 0) {
			ht_strlcpy(buf, b+6, bufsize);
			if (hist) insert_history_entry(hist, buf, NULL);

			d->done();
			delete d;
			return true;
		}
	}
	d->done();
	delete d;
	return false;
}
/**/

bool file_open_dialog(char **name, uint *mode)
{
	Bounds b, c;
	
	app->getbounds(&b);

	c = b;
	b.w = 60;
	b.h = 20;
	b.x = (c.w - b.w) / 2,
	b.y = (c.h - b.h) / 2;

	c = b;
	c.x = 0;
	c.y = 0;
	c.w -= 2;
	c.h -= 5;

	// FIXME: hacked!
	char cwd[HT_NAME_MAX];
	strcpy(cwd, "local:");
	if (!getcwd(cwd+6, (sizeof cwd)-6)) {
		cwd[6] = 0;
	}

	FileBrowser *d = new FileBrowser();
	d->init(&b, &c, "open file", cwd);
	
	List *hist = (List*)getAtomValue(HISTATOM_FILE);
	
	/* mode (input) */
	c = b;
	c.x = 6;
	c.y = b.h-4;
	c.w = 12;
	c.h = 1;

	ht_listpopup *mode_input = new ht_listpopup();
	mode_input->init(&c);
	
	mode_input->insertstring("autodetect");
	mode_input->insertstring("binary");
	mode_input->insertstring("text");
	
	mode_input->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);
	
	d->insert(mode_input);
	
	/* mode (text) */
	c = b;
	c.x = 1;
	c.y = b.h-4;
	c.w = 9;
	c.h = 1;

	ht_label *mode_text = new ht_label();
	mode_text->init(&c, "~mode", mode_input);
	mode_text->growmode = MK_GM(GMH_LEFT, GMV_BOTTOM);

	d->insert(mode_text);

	/* go! */
	if (d->run(false)) {
		struct {
			FileBrowserData browser;
			ht_listpopup_data mode;
		} data;

		ViewDataBuf vdb(d, &data, sizeof data);

		char buf[VFS_URL_MAX];
		d->extract_url(buf);

		// FIXME: urls not fully supported...
		if (ht_strncmp(buf, "local:", 6) == 0) {
			*name = ht_strdup(buf+6);

			if (hist) insert_history_entry(hist, *name, 0);

			switch (data.mode.cursor_pos) {
				case 0: *mode=FOM_AUTO; break;
				case 1: *mode=FOM_BIN; break;
				case 2: *mode=FOM_TEXT; break;
			}

			d->done();
			delete d;
			return true;
		}
	}
	d->done();
	delete d;
	return false;
}

static uint autodetect_file_open_mode(const char *filename)
{
#define AUTODETECT_SIZE	128
	FILE *f=fopen(filename, "rb");
	uint r=FOM_BIN;
	if (f) {
		byte buf[AUTODETECT_SIZE];
		int c=fread(buf, 1, AUTODETECT_SIZE, f);
		/* empty files are text files */
		if (!c) return FOM_TEXT;
		bool is_bin=false;
		uint prob_bin_chars=0;
		for (int i=0; i<c; i++) {
			if (buf[i]==0) {
				is_bin=true;
				break;
			} else if (buf[i]<32) {
				prob_bin_chars++;
			} else if (buf[i]>0xa9) {
				prob_bin_chars++;
			}
		}
		if (c) {
			if (prob_bin_chars*100/c>=50) is_bin=true;
		} else is_bin=true;
		if (!is_bin) r=FOM_TEXT;
		fclose(f);
		return r;
	}
	return FOM_BIN;
}

void file_window_load_fcfg_func(ObjectStream &f, void *context)
{
	ht_file_window *w = (ht_file_window*)context;

	pstat_t p;
	
	FileOfs oldsize = GETX_INT64D(f, "filesize");
	uint32 oldtime = GETX_INT32X(f, "filetime");
	FileOfs newsize = w->file->getSize();
	w->file->pstat(p);
	uint32 newtime = (p.caps & pstat_mtime) ? p.mtime : 0;
	
	if (newsize != oldsize || newtime != oldtime) {
		char s_oldtime[64], s_newtime[64];
		struct tm *t;
		time_t tt = newtime;
		t = localtime(&tt);
		strftime(s_newtime, sizeof s_newtime, "%X %d %b %Y", t);

		tt = oldtime;
		t = localtime(&tt);
		strftime(s_oldtime, sizeof s_oldtime, "%X %d %b %Y", t);

		String fn;
		if (confirmbox_c("\ecconfig file applies to different version of file '%y'.\n\n"
		    "\elcurrent: %10qd %s\n\elold:     %10qd %s\n\n"
		    "\ecload config file?", &w->file->getDesc(fn), newsize, s_newtime, oldsize, s_oldtime) != button_yes) {
			return;
		}
	}

	Analyser *a = f.getObject("analyser");

	htmsg m;
	m.msg = msg_set_analyser;
	m.type = mt_broadcast;
	m.data1.ptr = a;
	w->sendmsg(&m);
}

void file_window_store_fcfg_func(ObjectStream &f, void *context)
{
	ht_file_window *w = (ht_file_window*)context;
	htmsg m;
	m.msg = msg_get_analyser;
	m.type = mt_broadcast;
	m.data1.ptr = NULL;
	w->sendmsg(&m);
	if (m.msg == msg_retval && m.data1.ptr) {
		pstat_t s;
		w->file->pstat(s);
		uint32 t = (s.caps & pstat_mtime) ? s.mtime : 0;
		PUTX_INT64D(f, w->file->getSize(), "filesize");
		PUTX_INT32X(f, t, "filetime");
		
		Analyser *a = (Analyser*)m.data1.ptr;
		f.putObject(a, "analyser");
	}
}

void file_project_store_fcfg_func(ObjectStream &f, void *context)
{
	PUT_OBJECT(f, project);
}

void file_project_load_fcfg_func(ObjectStream &f, void *context)
{
	GET_OBJECT(f, project);
}

/*
 *   app_stream_error_func()
 */
#if 0
int app_stream_error_func(ht_stream *stream)
{
	int err=stream->get_error();
	const char *name = stream->get_desc();
	if (err & STERR_SYSTEM) {
		err=err&0xffff;
		switch (err) {
			case 4: {	/* EACCES*/
#ifdef DJGPP
				struct stat sbuf;

				stat(name, &sbuf);

				if (!(sbuf.st_mode & S_IWUSR)) {
					if (msgbox(btmask_yes | btmask_no, "title", 1, align_center, "%s: stream error (Permission denied), seems to be a (DOS) read-only file. Change attribute?", name)==button_yes) {
						if (chmod(name, S_IRUSR | S_IWUSR)) {
							errorbox_modal("%s: error (%04x) changing attribute", name, errno & 0xffff);
						} else {
							stat(name, &sbuf);

							if (!(sbuf.st_mode & S_IWUSR)) {
								errorbox_modal("%s: error (%04x) changing attribute", name, errno & 0xffff);
							} else {
								return SERR_RETRY;
							}
						}
					}
				}

				break;
#endif
			}
			default:
				errorbox_modal("%s: stream error %04x: %s", name, err, strerror(err));
		}
	} else {
		err=err&0xffff;
		errorbox_modal("%s: internal stream error %04x", name, err);
	}
	return SERR_FAIL;
}
#endif

/*
 *   app_out_of_memory_proc()
 */
void *app_memory_reserve = NULL;

int app_out_of_memory_proc(int size)
{
	if (app_memory_reserve) {
		free(app_memory_reserve);
		app_memory_reserve = 0;
		warnbox_modal("the memory is getting low...");
		return OUT_OF_MEMORY_RETRY;
	} else {
		return OUT_OF_MEMORY_FAIL;
	}
}

/*
 *	CLASS ht_project
 */

ht_project::ht_project(const char *fn)
	: AVLTree(true)
{
	filename = ht_strdup(fn);
}

ht_project::~ht_project()
{
	free(filename);
}

const char *ht_project::get_filename()
{
	return filename;
}

void ht_project::load(ObjectStream &s)
{
	// FIXME: probably not The Right Thing
	String fn;
	filename = ht_strdup(s.getDesc(fn).contentChar());
	AVLTree::load(s);
}

ObjectID ht_project::getObjectID() const
{
	return ATOM_HT_PROJECT;
}

void ht_project::store(ObjectStream &s) const
{
	AVLTree::store(s);
}

/*
 *	CLASS ht_project_item
 */

ht_project_item::ht_project_item(const char *f, const char *p)
{
	filename = ht_strdup(f);
	path = ht_strdup(p);
}

ht_project_item::~ht_project_item()
{
	free(filename);
	free(path);
}

const char *ht_project_item::get_filename() const
{
	return filename;
}

const char *ht_project_item::get_path() const
{
	return path;
}

int ht_project_item::compareTo(const Object *obj) const
{
	ht_project_item *b = (ht_project_item *)obj;
	int c = sys_filename_cmp(get_path(), b->get_path());
	return (c == 0) ? sys_filename_cmp(get_filename(), b->get_filename()) : c;
}

void ht_project_item::load(ObjectStream &s)
{
	GET_STRING(s, path);
	GET_STRING(s, filename);
}

ObjectID ht_project_item::getObjectID() const
{
	return ATOM_HT_PROJECT_ITEM;
}

void ht_project_item::store(ObjectStream &s) const
{
	PUT_STRING(s, path);
	PUT_STRING(s, filename);
}

/*
 *	CLASS ht_project_list
 */

void	ht_project_listbox::init(Bounds *b, ht_project *p)
{
	project = p;
	ht_listbox::init(b);
	colwidths[0] = 16;
	colwidths[1] = 16;
}

int  ht_project_listbox::calcCount()
{
	return project ? project->count() : 0;
}

void ht_project_listbox::draw()
{
	if (project) {
		ht_listbox::draw();
	} else {
	
		vcp fc = focused ? getcolor(palidx_generic_list_focused_unselected) :
			getcolor(palidx_generic_list_unfocused_unselected);

		clear(fc);
		buf->print(0, 0, fc, "<no project>");
	}
}

const char *ht_project_listbox::func(uint i, bool execute)
{
	return NULL;
}

void *ht_project_listbox::getFirst()
{
	if (project && project->count()) {
		return (void*)1;
	} else {
		return NULL;
	}
}

void *ht_project_listbox::getLast()
{
	if (project && project->count()) {
		return (void*)(project->count());
	} else {
		return NULL;
	}
}

void *ht_project_listbox::getNext(void *entry)
{
	unsigned long e=(unsigned long)entry;
	if (!e) return NULL;
	if (project && (e < project->count())) {
		return (void*)(e+1);
	} else {
		return NULL;
	}
}

void *ht_project_listbox::getPrev(void *entry)
{
	unsigned long e=(unsigned long)entry;
	if (e > 1) {
		return (void*)(e-1);
	} else {
		return NULL;
	}
}

const char *ht_project_listbox::getStr(int col, void *entry)
{
	static char mybuf[32];
	if (project) switch (col) {
		case 0:
			ht_snprintf(mybuf, sizeof mybuf, "%s", ((ht_project_item*)(*project)[(long)entry-1])->get_filename());
			break;
		case 1:
			ht_snprintf(mybuf, sizeof mybuf, "%s", ((ht_project_item*)(*project)[(long)entry-1])->get_path());
			break;
		default:
			strcpy(mybuf, "?");
	} else {
		strcpy(mybuf, "<no project>");
	}
	return mybuf;
}

void ht_project_listbox::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case cmd_project_add_item: {
			if (project) {
				char fn[FILENAME_MAX];
				if (file_chooser("Add project item", fn, sizeof fn)) {
					char ffn[HT_NAME_MAX];
					char *dir;
					if (sys_common_canonicalize(ffn, fn, NULL, sys_is_path_delim) == 0
					 && sys_basename(fn, ffn) == 0
					 && (dir = sys_dirname(ffn)) ) {
						ht_project_item *p = new ht_project_item(fn, dir);
						((ht_project*)project)->insert(p);
						app->sendmsg(msg_project_changed);
						free(dir);
					}
				}
			}
			clearmsg(msg);
			return;
		}
		case cmd_project_remove_item: {
			int p = pos;
			ht_project_item *pi = (ht_project_item*)(*project)[p];
			if (calcCount() && confirmbox("Really remove item '%s'?", pi->get_filename()) == button_ok) {
				cursorUp(1);
				project->delObj(pi);
				update();
				if (p) cursorDown(1);
				dirtyview();
				rearrangeColumns();
			}
			clearmsg(msg);
			return;
		}
		case msg_keypressed: {
			switch (msg->data1.integer) {
				case K_Return: {
					if (calcCount() && selectEntry(e_cursor)) {
						clearmsg(msg);
						return;
					}
				}
			}
			break;
		}
	}
	ht_listbox::handlemsg(msg);
}

int ht_project_listbox::numColumns()
{
	return 2;
}

void *ht_project_listbox::quickfind(const char *s)
{
	void *item = getFirst();
	int slen = strlen(s);
	while (item && (ht_strncmp(getStr(0, item), s, slen)!=0)) {
		item = getNext(item);
	}
	return item;
}

char	*ht_project_listbox::quickfindCompletition(const char *s)
{
	void *item = getFirst();
	char *res = NULL;
	int slen = strlen(s);
	while (item) {
		if (ht_strncmp(getStr(0, item), s, slen)==0) {
			if (!res) {
				res = ht_strdup(getStr(0, item));
			} else {
				int a = ht_strccomm(res, getStr(0, item));
				res[a] = 0;
			}
		}
		item = getNext(item);
	}
	return res;
}

bool ht_project_listbox::selectEntry(void *entry)
{
	int p = pos;
	ht_project_item *i = (ht_project_item *)(*project)[p];
	char fn[HT_NAME_MAX];
	if (sys_common_canonicalize(fn, i->get_filename(), i->get_path(), sys_is_path_delim)==0) {
		((ht_app*)app)->create_window_file(fn, FOM_AUTO, false);
	}
	return true;
}

void ht_project_listbox::set_project(ht_project *p)
{
	project = p;
	update();
}

/*
 *	CLASS ht_project_window
 */

void ht_project_window::init(Bounds *b, const char *desc, uint framestyle, uint number, ht_project **p)
{
	ht_window::init(b, desc, framestyle, number);
	project = p;

	Bounds c = *b;
	c.x = 0;
	c.y = 0;
	c.w -= 2;
	c.h -= 2;
	plb = new ht_project_listbox();
	plb->init(&c, *p);

	insert(plb);
	sendmsg(msg_project_changed);
}

void ht_project_window::done()
{
	ht_window::done();
}

void ht_project_window::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_project_changed: {
			const char *t = *project ? (*project)->get_filename() : NULL;
			if (t) {
				ht_snprintf(wtitle, sizeof wtitle, "project '%s'", t); 
			} else {
				strcpy(wtitle, "project window");
			}
			settitle(wtitle);
			plb->set_project(*project);
			break;
		}
		case msg_contextmenuquery: {
			ht_static_context_menu *projects = new ht_static_context_menu();
			projects->init("~Project");
			projects->insert_entry("~Add item", "Insert", cmd_project_add_item, K_Insert, 1);
			projects->insert_entry("~Remove item", "Delete", cmd_project_remove_item, K_Delete, 1);
//			projects->insert_entry("~Edit item", NULL, cmd_project_edit_item, 0, 1);

			msg->msg = msg_retval;
			msg->data1.ptr = projects;
			return;
		}
	}
	ht_window::handlemsg(msg);
}

/*
 *	CLASS ht_status
 */

void ht_status::init(Bounds *b)
{
	ht_view::init(b, VO_TRANSPARENT_CHARS | VO_RESIZE, 0);
	VIEW_DEBUG_NAME("ht_status");
	growmode = MK_GM(GMH_FIT, GMV_TOP);
	idle_count = 0;
	analy_ani = 0;
	clear_len = 0;
	format = get_config_string("misc/statusline");
	
	if (!format) {
		format = strdup(STATUS_DEFAULT_FORMAT);
	}
	
	render();
	register_idle_object(this);
}

void ht_status::done()
{
	unregister_idle_object(this);
	free(format);
	ht_view::done();
}

const char *ht_status::defaultpalette()
{
	return palkey_generic_menu_default;
}

void ht_status::draw()
{
	fill(size.w-clear_len, 0, clear_len, 1, getcolor(palidx_generic_text_focused), ' ');
	int len = strlen(workbuf);
	clear_len = len;
	buf->print(size.w-len, 0, getcolor(palidx_generic_text_focused), workbuf);
}

void ht_status::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
	case msg_config_changed:
		free(format);
		format = get_config_string("misc/statusline");
		break;
	}
	ht_view::handlemsg(msg);
}

void ht_status::getminbounds(int *width, int *height)
{
	*width = 1;
	*height = 1;
}

bool ht_status::idle()
{
	if (idle_count % 100 == 0) {
		char *oldstatus = ht_strdup(workbuf);
		render();
		if (strcmp(oldstatus, workbuf)) {
			dirtyview();
			redraw();
			screen->show();
		}
		free(oldstatus);
		
		analy_ani++;
		analy_ani &= 7;
	}
	idle_count++;
	return false;
}

void ht_status::render()
{
	char *f = format;
	char *buf = workbuf;
	if (f)
	while (*f) {
		if (*f == STATUS_ESCAPE) {
			switch (*(++f)) {
			case STATUS_ESCAPE:
				*(buf++) = STATUS_ESCAPE;
				break;
			case STATUS_ANALY_ACTIVE:
				if (some_analyser_active) {
					const char *analysers[] = {"Analy", "aNaly", "anAly", "anaLy", "analY", "anaLy", "anAly", "aNaly"};
					strcpy(buf, analysers[analy_ani]);
					buf += 5;
				}
				break;
			case STATUS_ANALY_LINES:
				if (some_analyser_active) {
					buf += sprintf(buf, "(%d)", num_ops_parsed);
				}
				break;
			case STATUS_TIME: {
				time_t Time;
				time(&Time);
				tm *t = localtime(&Time);
				buf += sprintf(buf, "%02d:%02d", t->tm_hour, t->tm_min);
				break;
			}
			case STATUS_DATE: {
				time_t Time;
				time(&Time);
				tm *t = localtime(&Time);
				buf += sprintf(buf, "%02d.%02d.%04d", t->tm_mday, t->tm_mon+1, t->tm_year+1900);
				break;
			}
			}
		} else {
			*(buf++) = *f;
		}
		f++;
	}
	*buf = 0;
}


/*
 *	CLASS ht_keyline
 */

void ht_keyline::init(Bounds *b)
{
	ht_view::init(b, VO_RESIZE, 0);
	VIEW_DEBUG_NAME("ht_keyline");
	growmode = MK_GM(GMH_FIT, GMV_BOTTOM);
}

void ht_keyline::done()
{
	ht_view::done();
}

const char *ht_keyline::defaultpalette()
{
	return palkey_generic_keys_default;
}

void ht_keyline::draw()
{
	clear(getcolor(palidx_generic_text_disabled));
	int x = 0;
	for (int i = 1; i <= 10; i++) {
		htmsg msg;
		msg.type = mt_empty;
		msg.msg = msg_funcquery;
		msg.data1.integer = i;
		baseview->sendmsg(&msg);
		buf->printChar(x, 0, getcolor(palidx_generic_text_shortcut), '0'+i%10);
		if (msg.msg == msg_retval) {
			char *s = msg.data1.str;
			if (s) {
				if (s[0]=='~') {
					buf->print(x+1, 0, getcolor(palidx_generic_text_disabled), s+1);
				} else {
					for (int j=0; j<size.w/10-1; j++) {
						buf->print(x+j+1, 0, getcolor(palidx_generic_text_focused), " ");
					}
					buf->print(x+1, 0, getcolor(palidx_generic_text_focused), s);
				}
			}
		}
		x += size.w / 10;
	}
}

void ht_keyline::getminbounds(int *width, int *height)
{
	*width = 1;
	*height = 1;
}

/*
 *	CLASS ht_desktop
 */

void ht_desktop::init(Bounds *b)
{
	ht_view::init(b, VO_OWNBUFFER | VO_RESIZE, 0);
	VIEW_DEBUG_NAME("ht_desktop");
	growmode = MK_GM(GMV_FIT, GMH_FIT);
}

const char *ht_desktop::defaultpalette()
{
	return palkey_generic_desktop_default;
}

void ht_desktop::draw()
{
	fill(0, 0, size.w, size.h, getcolor(palidx_generic_body), GC_MEDIUM, CP_GRAPHICAL);
}

/*
 *	CLASS ht_log_msg
 */
 
ht_log_msg::ht_log_msg(vcp Color, char *Msg)
{
	color = Color;
	msg = ht_strdup(Msg);
}

ht_log_msg::~ht_log_msg()
{
	free(msg);
}

/*
 *	CLASS ht_log
 */

ht_log::ht_log()
	: Array(true)
{
	maxlinecount = 128;
}

void ht_log::deletefirstline()
{
	del(0);
}

void	ht_log::insertline(LogColor color, char *line)
{
	if (count() >= maxlinecount) deletefirstline();
	vcp c;
	switch (color) {
		case LOG_NORMAL: c = VCP(VC_WHITE, VC_TRANSPARENT); break;
		case LOG_WARN: c = VCP(VC_LIGHT(VC_YELLOW), VC_TRANSPARENT); break;
		case LOG_ERROR: c = VCP(VC_LIGHT(VC_RED), VC_TRANSPARENT); break;
		default: c = VCP(VC_WHITE, VC_TRANSPARENT); break;
	}
	insert(new ht_log_msg(c, line));
}

void ht_log::log(LogColor c, char *line)
{
	insertline(c, line);
}

/*
 *	CLASS ht_logviewer
 */

void ht_logviewer::init(Bounds *b, ht_window *w, ht_log *l, bool ol)
{
	ht_viewer::init(b, "log", 0);
	VIEW_DEBUG_NAME("ht_logviewer");
	ofs = 0;
	xofs = 0;
	window = w;
	lines = l;
	own_lines = ol;
	update();
}

void ht_logviewer::done()
{
	if (own_lines) {
		delete lines;
	}
	ht_viewer::done();
}

int ht_logviewer::cursor_up(int n)
{
	ofs -= n;
	if (ofs < 0) ofs = 0;
	return n;
}

int ht_logviewer::cursor_down(int n)
{
	int c = lines->count();
	if (c >= size.h) {
		ofs += n;
		if (ofs > c-size.h) ofs = c-size.h;
	}
	return n;
}

void ht_logviewer::draw()
{
	clear(getcolor(palidx_generic_body));
	int c = lines->count();
	for (int i=0; i < size.h; i++) {
		if (i+ofs >= c) break;
		ht_log_msg *msg = (ht_log_msg*)(*lines)[i+ofs];
		int l = strlen(msg->msg);
		if (xofs < l) buf->print(0, i, /*getcolor(palidx_generic_body)*/msg->color, msg->msg+xofs);
	}
}

bool ht_logviewer::get_vscrollbar_pos(int *pstart, int *psize)
{
	return scrollbar_pos(ofs, size.h, lines->count(), pstart, psize);
}


void ht_logviewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_get_scrollinfo:
			switch (msg->data1.integer) {
			case gsi_vscrollbar: {
				gsi_scrollbar_t *p=(gsi_scrollbar_t*)msg->data2.ptr;
				if (!get_vscrollbar_pos(&p->pstart, &p->psize)) {
					p->pstart = 0;
					p->psize = 100;
				}
				clearmsg(msg);
				return;
			}
			}
			break;
		case msg_log_changed: {
			ofs = lines->count()-size.h;
			if (ofs < 0) ofs = 0;
			update();
			clearmsg(msg);
			return;
		}
		case msg_keypressed:
			switch (msg->data1.integer) {
			case K_Up:
				cursor_up(1);
				update();
				clearmsg(msg);
				return;
			case K_Down:
				cursor_down(1);
				update();
				clearmsg(msg);
				return;
			case K_PageUp:
				cursor_up(size.h);
				update();
				clearmsg(msg);
				return;
			case K_PageDown:
				cursor_down(size.h);
				update();
				clearmsg(msg);
				return;
			case K_Right: case K_Control_Right:
				xofs += 2;
				update();
				clearmsg(msg);
				return;
			case K_Left: case K_Control_Left:
				if (xofs-2 >= 0) xofs -= 2;
				update();
				clearmsg(msg);
				return;
			case K_Control_PageUp:
				ofs = 0;
				update();
				clearmsg(msg);
				return;
			case K_Control_PageDown:
				ofs = lines->count()-size.h;
				if (ofs < 0) ofs = 0;
				update();
				clearmsg(msg);
				return;
			}
			break;
	}
	ht_viewer::handlemsg(msg);
}

void ht_logviewer::update()
{
	dirtyview();
}

/*
 *	CLASS ht_app_window_entry
 */

ht_app_window_entry::ht_app_window_entry(ht_window *w, uint n, uint t, bool m, bool isf, FileLayer *l)
{
	window = w;
	number = n;
	type = t;
	minimized = m;
	isfile = isf;
	layer = l;
}

int ht_app_window_entry::compareTo(const Object *obj) const
{
	uint a = number;
	uint b = ((ht_app_window_entry*)obj)->number;
	if (a > b) return 1; else if (a < b) return -1;
	return 0;
}

/*
 *	CLASS ht_app
 */

static bool doFileChecks(File *file)
{
// FIXME: this is notify-user-only. should actually also take the actions it claims to...
//        (its done in stream.cc in ht_file::set_access_mode_internal()
	pstat_t s;
	file->pstat(s);
	if (s.caps & pstat_mode_type) {
		switch (s.mode & HT_S_IFMT) {
			case HT_S_IFREG: return true;
			default:
				LOG_EX(LOG_WARN, "file is not regular (but device, fifo or something...).");
				LOG_EX(LOG_WARN, "Write-access disabled for safety!");
				return true;
		}
	} else {
		LOG_EX(LOG_WARN, "can't determine file type (regular, device, directory...)! assuming non-regular...");
		LOG_EX(LOG_WARN, "file is not regular (but device, fifo or something...).");
		LOG_EX(LOG_WARN, "Write-access disabled for safety!");
		return true;
	}
	return false;
}

/*debug*/
//#define DRAW_TIMINGS
//#define NO_AVG
#define AVG_TIMINGS 10
int timings[AVG_TIMINGS];
int cur_timing=0, max_timing=0;
int h0;
/**/

void ht_app::init(Bounds *pq)
{
	ht_dialog::init(pq, 0, 0);
	menu = NULL;
	setframe(NULL);
	options |= VO_RESIZE;
	VIEW_DEBUG_NAME("ht_app");
	exit_program = false;
	focused = true;
	Bounds b;

	windows = new AVLTree(true);
	
	syntax_lexers = new Array(true);

	ht_c_syntax_lexer *c_lexer = new ht_c_syntax_lexer();
	c_lexer->init();

	syntax_lexers->insert(c_lexer);

	ht_html_syntax_lexer *html_lexer = new ht_html_syntax_lexer();
	html_lexer->init();

	syntax_lexers->insert(html_lexer);
	
	/* init timer */
	h0=new_timer();

	/* create menu */
	getbounds(&b);
	b.x=0;
	b.y=0;
	b.h=1;
	ht_menu *m=new ht_menu();
	m->init(&b);

	ht_static_context_menu *file=new ht_static_context_menu();
	file->init("~File");
	file->insert_entry("~New...", NULL, cmd_file_new, 0, 1);
	file->insert_entry("~Open...", "F3", cmd_file_open, 0, 1);
	file->insert_entry("~Save", NULL, cmd_file_save, 0, 1);
	file->insert_entry("Save ~As...", NULL, cmd_file_saveas, 0, 1);
	file->insert_separator();
	file->insert_entry("Open/Create ~project...", NULL, cmd_project_open, 0, 1);
	file->insert_entry("Close p~roject", NULL, cmd_project_close, 0, 1);
	file->insert_separator();
//	file->insert_entry("~Execute", "Alt+Z", cmd_file_exec_cmd, K_Meta_Z, 1);
	file->insert_entry("~Quit", "F10", cmd_quit, 0, 1);
	m->insert_menu(file);

	ht_static_context_menu *edit=new ht_static_context_menu();
	edit->init("~Edit");
	edit->insert_entry("Cu~t", "Shift+Del", cmd_edit_cut, 0, 1);
	edit->insert_entry("~Delete", "Ctrl+Del", cmd_edit_delete, 0, 1);
	edit->insert_entry("~Copy", "Ctrl+Ins", cmd_edit_copy, 0, 1);
	edit->insert_entry("~Paste", "Shift+Ins", cmd_edit_paste, 0, 1);
	edit->insert_entry("~Show clipboard", 0, cmd_edit_show_clipboard, 0, 1);
	edit->insert_entry("C~lear clipboard", 0, cmd_edit_clear_clipboard, 0, 1);
	edit->insert_separator();
	edit->insert_entry("Copy ~from file...", 0, cmd_edit_copy_from_file, 0, 1);
	edit->insert_entry("Paste ~into file...", 0, cmd_edit_paste_into_file, 0, 1);
	if (sys_get_caps() & SYSCAP_NATIVE_CLIPBOARD) {
		char s[100];
		edit->insert_separator();
		ht_snprintf(s, sizeof s, "Copy from %s", sys_native_clipboard_name());
		edit->insert_entry(s, 0, cmd_edit_copy_native, 0, 1);
		ht_snprintf(s, sizeof s, "Paste into %s", sys_native_clipboard_name());
		edit->insert_entry(s, 0, cmd_edit_paste_native, 0, 1);
	}
	edit->insert_separator();
	edit->insert_entry("~Evaluate...", 0, cmd_popup_dialog_eval, 0, 1);
	m->insert_menu(edit);

	ht_static_context_menu *windows=new ht_static_context_menu();
	windows->init("~Windows");
	windows->insert_entry("~Size/Move", "Alt+F5", cmd_window_resizemove, K_Meta_F5, 1);
	windows->insert_entry("~Close", "Alt+F3", cmd_window_close, K_Meta_F3, 1);
	windows->insert_entry("~Close (alt)", "Ctrl+W", cmd_window_close, K_Control_W, 1);
	windows->insert_entry("~List", "Alt+0", cmd_popup_dialog_window_list, K_Meta_0, 1);
	ht_static_context_menu *tile=new ht_static_context_menu();
	tile->init("~Tile");
	tile->insert_entry("~Vertically", NULL, cmd_window_tile_vertical, 0, 1);
	tile->insert_entry("~Horizontally", NULL, cmd_window_tile_horizontal, 0, 1);
	windows->insert_submenu(tile);
	windows->insert_separator();
	windows->insert_entry("Lo~g window", NULL, cmd_popup_window_log, 0, 1);
	windows->insert_entry("~Options", NULL, cmd_popup_window_options, 0, 1);
	windows->insert_entry("~Project", NULL, cmd_popup_window_project, 0, 1);
	m->insert_menu(windows);

	ht_static_context_menu *help=new ht_static_context_menu();
	help->init("~Help");
	help->insert_entry("~About "ht_name, "", cmd_about, 0, 1);
	help->insert_separator();
	help->insert_entry("~Help contents", "F1", cmd_popup_window_help, 0, 1);
	help->insert_entry("~Open info file...", NULL, cmd_popup_dialog_info_loader, 0, 1);
	m->insert_menu(help);

	m->insert_local_menu();

	menu = m;
	insert(menu);
	
	/* create status */
	/* the status should have the same Bounds as the menu */
	ht_status *status = new ht_status();
	status->init(&b);
	status->setpalette(menu->getpalette());
	insert(status);
	
	/* create desktop */
	getbounds(&b);
	b.x = 0;
	b.y = 1;
	b.h -= 2;
	desktop = new ht_desktop();
	desktop->init(&b);
	insert(desktop);
	
	/* create keyline */
	getbounds(&b);
	b.x = 0;
	b.y = b.h-1;
	b.h = 1;
	keyline = new ht_keyline();
	keyline->init(&b);
	insert(keyline);

	/* create battlefield */
	getbounds(&b);
	b.x = 0;
	b.y = 1;
	b.h -= 2;

	battlefield = new ht_group();
	battlefield->init(&b, VO_TRANSPARENT_CHARS | VO_RESIZE, "battlefield");
	battlefield->growmode = MK_GM(GMH_FIT, GMV_FIT);
	insert(battlefield);

	create_window_log();
}

void ht_app::done()
{
	delete_timer(h0);

	delete syntax_lexers;
	delete windows;

	ht_dialog::done();
}

bool ht_app::accept_close_all_windows()
{
	foreach(ht_app_window_entry, e, *windows, {
		htmsg m;
		m.msg = msg_accept_close;
		m.type = mt_empty;
		e->window->sendmsg(&m);
		if (m.msg != msg_accept_close) return false;
	});
	return true;
}

ht_window *ht_app::create_window_log()
{
	ht_window *w = get_window_by_type(AWT_LOG);
	if (w) {
		focus(w);
	} else {
		Bounds b;
/*		battlefield->getbounds(&b);
		b.x = 0;
		b.y = 0;*/
		get_stdbounds_file(&b);

		ht_window *logwindow=new ht_window();
		logwindow->init(&b, "log window", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0);
		
		Bounds k=b;
		k.x=b.w-2;
		k.y=0;
		k.w=1;
		k.h-=2;
		ht_scrollbar *hs=new ht_scrollbar();
		hs->init(&k, &logwindow->pal, true);

		logwindow->setvscrollbar(hs);

		b.x=0;
		b.y=0;
		b.w-=2;
		b.h-=2;
		ht_logviewer *logviewer=new ht_logviewer();
		logviewer->init(&b, logwindow, loglines, false);
		logwindow->insert(logviewer);
		
		insert_window(logwindow, AWT_LOG, 0, false, NULL);
	}
	return w;
}

ht_window *ht_app::create_window_term(const char *cmd)
{
	ht_window *w = get_window_by_type(AWT_TERM);
	if (w) {
		focus(w);
	} else {
		Bounds b;
		get_stdbounds_file(&b);

		ht_window *termwindow=new ht_window();
		termwindow->init(&b, "terminal", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0);
		
		Bounds k=b;
		k.x=3;
		k.y=k.h-2;
		k.w-=7;
		k.h=1;
		ht_statictext *ind=new ht_statictext();
		ind->init(&k, NULL, align_left, false, true);
		ind->disable_buffering();
		ind->growmode = MK_GM(GMH_FIT, GMV_BOTTOM);

		termwindow->setpindicator(ind);

		k=b;
		k.x=b.w-2;
		k.y=0;
		k.w=1;
		k.h-=2;
		ht_scrollbar *hs=new ht_scrollbar();
		hs->init(&k, &termwindow->pal, true);

		termwindow->setvscrollbar(hs);

/*FIXPORT
		File *in, *out, *err;
		int handle;
		int e;
		if ((e = sys_ipc_exec(&in, &out, &err, &handle, cmd, 0)) == 0) {
			Terminal *terminal = new Terminal();
			terminal->init(in, out, err, handle);

			b.x=0;
			b.y=0;
			b.w-=2;
			b.h-=2;
			TerminalViewer *termviewer=new TerminalViewer();
			termviewer->init(&b, terminal, true);
			termwindow->insert(termviewer);
		
			insert_window(termwindow, AWT_LOG, 0, false, NULL);
		} else {
			errorbox("couldn't create child-process (%d)", e);
			return NULL;
		}*/
	}
	return w;
}

ht_window *ht_app::create_window_clipboard()
{
	ht_window *w = get_window_by_type(AWT_CLIPBOARD);
	if (w) {
		focus(w);
		return w;
	} else {
		Bounds b;
		get_stdbounds_file(&b);
/*		ht_file_window *window=new ht_file_window();
		window->init(&b, "clipboard", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0, clipboard);*/
		ht_window *window = new ht_window();
		window->init(&b, "clipboard", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0);
		
/*		Bounds k=b;
		k.x=b.w-2;
		k.y=0;
		k.w=1;
		k.h-=2;
		ht_scrollbar *hs=new ht_scrollbar();
		hs->init(&k, &window->pal, true);

		window->setvscrollbar(hs);*/

		Bounds k;
		k = b;
		k.x=3;
		k.y=k.h-2;
		k.w-=7;
		k.h=1;
		ht_statictext *ind = new ht_statictext();
		ind->init(&k, NULL, align_left, false, true);
		ind->disable_buffering();
		ind->growmode = MK_GM(GMH_FIT, GMV_BOTTOM);

		window->setpindicator(ind);

		b.x=0;
		b.y=0;
		b.w-=2;
		b.h-=2;
		ht_clipboard_viewer *v = new ht_clipboard_viewer();
		v->init(&b, "clipboard", VC_EDIT | VC_GOTO | VC_SEARCH, clipboard, 0);

		window->insert(v);
		// FIXME: needs wrapper (layer)
		//insert_window(window, AWT_CLIPBOARD, 0, false, clipboard);
		insert_window(window, AWT_CLIPBOARD, 0, false, NULL);
	}
	return NULL;
}

ht_window *ht_app::create_window_file(const char *filename, uint mode, bool allow_duplicates)
{
	if (mode == FOM_AUTO) mode = autodetect_file_open_mode(filename);
	switch (mode) {
		case FOM_BIN: return create_window_file_bin(filename, allow_duplicates);
		case FOM_TEXT: return create_window_file_text(filename, allow_duplicates);
	}
	return NULL;
}

ht_window *ht_app::create_window_file_bin(const char *filename, bool allow_duplicates)
{
	Bounds b;
	get_stdbounds_file(&b);
	int e;
	char *fullfilename;
	if ((e = sys_canonicalize(&fullfilename, filename))) {
		LOG_EX(LOG_ERROR, "error loading file %s: %s", filename, strerror(e));
		return NULL;
	}
	String f(fullfilename);
	free(fullfilename);

	ht_window *w;
	if (!allow_duplicates && ((w = get_window_by_filename(f.contentChar())))) {
		focus(w);
		return w;
	}

	File *emfile = NULL;
	FileModificator *mfile = NULL;
	FileLayer *file = NULL;
	try {
		emfile = new LocalFile(f, IOAM_READ, FOM_EXISTS);
	    	if (!doFileChecks(emfile)) {
			delete emfile;
			return NULL;
		}
		mfile = new FileModificator(emfile, true);
		file = new FileLayer(mfile, true);
	} catch (const IOException &e) {
		LOG_EX(LOG_ERROR, "error loading file %y: %y", &f, &e);
		if (file) delete file;
		else if (mfile) delete mfile;
		else delete emfile;
		return NULL;
	}

	LOG("loading binary file %y...", &f);

	return create_window_file_bin(&b, file, f.contentChar(), true);
}

ht_window *ht_app::create_window_file_bin(Bounds *b, FileLayer *file, const char *title, bool isfile)
{
	ht_file_window *window = new ht_file_window();
	window->init(b, title, FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0, file);

	Bounds k=*b;
	k.x=b->w-2;
	k.y=0;
	k.w=1;
	k.h-=2;
	ht_scrollbar *hs=new ht_scrollbar();
	hs->init(&k, &window->pal, true);

	window->setvscrollbar(hs);

	k=*b;
	k.x=3;
	k.y=k.h-2;
	k.w-=7;
	k.h=1;
	ht_statictext *ind=new ht_statictext();
	ind->init(&k, NULL, align_left, false, true);
	ind->disable_buffering();
	ind->growmode = MK_GM(GMH_FIT, GMV_BOTTOM);

	window->setpindicator(ind);

	k=*b;
	k.x=0;
	k.y=0;
	k.w-=2;
	k.h-=2;
	ht_format_group *format_group=new ht_format_group();
	format_group->init(&k, VO_SELECTABLE | VO_RESIZE, VIEWERGROUP_NAME, file, true, true, format_viewer_ifs, NULL);

	window->insert(format_group);

	/**/
	if (isfile) {
		char cfgfilename[FILENAME_MAX];
		strcpy(cfgfilename, title);
		strcat(cfgfilename, HT_FILE_CONFIG_SUFFIX);

		String einfo;
		LOG("%s: loading config file...", cfgfilename);
		loadstore_result lsr = load_fileconfig(cfgfilename, ht_fileconfig_magic, ht_fileconfig_fileversion, file_window_load_fcfg_func, window, einfo);
		if (lsr == LS_ERROR_CORRUPTED) {
			LOG_EX(LOG_ERROR, "%s: error: ", cfgfilename, &einfo);
			errorbox("%s: error: %y", cfgfilename, &einfo);
		} else if (lsr == LS_ERROR_MAGIC || lsr == LS_ERROR_FORMAT) {
			LOG_EX(LOG_ERROR, "%s: wrong magic/format", cfgfilename);
			errorbox("%s: wrong magic/format", cfgfilename);
		} else if (lsr == LS_ERROR_VERSION) {
			LOG_EX(LOG_ERROR, "%s: wrong version", cfgfilename);
			errorbox("%s: wrong version", cfgfilename);
		} else if (lsr == LS_ERROR_NOT_FOUND) {
			LOG("%s: not found", cfgfilename);
		} else if (lsr != LS_OK) {
			LOG_EX(LOG_ERROR, "%s: error: %y", cfgfilename, &einfo);
			errorbox("%s: error: %y", cfgfilename, &einfo);
		} else {
			LOG("%s: ok", cfgfilename);
		}
	}

	/**/
	if (isfile) LOG("%s: done.", title);

	htmsg m;
	m.msg = msg_postinit;
	m.type = mt_broadcast;
	window->sendmsg(&m);

	insert_window(window, AWT_FILE, 0, isfile, file);
	return window;
}

ht_window *ht_app::create_window_file_text(const char *filename, bool allow_duplicates)
{
	Bounds b, c;
	get_stdbounds_file(&c);
	b = c;
	int e;
	char *fullfilename;
	if ((e = sys_canonicalize(&fullfilename, filename))) {
		LOG_EX(LOG_ERROR, "error loading file %s: %s", filename, strerror(e));
		return NULL;
	}
	String f(fullfilename);
	free(fullfilename);
	
	ht_window *w;
	if (!allow_duplicates && ((w = get_window_by_filename(f.contentChar())))) {
		focus(w);
		return w;
	}

	File *emfile = NULL;
	ht_ltextfile *tfile = NULL;
	FileLayer *file = NULL;
	try {
		File *emfile = new LocalFile(f, IOAM_READ, FOM_EXISTS);

		if (!doFileChecks(emfile)) {
			delete emfile;
			return NULL;
		}
		tfile = new ht_ltextfile(emfile, true, NULL);
		file = new ht_layer_textfile(tfile, true);
	} catch (const IOException &e) {
		LOG_EX(LOG_ERROR, "error loading file %y: %y", &f, &e);
		if (file) delete file;
		else if (tfile) delete tfile;
		else delete emfile;
		return NULL;
	}

	LOG("loading text file %y...", &f);

	return create_window_file_text(&b, file, f.contentChar(), true);
}

ht_window *ht_app::create_window_file_text(Bounds *c, FileLayer *f, const char *title, bool isfile)
{
	Bounds b=*c;

	ht_layer_textfile *file = (ht_layer_textfile *)f;
	
	ht_file_window *window = new ht_file_window();
	window->init(&b, title, FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0, file);

	b.x=0;
	b.y=0;
	b.w-=2;
	b.h-=2;
	ht_text_editor *text_editor=new ht_text_editor();
	text_editor->init(&b, true, file, syntax_lexers, TEXTEDITOPT_INPUTTABS|TEXTEDITOPT_UNDO);

	IString fn, base, fn_suf;
	file->getFilename(fn);
	if (fn.rightSplit('.', base, fn_suf)) {
		if (fn_suf == "c" || fn_suf == "cc"
		 || fn_suf == "cpp"
		 || fn_suf == "h" || fn_suf == "hpp") {
			text_editor->set_lexer((ht_syntax_lexer*)(*syntax_lexers)[0], false);
		}
#ifdef HT_HTML_SYNTAX_LEXER
		if (fn_suf == "htm" || fn_suf == "html") {
			text_editor->set_lexer((ht_syntax_lexer*)(*syntax_lexers)[1], false);
		}
#endif
	}

	Bounds k=*c;
	k.x=k.w-2;
	k.y=0;
	k.w=1;
	k.h-=2;
	ht_scrollbar *hs=new ht_scrollbar();
	hs->init(&k, &window->pal, true);

	window->setvscrollbar(hs);

	k=*c;
	k.x=3;
	k.y=k.h-2;
	k.w-=7;
	k.h=1;
	
	ht_statictext *ind=new ht_statictext();
	ind->init(&k, NULL, align_left, false, true);
	ind->disable_buffering();
	ind->growmode = MK_GM(GMH_FIT, GMV_BOTTOM);

	window->setpindicator(ind);

	window->insert(text_editor);
	if (isfile) LOG("%s: done.", title);

	insert_window(window, AWT_FILE, 0, isfile, file);
	return window;
}

ht_window *ht_app::create_window_help(const char *file, const char *node)
{
	ht_window *w = get_window_by_type(AWT_HELP);
	if (w) {
		focus(w);
		return w;
	} else {
		Bounds b, c;
		battlefield->getbounds(&c);
		b.w=c.w*7/8;
		b.h=c.h*7/8;
		b.x=(c.w-b.w)/2;
		b.y=(c.h-b.h)/2;
		Bounds k = b;

		ht_help_window *window=new ht_help_window();
		window->init(&b, "help", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0);

		b.x=0;
		b.y=0;
		b.w-=2;
		b.h-=2;
		c=b;

		b=c;
		b.x+=b.w;
		b.w=1;
		ht_scrollbar *scrollbar=new ht_scrollbar();
		scrollbar->init(&b, &window->pal, true);
		scrollbar->enable();
		window->setvscrollbar(scrollbar);

		k.x=3;
		k.y=k.h-2;
		k.w-=7;
		k.h=1;
		ht_statictext *ind=new ht_statictext();
		ind->init(&k, NULL, align_left, false, true);
		ind->disable_buffering();
		ind->growmode = MK_GM(GMH_FIT, GMV_BOTTOM);

		window->setpindicator(ind);

		b=c;
		ht_info_viewer *infoviewer=new ht_info_viewer();
		infoviewer->init(&b);
		window->insert(infoviewer);

		char ff[HT_NAME_MAX], cwd[HT_NAME_MAX];

		cwd[0] = 0;
		if (strcmp(file, "hthelp.info") != 0 && getcwd(cwd, sizeof cwd)) {
			sys_common_canonicalize(ff, file, cwd, sys_is_path_delim);
		} else {
			strcpy(ff, file);
		}
		if (infoviewer->gotonode(ff, node)) {
			insert_window(window, AWT_HELP, 0, false, NULL);
			
			window->setpalette(palkey_generic_cyan);
			return window;
		}
		errorbox("help topic '(%s)%s' not found", file, node);
		window->done();
		delete window;
	}
	return NULL;
}

ht_window *ht_app::create_window_project()
{
	ht_window *w = get_window_by_type(AWT_PROJECT);
	if (w) {
		focus(w);
		return w;
	} else {
		Bounds b;
		get_stdbounds_tool(&b);

		ht_project_window *project_window=new ht_project_window();
		project_window->init(&b, "project window", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0, (ht_project**)&project);

		Bounds k = b;
		k.x = b.w-2;
		k.y = 0;
		k.w = 1;
		k.h -= 2;
		ht_scrollbar *hs = new ht_scrollbar();
		hs->init(&k, &project_window->pal, true);

		project_window->sethscrollbar(hs);

/*		b.x = 0;
		b.y = 0;
		b.w -= 2;
		b.h -= 2;
		ht_project_listbox *project_viewer = new ht_project_listbox();
		project_viewer->init(&b, project);
		project_window->insert(project_viewer);*/

		insert_window(project_window, AWT_PROJECT, 0, false, NULL);
		
		project_window->setpalette(palkey_generic_cyan);
		return project_window;
	}
}

#if 0
ht_view *create_ofm_single(Bounds *c, char *url, ht_vfs_viewer **x)
{
	Bounds b=*c;
	b.h-=2;
	ht_group *g=new ht_group();
	g->init(&b, VO_SELECTABLE, 0);

	Bounds d=b;
	
	b.x=0;
	b.y=0;
	b.h=1;
	ht_vfs_viewer_status *vst=new ht_vfs_viewer_status();
	vst->init(&b);
	   
	d.x=0;
	d.y=1;
	d.h--;
	ht_vfs_viewer *v=new ht_vfs_viewer();
	v->init(&d, "vfs viewer", 0, 0, 0, vst);
	   
	g->insert(v);

	g->insert(vst);

	ht_vfs_sub *vs=new ht_vfs_sub();
	vs->init(url);

	v->insertsub(vs);

	v->sendmsg(msg_complete_init, 0);

	*x=v;
	return g;
}
#endif

ht_window *ht_app::create_window_ofm(const char *url1, const char *url2)
{
	Bounds b;
	get_stdbounds_file(&b);

	ht_window *window=new ht_window();
	window->init(&b, "file manager", FS_KILLER | FS_TITLE | FS_NUMBER | FS_MOVE | FS_RESIZE, 0);
#if 1
	b.w-=2;
	b.h-=2;

	VfsListbox2 *l = new VfsListbox2();
	l->init(&b, virtual_fs_list, window->getframe());

//	l->changeURL("local:/bp/././..");
	l->changeURL(url1);

	window->insert(l);
#else
	Bounds b1, b2, b3;

	ht_vfs_viewer *v1, *v2=NULL;

	b.w-=2;

	b1=b;
	b1.w/=2;
	b1.w--;
	window->insert(create_ofm_single(&b1, url1, &v1));

	if (url2) {
		b2=b;
		b2.w/=2;
		b2.x=b2.w-1;
		b2.w=1;
		b2.h-=2;
		ht_vbar *x=new ht_vbar();
		x->init(&b2, 0, 0);
		window->insert(x);

		b3=b;
		b3.w/=2;
		b3.x=b3.w;
		window->insert(create_ofm_single(&b3, url2, &v2));
	}

	if (v2) {
		v1->set_assoc_vfs_viewer(v2);
		v2->set_assoc_vfs_viewer(v1);
	}
#endif
	insert_window(window, AWT_OFM, 0, false, NULL);
	return window;
}

const char *ht_app::defaultpalette()
{
	return NULL;
}

const char *ht_app::defaultpaletteclass()
{
	return NULL;
}

int analy_id = 0;
void ht_app::draw()
{
/* show draw timings */
#ifdef DRAW_TIMING
	int xyz=get_timer_1024tick(h0);
	buf->printf(17, 1, 7, "cur: %d (%d msec)", xyz*1024, get_timer_msec(h0));
#ifndef NO_AVG
	if (cur_timing>=AVG_TIMINGS-1) cur_timing=0;
	timings[cur_timing++]=xyz;
	if (cur_timing>max_timing) max_timing=cur_timing;
	int avg=0;
	for (int i=0; i<max_timing; i++) avg+=timings[i];
	avg=avg/max_timing;
	buf->printf(57, 1, 7, "avg%d: %d", max_timing+1, avg*1024);
#endif
#endif
}

void ht_app::delete_window(ht_window *window)
{
	ObjHandle oh = get_window_listindex(window);
	if (oh != invObjHandle) {
		battlefield->remove(window);

		windows->del(oh);

		window->done();
		delete window;
	}
}

uint ht_app::find_free_window_number()
{
	ht_app_window_entry e(NULL, 1, 0, false, false, NULL);
	ObjHandle oh;
	while ((oh = windows->find(&e)) != invObjHandle) {
		e.number++;
	}
	return e.number;
}

bool ht_app::focus(ht_view *view)
{
	return ht_dialog::focus(view);
}

const char *ht_app::func(uint i, bool execute)
{
	switch (i) {
		case 1:
			if (execute) sendmsg(cmd_popup_window_help);
			return "help";
		case 3:
			if (execute) sendmsg(cmd_file_open);
			return "open";
		case 6:
			if (execute) sendmsg(cmd_popup_dialog_view_list);
			return "mode";
		/* FIXME: experimental */
/*		case 9:
			if (execute) {
				create_window_term("make");
			}
			return "make";*/
		case 10:
			if (execute) sendmsg(cmd_quit);
			return "quit";
	}
	return 0;
}

void ht_app::get_stdbounds_file(Bounds *b)
{
	if (project) {
		Bounds c;
		get_stdbounds_tool(&c);
		battlefield->getbounds(b);
		b->x = 0;
		b->y = 0;
		b->h -= c.h;
	} else {
		battlefield->getbounds(b);
		b->x = 0;
		b->y = 0;
	}
}

void ht_app::get_stdbounds_tool(Bounds *b)
{
	uint h = MAX(size.h/4, 3);
	battlefield->getbounds(b);
	b->x = 0;
	b->y = b->h - h;
	b->h = h;
}

ht_window *ht_app::get_window_by_filename(const char *filename)
{
	foreach(ht_app_window_entry, e, *windows, {
		// FIXME: filename_compare (taking into account slash/backslash, and case)
		if (strcmp(e->window->desc, filename) == 0) return e->window;
	});
	return NULL;
}

ht_window *ht_app::get_window_by_number(uint number)
{
	ht_app_window_entry *e;
	firstThat(ht_app_window_entry, e, *windows, e->number==number);
	return e ? e->window : NULL;
}

ht_window *ht_app::get_window_by_type(uint type)
{
	ht_app_window_entry *e;
	firstThat(ht_app_window_entry, e, *windows, e->type==type);
	return e ? e->window : NULL;
}

uint ht_app::get_window_number(ht_window *window)
{
	ht_app_window_entry *e;
	firstThat(ht_app_window_entry, e, *windows, e->window==window);
	return e ? e->number : 0;
}

ObjHandle ht_app::get_window_listindex(ht_window *window)
{
	ObjHandle oh;
	for (oh = windows->findFirst(); oh != invObjHandle; ) {
		ht_app_window_entry *e = (ht_app_window_entry*)windows->get(oh);
		if (e->window == window) break;
		oh = windows->findNext(oh);
	}
	return oh;
}

#include "cstream.h"
void ht_app::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case cmd_edit_copy_native: {
			int dz = sys_native_clipboard_get_size();
			if (dz) {
				void *data = ht_malloc(dz);
				if ((dz = sys_native_clipboard_read(data, dz))) {
					clipboard_copy(sys_native_clipboard_name(), data, dz);
				}
				free(data);
			}
			break;
		}
		case cmd_edit_paste_native: {
			int maxsize = clipboard_getsize();
			if (maxsize) {
				byte *buf = ht_malloc(maxsize);
				int r = clipboard_paste(buf, maxsize);
				if (r) {
					sys_native_clipboard_write(buf, r);
				}
				free(buf);
			}
			break;
		}
		case cmd_file_save: {
			ObjHandle oh = get_window_listindex((ht_window*)battlefield->current);
			ht_app_window_entry *e = (ht_app_window_entry*)windows->get(oh);
			if (e && e->layer && e->isfile) break;
		}
		case cmd_file_saveas: {
			ObjHandle oh = get_window_listindex((ht_window*)battlefield->current);
			ht_app_window_entry *e = (ht_app_window_entry*)windows->get(oh);
			if (e && e->layer) {
				char fn[HT_NAME_MAX];
				fn[0] = 0;
				if (file_chooser("Save as", fn, sizeof fn)) {
					bool work = true;

					if (access(fn, F_OK) == 0) {
						work = (confirmbox("File '%s' already exists. Overwrite?", fn) == button_yes);
					}

					if (work) {
						LocalFile *f = NULL;
						try {
							String n(fn);
							f = new LocalFile(n, IOAM_WRITE, FOM_CREATE);
							e->layer->seek(0);
							e->layer->copyAllTo(f);
						} catch (const IOException &e) {
							LOG_EX(LOG_ERROR, "error saving file %s: %y", fn, &e);
							errorbox("error saving file %s: %y", fn, &e);
							delete f;
							return;
						}
//						asm(".byte 0xcc");

						File *old = e->layer->getLayered();

						if (f->setAccessMode(old->getAccessMode()) == 0) {
							FileLayer *l;
							// FIXME: UGLY hack
							if (dynamic_cast<ht_ltextfile *>(old)) {
								l = new ht_ltextfile(f, true, NULL);
							} else {
								l = new FileModificator(f, true);
							}
							e->layer->setLayered(l, true);
							e->isfile = true;

							delete old;

							char *fullfn;
							if (sys_canonicalize(&fullfn, fn)==0) {
								e->window->settitle(fullfn);
								free(fullfn);
							} else {
								e->window->settitle(fn);
							}
							clearmsg(msg);
						} else {
							String s1, s2;
							errorbox("couldn't inherit access_mode from '%y' to '%y'", &old->getDesc(s1), &f->getDesc(s2));
						}
					}
				}
			}
			return;
		}
		case msg_kill: {
			htmsg m;
			ht_window *w = (ht_window*)msg->data1.ptr;
			m.msg = msg_accept_close;
			m.type = mt_broadcast;
			m.data1.ptr = NULL;
			m.data2.ptr = NULL;
			w->sendmsg(&m);
			if (m.msg == msg_accept_close) delete_window(w);
			clearmsg(msg);
			return;
		}
	}
	if (msg->msg == msg_draw) {
		start_timer(h0);
		if (msg->type == mt_broadcast) {
			ht_view *v = first;
			while (v) {
				v->handlemsg(msg);
				v = v->next;
			}
		} else {
			current->handlemsg(msg);
		}
		stop_timer(h0);
		draw();
		screen->show();
	} else {
		ht_group::handlemsg(msg);
	}
	switch (msg->msg) {
		case msg_keypressed: {
			int i=0;
			switch (msg->data1.integer) {
				case K_Meta_9: i++;
				case K_Meta_8: i++;
				case K_Meta_7: i++;
				case K_Meta_6: i++;
				case K_Meta_5: i++;
				case K_Meta_4: i++;
				case K_Meta_3: i++;
				case K_Meta_2: i++;
				case K_Meta_1: i++;
					focus(get_window_by_number(i));
					clearmsg(msg);
					return;
				case K_F12: i++;
				case K_F11: i++;
				case K_F10: i++;
				case K_F9: i++;
				case K_F8: i++;
				case K_F7: i++;
				case K_F6: i++;
				case K_F5: i++;
				case K_F4: i++;
				case K_F3: i++;
				case K_F2: i++;
				case K_F1:
					i++;
					htmsg m;
					m.msg=msg_funcquery;
					m.type=mt_empty;
					m.data1.integer=i;
					sendmsg(&m);
					if (m.msg==msg_retval) {
						sendmsg(msg_funcexec, i);
						clearmsg(msg);
						return;
					}
					break;
/*					
				case K_Meta_R: {
					const char *n = "ht.reg";
					LocalFile f(n, IOAM_WRITE, FOM_CREATE);
					CompressedStream c(&f, false);
//					StreamLayer c(&f, false);
					ObjectStreamBin b(&c, false);

					b.putObject(registry, NULL);

					infobox("registry dumped to '%s'", n);
					
					clearmsg(msg);
					return;
				}
*/
#if 0
				/* FIXME: experimental */
				case K_Control_F9:
					((ht_app*)app)->create_window_term("main.exe");
					clearmsg(msg);
					return;
				case K_Meta_T:
					create_window_ofm("reg:/", "local:/");
					clearmsg(msg);
					return;*/
				case K_Control_A:
					create_window_help("/HT/res/info/intidx.info", "Top");
//					create_window_help("c:/djgpp/projects/enew/res/info/ibnidx.info", "Interrupts By Number");
					dirtyview();
					clearmsg(msg);
					return;
#endif
				case K_Space:
					sendmsg(cmd_popup_dialog_view_list);
					clearmsg(msg);
					return;
			}
			break;
		}
		case cmd_about:
			msgbox(btmask_ok, "About "ht_name, 0, align_custom, "\n"
				"\ec"ht_name" "ht_version" (%s)\n\n"
				"\el"ht_copyright1"\n"ht_copyright2"\n\n"
				"This program is GPL'd. See Help for more information.", sys_get_name());
			break;
		case msg_funcexec:
			if (func(msg->data1.integer, 1)) {
				clearmsg(msg);
				return;
			}
			break;
		case msg_funcquery: {
			const char *s = func(msg->data1.integer, 0);
			if (s) {
				msg->msg = msg_retval;
				msg->data1.cstr = s;
			} else clearmsg(msg);
			return;
		}
		case cmd_file_exec_cmd: {
			char cmd[HT_NAME_MAX];
			cmd[0] = 0;
			if (inputbox("execute shell command (experimental!)",
			    (sys_get_caps() & SYSCAP_NBIPC) ? "command"
			    : "non-interactive (!) command",
			    cmd, sizeof cmd, HISTATOM_FILE) == button_ok) {
				if (cmd[0]) create_window_term(cmd);
			}
			clearmsg(msg);
			return;
		}
		case cmd_file_extend: {
			File *f = (File *)msg->data1.ptr;
			FileOfs s = msg->data2.q;
			// don't ask. only for truncates
//			if (confirmbox("really extend %s to offset %08qx/%qd?", f->get_filename(), s, s) == button_ok) {
				IOAccessMode oam = f->getAccessMode();
				if (!(oam & IOAM_WRITE)) f->setAccessMode(oam | IOAM_WRITE);
				try {
					f->extend(s);
				} catch (const IOException &e) {
					errorbox("couldn't extend file to offset 0x%08qx/%qd: %y", s, s, &e);
				}
				if (!(oam & IOAM_WRITE)) f->setAccessMode(oam);
//			}
			clearmsg(msg);
			return;
		}
		case cmd_file_truncate: {
			File *f = (File *)msg->data1.ptr;
			FileOfs s = msg->data2.q;
/*			ht_app_window_entry *e;
			if ((e = windows->enum_first())) {
				do {
					if (e->type == AWT_FILE && (File*)e->data == f) {
						check_collide();
					}
				} while ((e = windows->enum_next()));
			}*/
			String fn;
			if (confirmbox("really truncate %y at offset 0x%08qx/%qd?", &f->getFilename(fn), s, s) == button_ok) {
				f->truncate(s);
			}
			clearmsg(msg);
			return;
		}
		case cmd_quit:
			if (accept_close_all_windows()) {
				LOG("terminating...");
				exit_program = true;
				sendmsg(cmd_project_close);
				clearmsg(msg);
			}
			return;
		case cmd_file_open: {
			char *name;
			uint mode;
			if (file_open_dialog(&name, &mode)) {
				if (name[0]) create_window_file(name, mode, true);
				free(name);
			}
			clearmsg(msg);
			return;
		}
		case cmd_file_new: {
			Bounds b;
			get_stdbounds_file(&b);
			
			uint mode;
			
			if (file_new_dialog(&mode)) {
				MemoryFile *mfile = new MemoryFile();
				switch (mode) {
				case FOM_TEXT: {
					ht_syntax_lexer *lexer = NULL;

					ht_ltextfile *tfile = new ht_ltextfile(mfile, true, lexer);
					ht_layer_textfile *file = new ht_layer_textfile(tfile, true);

					create_window_file_text(&b, file, "Untitled", false/* because mem_file is underlying, not ht_file, etc.*/);
					break;
				}
				case FOM_BIN: {
					FileModificator *modfile = new FileModificator(mfile, true);
					FileLayer *file = new FileLayer(modfile, true);

					ht_window *w = create_window_file_bin(&b, file, "Untitled", false);
					htmsg m;
					m.msg = cmd_file_resize;
					m.type = mt_empty;
					w->sendmsg(&m);
				}
				}
			}
			clearmsg(msg);
			return;
		}
		case cmd_edit_show_clipboard: {
			create_window_clipboard();
			clearmsg(msg);
			return;
		}
		case cmd_edit_clear_clipboard: {
			if (confirmbox("Do you really want to delete the clipboard?")==button_ok) {
				clipboard_clear();
			}
			clearmsg(msg);
			return;
		}
		case cmd_edit_paste_into_file: {
			char filename[HT_NAME_MAX];
			filename[0] = 0;
			if (file_chooser("clipboard - paste into file", filename, sizeof filename)) {
				try {
					String fn(filename);
					LocalFile f(fn, IOAM_WRITE, FOM_CREATE);
					clipboard_paste(&f, 0);
				} catch (const IOException &e) {
					errorbox("error writing: '%s': '%y'", filename, &e);
				}
			}
			clearmsg(msg);
			return;
		}
		case cmd_edit_copy_from_file: {
			char filename[HT_NAME_MAX];
			filename[0] = 0;
			if (file_chooser("clipboard - copy from file", filename, sizeof filename)) {
				char desc[HT_NAME_MAX+5];
				try {
					String fn(filename);
					LocalFile f(fn, IOAM_READ, FOM_EXISTS);
					ht_snprintf(desc, sizeof desc, "file %s", filename);
					clipboard_copy(desc, &f, 0, f.getSize());
				} catch (const IOException &e) {
					errorbox("error reading: '%s': '%y'", filename, &e);
				}
			}
			clearmsg(msg);
			return;
		}
		case cmd_project_open: {
			char fn[HT_NAME_MAX];
			fn[0] = 0;
			if (file_chooser("Open project", fn, sizeof fn)) {
				sendmsg(cmd_project_close);
				project_opencreate(fn);
			}
			clearmsg(msg);
			return;
		}
		case cmd_project_close: {
			if (project) {
				const char *fn = ((ht_project*)project)->get_filename();
				LOG("%s: saving project", fn);
				do {
					String err;
					if (save_fileconfig(fn, ht_projectconfig_magic, ht_projectconfig_fileversion, file_project_store_fcfg_func, NULL, err) != LS_OK) {
						String e;
						e.assignFormat("Couldn't save '%y': %y\n\nRetry?", &fn, &err);
						switch (msgbox(btmask_yes+btmask_no+btmask_cancel, "confirmation", 0, align_center, e.contentChar())) {
						case button_ok: continue;
						case button_cancel: return;
						}
						LOG("%s: failed", fn);
					} else {
						LOG("%s: done", fn);
					}
					break;
				} while (1);
				delete ((ht_project*)project);
				project = NULL;
				htmsg m;
				m.type = mt_broadcast;
				m.msg = msg_project_changed;
				sendmsg(&m);
			}
			clearmsg(msg);
			return;
		}
		case cmd_window_close:
			if (battlefield->current) sendmsg(msg_kill, battlefield->current);
			clearmsg(msg);
			return;
		case cmd_window_tile_vertical:
			tile(true);
			clearmsg(msg);
			return;
		case cmd_window_tile_horizontal:
			tile(false);
			clearmsg(msg);
			return;
		case cmd_popup_dialog_eval: {
			eval_dialog(NULL, NULL, NULL);
			clearmsg(msg);
			return;
		}
		case cmd_popup_dialog_view_list: {
			ht_view *v = popup_view_list("select mode");
			if (v) focus(v);
			clearmsg(msg);
			return;
		}
		case cmd_popup_dialog_window_list: {
			ht_window *w = popup_window_list("select window");
			if (w) focus(w);
			clearmsg(msg);
			return;
		}
		case cmd_popup_dialog_info_loader: {
			char file[256];
			file[0] = 0;
			if (inputbox("open info file", "filename", file, sizeof file, HISTATOM_FILE) == button_ok) {
				char node[256];
				strcpy(node, "Top");
				if (inputbox("open info file", "nodename", node, sizeof node, HISTATOM_GOTO) == button_ok) {
					create_window_help(file, node);
					dirtyview();
				}
			}
			clearmsg(msg);
			return;
		}
		case cmd_popup_window_log:
			create_window_log();
			clearmsg(msg);
			return;
		case cmd_popup_window_project:
			create_window_project();
			clearmsg(msg);
			return;
		case cmd_popup_window_options:
			create_window_ofm("reg:/", NULL);
			clearmsg(msg);
			return;
		case cmd_popup_window_help:
			create_window_help(MAGIC_HT_HELP, "Top");
			clearmsg(msg);
			return;
		case msg_project_changed: {
			ht_window *w = ((ht_app*)app)->get_window_by_type(AWT_PROJECT);
			if (w) w->sendmsg(msg_dirtyview);
			app->sendmsg(msg_draw);
			return;
		}
	}
}

void	ht_app::insert_window(ht_window *window, uint type, bool minimized, bool isfile, FileLayer *layer)
{
	uint n=find_free_window_number();
	ht_app_window_entry *e=new ht_app_window_entry(window, n, type, minimized, isfile, layer);
	windows->insert(e);
	window->setnumber(n);
	battlefield->insert(window);
	focus(window);
}

void ht_app::load(ObjectStream &f)
{
	ht_registry *tmp = registry;
	GET_OBJECT(f, registry);

	if (tmp) {
		tmp->done();
		delete tmp;
	}
	
	load_history(f);
	
	htmsg m;
	m.msg = msg_config_changed;
	m.type = mt_broadcast;
	app->sendmsg(&m);
}

ObjectID ht_app::getObjectID() const
{
	return ATOM_HT_APP;
}

static int my_compare_func(const char *a, const char *b)
{
	return strcmp(a, b);
}

ht_view *ht_app::popup_view_list(const char *dialog_title)
{
	if (!battlefield->current) return NULL;
	Bounds b, c;
	getbounds(&b);
	b.x=b.w/4;
	b.y=b.h/4;
	b.w/=2;
	b.h/=2;
	ht_dialog *dialog=new ht_dialog();
	dialog->init(&b, dialog_title, FS_KILLER | FS_TITLE | FS_MOVE);

	/* create listbox */
	c=b;
	c.x=0;
	c.y=0;
	c.w-=2;
	c.h-=2;
	ht_text_listbox *listbox=new ht_text_listbox();
	listbox->init(&c, 1, 0, LISTBOX_NORMAL);

	/* insert all browsable views */
	Array structure(false);

	int index=0;
/*	int count=*/popup_view_list_dump(battlefield->current, listbox, &structure, 0, &index, battlefield->getselected());
/* and seek to the currently selected one */
	listbox->update();
	listbox->gotoItemByPosition(index);

	ht_text_listbox_sort_order so[1];
	so[0].col = 0;
	so[0].compare_func = my_compare_func;
/*	so[1].col = 1;
	so[1].compare_func = my_compare_func;*/
	
//	listbox->sort(1, so);

	dialog->insert(listbox);
	dialog->setpalette(palkey_generic_special);

	ht_view *result = NULL;
	if (dialog->run(false)) {
		ht_listbox_data data;
		ViewDataBuf vdb(listbox, &data, sizeof data);
		result = (ht_view*)structure[listbox->getID(data.data->cursor_ptr)];
	}

	dialog->done();
	delete dialog;
	return result;
}

int ht_app::popup_view_list_dump(ht_view *view, ht_text_listbox *listbox, List *structure, int depth, int *currenti, ht_view *currentv)
{
	if (!view) return 0;
	char str[256];	/* secure */
	char *s=str;
	if (!(view->options & VO_BROWSABLE)) {
		depth--;
	}

	for (int i=0; i<depth; i++) { *(s++)=' ';*(s++)=' '; }
	*s=0;

	int c=view->childcount();
	int count=0;
	for (int i=0; i<c; i++) {
		ht_view *v = view->getfirstchild();
		while (v) {
			if (v->browse_idx == i) break;
			v = v->next;
		}
		if (!v) return count;

		// FIXME: "viewergroup": dirty hack !!!
		if (v->desc && (strcmp(v->desc, VIEWERGROUP_NAME)) && (v->options & VO_BROWSABLE)) {
			ht_snprintf(s, sizeof str-(s-str), "- %s", v->desc);
			structure->insert(v);
			if (v==currentv)
				*currenti=structure->count()-1;
			listbox->insert_str(structure->count()-1, str);
			count++;
		}
		count += popup_view_list_dump(v, listbox, structure, depth+1, currenti, currentv);
	}
	return count;
}

ht_window *ht_app::popup_window_list(const char *dialog_title)
{
	Bounds b, c;
	getbounds(&b);
	c=b;
	b.w=b.w*2/3;
	b.h=b.h*2/3;
	b.x=(c.w-b.w)/2;
	b.y=(c.h-b.h)/2;
	ht_dialog *dialog=new ht_dialog();
	dialog->init(&b, dialog_title, FS_KILLER | FS_TITLE | FS_MOVE);

	/* create listbox */
	c=b;
	c.x=0;
	c.y=0;
	c.w-=2;
	c.h-=2;
	ht_text_listbox *listbox=new ht_itext_listbox();
	listbox->init(&c, 2, 1);

	foreach(ht_app_window_entry, e, *windows, {
		char l[16];	/* secure */
		ht_snprintf(l, sizeof l, " %2d", e->number);
		listbox->insert_str(e->number, l, e->window->desc);		
	});
	listbox->update();
	if (battlefield->current) listbox->gotoItemByPosition(battlefield->current->getnumber()-1);

	dialog->insert(listbox);
	dialog->setpalette(palkey_generic_special);

	ht_window *result = NULL;
	if (dialog->run(false)) {
		ht_listbox_data data;
		ViewDataBuf vdb(listbox, &data, sizeof data);
		result = get_window_by_number(listbox->getID(data.data->cursor_ptr));
	}
	dialog->done();
	delete dialog;
	return result;
}

void ht_app::project_opencreate(const char *filename)
{
	char fn[HT_NAME_MAX];
	char cwd[HT_NAME_MAX];
	if (getcwd(cwd, sizeof cwd) == NULL) {
		LOG("getcwd(): %s", strerror(errno));
		return;
	}
	if (sys_common_canonicalize(fn, filename, cwd, sys_is_path_delim) != 0) {
		LOG("%s: invalid filename", filename);
		return;
	}
	const char *suf = sys_filename_suffix(fn);
	/* append HT project file suffix if not already there */
	if (!(suf && (strcmp(suf, HT_PROJECT_CONFIG_SUFFIX+1)==0))) {
		strcat(fn, HT_PROJECT_CONFIG_SUFFIX);
	}

	Object *old_project = project;
	project = NULL;

	String einfo;
	LOG("%s: loading project file...", fn);
	bool error = true;
	loadstore_result lsr = load_fileconfig(fn, ht_projectconfig_magic, ht_projectconfig_fileversion, file_project_load_fcfg_func, NULL, einfo);
	if (lsr == LS_ERROR_CORRUPTED) {
		LOG_EX(LOG_ERROR, "%s: error: %y", fn, &einfo);
		errorbox("%s: error: %y", fn, &einfo);
	} else if (lsr == LS_ERROR_NOT_FOUND) {
		if (confirmbox("%s: no such project.\nDo you want to create this project?", fn) == button_yes) {
			project = new ht_project(fn);
			LOG("%s: new project created", fn);
			error = false;
		}
	} else if (lsr != LS_OK) {
		LOG_EX(LOG_ERROR, "%s: error: %y", fn, &einfo);
		errorbox("%s: error: %y", fn, &einfo);
	} else {
		LOG("%s: done", fn);
		error = false;
	}
	if (error) {
		// FIXME: free project ???
		project = old_project;
	} else {
		htmsg m;
		m.type = mt_broadcast;
		m.msg = msg_project_changed;
		sendmsg(&m);
		create_window_project();
	}
}

void ht_app::modal_resize()
{
	sys_set_winch_flag(0);
	int w, h;
	if (sys_get_screen_size(w, h)) {
		screen->resize(w - screen->w, h - screen->h);
		resize(w - size.w, h - size.h);
		sendmsg(msg_dirtyview);
		sendmsg(msg_draw);
	}
}

void do_modal_resize()
{
	((ht_app*)app)->modal_resize();
}

static uint isqr(uint u)
{
	uint a = 2;
	uint b = u/a;
	while (abs(a - b) > 1) {
		a = (a+b)/2;
		b = u/a;
        }
	return MIN(a, b);
}

static void mostEqualDivisors(int n, int &x, int &y)
{
	int i;

	i = isqr(n);
	if (n%i && n%(i+1)==0) {
		i++;
	}
	if (i < (n/i)) {
		i = n/i;
	}
	x = n/i;
	y = i;
}

static int dividerLoc(int lo, int hi, int num, int pos)
{
	return int(long(hi-lo)*pos/long(num)+lo);
}

static void calcTileRect(int pos, int numRows, int numCols, int leftOver, const Bounds &b, Bounds &nB)
{
	int x, y;

	int d = (numCols - leftOver) * numRows;
	if (pos <  d) {
		x = pos / numRows;
		y = pos % numRows;
	} else {
		x = (pos-d)/(numRows+1) + (numCols-leftOver);
		y = (pos-d)%(numRows+1);
	}
	nB.x = dividerLoc(b.x, b.x+b.w, numCols, x);
	nB.w = dividerLoc(b.x, b.x+b.w, numCols, x+1) - nB.x;
	if (pos >= d) {
		nB.y = dividerLoc(b.y, b.y+b.h, numRows+1, y);
		nB.h = dividerLoc(b.y, b.y+b.h, numRows+1, y+1) - nB.y;
	} else {
		nB.y = dividerLoc(b.y, b.y+b.h, numRows, y);
		nB.h = dividerLoc(b.y, b.y+b.h, numRows, y+1) - nB.y;
	}
}

void ht_app::tile(bool vertical)
{
	Bounds b, bf;
	get_stdbounds_file(&b);
	battlefield->getbounds(&bf);
	int numTileable = 0;
	foreach(ht_app_window_entry, e, *windows, {
		if (e->isfile) numTileable++;
	});
	// count
	if (numTileable > 0) {
		int numRows, numCols;
		if (vertical) {
			mostEqualDivisors(numTileable, numRows, numCols);
		} else {
			mostEqualDivisors(numTileable, numCols, numRows);
		}
		if (b.w/numCols==0 || b.h/numRows==0) {
			return;
		} else {
			int leftOver = numTileable % numCols;
			int tileNum = numTileable - 1;
			
			foreachbwd(ht_app_window_entry, e, *windows, {
				if (e->isfile) {
					Bounds nb, ob;
					calcTileRect(tileNum, numRows, numCols, leftOver, b, nb);
					e->window->getbounds(&ob);
					e->window->move(nb.x-(ob.x-bf.x), nb.y-(ob.y-bf.y));
					e->window->resize(nb.w-ob.w, nb.h-ob.h);
					tileNum--;
				}
			});
		}
	}
	sendmsg(msg_dirtyview);
}

int ht_app::run(bool modal)
{
	sendmsg(msg_draw, 0);
	while (!exit_program) {
		try {
			if (keyb_keypressed()) {
				ht_key k = keyb_getkey();
				sendmsg(msg_keypressed, k);
				sendmsg(msg_draw);
			}
			if (sys_get_winch_flag()) {
				modal_resize();
			}
			ht_queued_msg *q;
			while ((q = dequeuemsg())) {
				htmsg m = q->msg;
				q->target->sendmsg(&m);
				sendmsg(msg_draw);
				delete q;
			}
			do_idle();
		} catch (const Exception &x) {
			errorbox("unhandled exception: %y", &x);
		} catch (const std::exception &x) {
			errorbox("unhandled exception: %s", x.what());
		} catch (...) {
			errorbox("unhandled exception: unknown");
		}
	}
	return 0;
}

void ht_app::store(ObjectStream &f) const
{
	PUT_OBJECT(f, registry);
	store_history(f);
}

/*
 *	CLASS ht_vstate_history_entry
 */

ht_vstate_history_entry::ht_vstate_history_entry(Object *Data, ht_view *View)
{
	data = Data;
	view = View;
}

ht_vstate_history_entry::~ht_vstate_history_entry()
{
	data->done();
	delete data;
}

/*
 *	CLASS ht_file_window
 */
ht_file_window::ht_file_window()
	: vstate_history(true)
{
}

void ht_file_window::init(Bounds *b, const char *desc, uint framestyle, uint number, File *f)
{
	ht_window::init(b, desc, framestyle, number);
	file = f;
	vstate_history_pos = 0;
}

void ht_file_window::done()
{
	ht_window::done();
}

void ht_file_window::add_vstate_history(ht_vstate_history_entry *e)
{
	int c = vstate_history.count();
	if (c > vstate_history_pos) {
		vstate_history.delRange(vstate_history_pos, c);
	}
	vstate_history += e;
	vstate_history_pos++;
}

void ht_file_window::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
	case cmd_vstate_restore:
		if (vstate_history_pos) {
			vstate_history_pos--;
			ht_vstate_history_entry *e = (ht_vstate_history_entry*)
				vstate_history[vstate_history_pos];
			htmsg m;
			m.msg = msg_vstate_restore;
			m.type = mt_empty;
			m.data1.ptr = e->data;
			e->view->sendmsg(&m);
			focus(e->view);
		}
		clearmsg(msg);
		return;
	case msg_vstate_save: {
		Object *data = (Object*)msg->data1.ptr;
		ht_view *view = (ht_view*)msg->data2.ptr;
		add_vstate_history(new ht_vstate_history_entry(data, view));
		break;
	}
	case msg_accept_close: if (file) {
		bool modified = false;
		String fn;
		if (file->getFilename(fn).isEmpty()) {
			modified = true;
		} else {			
			FileOfs start = 0;
			file->cntl(FCNTL_MODS_IS_DIRTY, start, file->getSize(), &modified);
		}
		if (modified) {
			char q[1024];
			if (fn.isEmpty()) {
				ht_snprintf(q, sizeof q, "untitled file has been modified, save?");
			} else {
				ht_snprintf(q, sizeof q, "file %y has been modified, save?", &fn);
			}
			switch (msgbox(btmask_yes+btmask_no+btmask_cancel, "confirmation", 0, align_center, q)) {
			case button_yes: {
				app->focus(this);
				htmsg msg;
				msg.msg = cmd_file_save;
				msg.type = mt_empty;
				app->sendmsg(&msg);
				if ((uint)msg.msg != cmd_file_save) break;
			}
			case button_cancel:
				clearmsg(msg);
				return;
			}
		}
		// flush so that cmd_analyser_save get correct mtime
		file->cntl(FCNTL_FLUSH_STAT);

		Analyser *a;
		htmsg m;
		m.msg = msg_get_analyser;
		m.type = mt_broadcast;
		m.data1.ptr = NULL;
		sendmsg(&m);
		a = (Analyser*)m.data1.ptr;
		if (m.msg == msg_retval && a && a->isDirty()) {
			sendmsg(cmd_analyser_save);
		}
		}
		break;
	case cmd_analyser_save: {
		String filename;
		file->getFilename(filename);
		filename += HT_FILE_CONFIG_SUFFIX;
		LOG("%y: saving config", &filename);
		do {
			String err;
			if (save_fileconfig(filename.contentChar(), ht_fileconfig_magic, ht_fileconfig_fileversion, file_window_store_fcfg_func, this, err) != LS_OK) {
				String e;
				e.assignFormat("Couldn't save '%y': %y\n\nRetry?", &filename, &err);
				switch (msgbox(btmask_yes+btmask_no+btmask_cancel, "confirmation", 0, align_center, e.contentChar())) {
				case button_ok: continue;
				case button_cancel: return;
				}
				LOG("%y: failed", &filename);
			} else {
				LOG("%y: done", &filename);
			}
			break;
		} while (1);
		clearmsg(msg);
		break;
	}
	}
	ht_window::handlemsg(msg);
	switch (msg->msg) {
	case msg_keypressed:
		switch (msg->data1.integer) {
		case K_Meta_Backspace:
		case K_Backspace:
			sendmsg(cmd_vstate_restore);
			clearmsg(msg);
			return;
		}
		break;
	}
}

/**/

List *build_vfs_list()
{
	/* build vfs list */
	List *vfslist = new Array(true);

#if 1
	/* LocalFS */
	LocalFs *localfs = new LocalFs();
	localfs->init();

	/* RegistryFS */
	RegistryFs *registryfs = new RegistryFs();
	registryfs->init();

	vfslist->insert(localfs);
	vfslist->insert(registryfs);
#else
	/* file_vfs */
	ht_file_vfs *file_vfs = new ht_file_vfs();
	file_vfs->init();

	/* reg_vfs */
	ht_reg_vfs *reg_vfs = new ht_reg_vfs();
	reg_vfs->init();

	vfslist->insert(file_vfs);
	vfslist->insert(reg_vfs);
#endif

	/**/
	return vfslist;
}

BUILDER(ATOM_HT_APP, ht_app, ht_dialog);
BUILDER(ATOM_HT_PROJECT, ht_project, AVLTree);
BUILDER(ATOM_HT_PROJECT_ITEM, ht_project_item, Object);

/*
 *	INIT
 */

bool init_app()
{
	Bounds b;
	screen = allocSystemDisplay(ht_name" "ht_version);

	loglines = new ht_log();
	loglines->init();

	virtual_fs_list = build_vfs_list();

	project = NULL;

	b.x = 0;
	b.y = 0;
	b.w = screen->w;
	b.h = screen->h;
	app = new ht_app();
	((ht_app*)app)->init(&b);
	baseview = app;

	app_memory_reserve = malloc(16384); // malloc, not smalloc
	out_of_memory_func = &app_out_of_memory_proc;

	REGISTER(ATOM_HT_APP, ht_app);
	REGISTER(ATOM_HT_PROJECT, ht_project);
	REGISTER(ATOM_HT_PROJECT_ITEM, ht_project_item);

	return true;
}

/*
 *	DONE
 */

void done_app()
{
	UNREGISTER(ATOM_HT_APP, ht_app);
	UNREGISTER(ATOM_HT_PROJECT, ht_project);
	UNREGISTER(ATOM_HT_PROJECT_ITEM, ht_project_item);
	
	out_of_memory_func = &out_of_memory;
	free(app_memory_reserve);

	delete (Object*)project;
	delete virtual_fs_list;

	delete loglines;

	if (app) {
		app->done();
		delete app;
	}
	
	delete screen;
}

