/*
 *	HT Editor
 *	terminal.h
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

#if 0
#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include "textedit.h"

/*
 *	CLASS Terminal
 */

class Terminal: public ht_ltextfile {
private:
	File *in, *out, *err;
	int sys_ipc_handle;
// FIXME: we need line buffering using String2	

			bool append(File *file);
			
public:
		Terminal(File *in, File *out, File *err, int sys_ipc_handle);
	virtual	~Terminal();
/* oerwritten */	
	virtual	uint write(const void *buf, uint size);
/* new */
			bool connected();
			bool update();
};

/*
 *	CLASS TerminalViewer
 */

class TerminalViewer: public ht_text_viewer {
private:
	Terminal	*term;
	
			void do_update();
			int get_pindicator_str(char *buf, int max_len);
public:
			void init(Bounds *b, Terminal *term, bool own_term);
	virtual	void done();
/* overwritten */
	virtual	void handlemsg(htmsg *msg);
	virtual	bool idle();
};

#endif /* __TERMINAL_H__ */

#endif
