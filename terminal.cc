/*
 *	HT Editor
 *	terminal.cc
 *
 *	Copyright (C) 2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#include <stdlib.h>
#include <string.h>

#include "htctrl.h"
#include "htidle.h"
#include "terminal.h"

/**/
#ifdef DJGPP
#include <unistd.h>
int sys_ipc_exec(FILE **in, FILE **out, FILE **err, const char *cmd)
{
	int save_stdout = dup(STDOUT_FILENO);
	int save_stderr = dup(STDERR_FILENO);
     *in = NULL;
     *out = tmpfile();
     *err = tmpfile();
	dup2(fileno(*out), STDOUT_FILENO);
	dup2(fileno(*err), STDERR_FILENO);
	int r = system(cmd);
	dup2(save_stdout, STDOUT_FILENO);
	dup2(save_stderr, STDERR_FILENO);
	close(save_stdout);
	close(save_stderr);
     fseek(*out, 0, SEEK_SET);
     fseek(*err, 0, SEEK_SET);
     return r;
}
#else
int sys_ipc_exec(FILE **in, FILE **out, FILE **err, const char *cmd)
{
	return 1;
}
#endif
/**/

/*
 *	CLASS Terminal
 */

void append(FILE *file, ht_stream *stream)
{
#define STREAM_COPYBUF_SIZE	(1024)
	const UINT bufsize=STREAM_COPYBUF_SIZE;
	byte *buf=(byte*)malloc(bufsize);
	UINT r;
	do {
		r = fread(buf, 1, bufsize, file);
		stream->write(buf, r);
	} while (r == bufsize);
	free(buf);
}

void Terminal::init(FILE *_in, FILE *_out, FILE *_err)
{
	ht_mem_file *m = new ht_mem_file();
     m->init();
	ht_ltextfile::init(m, true, NULL);
     in = _in;
     out = _out;
     err = _err;
}

void Terminal::done()
{
	fclose(in);
	fclose(out);
	fclose(err);
     ht_ltextfile::done();
}

bool Terminal::update()
{
     fd_set rfds, wfds;
     FD_ZERO(&rfds);
     FD_ZERO(&wfds);
	FD_SET(fileno(out), &wfds);
	FD_SET(fileno(err), &wfds);
	FD_SET(fileno(in), &rfds);
	struct timeval timeout;
     
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
     
     int n = select(FD_SETSIZE, &rfds, &wfds, NULL, &timeout);
     bool worked = false;
     if (FD_ISSET(fileno(out), &wfds) && (!feof(out))) {
		seek(get_size());
          append(out, this);
          worked = true;
	}
     if (FD_ISSET(fileno(err), &wfds) && (!feof(err))) {
		seek(get_size());
          append(err, this);
          worked = true;
     }
	return worked;
}

/*
 *	CLASS TerminalViewer
 */

void TerminalViewer::init(bounds *b, Terminal *t, bool ot)
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

void TerminalViewer::handlemsg(htmsg *msg)
{
	switch (msg->msg) {
		case msg_keypressed:
			switch (msg->data1.integer) {
               	case K_F8: {
                    	term->seek(term->get_size());
                    	term->write("blabla\x08", 7);
                    	dirtyview();
                    	clearmsg(msg);
                         return;
                    }
               	case K_F9: {
                    	term->seek(term->get_size());
                    	term->write("\n", 1);
                    	dirtyview();
                    	clearmsg(msg);
                         return;
                    }
               }
	}
	return ht_text_viewer::handlemsg(msg);
}

bool TerminalViewer::idle()
{
	if (term->update()) {
     	dirtyview();
		app->sendmsg(msg_draw, 0);
          return true;
     }
	return false;
}

