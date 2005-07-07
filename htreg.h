/* 
 *	HT Editor
 *	htreg.h
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

#ifndef __HTREG_H__
#define __HTREG_H__

#include "htdata.h"
#include "htstring.h"

/*
 *	CLASS ht_registry_data
 */

class ht_registry_data: public ht_data {
public:
/* new */
	virtual	bool editdialog(const char *keyname);
	virtual   void strvalue(char *buf32bytes);
};

/*
 *	CLASS ht_registry_data_stree
 */

class ht_registry_data_stree: public ht_registry_data {
public:
	ht_stree *tree;

			ht_registry_data_stree(ht_stree *tree=0);
			~ht_registry_data_stree();
/* overwritten */
	virtual	int  load(ObjectStream &f);
	virtual	ObjectID getObjectID() const;
	virtual	void store(ObjectStream &f);
	virtual	void strvalue(char *buf32bytes);
};

/*
 *	CLASS ht_registry_data_dword
 */

class ht_registry_data_dword: public ht_registry_data {
public:
	uint32 value;

			ht_registry_data_dword(uint32 value=0);
/* overwritten */
	virtual	bool editdialog(const char *keyname);
	virtual	int  load(ObjectStream &f);
	virtual	ObjectID getObjectID() const;
	virtual	void store(ObjectStream &f);
	virtual	void strvalue(char *buf32bytes);
};

/*
 *	CLASS ht_registry_data_raw
 */

class ht_registry_data_raw: public ht_registry_data {
public:
	void *value;
	uint size;

			ht_registry_data_raw(const void *value = 0, uint size = 0);
			~ht_registry_data_raw();
/* overwritten */
	virtual	bool editdialog(const char *keyname);
	virtual	int  load(ObjectStream &f);
	virtual	ObjectID getObjectID() const;
	virtual	void store(ObjectStream &f);
	virtual	void strvalue(char *buf32bytes);
};

/*
 *	CLASS ht_registry_data_string
 */

class ht_registry_data_string: public ht_registry_data {
public:
	char *value;

			ht_registry_data_string(const char *s = 0);
			~ht_registry_data_string();
/* overwritten */
	virtual	bool editdialog(const char *keyname);
	virtual	int  load(ObjectStream &f);
	virtual	ObjectID getObjectID() const;
	virtual	void store(ObjectStream &f);
	virtual	void strvalue(char *buf32bytes);
};

/*
 *	CLASS ht_registry_node
 */

typedef ht_registry_data* (*create_empty_registry_data_func)();

typedef uint ht_registry_node_type;

class ht_registry_node_type_desc: public ht_data {
public:
	ht_registry_node_type type;
	create_empty_registry_data_func create_empty_registry_data;
	
	virtual	int  load(ObjectStream &f);
	virtual	ObjectID getObjectID() const;
	virtual	void store(ObjectStream &f);
};

#define RNT_INVALID		0	/* returned by some functions */
// these are predefined
#define RNT_SUBDIR		1
#define RNT_SYMLINK		2
#define RNT_DWORD  		3
#define RNT_STRING 		4
#define RNT_RAW		5

#define RNT_USER    	0x100
// the rest may be allocated dynamically

class ht_registry_node: public ht_data {
public:
	ht_registry_node_type type;
	ht_registry_data *data;

			void init(ht_registry_node_type type);
	virtual	void done();
/* overwritten */
	virtual	int  load(ObjectStream &f);
	virtual	void store(ObjectStream &f);
	virtual	ObjectID getObjectID() const;
};

/*
 *	CLASS ht_registry
 */

#define MAX_SYMLINK_REC_DEPTH 20

class ht_registry: public ht_data {
protected:
	ht_registry_node *root;
	uint rec_depth;

			ht_registry_node *find_entry_i(ht_tree **dir, const char *key, bool follow_symlinks);
			ht_registry_node *find_entry_get_node(ht_tree *dir, const char *nodename);
			ht_registry_node *find_entry_get_subdir(ht_tree *dir, const char *nodename);
			ht_registry_node *find_entry_get_data(ht_tree *dir, const char *nodename, bool follow_symlinks);
			bool splitfind(const char *key, const char **name, ht_registry_node **node);
public:
	ht_stree *node_types;

			void init();
	virtual	void done();
/* new */
			int create_node(const char *key, ht_registry_node_type type);
			int create_subdir(const char *key);
			int delete_node(const char *key);
			const char *enum_next(ht_registry_data **data, ht_registry_node_type *type, const char *dir, const char *prevkey);
			const char *enum_prev(ht_registry_data **data, ht_registry_node_type *type, const char *dir, const char *nextkey);
			
			bool find_any_entry(const char *key, ht_registry_data **data, ht_registry_node_type *type);
			bool find_data_entry(const char *key, ht_registry_data **data, ht_registry_node_type *type, bool follow_symlinks);
			/* node type*/
			ht_registry_node_type lookup_node_type(const char *identifier);
			ht_registry_node_type_desc *get_node_type_desc(ht_registry_node_type t, char **identifier);
			ht_registry_node_type have_node_type(const char *identifier, create_empty_registry_data_func create_empty_registry_data);
			ht_registry_node_type register_node_type(const char *identifier, create_empty_registry_data_func create_empty_registry_data);
			/**/
			int set_dword(const char *key, uint32 d);
			int set_raw(const char *key, const void *data, uint size);
			int set_node(const char *key, ht_registry_node_type type, ht_registry_data *data);
			int set_string(const char *key, const char *string);
			int set_symlink(const char *key, const char *dest);
			bool valid_nodename(const char *nodename);
/* overwritten */
	virtual	int  load(ObjectStream &f);
	virtual	void store(ObjectStream &f);
	virtual	ObjectID getObjectID() const;
/* debug */
			void debug_dump();
			void debug_dump_i(FILE *f, ht_tree *t, int ident);
};

uint32 get_config_dword(char *ident);
char *get_config_string(char *ident);

extern ht_registry *registry;

/*
 *	INIT
 */

bool init_registry();

/*
 *	DONE
 */

void done_registry();

#endif /* __HTREG_H__ */
