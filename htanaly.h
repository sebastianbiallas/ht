/*
 *	HT Editor
 *	htanaly.h
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@web-productions.de)
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

#ifndef HTANALY_H
#define HTANALY_H

#include "analy.h"
#include "cmds.h"
#include "global.h"
#include "htdialog.h"
#include "htformat.h"
#include "httree.h"
#include "out_ht.h"

/*
 *	Commands
 */

#define cmd_analyser_call_assembler	HT_COMMAND(501)
#define cmd_analyser_this_function	HT_COMMAND(502)
#define cmd_analyser_previous_label	HT_COMMAND(503)
#define cmd_analyser_continue		HT_COMMAND(504)
#define cmd_analyser_comments		HT_COMMAND(505)
#define cmd_analyser_name_addr		HT_COMMAND(506)
#define cmd_analyser_xrefs		HT_COMMAND(507)
#define cmd_analyser_follow		HT_COMMAND(508)
#define cmd_analyser_follow_ex		HT_COMMAND(509)
#define cmd_analyser_pause_resume	HT_COMMAND(510)
#define cmd_analyser_del_addr_bindings	HT_COMMAND(511)
#define cmd_analyser_call_chain		HT_COMMAND(512)
#define cmd_analyser_generate_output	HT_COMMAND(513)
#define cmd_analyser_data_string	HT_COMMAND(514)
#define cmd_analyser_info		HT_COMMAND(515)

/* FIXME: srt-experimental */
#define cmd_analyser_srt		HT_COMMAND(550)

class ht_aviewer;

/*
 *
 */

class analyser_information: public ht_statictext {
	ht_aviewer	*analy;     
	char			buf[1024];
	int			addrs, labels;
	char			*aname, *atype, *adis;
public:
			void	init(bounds *b, ht_aviewer *a);
	virtual	void done();
	virtual	char *gettext();
	virtual	bool idle();
};

/*
 *
 */
class symbolbox: public ht_listbox {
public:
	analyser	*analy;
	char		*str;
	int		idle_count;
	int		symbols;

			void init(bounds *b, analyser *Analy);
	virtual   void done();
	virtual   int  calc_count();
	virtual   int  cursor_adjust();
	virtual	int  estimate_entry_pos(void *entry);
	virtual   void *getfirst();
	virtual   void *getlast();
	virtual   void *getnext(void *entry);
	virtual   void *getnth(int n);
	virtual   void *getprev(void *entry);
	virtual   char *getstr(int col, void *entry);
	virtual	bool idle();
	virtual   int	num_cols();
	virtual	void *quickfind(char *s);
	virtual	char	*quickfind_completition(char *s);
};

struct call_chain_node {
	call_chain_node *next, *prev, *child;
	bool examined;
	ADDR xa;
	ADDR fa;
	taddr *faddr;
	bool expanded;
};

class call_chain: public ht_treeview {
		analyser		*analy;
		call_chain_node *root;
public:
			   void	init(bounds *b, analyser *Analy, ADDR a, char *desc);
		virtual void	done();
		virtual void	adjust(void *node, bool expand);
		virtual void   *get_child(void *node, int i);
		virtual void	*get_next_node(void *node);
		virtual void	*get_prev_node(void *node);
		virtual void	*get_root();
		virtual char	*get_text(void *node);
		virtual bool	has_children(void *node);
		virtual bool	is_expanded(void *node);
		virtual void	select_node(void *node);
private:
	call_chain_node	*create_node(ADDR A);
			void		examine_node(call_chain_node *n);
};
/*
 *
 */
#define ANALY_STATUS_DEFAULT "<%s> @%O  %u\n%f"
#define ANALY_STATUS_ARG_SECTION 's'
#define ANALY_STATUS_ARG_FILEOFFSET 'O'
#define ANALY_STATUS_ARG_RAW_UNASM 'u'
#define ANALY_STATUS_ARG_FUNCTION 'f'
#define ANALY_STATUS_ARG_OFFSET 'o'

class analy_infoline: public ht_statictext {
public:
	ht_aviewer	*analy;
	char			*s;
	dword		fofs;
	ADDR			addr;
	char			*displayformat;
			void	init(bounds *b, ht_aviewer *A, char *Format);
	virtual	void done();
	virtual	char *gettext();
			void update(ADDR cursor_addr, ADDR ecursor_addr);
			bool valid();
};

/*
 *	CLASS ht_analy_sub
 */

class ht_analy_sub: public ht_sub {
public:
	analyser		*analy;
	ADDR			lowestaddress, highestaddress;
	analyser_ht_output *output;
	
			void init(ht_streamfile *file, analyser *A, ADDR Lowestaddress, ADDR Highestaddress);
	virtual	void	done();
	virtual	bool convert_addr_to_id(fmt_vaddress addr, ID *id1, ID *id2);
	virtual	bool convert_id_to_addr(ID id1, ID id2, fmt_vaddress *addr);
	virtual	bool convert_ofs_to_id(FILEOFS offset, ID *id1, ID *id2);
	virtual	bool closest_line_id(ID *id1, ID *id2);
	virtual	void	first_line_id(ID *id1, ID *id2);
	virtual	bool	getline(char *line, ID id1, ID id2);
	virtual	void	last_line_id(ID *id1, ID *id2);
	virtual	int	next_line_id(ID *id1, ID *id2, int n);
	virtual	int	prev_line_id(ID *id1, ID *id2, int n);
			void	set_analyser(analyser *Analy);
	virtual	ht_search_result *search(ht_search_request *search, FILEOFS start, FILEOFS end);
};

/*
 *	CLASS ht_aviewer
 */

class ht_aviewer: public ht_uformat_viewer {
public:
	int idle_count;
	analyser *analy;
	int last_active;
	analy_infoline *infoline;
	ht_analy_sub *analy_sub;
	bool one_load_hack;
	bool pause;
			void init(bounds *b, char *desc, int caps, ht_streamfile *file, ht_format_group *format_group, analyser *Analy);
	virtual	void	done();
	virtual	bool address_to_offset(fmt_vaddress addr, FILEOFS *ofs);
			bool address_to_string(char *result, fmt_vaddress vaddr);
			void	attach_infoline(analy_infoline *V);
			bool can_create_address(ADDR addr, bool error_msg);
			void data_string_dialog();
	virtual	char *func(UINT i, bool execute);
			void generate_output_dialog();
	virtual	bool get_current_offset(FILEOFS *ofs);
	virtual	void get_pindicator_str(char *buf);
	virtual	bool get_hscrollbar_pos(int *pstart, int *psize);
	virtual	void	handlemsg(htmsg *msg);
	virtual	bool	idle();
	virtual	bool offset_to_address(FILEOFS ofs, fmt_vaddress *addr);
	virtual	int	ref_sel(ID id_low, ID id_high);
	virtual	void reloadpalette();
	virtual	void set_analyser(analyser *a) = 0;
			void show_call_chain(ADDR Addr);
			void show_comments(ADDR Addr);
			void show_info(ADDR Addr);
			void show_xrefs(ADDR Addr);
			void search_for_xrefs(ADDR Addr);
	virtual	bool string_to_address(char *string, fmt_vaddress *vaddr);
};

#endif
