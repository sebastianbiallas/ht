/* 
 *	HT Editor
 *	analy.h
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

#ifndef analy_h
#define analy_h

#include "asm.h"
#include "global.h"
#include "common.h"
#include "codeanaly.h"
#include "dataanaly.h"
#include "htdata.h"
#include "stddata.h"

extern int num_ops_parsed;

//#define ANALY_TIMINGS

class analyser;

/*
 *	these are the different possibilities of a branch
 *	to support further processors other types can be added
 */
typedef enum {
			brnobranch,						// straight exec. flow
			brjump,
			brreturn,
			brcall,
			brjXX
} tbranchtype;

/*
 *   internal opcodes are interchanged in this format
 */
#define OPCODE dis_insn

/*
 *
 */
class analy_disassembler: public object {
public:
	analyser			*analy;
	disassembler        *disasm;
			void			init(analyser *A);
			int 			load(ht_object_stream *f);
	virtual	void			done();

	virtual	ADDR			branch_addr(OPCODE *opcode, tbranchtype branchtype, bool examine) = 0;
	virtual	void			examine_opcode(OPCODE *opcode) = 0;
	virtual	void			init_disasm();
	virtual	tbranchtype 	is_branch(OPCODE *opcode) = 0;
	virtual	void			store(ht_object_stream *f);
};

/***************************************************************************/

class ht_addr: public ht_data {
public:
	ADDR addr;
     					ht_addr();
					     ht_addr(ADDR Addr);
			int 			load(ht_object_stream *f);
	virtual	OBJECT_ID		object_id();
	virtual	void			store(ht_object_stream *f);
};

typedef enum {xrefread, xrefwrite, xrefoffset, xrefjump, xrefcall, xrefijump, xreficall} xref_type_t;

class addr_xref: public ht_data {
public:
	xref_type_t	type;
                              addr_xref();
						addr_xref(xref_type_t Type);
			int 			load(ht_object_stream *f);
	virtual	OBJECT_ID		object_id();
	virtual	void			store(ht_object_stream *f);
};

/*
struct txref {
	txref		*next;
	ADDR			addr;
	xref_type_t	type;
};
*/

class comment_list: public ht_clist {
public:
	void init();
	void append_pre_comment(char *s);
	void append_pre_comment(int special);
	void append_post_comment(char *s);
	void append_post_comment(int special);
	char *get_name(UINT i);
};

struct tlabel;

struct taddr {
	// the address
	ADDR			addr;
	// this is a tree structure (key is addr)
	taddr		*left, *right;
	// attached label
	tlabel		*label;
	// attached xrefs
	ht_tree		*xrefs;
	// attached comments
	comment_list	*comments;
	// for data types
	taddr_type	type;
	// the function the address belongs to (if applicable)
	taddr		*thisfunc;
	// some flags
	int			flags;
};

/*
 * taddr.flags:
 */
#define AF_DELETED 1
#define AF_FUNCTION_SET 2
#define AF_FUNCTION_END 4

typedef enum {scvalid, scread, scwrite,	screadwrite, sccode, scinitialized} tsectype;

typedef enum {	acread, acwrite, acoffset } taccesstype;

struct taccess	{
	bool			indexed;
	int			size;
	taccesstype 	type;
};

typedef enum { label_unknown=0, label_func, label_loc, label_data } labeltype;

struct tlabel {
	labeltype	type;
	taddr	*addr;
	char		*name;
	tlabel	*left, *right;
};

class addrqueueitem: public ht_data {
public:
	ADDR		addr;
	ADDR		func;
			addrqueueitem();
			addrqueueitem(ADDR Addr, ADDR Func);
			OBJECT_ID	object_id();
			int	load(ht_object_stream *f);
	virtual	void	store(ht_object_stream *f);
};

class code_analyser;
class data_analyser;


class analyser: public object	{
public:
	ADDR			addr;
	ht_queue		*addr_queue;
	int			ops_parsed;							// for continuing
	bool			active;
	ADDR			next_explored,	first_explored, last_explored;
	area			*explored;
	area			*initialized;
	taddr		*addrs;
	code_analyser	*code;
	data_analyser	*data;
	analy_disassembler	*analy_disasm;
	disassembler	*disasm;
	tlabel		*labels;
	int			addrthreshold, labelthreshold;
	int			cur_addr_ops, cur_label_ops;                 // for threshold
	int			maxopcodelength;
	taddr          *cur_func;
	bool			dirty;

	int			label_count;
	int			addr_count;
	
#ifdef ANALY_TIMINGS
	int			timer;
#endif

			void			init();
			int			load(ht_object_stream *f);
	virtual	void			done();

			bool			add_addr_label(ADDR Addr, char *Prefix, labeltype type, taddr *infunc=NULL);
			void	 		add_comment(ADDR Addr, int line, char *c);
			bool			add_label(ADDR Addr, char *label, labeltype type, taddr *infunc=NULL);
			void			add_xref(ADDR from, ADDR to, xref_type_t action);
			void	 		assign_comment(ADDR Addr, int line, char *c);
			bool			assign_label(ADDR Addr, char *label, labeltype type, taddr *infunc=NULL);
	virtual	void			begin_analysis();
	virtual	UINT			bufptr(ADDR Addr, byte *buf, int size) = 0;
			bool	  		continue_analysis();
			void			continue_analysis_at(ADDR Addr);
			void			data_access(ADDR Addr, taccess access);
			void			delete_addr(ADDR Addr);
			void			delete_label(ADDR Addr);
			void			disable_label(tlabel *label);
			void			do_branch(tbranchtype branch, OPCODE *opcode, int len);
			void			engage_codeanalyser();
			taddr		*enum_addrs(ADDR Addr);
			taddr		*enum_addrs_back(ADDR Addr);
			tlabel		*enum_labels(char *at);
			tlabel		*enum_labels_back(char *at);
	virtual	taddr_typetype examine_data(ADDR Addr);
	virtual	FILEADDR		file_addr(ADDR Addr) = 0;
			taddr		*find_addr(ADDR Addr);
			taddr		*find_addr_context(ADDR Addr);
			taddr		*find_addr_func(ADDR Addr);
			taddr		*find_addr_label(ADDR Addr);
			tlabel		*find_label(char *label);
			void			finish();
			void			free_addr(taddr *Addr);
			void			free_addrs(taddr *addrs);
			void			free_comments(taddr *Addr);
			void			free_label(tlabel *label);
			void			free_labels(tlabel *labels);
			int			get_addr_count();
			int			get_label_count();
			tlabel 		*get_addr_label(ADDR Addr);
	virtual	char			*get_addr_section_name(ADDR Addr);
			bool			goto_addr(ADDR Addr, ADDR func);
	virtual	void 		init_code_analyser();
	virtual	void			init_data_analyser();
	virtual	void			init_unasm() = 0;
	virtual	void			log(char *s);						// stub
	virtual	CPU_ADDR 		map_addr(ADDR Addr);					// stub
			taddr		*new_addr(ADDR Addr);
			taddr		*new_addr(taddr *&addrs, ADDR Addr);
			tlabel		*new_label(char *label, taddr *Addr, labeltype type, taddr *infunc);
			tlabel		*new_label(tlabel *&labels, char *label, taddr *Addr, labeltype type);
	virtual	ADDR			next_valid(ADDR Addr) = 0;
	virtual	void			notify_progress(ADDR Addr);			// stub
			void			optimize_addr_tree();
			void			optimize_label_tree();
			bool			pop_addr(ADDR *Addr, ADDR *func);
			void			push_addr(ADDR Addr, ADDR func);
	virtual	int			query_config(int mode);				// stub
			bool			savety_change_addr(ADDR Addr);
			void			set_active(bool mode);
			void			set_addr_func(taddr *a, taddr *func);
			void			set_disasm(disassembler *Disasm);
			void			set_addr_tree_optimize_threshold(int threshold);
			void			set_label_tree_optimize_threshold(int threshold);
	virtual	void			store(ht_object_stream *f);
	virtual	bool			valid_addr(ADDR Addr, tsectype action) = 0;
			bool			valid_code_addr(ADDR Addr);
			bool			valid_read_addr(ADDR Addr);
			bool			valid_write_addr(ADDR Addr);

//  interface only (there's no internal use)
	int		mode;

	virtual   assembler      *create_assembler();
			comment_list	*get_comments(ADDR Addr);
			char			*get_disasm_str(ADDR Addr);
			char			*get_disasm_str_formatted(ADDR Addr);
//			bool			get_formatted_line(ADDR Addr, int line, char *buf, int *length);
//			bool			get_formatted_line_length(ADDR Addr, int line, int *length);
	virtual   char			*get_name();
	virtual   char			*get_type();
			ht_tree		*get_xrefs(ADDR Addr);
			bool			is_dirty();
			void			make_dirty();
			void			set_display_mode(int enable, int disable);
			void			toggle_display_mode(int toggle);
	virtual	ADDR			vaddr(FILEADDR fileaddr);
};

/* display modes */
#define ANALY_EDIT_BYTES 1
#define ANALY_TRANSLATE_SYMBOLS 2
#define ANALY_COLLAPSE_XREFS 4

/* query_config() constants */
#define Q_DO_ANALYSIS 1
#define Q_ENGAGE_CODE_ANALYSER 2
#define Q_ENGAGE_DATA_ANALYSER 3

/* interesting constants */
#define INVALID_ADDR ((ADDR)-1)
#define EXTERNAL_ENTRY (((ADDR)-1)-1)
#define INVALID_FILE_OFS ((dword)-1)

/* analyser system constants */
#define MAX_OPS_PER_CONTINUE 10

/*
 * test;
 */
extern analyser *testanaly;

#endif
