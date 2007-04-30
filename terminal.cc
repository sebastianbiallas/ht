/*
 *	HT Editor
 *	terminal.cc
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
#include <stdlib.h>
#include <string.h>
// #include <sys/select.h>

#include "htctrl.h"
#include "htidle.h"
#include "htiobox.h"
#include "sys.h"
#include "terminal.h"
#include <unistd.h>

/*
 *	CLASS Terminal
 */

Terminal::Terminal(File *_in, File *_out, File *_err, int _sys_ipc_handle)
	: ht_ltextfile(new MemoryFile(), true, NULL)
{
	in = _in;
	out = _out;
	err = _err;
	sys_ipc_handle = _sys_ipc_handle;
}

Terminal::~Terminal()
{
	delete in;
	delete out;
	delete err;
	sys_ipc_terminate(sys_ipc_handle);
}

bool Terminal::append(File *file)
{
#define STREAM_COPYBUF_SIZE	(128)
	const int bufsize=STREAM_COPYBUF_SIZE;
	byte *buf = ht_malloc(bufsize);
	int r, w = 0;
	do {
		r = file->read(buf, bufsize);
		if (r<0) break;
		w += r;
		ht_ltextfile::write(buf, r);
	} while (r == bufsize);
	free(buf);
	return w != 0;
}

bool Terminal::connected()
{
	return sys_ipc_is_valid(sys_ipc_handle);
}

uint Terminal::write(const void *buf, uint size)
{
	if (connected()) {
		uint r = ht_ltextfile::write(buf, size);
		in->write(buf, size);
		return r;
	}
	return 0;		
}

bool Terminal::update()
{
#if 0
	fd_set rfds, wfds;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_SET(out, &wfds);
	FD_SET(err, &wfds);
	FD_SET(in, &rfds);
	// FIXME: FreeBSD problems here
	struct timeval timeout;
	
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	
/*	int n =*/ select(FD_SETSIZE, &rfds, &wfds, NULL, &timeout);
#endif
	bool worked = false;
//	if (FD_ISSET(err, &wfds)/* && (!feof(err))*/) {
		seek(get_size());
		worked |= append(err);
//	}
//	if (FD_ISSET(out, &wfds)/* && (!feof(out))*/) {
		seek(get_size());
		worked |= append(out);
//	}
	return worked;
}

/*
 *	CLASS TerminalViewer
 */

void TerminalViewer::init(Bounds *b, Terminal *t, bool ot)
{
	ht_text_viewer::init(b, ot, t, NULL);
	term = t;
	register_idle_object(this);
}

void TerminalViewer::done()
{
	unregister_idle_object(this);
	ht_text_viewer::done();
}

void TerminalViewer::do_update()
{
	uint l = term->linecount();
	if (l) {
		goto_line(l-1);
		cursor_end();
	}
	dirtyview();
	app->sendmsg(msg_draw, 0);
}

void TerminalViewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_keypressed:
			switch (msg->data1.integer) {
				case K_F8: {
					char buf[128];
					buf[0] = 0;
					if (inputbox("input", "input", buf, sizeof buf) == button_ok) {
						term->seek(term->get_size());
						term->write(buf, strlen(buf));
						do_update();
					}
					clearmsg(msg);
					return;
				}
				case K_Return: {
					term->seek(term->get_size());
					term->write("\n", 1);
					do_update();
					clearmsg(msg);
					return;
				}
// FIXME: unsupported...				
/*				case K_BackSpace: {
					term->seek(term->get_size());
					term->write("a\t", 2);
					term->write("b\v", 2);
					do_update();
					clearmsg(msg);
					return;
				}*/
				default: {
					int ch = msg->data1.integer;
					if ((ch >= 32) && (ch<=0x7f)) {
						char buf[2];
						buf[0] = ch;
						buf[1] = 0;
						term->seek(term->get_size());
						term->write(buf, 1);
						do_update();
						clearmsg(msg);
						return;
					}						
					break;
				}
			}
			break;
	}
	return ht_text_viewer::handlemsg(msg);
}

bool TerminalViewer::idle()
{
	if (term->update()) {
		do_update();
		return true;
	}
	return false;
}

void TerminalViewer::get_pindicator_str(char *buf)
{
	sprintf(buf, term->connected() ? " connected " : " disconnected ");
}

#endif
