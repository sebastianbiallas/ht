/*
 *	HT Editor
 *	htobj.h
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

#ifndef __HTOBJ_H__
#define __HTOBJ_H__

class ht_view;
class ht_group;

#include "io/types.h"
#include "io/display.h"
#include "data.h"

struct palette {
	uint size;
	vcp *data;
};

/* messages (like in "MESS of AGES") */
#define msg_message			0x00000000
#define HT_MESSAGE(m)			(msg_message+(m))

#define msg_empty			HT_MESSAGE(0)
#define msg_retval			HT_MESSAGE(1)
#define msg_draw 			HT_MESSAGE(2)
#define msg_keypressed 			HT_MESSAGE(3)
#define msg_kill			HT_MESSAGE(4)
#define msg_complete_init		HT_MESSAGE(5)
#define msg_funcexec			HT_MESSAGE(6)
#define msg_funcquery			HT_MESSAGE(7)
#define msg_menucapquery		HT_MESSAGE(8)
#define msg_menuquery			HT_MESSAGE(9)
#define msg_button_pressed		HT_MESSAGE(10)
#define msg_dirtyview			HT_MESSAGE(11)
#define msg_config_changed		HT_MESSAGE(12)
#define msg_accept_close		HT_MESSAGE(13)
#define msg_file_changed		HT_MESSAGE(14)
#define msg_get_scrollinfo		HT_MESSAGE(15)
#define msg_get_pindicator		HT_MESSAGE(16)
#define msg_get_analyser		HT_MESSAGE(17)
#define msg_set_analyser		HT_MESSAGE(18) // (Analyser *)
#define msg_postinit			HT_MESSAGE(19)
#define msg_contextmenuquery		HT_MESSAGE(20)
#define msg_project_changed		HT_MESSAGE(21)
#define msg_vstate_save			HT_MESSAGE(22) // (Object *data, ht_view *)
#define msg_vstate_restore		HT_MESSAGE(23) // (Object *data)
#define msg_goto_offset			HT_MESSAGE(24) // (FileOfs ofs)

#define msg_filesize_changed		HT_MESSAGE(100)
#define msg_log_changed			HT_MESSAGE(101)

#define gsi_hscrollbar			1
#define gsi_vscrollbar			2

struct gsi_scrollbar_t {
	int pstart;
	int psize;
};

/* message types */
#define mt_empty		0
#define mt_broadcast		1
#define mt_preprocess		2
#define mt_postprocess		3

/*
 *	CLASS ht_view
 */

/* options */
#define VO_OWNBUFFER		1
#define VO_BROWSABLE		2
#define VO_SELECTABLE		4
#define VO_SELBOUND		8
#define VO_PREPROCESS		16
#define VO_POSTPROCESS		32
#define VO_MOVE			64
#define VO_RESIZE		128
#define VO_FORMAT_VIEW		256
#define VO_TRANSPARENT_CHARS	512

/* grow modes */

#define VIEW_DEBUG_NAME(name)	ht_view::view_debug_name=name;

#define GMV_TOP		0
#define GMV_BOTTOM	1
#define GMV_FIT		2

#define GMH_LEFT	0
#define GMH_RIGHT	1
#define GMH_FIT		2

#define GET_GM_H(gm)	((gm)>>16)
#define GET_GM_V(gm)	((gm)&0xffff)

#define MK_GM(gmh, gmv)	((gmv) | ((gmh)<<16))

void clearmsg(htmsg *msg);

class ht_view: public Object {
protected:
		bool view_is_dirty;
			
		void cleanview();
	virtual	const char *defaultpalette();
	virtual	const char *defaultpaletteclass();
	virtual	void reloadpalette();
public:
	bool focused;
	bool enabled;
	ht_group *group;
	int options;
	char *desc;
	int browse_idx;
	Display *buf;
	ht_view *prev, *next;

	Bounds size;
	Bounds vsize;	/* visual Bounds */
	uint growmode;
	uint g_hdist, g_vdist;

	palette pal;
	const char *pal_class;
	const char *pal_name;

/*debug:*/const char *view_debug_name;

				ht_view() {}
				ht_view(BuildCtorArg&a): Object(a) {};

		void		init(Bounds *b, int options, const char *desc);
	virtual	void		done();
/* new */
	virtual	int		aclone();
/*
		int		buf_lprint(int x, int y, int c, int l, const char *text, Codepage cp = CP_DEVICE);
		int		buf_lprintw(int x, int y, int c, int l, const AbstractChar *text, Codepage cp = CP_DEVICE);
		int		buf_print(int x, int y, int c, const char *text, Codepage cp = CP_DEVICE);
		void		buf_printchar(int x, int y, int c, int ch, Codepage cp = CP_DEVICE);
		int		buf_printf(int x, int y, int c, Codepage cp, const char *format, ...);
		int		buf_printw(int x, int y, int c, const AbstractChar *text, Codepage cp = CP_DEVICE);
*/
	virtual	int		childcount() const;
		void		clear(int color);
	virtual	void		clipbounds(Bounds *b);
	virtual	void		config_changed();
	virtual	int		countselectables();
		void		databuf_free(void *handle);
		void		*databuf_get(void *buf, int bufsize);
		void		databuf_set(void *buf, int bufsize);
	virtual	int		datasize();
		void		dirtyview();
	virtual	void		disable();
		void		disable_buffering();
	virtual void		draw();
	virtual	void		enable();
		void		enable_buffering();
	virtual	int		enum_start();
	virtual	ht_view 	*enum_next(int *handle);
		bool		exposed();
		void		fill(int x, int y, int w, int h, int c, char chr, Codepage cp = CP_DEVICE);
	virtual	bool		focus(ht_view *view);
		void		getbounds(Bounds *b);
	virtual	void		getminbounds(int *width, int *height);
		vcp		getcolor(uint index);
	virtual	void		getdata(ObjectStream &s);
	virtual ht_view 	*getfirstchild();
	virtual	uint		getnumber();
		const char	*getpalette();
	virtual	ht_view		*getselected();
	virtual void		handlemsg(htmsg *msg);
		void		hidecursor();
		int		isviewdirty();
	virtual	int		isaclone(ht_view *view);
	virtual	void		load(ObjectStream &s);
	virtual	void		move(int rx, int ry);
	virtual	ObjectID	getObjectID() const;
		bool		pointvisible(int x, int y);
	virtual	void		receivefocus();
	virtual	void		redraw();
		void		relocate_to(ht_view *view);
	virtual	void		resize(int rw, int rh);
	virtual void		releasefocus();
	virtual	int		select(ht_view *view);
	virtual	void		selectfirst();
	virtual	void		selectlast();
		void		sendmsg(htmsg *msg);
		void		sendmsg(int msg, int data1=0, int data2=0);
		void		sendmsg(int msg, void *data1, void *data2=0);
		void		setbounds(Bounds *b);
		void		setvisualbounds(Bounds *b);
		void		setcursor(int x, int y, CursorMode c=CURSOR_NORMAL);
	virtual	void		setdata(ObjectStream &s);
	virtual	void		setgroup(ht_group *group);
	virtual	void		setnumber(uint number);
		void		setoptions(int options);
	virtual	void		setpalette(const char *pal_name);
		void		setpalettefull(const char *pal_name, const char *pal_class);
	virtual	void		store(ObjectStream &s) const;
		void 		unrelocate_to(ht_view *view);
};

/*
 *	Easier use of data buffers:
 */

class ViewDataBuf: public Object {
	void *mBuf;
	ht_view *mView;
public:
	ViewDataBuf(ht_view *view, void *buf, int bufsize)
		: mBuf(view->databuf_get(buf, bufsize)), mView(view)
	{
	}

	virtual ~ViewDataBuf()
	{
		mView->databuf_free(mBuf);
	}
};

class ht_dialog_widget: public ht_view {
public:
	void getminbounds(int *width, int *height);
};

/*
 *	CLASS ht_group
 */

class ht_group: public ht_view {
protected:
	int 		view_count;

public:
	ht_view 	*first, *current, *last;
	void		*shared_data;

		ht_group() {}
		ht_group(BuildCtorArg&a): ht_view(a) {};

		void init(Bounds *b, int options, const char *desc);
	virtual	void done();
/* overwritten */
	virtual	int childcount() const;
	virtual	int countselectables();
	virtual	int datasize();
	virtual	int enum_start();
	virtual	ht_view *enum_next(int *handle);
	virtual	bool focus(ht_view *view);
	virtual	void getdata(ObjectStream &s);
	virtual ht_view *getfirstchild();
	virtual	void getminbounds(int *width, int *height);
	virtual	ht_view *getselected();
	virtual	void handlemsg(htmsg *msg);
	virtual	int isaclone(ht_view *view);
		int isviewdirty();
	virtual	void load(ObjectStream &s);
	virtual	void move(int x, int y);
	virtual	ObjectID getObjectID() const;
		void putontop(ht_view *view);
	virtual void receivefocus();
	virtual	void resize(int rw, int rh);
	virtual void releasefocus();
	virtual	int select(ht_view *view);
	virtual	void selectfirst();
	virtual	void selectlast();
	virtual	void setdata(ObjectStream &s);
	virtual	void setpalette(const char *pal_name);
	virtual	void store(ObjectStream &s) const;
	/* new */
	virtual	void reorder_view(ht_view *v, int rx, int ry);
		void remove(ht_view *view);
	virtual	void insert(ht_view *view);
		bool focusnext();
		bool focusprev();
		ht_view *get_by_browse_idx(int i);
};

/*
 *	CLASS ht_xgroup
 */

class ht_xgroup: public ht_group {
public:
		ht_xgroup() {}
		ht_xgroup(BuildCtorArg&a): ht_group(a) {};

		void		init(Bounds *b, int options, const char *desc);
	virtual	void		done();
	/* overwritten */
	virtual	int		countselectables();
	virtual	void		handlemsg(htmsg *msg);
	virtual	int		isaclone(ht_view *view);
	virtual	void		load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void		redraw();
	virtual	void		selectfirst();
	virtual	void		selectlast();
	virtual	void		store(ObjectStream &s) const;
};

/*
 *	CLASS ht_scrollbar
 */

class ht_scrollbar: public ht_view {
protected:
	int pstart, psize;
	palette *gpal;
	bool isvertical;
public:
		ht_scrollbar() {}
		ht_scrollbar(BuildCtorArg&a): ht_view(a) {};

		void init(Bounds *b, palette *gpal, bool isvertical);
	virtual	void done();
	/* overwritten */
	virtual	void enable();
	virtual	void disable();
	virtual void draw();
	virtual	ObjectID getObjectID() const;
	virtual void getminbounds(int *width, int *height);
	/* new */
	virtual	void setpos(int pstart, int psize);
};

/*
 *	CLASS ht_text
 */

class ht_text: public ht_dialog_widget {
public:
/* new */
	virtual	void settext(const char *text);
};

/*
 *	CLASS ht_frame
 */

#define FS_KILLER		1
#define FS_TITLE 		2
#define FS_NUMBER		4
#define FS_RESIZE		8
#define FS_MOVE			16
#define FS_THICK		32

#define FST_FOCUSED		0
#define FST_UNFOCUSED		1
#define FST_MOVE      		2
#define FST_RESIZE      	3

class ht_frame: public ht_text {
protected:
	uint number;
	uint style;
	uint framestate;

	/* new */
	virtual	vcp getcurcol_normal();
	virtual	vcp getcurcol_killer();
public:
		void		init(Bounds *b, const char *desc, uint style, uint number=0);
	virtual	void		done();
	/* overwritten */
	virtual	void		draw();
	virtual	uint		getnumber();
	virtual	ObjectID	getObjectID() const;
	virtual	void		setnumber(uint number);
	virtual	void		settext(const char *text);
	/* new */
		uint		getstyle();
		void		setframestate(uint framestate);
		void		setstyle(uint style);
};

/*
 *	CLASS ht_window
 */

#define WAC_NORMAL	0
#define WAC_MOVE	1
#define WAC_RESIZE	2

class ht_window: public ht_group {
protected:
	ht_frame *frame;
	ht_scrollbar *hscrollbar;
	ht_scrollbar *vscrollbar;
	ht_text *pindicator;
	uint number;

	int action_state;
	
		bool next_action_state();
public:
		ht_window() {}
		ht_window(BuildCtorArg&a): ht_group(a) {};

		void init(Bounds *b, const char *desc, uint framestyle, uint number=0);
	virtual	void done();
	/* overwritten */
	virtual	void draw();
	virtual	uint getnumber();
	virtual	void handlemsg(htmsg *msg);
	virtual	void insert(ht_view *view);
	virtual	void load(ObjectStream &s);
	virtual	ObjectID	getObjectID() const;
	virtual	void receivefocus();
	virtual	void releasefocus();
	virtual	void redraw();
	virtual	void setnumber(uint number);
	virtual	void store(ObjectStream &s) const;
	/* new */
		void getclientarea(Bounds *b);
		ht_frame *getframe();
		void setframe(ht_frame *frame);
		void sethscrollbar(ht_scrollbar *scrollbar);
		void setpindicator(ht_text *pindicator);
		void settitle(char *title);
		void setvscrollbar(ht_scrollbar *scrollbar);
};

bool scrollbar_pos(sint64 start, sint64 size, sint64 all, int *pstart, int *psize);

/*
 *	CLASS ht_hbar
 */

class ht_hbar: public ht_view {
public:
	/* overwritten */
	virtual	 void draw();
};

/*
 *	CLASS ht_vbar
 */

class ht_vbar: public ht_view {
public:
	/* overwritten */
	virtual	 void draw();
};

/*
 *	INIT
 */

bool init_obj();

/*
 *	DONE
 */

void done_obj();

#endif /* !__HTOBJ_H__ */
