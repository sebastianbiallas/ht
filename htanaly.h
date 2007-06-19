/*
 *	HT Editor
 *	htanaly.h
 *
 *	Copyright (C) 1999-2002 Sebastian Biallas (sb@biallas.net)
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
#include "io/types.h"
#include "htdialog.h"
#include "htformat.h"
#include "httree.h"
#include "out.h"

/*
 *	Commands
 */

#define cmd_analyser_call_assembler     HT_COMMAND(501)
#define cmd_analyser_this_function      HT_COMMAND(502)
#define cmd_analyser_previous_label	HT_COMMAND(503)
#define cmd_analyser_continue           HT_COMMAND(504)
#define cmd_analyser_comments           HT_COMMAND(505)
#define cmd_analyser_name_addr          HT_COMMAND(506)
#define cmd_analyser_xrefs              HT_COMMAND(507)
#define cmd_analyser_follow             HT_COMMAND(508)
#define cmd_analyser_follow_ex          HT_COMMAND(509)
#define cmd_analyser_pause_resume       HT_COMMAND(510)
#define cmd_analyser_del_addr_bindings  HT_COMMAND(511)
#define cmd_analyser_call_chain         HT_COMMAND(512)
#define cmd_analyser_generate_output    HT_COMMAND(513)
#define cmd_analyser_data_string        HT_COMMAND(514)
#define cmd_analyser_info               HT_COMMAND(515)
#define cmd_analyser_symbols            HT_COMMAND(516)
#define cmd_analyser_export_file	HT_COMMAND(517)
#define cmd_analyser_data_int        	HT_COMMAND(518)
#define cmd_analyser_data_half		HT_COMMAND(519)
#define cmd_analyser_data_byte        	HT_COMMAND(520)

/* FIXME: srt-experimental */

#define cmd_analyser_srt				HT_COMMAND(550)

class ht_aviewer;

/*
 *
 */

class AnalyserInformation: public ht_statictext {
	ht_aviewer	*analy;     
	int		addrs, labels;
	const char	*atype, *adis;
	String		aname;
public:
		void init(Bounds *b, ht_aviewer *a);
	virtual	void done();
	virtual	int  gettext(char *text, int maxlen);
	virtual	bool idle();
};

/*
 *
 */
class SymbolBox: public ht_listbox {
public:
	Analyser	*analy;
	char		*str;
	int		idle_count;
	int		symbols;

		void	init(Bounds *b, Analyser *Analy);
	virtual void	done();
	virtual int	calcCount();
	virtual int	cursorAdjust();
	virtual	int	estimateEntryPos(void *entry);
	virtual void *	getFirst();
	virtual void *	getLast();
	virtual void *	getNext(void *entry);
	virtual void *	getPrev(void *entry);
	virtual const char *getStr(int col, void *entry);
	virtual	bool	idle();
	virtual int	numColumns();
	virtual	void *	quickfind(const char *s);
	virtual	char *	quickfindCompletition(const char *s);
};

struct CallChainNode {
	CallChainNode *next, *prev, *child;
	bool examined;
	Address *xa;
	Address *fa;
	Location *faddr;
	bool expanded;
};

class CallChain: public ht_treeview {
		Analyser		*analy;
		CallChainNode *root;
public:
			   void	init(Bounds *b, Analyser *analy, Address *a, char *desc);
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
	CallChainNode		*createNode(Address *a);
			void		examineNode(CallChainNode *n);
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

class AnalyInfoline: public ht_statictext {
public:
	ht_aviewer	*analy;
	char		*s;
	FileOfs		fofs;
	Address		*addr;
	char		*displayformat;
		void	init(Bounds *b, ht_aviewer *A, const char *Format);
	virtual	void	done();
	virtual	int	gettext(char *text, int maxlen);
		void	update(Address *cursor_addr, FileOfs ecursor_addr);
		bool	valid();
};

/*
 *	CLASS ht_analy_sub
 */

class ht_aviewer;
class ht_analy_sub: public ht_sub {
public:
	Analyser	*analy;
	Address        *lowestaddress, *highestaddress;
	AnalyserOutput *output;
	ht_aviewer	*aviewer;
	
		void	init(File *file, ht_aviewer *A, Analyser *analyser, Address *Lowestaddress, Address *Highestaddress);
	virtual	void	done();
	virtual	bool	convert_ofs_to_id(const FileOfs offset, LINE_ID *line_id);
	virtual	bool	closest_line_id(LINE_ID *line_id);
	virtual	void	first_line_id(LINE_ID *line_id);
	virtual	bool	getline(char *line, int maxlen, const LINE_ID line_id);
	virtual	void	last_line_id(LINE_ID *line_id);
	virtual	int	next_line_id(LINE_ID *line_id, int n);
	virtual	int	prev_line_id(LINE_ID *line_id, int n);
			void	setAnalyser(Analyser *Analy);
	virtual	ht_search_result *search(ht_search_request *search, FileOfs start, FileOfs end);
};

/*
 *	CLASS ht_aviewer
 */

class ht_aviewer: public ht_uformat_viewer {
public:
	int idle_count;
	Analyser *analy;
	int last_active;
	AnalyInfoline *infoline;
	ht_analy_sub *analy_sub;
	bool one_load_hack;
	bool pause;
		void init(Bounds *b, const char *desc, int caps, File *file, ht_format_group *format_group, Analyser *Analy);
	virtual	void done();
		bool convertAddressToViewerPos(Address *a, viewer_pos *p);
		bool convertViewerPosToAddress(const viewer_pos &p, Address **a);
		void attachInfoline(AnalyInfoline *V);
		bool canCreateAddress(Address *addr, bool error_msg);
		void dataStringDialog();
		void dataIntDialog(taddr_int_subtype subtype, int length);
		void exportFileDialog();
	virtual	const char *func(uint i, bool execute);
		void generateOutputDialog();
		bool getCurrentAddress(Address **a);
	virtual	bool get_current_offset(FileOfs *ofs);
	virtual	int  get_pindicator_str(char *buf, int max_len);
	virtual	bool get_hscrollbar_pos(int *pstart, int *psize);
	virtual	void getminbounds(int *width, int *height);
		bool gotoAddress(Address *a, ht_view *source_object);
	virtual	void handlemsg(htmsg *msg);
	virtual	bool idle();
	virtual	bool offset_to_pos(FileOfs ofs, viewer_pos *p);
	virtual	bool pos_to_offset(viewer_pos p, FileOfs *ofs);
		bool pos_to_string(viewer_pos p, char *result, int maxlen);
	virtual	bool ref_sel(LINE_ID *id);
	virtual	void reloadpalette();
	virtual	void setAnalyser(Analyser *a) = 0;
		void showCallChain(Address *addr);
		void showComments(Address *addr);
		void showInfo(Address *addr);
		void showSymbols(Address *addr);
		void showXRefs(Address *addr);
		void searchForXRefs(Address *addr);
	virtual	bool qword_to_pos(uint64 q, viewer_pos *p);
	virtual	bool func_handler(eval_scalar *result, char *name, eval_scalarlist *params);
	virtual	bool symbol_handler(eval_scalar *result, char *name);
};

#endif
