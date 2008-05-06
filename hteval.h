/* 
 *	HT Editor
 *	hteval.h
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

#ifndef __HTEVAL_H__
#define __HTEVAL_H__

void eval_dialog(eval_func_handler func_handler, eval_symbol_handler symbol_handler, void *context);
void dialog_eval_help(eval_func_handler func_handler, eval_symbol_handler symbol_handler, void *context);

#endif /* __HTEVAL_H__ */
