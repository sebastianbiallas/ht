/* 
 *	HT Editor
 *	codeanaly.cc
 *
 * 	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License version 2 as
 * 	published by the Free Software Foundation.
 *
 * 	This program is distributed in the hope that it will be useful,
 * 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 * 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * 	GNU General Public License for more details.
 *
 * 	You should have received a copy of the GNU General Public License
 * 	along with this program; if not, write to the Free Software
 * 	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "analy.h"
#include "analy_register.h"
#include "language.h"
#include "codeanaly.h"
#include "htdebug.h"

void code_analyser::init(analyser *A)
{
	a = A;
	active = false;
	curline = 0;
	signs = NULL;
	error = false;
	signfile = NULL;
}

int  code_analyser::load(ht_object_stream *f)
{
	return 0;
}

void code_analyser::done()
{
}


OBJECT_ID	code_analyser::object_id()
{
	return ATOM_CODE_ANALYSER;
}

void	code_analyser::addsign(sign **Sign, SIGNADDR Signaddr, char *label)
{
	if (*Sign) {
/*		int i = stricmp((*Sign)->label, label); */
		int i=0;
		if (!i) {
			raisewarning(WARN_DUPLICATE_LABEL, label);
		} else if (i==1) {
			addsign(&(*Sign)->left, Signaddr, label);
		} else {
			addsign(&(*Sign)->right, Signaddr, label);
		}
	} else {
		*Sign = (sign *) malloc(sizeof(sign));
		(*Sign)->signaddr = Signaddr;
		(*Sign)->left = (*Sign)->right = NULL;
	}
}

void code_analyser::addsign(SIGNADDR Signaddr, char *label)
{
	addsign(&signs, Signaddr, label);
}

void code_analyser::beginanalysis()
{
	if (!signfile) {
		raisewarning(WARN_NOSIGNFILE);
		return;
	}
	active = true;
}

void	code_analyser::consume(char *str)
{
	nexttoken();
	while (*str++ == *bufptr) nextchar();
	str--;
	if (*str) raiseerror(ERR_MISSING, str);
}

void code_analyser::continueanalysis()
{
	if (active) {
	}
}

void code_analyser::doscan(ADDR Addr, char *label)
{

}

void code_analyser::doscan(ADDR Addr, char *label, SIGNADDR Signaddr)
{
}

bool code_analyser::labelmatch(char *signlabel, char *codelabel)
{
	return true;
}

void code_analyser::loaddefs(char *name)
{
/*     signfile = new htrwfile(name);
	char *buffer = (char *) malloc(2048);
	char *bufferpos = buffer;
	char *bufferend = buffer + signfile->read(buffer, 2048);
	bool eof = (bufferpos == bufferend);

	while (!eof) {
		if (((*bufferpos)=='\n') || (mapchar[*bufferpos]==WHITESPACE)) { SEEKNEXT; continue; }
		if ((*bufferpos)=='#') {
			while (((*bufferpos)!='\n') && !eof) { SEEKNEXT; }
			if (eof) break;
		}

	}

	delete signfile;
	free(buffer);*/
}

char *code_analyser::nextchar()
{
	assert(buffer);
	assert(bufptr);
	assert(bufend);
	bufptr++;
	if (bufptr > bufend) raiseerror("");
	if (mapchar[*bufptr] == INV) raiseerror(ERR_INVCHAR, *bufptr);
	return bufptr;
}

char *code_analyser::nexttoken()
{
	assert(buffer);
	assert(bufptr);
	assert(bufend);
	while (1) {
		if (bufptr > bufend) {
			raiseerror("");
			return 0;
		}
		byte test = mapchar[*bufptr];
		if (test == INV) {
			raiseerror(ERR_INVCHAR, *bufptr);
			return 0;
		}
		if (test == COMMENT) {
			do {
				bufptr++;
				if (bufptr > bufend) {
					raiseerror("");
					return 0;
				}
			} while (mapchar[*bufptr] != NL);
			continue;
		}
		if (test != WHITESPACE) break;
		bufptr++;
	}
	return bufptr;
}

void code_analyser::raiseerror(char *msg, ...)
{
    va_list arg;
    va_start(arg, msg);
    char Msg[200];
    sprintf(Msg, "code_analyser: file `%s' line %d: fatal: %s\n", signfile->get_desc(), curline, msg);
    vsprintf(Msg, Msg, arg);
    a->log(Msg);
    va_end(arg);
    error = true;
}

void	code_analyser::raisewarning(char *msg, ...)
{
    va_list arg;
    va_start(arg, msg);
    char Msg[200];
    sprintf(Msg, "code_analyser: file `%s' line %d: warning: %s\n", signfile->get_desc(), curline, msg);
    vsprintf(Msg, Msg, arg);
    a->log(Msg);
    va_end(arg);
}

void code_analyser::store(ht_object_stream *f)
{
}

