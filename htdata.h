/* 
 *	HT Editor
 *	htdata.h
 *
 *	Copyright (C) 1999, 2000, 2001 Stefan Weyergraf (stefan@weyergraf.de)
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

#ifndef __HTDATA_H__
#define __HTDATA_H__

class Object;
typedef Object ht_data;

#include "common.h"

#define ATOM_HT_DATA_UINT		MAGICD("DAT\x00")
#define ATOM_HT_DATA_DWORD		MAGICD("DAT\x01")
#define ATOM_HT_DATA_MEM			MAGICD("DAT\x02")

#define ATOM_HT_STREE			MAGICD("DAT\x10")
#define ATOM_HT_CLIST			MAGICD("DAT\x11")

#define ATOM_COMPARE_KEYS_HT_DATA	MAGICD("DAT\x20")
#define ATOM_COMPARE_KEYS_INT		MAGICD("DAT\x21")
#define ATOM_COMPARE_KEYS_UINT	MAGICD("DAT\x22")

/*
 *	CLASS ht_data
 */

/*class ht_data {
public:
			void init() {}
	virtual	void done() {}
};*/

typedef int (*compare_keys_func_ptr)(ht_data *key_a, ht_data *key_b);

/*
 *	CLASS ht_data_uint
 */

class ht_data_uint: public ht_data {
public:
	UINT value;

	ht_data_uint(UINT v=0);
/* overwritten */
	virtual	int load(ht_object_stream *s);
	virtual	void store(ht_object_stream *s);
	virtual	OBJECT_ID object_id() const;
};

/*
 *	CLASS ht_data_dword
 */

class ht_data_dword: public ht_data {
public:
	dword value;

	ht_data_dword(dword v=0);
/* overwritten */
	virtual	int load(ht_object_stream *s);
	virtual	void store(ht_object_stream *s);
	virtual	OBJECT_ID object_id() const;
};

/*
 *	CLASS ht_data_ptr
 */

class ht_data_ptr: public ht_data {
public:
	const void *value;

	ht_data_ptr(const void *v=0);
};

/*
 *	CLASS ht_data_mem
 */

class ht_data_mem: public ht_data {
public:
	void *value;
	UINT size;

			ht_data_mem(const void *v=0, UINT size=0);
	virtual	~ht_data_mem();
/* overwritten */
	virtual	int load(ht_object_stream *s);
	virtual	void store(ht_object_stream *s);
	virtual	OBJECT_ID object_id() const;
};

/*
 *	CLASS ht_tree
 */

struct ht_tree_node {
	ht_data *key;
	ht_data *value;
	ht_tree_node *left, *right;
};

class ht_tree: public ht_data {
public:
	compare_keys_func_ptr compare_keys;

			void init(compare_keys_func_ptr compare_keys);
	virtual	void done();
	virtual	void destroy();
/* new */
	virtual	void balance();
	virtual	UINT count();
	virtual	bool del(ht_data *key);
	virtual ht_data *enum_next(ht_data **value, ht_data *prevkey);
	virtual ht_data *enum_prev(ht_data **value, ht_data *nextkey);
	virtual	ht_data *get(ht_data *key);
	virtual   ht_data *get_insert(ht_data *key);
	virtual	bool insert(ht_data *key, ht_data *value);
	virtual void set_compare_keys(compare_keys_func_ptr new_compare_keys);
};

/*
 *	CLASS ht_stree	(simple tree)
 */

class ht_stree: public ht_tree {
protected:
public:
	ht_tree_node *root;
	UINT node_count;

			void enum_next_i(ht_tree_node *node, ht_data *prevkey, ht_tree_node **retv);
			void enum_prev_i(ht_tree_node *node, ht_data *nextkey, ht_tree_node **retv);
			void free_all(ht_tree_node *node);
			void free_skeleton(ht_tree_node *node);
			ht_tree_node *get_leftmost_node(ht_tree_node *node);
			bool get_node_and_parent(ht_data *key, ht_tree_node **node, ht_tree_node **parent_node, int *direction);
			ht_tree_node *get_node_i(ht_data *key);
			ht_tree_node *get_rightmost_node(ht_tree_node *node);
			void insert_ltable(ht_tree_node **start, ht_tree_node **end);
			void populate_ltable(ht_tree_node ***ltable, ht_tree_node *node);
public:
			void init(compare_keys_func_ptr compare_keys);
	virtual	void done();
	virtual	void destroy();
/* overwritten */
	virtual	void balance();
	virtual	UINT count();
	virtual	bool del(ht_data *key);
	virtual	void empty();
	virtual ht_data *enum_next(ht_data **value, ht_data *prevkey);
	virtual ht_data *enum_prev(ht_data **value, ht_data *nextkey);
	virtual	ht_data *get(ht_data *key);
	virtual	bool insert(ht_data *key, ht_data *value);
	virtual	int load(ht_object_stream *s);
	virtual	OBJECT_ID object_id() const;
	virtual	void store(ht_object_stream *s);
	virtual void set_compare_keys(compare_keys_func_ptr new_compare_keys);
};

/*
 *	CLASS ht_dtree (dead node tree)
 */

#define DEFAULT_MAX_UB_DELETE 500
#define DEFAULT_MAX_UB_INSERT 500

class ht_dtree: public ht_stree {
protected:
	UINT dead_node_count;
	UINT ub_delete, max_ub_delete;
	UINT ub_insert, max_ub_insert;

		void hardcount(UINT *nc, UINT *dnc);
public:
			void init(compare_keys_func_ptr compare_keys, UINT _max_ub_delete=DEFAULT_MAX_UB_DELETE, UINT _max_ub_insert=DEFAULT_MAX_UB_INSERT);
	virtual	void done();
/* overwritten */
	virtual	UINT count();
	virtual	bool del(ht_data *key);
	virtual ht_data *enum_next(ht_data **value, ht_data *prevkey);
	virtual ht_data *enum_prev(ht_data **value, ht_data *nextkey);
	virtual	bool insert(ht_data *key, ht_data *value);
	virtual void set_compare_keys(compare_keys_func_ptr new_compare_keys);
};

/*
 *	CLASS ht_list
 */

#define LIST_UNDEFINED 0xffffffff

class ht_list: public ht_data {
protected:
	compare_keys_func_ptr compare_keys;
public:
			void init(compare_keys_func_ptr compare_keys=0);
	virtual	void done();
	virtual	void destroy();
/* new */
	virtual	void append(ht_data *data);
	virtual	UINT count();
			void copy_to(UINT i, UINT count, ht_list *destlist);
	virtual	ht_list *cut(UINT i, UINT count);
	virtual	bool del(UINT i);
			bool del_multiple(UINT i, UINT count);
	virtual   void empty();
	virtual	UINT find(ht_data *data);
	virtual	ht_data *get(UINT i);
	virtual	void insert(ht_data *data);
	virtual	void insert_after(ht_data *data, UINT i);
	virtual	void insert_before(ht_data *data, UINT i);
	virtual	void move(UINT source, UINT dest);
	virtual	void move_multiple(UINT source, UINT dest, UINT count);
	virtual	void prepend(ht_data *data);
	virtual	ht_data *remove(UINT i);
			bool remove_multiple(UINT i, UINT count);
	virtual	bool set(UINT i, ht_data *data);
	virtual	bool sort();
};

/*
 *	CLASS ht_clist
 */

class ht_clist: public ht_list {
protected:
	ht_data **items;
	UINT c_size, c_entry_count;
	UINT enum_pos;

			void extend_list();
			void do_free(UINT i);
			void do_remove(UINT i);
//	virtual	bool qsort_i(UINT l, UINT r);
public:
			void init(compare_keys_func_ptr compare_keys=0);
	virtual	void done();
	virtual	void destroy();
/* overwritten */
	virtual	void append(ht_data *data);
	virtual	UINT count();
	virtual	ht_list *cut(UINT i, UINT count);
	virtual	bool del(UINT i);
	virtual	Object *duplicate();
	virtual void empty();
	virtual	UINT find(ht_data *data);
	virtual	ht_data *get(UINT i);
	virtual	void insert(ht_data *data);
	virtual	void insert_after(ht_data *data, UINT i);
	virtual	void insert_before(ht_data *data, UINT i);
	virtual	int  load(ht_object_stream *s);
	virtual	void move(UINT source, UINT dest);
	virtual	void move_multiple(UINT source, UINT dest, UINT count);
	virtual	OBJECT_ID object_id() const;
	virtual	void prepend(ht_data *data);
	virtual	ht_data *remove(UINT i);
	virtual	bool set(UINT i, ht_data *data);
	virtual	bool sort();
	virtual	void store(ht_object_stream *s);
};

/*
 *	CLASS ht_sorted_list
 */

class ht_sorted_list: public ht_clist {
public:
			void init(compare_keys_func_ptr compare_keys);
	virtual	void done();
/* overwritten */
	virtual	void append(ht_data *data);
	virtual	UINT find(ht_data *data);
	virtual	void insert(ht_data *data);
	virtual	void insert_after(ht_data *data, UINT i);
	virtual	void insert_before(ht_data *data, UINT i);
	virtual	void move(UINT source, UINT dest);
	virtual	void move_multiple(UINT source, UINT dest, UINT count);
	virtual	void prepend(ht_data *data);
	virtual	bool set(UINT i, ht_data *data);
};

/*
 *	CLASS ht_stack
 */

class ht_stack: public ht_clist {
public:
/* new */
			ht_data *pop();
			void	push(ht_data *data);
};

/*
 *	CLASS ht_queue
 */

class ht_queue: public ht_clist {
public:
/* new */
			void	enqueue(ht_data *data);
			ht_data *dequeue();
/* sepp-wrap */
			ht_data *pop();
			void	push(ht_data *data);
};

int compare_keys_ht_data(ht_data *key_a, ht_data *key_b);
int compare_keys_int(ht_data *key_a, ht_data *key_b);
int compare_keys_uint(ht_data *key_a, ht_data *key_b);

/*
 *	char_set
 */

#define CS_SETSIZE 256

typedef struct char_set {
  unsigned char char_bits [((CS_SETSIZE) + 7) / 8];
} char_set;

#define CS_SET(n, p)    ((p)->char_bits[(n) / 8] |= (1 << ((n) & 7)))
#define CS_CLR(n, p)	((p)->char_bits[(n) / 8] &= ~(1 << ((n) & 7)))
#define CS_ISSET(n, p)	((p)->char_bits[(n) / 8] & (1 << ((n) & 7)))
#define CS_ZERO(p)	memset ((void *)(p), 0, sizeof (*(p)))

/*
 *
 */

#define BITMAP(a0, a1, a2, a3, a4, a5, a6, a7) (((a0)<<0) | ((a1)<<1) | ((a2)<<2) | ((a3)<<3) | ((a4)<<4) | ((a5)<<5) | ((a6)<<6) | ((a7)<<7))

#define BITBIT(bitmap, p) ((bitmap)>>(p)&1)

/*
 *	simple int hash
 */

struct int_hash {
	int value;
	char *desc;
};

char *matchhash(int value, int_hash *hash_table);

/*
 *	INIT
 */

bool init_data();

/*
 *	DONE
 */

void done_data();

#endif /* __HTDATA_H__ */
