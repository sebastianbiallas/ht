/* 
 *	HT Editor
 *	codeanaly.h
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

#ifndef CODEANALY_H
#define CODEANALY_H

#include "global.h"
#include "common.h"
#include "stream.h"
#include "analy.h"


#define SIGNADDR dword

#define ERR_SYNTAX "syntax error."
#define ERR_MISSING "missing `%s'."
#define ERR_INVCHAR "invalid char `%c'."
#define WARN_DUPLICATE_LABEL "duplicate label `%s'.\nOnly first one will be analysed."
#define WARN_NOSIGNFILE "no sign file."

class analyser;

struct sign {
	SIGNADDR	signaddr;
	dword	length;
	char		*label;
	sign		*left, *right;
};

class code_analyser: public object {
public:
	analyser		*a;
	bool			active;
	sign			*signs;
	ht_stream		*signfile;
	int			curline;
	bool			error;
	char			*buffer, *bufptr, *bufend;

			void      	init(analyser *A);
			int 			load(ht_object_stream *f);
	virtual	void 		done();
	virtual	OBJECT_ID		object_id();

			void			addsign(sign **Sign, SIGNADDR Signaddr, char *label);
			void			addsign(SIGNADDR Signaddr, char *label);
			void			beginanalysis();
			void			consume(char *str);
			void			continueanalysis();
			void			doscan(ADDR Addr, char *label);
			void			doscan(ADDR Addr, char *label, SIGNADDR Signaddr);
			bool			labelmatch(char *signlabel, char *codelabel);
			void			loaddefs(char *name);
			char			*nextchar();
			char 		*nexttoken();
			void			raiseerror(char *msg, ...);
			void			raisewarning(char *msg, ...);
	virtual	void			store(ht_object_stream *f);
};

#endif
