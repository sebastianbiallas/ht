/*
 *	HT Editor
 *	htreg.cc
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "defreg.h"
#include "htatom.h"
#include "htctrl.h"
#include "htdialog.h"
#include "htio.h"
#include "htiobox.h"
#include "htreg.h"
#include "log.h"
#include "snprintf.h"
#include "store.h"
#include "tools.h"

ht_registry *registry;

#define ATOM_HT_REGISTRY				MAGICD("REG\x00")
#define ATOM_HT_REGISTRY_NODE			MAGICD("REG\x01")
#define ATOM_HT_REGISTRY_DATA_STREE	MAGICD("REG\x02")
#define ATOM_HT_REGISTRY_DATA_DWORD	MAGICD("REG\x03")
#define ATOM_HT_REGISTRY_DATA_RAW		MAGICD("REG\x04")
#define ATOM_HT_REGISTRY_DATA_STRING	MAGICD("REG\x05")
#define ATOM_HT_REGISTRY_NODE_TYPE_DESC	MAGICD("REG\x10")
#define ATOM_HT_CREATE_EMPTY_SUBDIR	MAGICD("REG\x20")
#define ATOM_HT_CREATE_EMPTY_SYMLINK	MAGICD("REG\x21")
#define ATOM_HT_CREATE_EMPTY_DWORD		MAGICD("REG\x22")
#define ATOM_HT_CREATE_EMPTY_STRING	MAGICD("REG\x23")
#define ATOM_HT_CREATE_EMPTY_RAW		MAGICD("REG\x24")

/*
 *	CLASS ht_registry_data
 */

bool ht_registry_data::editdialog(const char *keyname)
{
	return false;
}

void ht_registry_data::strvalue(char *buf32bytes)
{
	strcpy(buf32bytes, "<unknown>");
}

/*
 *	CLASS ht_registry_data_stree
 */

ht_registry_data_stree::ht_registry_data_stree(ht_stree *t)
{
	tree = t;
}

ht_registry_data_stree::~ht_registry_data_stree()
{
	if (tree) {
		tree->destroy();
		delete tree;
	}
}

int ht_registry_data_stree::load(ht_object_stream *f)
{
	if (!(tree = (ht_stree*)f->getObject("tree"))) return 1;
	return f->get_error();
}

OBJECT_ID ht_registry_data_stree::object_id()
{
	return ATOM_HT_REGISTRY_DATA_STREE;
}

void ht_registry_data_stree::store(ht_object_stream *f)
{
	f->putObject(tree, "tree");
}

void ht_registry_data_stree::strvalue(char *buf32bytes)
{
	*buf32bytes = 0;
}

/*
 *	CLASS ht_registry_data_dword
 */

ht_registry_data_dword::ht_registry_data_dword(dword v)
{
	value = v;
}

bool ht_registry_data_dword::editdialog(const char *keyname)
{
// FIXME: use eval instead of strtol
	char result[32];
	ht_snprintf(result, sizeof result, "%d", value);
	if (inputbox("edit dword", "number", result, sizeof result, 0)==button_ok) {
		char *r;
		int i = (int)strtol(result, &r, 10);
		if (*r == 0) {
			value = i;
			return true;
		}
	}
	return false;
}

int ht_registry_data_dword::load(ht_object_stream *f)
{
	value = f->getIntHex(4, "dword");
	return f->get_error();
}

OBJECT_ID ht_registry_data_dword::object_id()
{
	return ATOM_HT_REGISTRY_DATA_DWORD;
}

void ht_registry_data_dword::store(ht_object_stream *f)
{
	f->putIntHex(value, 4, "dword");
}

void ht_registry_data_dword::strvalue(char *buf32bytes)
{
	ht_snprintf(buf32bytes, 32, "%d", value);
}

/*
 *	CLASS ht_registry_data_raw
 */

ht_registry_data_raw::ht_registry_data_raw(const void *v, UINT s)
{
	size = s;
	if (size) {
		value = malloc(s);
		memmove(value, v, s);
	} else {
		value = 0;
	}
}

ht_registry_data_raw::~ht_registry_data_raw()
{
	if (value) free(value);
}

bool ht_registry_data_raw::editdialog(const char *keyname)
{
	bool r = false;
	bounds b;
	b.w = 50;
	b.h = 15;
	b.x = (screen->size.w-b.w)/2;
	b.y = (screen->size.h-b.h)/2;
	
	ht_dialog *d = new ht_dialog();
	d->init(&b, "edit data_raw", FS_TITLE | FS_KILLER);
	
	if (d->run(false)) {
		r = true;
	}
	
	d->done();
	delete d;
	return r;
}

int ht_registry_data_raw::load(ht_object_stream *f)
{
	size = f->getIntDec(4, "size");
	value = f->getBinary(size, NULL);
	return f->get_error();
}

OBJECT_ID ht_registry_data_raw::object_id()
{
	return ATOM_HT_REGISTRY_DATA_RAW;
}

void ht_registry_data_raw::store(ht_object_stream *f)
{
	f->putIntDec(size, 4, "size");
	f->putBinary(value, size, NULL);
}

void ht_registry_data_raw::strvalue(char *buf32bytes)
{
	strcpy(buf32bytes, "raw");
}

/*
 *	CLASS ht_registry_data_string
 */

ht_registry_data_string::ht_registry_data_string(const char *s)
{
	value = ht_strdup(s);
}

ht_registry_data_string::~ht_registry_data_string()
{
	if (value) free(value);
}

bool ht_registry_data_string::editdialog(const char *keyname)
{
	char res[256];
	strncpy(res, value, 255);
	res[255] = 0;
	if (inputbox("edit string", "string", res, 256, 0)) {
		if (value) delete value;
		value = strdup(res);
		return true;
	}
	return false;
}

int ht_registry_data_string::load(ht_object_stream *f)
{
	value = f->getString("string");
	return f->get_error();
}

OBJECT_ID ht_registry_data_string::object_id()
{
	return ATOM_HT_REGISTRY_DATA_STRING;
}

void ht_registry_data_string::store(ht_object_stream *f)
{
	f->putString(value, "string");
}

void ht_registry_data_string::strvalue(char *buf32bytes)
{
	strncpy(buf32bytes, value, 31);
	buf32bytes[31] = 0;
}

/*
 *	CLASS ht_registry_node
 */

void ht_registry_node::init(ht_registry_node_type Type)
{
	type = Type;
	data = NULL;
}

void ht_registry_node::done()
{
	if (data) {
		data->done();
		delete data;
	}
}

int ht_registry_node::load(ht_object_stream *f)
{
	type = f->getIntHex(4, "type");
	if (!(data = (ht_registry_data*)f->getObject(NULL))) return 1;
	return f->get_error();
}

void ht_registry_node::store(ht_object_stream *f)
{
	f->putIntHex(type, 4, "type");
	f->putObject(data, NULL);
}

OBJECT_ID ht_registry_node::object_id()
{
	return ATOM_HT_REGISTRY_NODE;
}

/*
 *	create_empty_*
 */

ht_registry_data *create_empty_symlink()
{
	return new ht_registry_data_string("");
}

ht_registry_data *create_empty_dword()
{
	return new ht_registry_data_dword();
}

ht_registry_data *create_empty_string()
{
	return new ht_registry_data_string("");
}

ht_registry_data *create_empty_raw()
{
	return new ht_registry_data_raw();
}

/*
 *	CLASS ht_registry
 */

int ht_registry_node_type_desc::load(ht_object_stream *f)
{
	type = f->getIntDec(4, "type");
	HT_ATOM a = f->getIntHex(4, NULL);
	void *p = find_atom(a);
	create_empty_registry_data=(create_empty_registry_data_func)p;
	return f->get_error();
}

OBJECT_ID ht_registry_node_type_desc::object_id()
{
	return ATOM_HT_REGISTRY_NODE_TYPE_DESC;
}

void ht_registry_node_type_desc::store(ht_object_stream *f)
{
	f->putIntDec(type, 4, "type");
	HT_ATOM a = find_atom_rev((void*)create_empty_registry_data);
	f->putIntHex(a, 4, NULL);
}


void ht_registry::init()
{
// build registry root
	root=new ht_registry_node();

	root->init(RNT_SUBDIR);

	ht_stree *s=new ht_stree();
	s->init(compare_keys_string);

	root->data=new ht_registry_data_stree(s);

// build node_types tree
	node_types=new ht_stree();
	node_types->init(compare_keys_string);

	struct bla {
		char *identifier;
		ht_registry_node_type type;
		create_empty_registry_data_func create_empty_registry_data;
	};
	bla b[]=
	{
		{"subdir", RNT_SUBDIR, NULL},
		{"symlink", RNT_SYMLINK, create_empty_symlink},
		{"dword", RNT_DWORD, create_empty_dword},
		{"string", RNT_STRING, create_empty_string},
		{"raw", RNT_RAW, create_empty_raw}
	};

	for (UINT i=0; i<sizeof (b) / sizeof b[0]; i++) {
		ht_registry_node_type_desc *d=new ht_registry_node_type_desc();
		d->type = b[i].type;
		d->create_empty_registry_data = b[i].create_empty_registry_data;
		node_types->insert(new ht_data_string(b[i].identifier), d);
	}
}

void ht_registry::done()
{
	node_types->destroy();
	delete node_types;

	root->done();
	delete root;
}

int ht_registry::create_node(const char *key, ht_registry_node_type type)
{
	const char *name;
	ht_registry_node *m;
	if (!splitfind(key, &name, &m)) return EINVAL;

	if (!valid_nodename(name)) return EINVAL;

	ht_registry_node_type_desc *d = get_node_type_desc(type, NULL);
	if (d && d->create_empty_registry_data) {
		ht_registry_data *data = d->create_empty_registry_data();

		ht_registry_node *n=new ht_registry_node();
		n->init(type);
		n->data=data;

		return ((ht_registry_data_stree*)m->data)->tree->insert(new ht_data_string(name), n) ? 0 : EEXIST;
	}

	return ENOSYS;
}

int ht_registry::create_subdir(const char *key)
{
	const char *name;
	ht_registry_node *m;
	if (!splitfind(key, &name, &m)) return EINVAL;

	if (!valid_nodename(name)) return EINVAL;

	ht_stree *s=new ht_stree();
	s->init(compare_keys_string);

	ht_registry_node *n=new ht_registry_node();
	n->init(RNT_SUBDIR);
	n->data=new ht_registry_data_stree(s);

	return ((ht_registry_data_stree*)m->data)->tree->insert(new ht_data_string(name), n) ? 0 : EEXIST;
}

int ht_registry::delete_node(const char *key)
{
	ht_tree *dir;
	ht_registry_node *n=find_entry_i(&dir, key, 0);
	if (!n) return ENOENT;
	const char *s=strrchr(key, '/');
	if (s) s++; else s=key;
	ht_data_string ss(s);
	return dir->del(&ss) ? 0 : ENOENT;
}

void ht_registry::debug_dump()
{
#if 0
//	FILE *f=fopen("", "");
	debug_dump_i(stderr, ((ht_registry_data_stree*)root->data)->tree, 0);
//	fclose(f);
#endif
}

void ht_registry::debug_dump_i(FILE *f, ht_tree *t, int ident)
{
#if 0
	ht_data_string *key=NULL;
	ht_registry_node *n;
	while ((key=(ht_data_string*)t->enum_next((ht_data**)&n, key))) {
		for (int i=0; i<ident; i++) fprintf(f, "     ");
		fprintf(f, "%s ", key->value);
		switch (n->type) {
			case RNT_DWORD:
				fprintf(f, "= (dword) %08d (%08x)\n", ((ht_data_dword*)n->data)->value, ((ht_data_dword*)n->data)->value);
				break;
			case RNT_STRING:
				fprintf(f, "= (string) \"%s\"\n", ((ht_data_string*)n->data)->value);
				break;
			case RNT_SYMLINK:
				fprintf(f, "=> \"%s\"\n", ((ht_data_string*)n->data)->value);
				break;
			case RNT_SUBDIR:
				fprintf(f, "{\n");
				debug_dump_i(f, ((ht_registry_data_stree*)n->data)->tree, ident+1);
				for (int i=0; i<ident; i++) fprintf(f, "     ");
				fprintf(f, "}\n");
				break;
			case RNT_RAW:
				fprintf(f, "= (raw) nyi!\n");
				break;
			default: {
				char *name=lookup_node_type_name(n->type);
				if (!name) name="?";
				fprintf(f, "= ('%s'=%d)\n", name, n->type);
				break;
			}
		}
	}
#endif
}

const char *ht_registry::enum_next(ht_registry_data **data, ht_registry_node_type *type, const char *dir, const char *prevkey)
{
	ht_tree *t;
	rec_depth = 0;
	ht_registry_node *n = find_entry_i(0, dir, 1);
	if (n) {
		if (n->type != RNT_SUBDIR) return NULL;
		t = ((ht_registry_data_stree*)n->data)->tree;

		ht_data_string ok(prevkey);
		ht_data_string *k;
		ht_registry_node *d;
		if ((k = (ht_data_string*)t->enum_next((ht_data**)&d, prevkey ? &ok : NULL))) {
			*data = d->data;
			*type = d->type;
			return k->value;
		}
	}
	return NULL;
}

const char *ht_registry::enum_prev(ht_registry_data **data, ht_registry_node_type *type, const char *dir, const char *nextkey)
{
	ht_tree *t;
	rec_depth = 0;
	ht_registry_node *n = find_entry_i(0, dir, 1);
	if (n) {
		if (n->type != RNT_SUBDIR) return NULL;
		t = ((ht_registry_data_stree*)n->data)->tree;

		ht_data_string ok(nextkey);
		ht_data_string *k;
		ht_registry_node *d;
		if ((k = (ht_data_string*)t->enum_prev((ht_data**)&d, nextkey ? &ok : NULL))) {
			*data = d->data;
			*type = d->type;
			return k->value;
		}
	}
	return NULL;
}

bool ht_registry::find_any_entry(const char *key, ht_registry_data **data, ht_registry_node_type *type)
{
	rec_depth=0;
	ht_registry_node *n=find_entry_i(0, key, 1);
	if (n) {
		*data=n->data;
		*type=n->type;
		return true;
	}
	return false;
}

bool ht_registry::find_data_entry(const char *key, ht_registry_data **data, ht_registry_node_type *type, bool follow_symlinks)
{
	rec_depth=0;
	ht_registry_node *n=find_entry_i(0, key, follow_symlinks);
	if (n) {
		if (n->type==RNT_SUBDIR) return false;
		*data=n->data;
		*type=n->type;
		return true;
	}
	return false;
}

ht_registry_node *ht_registry::find_entry_i(ht_tree **rdir, const char *key, bool follow_symlinks)
{
	ht_registry_node *dir=root;
	char *s;
	char t[256]; /* FIXME: possible buffer overflow */
	if (key[0]=='/') key++;
	while (1) {
		s=strchr(key, '/');
		if (s) {
			strncpy(t, key, s-key);
			t[s-key]=0;
			dir=find_entry_get_subdir(((ht_registry_data_stree*)dir->data)->tree, t);
			if (!dir) break;
			key=s+1;
		} else {
			ht_registry_node *n;
			if (*key==0) {
				n=dir;
			} else {
				n=find_entry_get_data(((ht_registry_data_stree*)dir->data)->tree, key, follow_symlinks);
			}
			if (rdir) *rdir=((ht_registry_data_stree*)dir->data)->tree;
			return n;
		}
	}
	return 0;
}

ht_registry_node *ht_registry::find_entry_get_node(ht_tree *dir, const char *nodename)
{
	if (nodename) {
		ht_data_string keystr(nodename);
		ht_registry_node *n=(ht_registry_node*)dir->get(&keystr);
		return n;
	}
	return NULL;
}

ht_registry_node *ht_registry::find_entry_get_subdir(ht_tree *dir, const char *nodename)
{
	ht_registry_node *n=find_entry_get_node(dir, nodename);
start:
	if (!n) return 0;
	switch (n->type) {
		case RNT_SYMLINK: {
			rec_depth++;
			if (rec_depth>MAX_SYMLINK_REC_DEPTH) return 0;
			char *sl=((ht_registry_data_string*)n->data)->value;
			if (sl[0]=='/') {
				n=find_entry_i(0, sl, 1);
				goto start;
			} else {
				return find_entry_get_subdir(dir, sl);
			}
		}
		case RNT_SUBDIR:
			return n;
	}
	return 0;
}

ht_registry_node *ht_registry::find_entry_get_data(ht_tree *dir, const char *nodename, bool follow_symlinks)
{
	ht_registry_node *n=find_entry_get_node(dir, nodename);
start:
	if (!n) return 0;
	if ((follow_symlinks) && (n->type==RNT_SYMLINK)) {
		rec_depth++;
		if (rec_depth>MAX_SYMLINK_REC_DEPTH) return 0;
		char *sl=((ht_registry_data_string*)n->data)->value;
		if (sl[0]=='/') {
			n=find_entry_i(0, sl, 1);
			goto start;
		} else {
			return find_entry_get_data(dir, sl, follow_symlinks);
		}
	}
	return n;
}

ht_registry_node_type ht_registry::have_node_type(const char *identifier, create_empty_registry_data_func create_empty_registry_data)
{
	ht_registry_node_type t=lookup_node_type(identifier);
	if (!t) t=register_node_type(identifier, create_empty_registry_data);
	return t;
}

int ht_registry::load(ht_object_stream *f)
{
	if (!(node_types=(ht_stree*)f->getObject("types"))) return 1;
	if (!(root=(ht_registry_node*)f->getObject("root"))) return 1;
	return f->get_error();
}

ht_registry_node_type_desc *ht_registry::get_node_type_desc(ht_registry_node_type t, char **identifier)
{
	ht_data_string *key=NULL;
	ht_registry_node_type_desc *data;
	while ((key=(ht_data_string*)node_types->enum_next((ht_data**)&data, key))) {
		if (t==data->type) {
			if (identifier) *identifier = key->value;
			return data;
		}
	}
	return NULL;
}

ht_registry_node_type ht_registry::lookup_node_type(const char *identifier)
{
	ht_data_string s(identifier);
	ht_registry_node_type_desc *d=(ht_registry_node_type_desc*)node_types->get(&s);
	return d ? d->type : 0;
}

OBJECT_ID ht_registry::object_id()
{
	return ATOM_HT_REGISTRY;
}

ht_registry_node_type ht_registry::register_node_type(const char *identifier, create_empty_registry_data_func create_empty_registry_data)
{
//	ht_registry_node_type t = RNT_USER;
	ht_registry_node_type t = 0;
	ht_data_string *key;
	ht_registry_node_type_desc *data;
	bool found = false; 
	while (!found) {
		t++;
		found = true;
		key = NULL;
		while ((key=(ht_data_string*)node_types->enum_next((ht_data**)&data, key))) {
			if (t == data->type) {
				found = false;
				break;
			}
		}
	}

	ht_registry_node_type_desc *v = new ht_registry_node_type_desc();
	v->type = t;
	v->create_empty_registry_data = create_empty_registry_data;
	bool b = node_types->insert(new ht_data_string(identifier), v);
	if (b) return t;
	return RNT_INVALID;
}

int ht_registry::set_dword(const char *key, dword d)
{
	return set_node(key, RNT_DWORD, new ht_registry_data_dword(d));
}

int ht_registry::set_raw(const char *key, const void *data, UINT size)
{
	return set_node(key, RNT_RAW, new ht_registry_data_raw(data, size));
}

int ht_registry::set_node(const char *key, ht_registry_node_type type, ht_registry_data *data)
{
	ht_registry_node *n = find_entry_i(0, key, 0);
	if (!n) return ENOENT;

	if (n->type == type) {
		if (n->data) {
			n->data->done();
			delete n->data;
		}
		n->data = data;
		return 0;
	}
	return EPERM;
}

int ht_registry::set_string(const char *key, const char *string)
{
	return set_node(key, RNT_STRING, new ht_registry_data_string(string));
}

int ht_registry::set_symlink(const char *key, const char *dest)
{
	return set_node(key, RNT_SYMLINK, new ht_registry_data_string(dest));
}

bool ht_registry::splitfind(const char *key, const char **name, ht_registry_node **node)
{
	char dir[256]; /* FIXME: possible buffer overflow */
	const char *n=strrchr(key, '/');
	if (n) {
		strncpy(dir, key, n-key);
		dir[n-key]=0;
		n++;
	} else {
		dir[0]=0;
		n=key;
	}

	ht_registry_node *m=find_entry_i(0, dir, 1);
	if (!m) return 0;
	if (m->type!=RNT_SUBDIR) return 0;
	*node=m;
	*name=n;
	return 1;
}

void ht_registry::store(ht_object_stream *f)
{
	f->putObject(node_types, "types");
	f->putObject(root, "root");
}

unsigned char valid_nodename_chars[256/8]=
{
/* 00, 08, 10, 18 */
	0, 0, 0, 0,
/* 20, 28, 30, 38 */
/* space '-' 0-9 '&' ':' '.' */
	BITMAP(1, 0, 0, 0, 0, 0, 1, 0), BITMAP(0, 0, 0, 0, 0, 1, 1, 0),
	BITMAP(1, 1, 1, 1, 1, 1, 1, 1), BITMAP(1, 1, 1, 0, 0, 0, 0, 0),
/* 40, 48, 50, 58 */
/* A-Z '_' */
	BITMAP(0, 1, 1, 1, 1, 1, 1, 1), BITMAP(1, 1, 1, 1, 1, 1, 1, 1),
	BITMAP(1, 1, 1, 1, 1, 1, 1, 1), BITMAP(1, 1, 1, 0, 0, 0, 0, 1),
/* 60, 68, 70, 78 */
/* a-z */
	BITMAP(0, 1, 1, 1, 1, 1, 1, 1), BITMAP(1, 1, 1, 1, 1, 1, 1, 1),
	BITMAP(1, 1, 1, 1, 1, 1, 1, 1), BITMAP(1, 1, 1, 0, 0, 0, 0, 0),
/* 80, 88, 90, 98 */
	0, 0, 0, 0,
/* a0, a8, b0, b8 */
	0, 0, 0, 0,
/* c0, c8, d0, d8 */
	0, 0, 0, 0,
/* e0, e8, f0, f8 */
	0, 0, 0, 0
};

unsigned char valid_nodename_chars_first[256/8]=
{
/* 00, 08, 10, 18 */
	0, 0, 0, 0,
/* 20, 28, 30, 38 */
/* '.' '-' 0-9 */
	BITMAP(0, 0, 0, 0, 0, 0, 1, 0), BITMAP(0, 0, 0, 0, 0, 1, 1, 0),
	BITMAP(1, 1, 1, 1, 1, 1, 1, 1), BITMAP(1, 1, 0, 0, 0, 0, 0, 0),
/* 40, 48, 50, 58 */
/* A-Z '_' */
	BITMAP(0, 1, 1, 1, 1, 1, 1, 1), BITMAP(1, 1, 1, 1, 1, 1, 1, 1),
	BITMAP(1, 1, 1, 1, 1, 1, 1, 1), BITMAP(1, 1, 1, 0, 0, 0, 0, 1),
/* 60, 68, 70, 78 */
/* a-z */
	BITMAP(0, 1, 1, 1, 1, 1, 1, 1), BITMAP(1, 1, 1, 1, 1, 1, 1, 1),
	BITMAP(1, 1, 1, 1, 1, 1, 1, 1), BITMAP(1, 1, 1, 0, 0, 0, 0, 0),
/* 80, 88, 90, 98 */
	0, 0, 0, 0,
/* a0, a8, b0, b8 */
	0, 0, 0, 0,
/* c0, c8, d0, d8 */
	0, 0, 0, 0,
/* e0, e8, f0, f8 */
	0, 0, 0, 0
};

bool valid_char(unsigned char *bitmap, char c)
{
	int o=c/8;
	int p=c%8;
	return BITBIT(bitmap[o], p);
}

bool ht_registry::valid_nodename(const char *nodename)
{
	if ((strcmp(nodename, "..")==0) || (strcmp(nodename, ".")==0)) {
		return false;
	}
	if (!valid_char(valid_nodename_chars_first, *nodename)) return false;
	nodename++;
	while (*nodename) {
		if (!valid_char(valid_nodename_chars, *nodename)) return false;
		nodename++;
	}
	return true;
}

dword get_config_dword(char *ident)
{
	char e[HT_NAME_MAX], *ee = e;
	strcpy(ee, "/config/"); ee += strlen(ee);
	strncpy(ee, ident, sizeof (e) - (ee-e));
	ht_registry_data *d;
	ht_registry_node_type t;
	if (registry->find_data_entry(e, &d, &t, true)) {
		if (t == RNT_DWORD) {
			ht_registry_data_dword *s = (ht_registry_data_dword *)d;
			return s->value;
		} else {
			char *q = "?";
			registry->get_node_type_desc(t, &q);
			LOG_EX(LOG_ERROR, "registry key '%s' not of type %s, but: %s", e, "dword", q);
		}
	} else LOG_EX(LOG_ERROR, "registry key '%s' not found", e);
	return 0;
}

char *get_config_string(char *ident)
{
	char e[HT_NAME_MAX], *ee = e;
	strcpy(ee, "/config/"); ee += strlen(ee);
	strncpy(ee, ident, sizeof (e) - (ee-e));
	ht_registry_data *d;
	ht_registry_node_type t;
	if (registry->find_data_entry(e, &d, &t, true)) {
		if (t == RNT_STRING) {
			ht_registry_data_string *s = (ht_registry_data_string *)d;
			return ht_strdup(s->value);
		} else {
			char *q = "?";
			registry->get_node_type_desc(t, &q);
			LOG_EX(LOG_ERROR, "registry key '%s' not of type %s, but: %s", e, "string", q);
		}
	} else LOG_EX(LOG_ERROR, "registry key '%s' not found", e);
	return NULL;
}

BUILDER(ATOM_HT_REGISTRY, ht_registry);
BUILDER(ATOM_HT_REGISTRY_NODE, ht_registry_node);
BUILDER(ATOM_HT_REGISTRY_DATA_STREE, ht_registry_data_stree);
BUILDER(ATOM_HT_REGISTRY_DATA_DWORD, ht_registry_data_dword);
BUILDER(ATOM_HT_REGISTRY_DATA_RAW, ht_registry_data_raw);
BUILDER(ATOM_HT_REGISTRY_DATA_STRING, ht_registry_data_string);
BUILDER(ATOM_HT_REGISTRY_NODE_TYPE_DESC, ht_registry_node_type_desc);
	
/*
 *	INIT
 */

bool init_registry()
{
	REGISTER(ATOM_HT_REGISTRY, ht_registry);
	REGISTER(ATOM_HT_REGISTRY_NODE, ht_registry_node);
	REGISTER(ATOM_HT_REGISTRY_DATA_STREE, ht_registry_data_stree);
	REGISTER(ATOM_HT_REGISTRY_DATA_DWORD, ht_registry_data_dword);
	REGISTER(ATOM_HT_REGISTRY_DATA_RAW, ht_registry_data_raw);
	REGISTER(ATOM_HT_REGISTRY_DATA_STRING, ht_registry_data_string);
	REGISTER(ATOM_HT_REGISTRY_NODE_TYPE_DESC, ht_registry_node_type_desc);
//	register_atom(ATOM_HT_CREATE_EMPTY_SUBDIR, (void*));
	register_atom(ATOM_HT_CREATE_EMPTY_SYMLINK, (void*)create_empty_symlink);
	register_atom(ATOM_HT_CREATE_EMPTY_DWORD, (void*)create_empty_dword);
	register_atom(ATOM_HT_CREATE_EMPTY_STRING, (void*)create_empty_string);
	register_atom(ATOM_HT_CREATE_EMPTY_RAW, (void*)create_empty_raw);

/*
 *	load default registry
 */
	ht_memmap_file *f = new ht_memmap_file();
	f->init((unsigned char*)default_reg, sizeof default_reg);

	ht_object_stream_bin *o = new ht_object_stream_bin();
	o->init(f);

	registry = (ht_registry*)o->getObject(NULL);

	o->done();
	delete o;

	f->done();
	delete f;

	return true;
}

/*
 *	DONE
 */

void done_registry()
{
	UNREGISTER(ATOM_HT_REGISTRY, ht_registry);
	UNREGISTER(ATOM_HT_REGISTRY_NODE, ht_registry_node);
	UNREGISTER(ATOM_HT_REGISTRY_DATA_STREE, ht_registry_data_stree);
	UNREGISTER(ATOM_HT_REGISTRY_DATA_DWORD, ht_registry_data_dword);
	UNREGISTER(ATOM_HT_REGISTRY_DATA_RAW, ht_registry_data_raw);
	UNREGISTER(ATOM_HT_REGISTRY_DATA_STRING, ht_registry_data_string);
	UNREGISTER(ATOM_HT_REGISTRY_NODE_TYPE_DESC, ht_registry_node_type_desc);
//	unregister_atom(ATOM_HT_CREATE_EMPTY_SUBDIR);
	unregister_atom(ATOM_HT_CREATE_EMPTY_SYMLINK);
	unregister_atom(ATOM_HT_CREATE_EMPTY_DWORD);
	unregister_atom(ATOM_HT_CREATE_EMPTY_STRING);
	unregister_atom(ATOM_HT_CREATE_EMPTY_RAW);
	
	registry->done();
	delete registry;
}

