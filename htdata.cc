/* 
 *	HT Editor
 *	htdata.cc
 *
 *	Copyright (C) 1999-2002 Stefan Weyergraf (stefan@weyergraf.de)
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

#include "htatom.h"
#include "htdata.h"
#include "htdebug.h"
#include "stream.h"
#include "tools.h"

#include <string.h>
#include <stdlib.h>

int (*qsort_compare_compare_keys)(ht_data *key_a, ht_data *key_b);

int qsort_compare(const void *e1, const void *e2)
{
	ht_data **d1=(ht_data **)e1;
	ht_data **d2=(ht_data **)e2;
	return qsort_compare_compare_keys(*d1, *d2);
}

/*
 *	CLASS ht_data_uint
 */

ht_data_uint::ht_data_uint(UINT v)
{
	value=v;
}

int ht_data_uint::load(ht_object_stream *s)
{
	value=s->getIntHex(4, NULL);
	return s->get_error();
}

void ht_data_uint::store(ht_object_stream *s)
{
	s->putIntHex(value, 4, NULL);
}

OBJECT_ID ht_data_uint::object_id() const
{
	return ATOM_HT_DATA_UINT;
}

/*
 *	CLASS ht_data_dword
 */

ht_data_dword::ht_data_dword(dword v)
{
	value=v;
}

int ht_data_dword::load(ht_object_stream *s)
{
	value=s->getIntHex(4, NULL);
	return s->get_error();
}

void ht_data_dword::store(ht_object_stream *s)
{
	s->putIntHex(value, 4, NULL);
}

OBJECT_ID ht_data_dword::object_id() const
{
	return ATOM_HT_DATA_DWORD;
}

/*
 *	CLASS ht_data_ptr
 */

ht_data_ptr::ht_data_ptr(const void *v)
{
	value=v;
}

/*
 *	CLASS ht_data_mem
 */

ht_data_mem::ht_data_mem(const void *v, UINT _size)
{
	size=_size;
	if (size) {
		value=malloc(size);
		memmove(value, v, size);
	} else {
		value=NULL;
	}
}

ht_data_mem::~ht_data_mem()
{
	if (value) free(value);
}

int ht_data_mem::load(ht_object_stream *s)
{
	value=s->getBinary(size, NULL);
	return s->get_error();
}

void ht_data_mem::store(ht_object_stream *s)
{
	s->putBinary(value, size, NULL);
}

OBJECT_ID ht_data_mem::object_id() const
{
	return ATOM_HT_DATA_MEM;
}

/*
 *	CLASS ht_tree
 */

void ht_tree::init(compare_keys_func_ptr _compare_keys)
{
	ht_data::init();
	assert(_compare_keys);
	compare_keys=_compare_keys;
}

void ht_tree::done()
{
	ht_data::done();
}

void ht_tree::destroy()
{
	done();
}

void ht_tree::balance()
{
}

UINT ht_tree::count()
{
	return 0;
}

bool ht_tree::del(ht_data *key)
{
	return false;
}

ht_data *ht_tree::enum_next(ht_data **value, ht_data *prevkey)
{
	return NULL;
}

ht_data *ht_tree::enum_prev(ht_data **value, ht_data *nextkey)
{
	return NULL;
}

ht_data *ht_tree::get(ht_data *key)
{
	return NULL;
}

ht_data *ht_tree::get_insert(ht_data *key)
{
	return NULL;
}

bool ht_tree::insert(ht_data *key, ht_data *data)
{
	return false;
}

void ht_tree::set_compare_keys(compare_keys_func_ptr new_compare_keys)
{
}

/*
 *	CLASS ht_stree	(simple tree)
 */

void ht_stree::init(compare_keys_func_ptr compare_keys)
{
	ht_tree::init(compare_keys);
	root=NULL;
	node_count=0;
}

void ht_stree::done()
{
	ht_tree::done();
}

void ht_stree::destroy()
{
	empty();
	ht_tree::done();
}

void ht_stree::set_compare_keys(compare_keys_func_ptr new_compare_keys)
{
	if (node_count) {
		ht_tree_node **ltable=(ht_tree_node**)malloc(sizeof (ht_tree_node*) * node_count);
		ht_tree_node **l=ltable;
		/* create ltable from tree */
		populate_ltable(&l, root);
		/* save old tree, empty tree */
		UINT old_node_count = node_count;
		ht_tree_node *old_root = root;
		node_count = 0;
		root = NULL;
		if (compare_keys != new_compare_keys) {
			/* set new keying method */
			compare_keys = new_compare_keys;
			/* re-sort ltable */
			qsort_compare_compare_keys = compare_keys;
			qsort(ltable, old_node_count, sizeof *ltable, qsort_compare);
		}
		/* rebuild tree from ltable and old_tree */
		insert_ltable(ltable, ltable+old_node_count-1);
		/* destroy old_tree */
		node_count = old_node_count;
		free_skeleton(old_root);
		/* destroy ltable */
		free(ltable);
		/**/
		node_count = old_node_count;
	}
}

void ht_stree::balance()
{
	set_compare_keys(compare_keys);
}

UINT ht_stree::count()
{
	return node_count;
}

bool ht_stree::del(ht_data *key)
{
	ht_tree_node *node, *parent;
	int direction;
	if (!get_node_and_parent(key, &node, &parent, &direction)) return false;

	node->key->done();
	delete node->key;

	node->value->done();
	delete node->value;

	if (node->left) {
		ht_tree_node *c=node->left, *a=node->right;
		*node=*c;
		delete c;
		get_rightmost_node(node)->right=a;
	} else if (node->right) {
		ht_tree_node *c=node->right, *a=node->left;
		*node=*c;
		delete c;
		get_leftmost_node(node)->left=a;
	} else {
		ht_tree_node **pp;
		if (parent) {
			if (direction>0) pp=&parent->right; else pp=&parent->left;
		} else {
			pp=&root;
		}
		*pp=NULL;
		delete node;
	}
	node_count--;
	return true;
}

void ht_stree::empty()
{
	if (root) {
		free_all(root);
		root=NULL;
	}
}

void ht_stree::enum_next_i(ht_tree_node *node, ht_data *prevkey, ht_tree_node **retv)
{
	if ((prevkey == NULL) || (compare_keys(prevkey, node->key) < 0)) {
		*retv = node;
		if (node->left) enum_next_i(node->left, prevkey, retv);
	} else {
		if (node->right) enum_next_i(node->right, prevkey, retv);
	}
}

void ht_stree::enum_prev_i(ht_tree_node *node, ht_data *nextkey, ht_tree_node **retv)
{
	if ((nextkey == NULL) || (compare_keys(node->key, nextkey) < 0)) {
		*retv = node;
		if (node->right) enum_prev_i(node->right, nextkey, retv);
	} else {
		if (node->left) enum_prev_i(node->left, nextkey, retv);
	}
}

ht_data *ht_stree::enum_next(ht_data **value, ht_data *prevkey)
{
	ht_tree_node *n = NULL;
	if (root) enum_next_i(root, prevkey, &n);
	if (n) {
		*value = n->value;
		return n->key;
	}
	return NULL;
}

ht_data *ht_stree::enum_prev(ht_data **value, ht_data *nextkey)
{
	ht_tree_node *n = NULL;
	if (root) enum_prev_i(root, nextkey, &n);
	if (n) {
		*value = n->value;
		return n->key;
	}
	return NULL;
}

void ht_stree::free_all(ht_tree_node *node)
{
	if (node->left) free_all(node->left);
	if (node->right) free_all(node->right);
	node->key->done();
	delete node->key;

	if (node->value) {
		node->value->done();
		delete node->value;
	}		

	delete node;
	node_count--;
}

void ht_stree::free_skeleton(ht_tree_node *node)
{
	if (node->left) free_skeleton(node->left);
	if (node->right) free_skeleton(node->right);
	delete node;
	node_count--;
}

ht_data *ht_stree::get(ht_data *key)
{
	ht_tree_node *n=get_node_i(key);
	if (n) return n->value;
	return NULL;
}

ht_tree_node *ht_stree::get_node_i(ht_data *key)
{
	ht_tree_node *n=root;
	while (n) {
		int c=compare_keys(key, n->key);
		if (c>0) n=n->right; else
		if (c<0) n=n->left; else
			return n;
	}
	return NULL;
}

bool ht_stree::get_node_and_parent(ht_data *key, ht_tree_node **node, ht_tree_node **parent_node, int *direction)
{
	ht_tree_node *p=NULL, *n=root;
	int pc=0;
	while (n) {
		int c=compare_keys(key, n->key);
		if (c>0) {
			p=n;
			pc=c;
			n=n->right;
		} else if (c<0) {
			p=n;
			pc=c;
			n=n->left;
		} else {
			*node=n;
			*parent_node=p;
			*direction=pc;
			return true;
		}
	}
	return false;
}

ht_tree_node *ht_stree::get_leftmost_node(ht_tree_node *node)
{
	if (node) while (node->left) node=node->left;
	return node;
}

ht_tree_node *ht_stree::get_rightmost_node(ht_tree_node *node)
{
	if (node) while (node->right) node=node->right;
	return node;
}

bool ht_stree::insert(ht_data *key, ht_data *value)
{
//	if ((!key) || (!value)) return false;
	if (!key) return false;
	ht_tree_node **n=&root;
	while (*n) {
		int c=compare_keys(key, (*n)->key);
		if (c>0) n=&(*n)->right; else
		if (c<0) n=&(*n)->left; else
			return false;
	}
	*n=new ht_tree_node();
	(*n)->key=key;
	(*n)->value=value;
	(*n)->left=NULL;
	(*n)->right=NULL;
	node_count++;
	return true;
}

void ht_stree::insert_ltable(ht_tree_node **start, ht_tree_node **end)
{
	if (start<=end) {
		ht_tree_node **m=start+(end-start)/2;
		ht_stree::insert((*m)->key, (*m)->value);
		insert_ltable(start, m-1);
		insert_ltable(m+1, end);
	}
}

void stree_load(ht_object_stream *s, ht_tree_node **n, UINT *node_count, int l, int r)
{
	if (l>r) {
		*n = NULL;
		return;
	}
	*n = new ht_tree_node();
	int m = (l+r)/2;
	stree_load(s, &(*n)->left, node_count, l, m-1);

	(*n)->key = s->getObject(NULL);
	(*n)->value = s->getObject(NULL);
	(*node_count)++;

	stree_load(s, &(*n)->right, node_count, m+1, r);
}

int ht_stree::load(ht_object_stream *s)
{
	void *d=find_atom(s->getIntHex(4, NULL));
	if (!d) return 1;
	compare_keys=(compare_keys_func_ptr)d;
	
	UINT c=s->getIntDec(4, NULL);
#if 0
	root=NULL;
	node_count=0;
	
	for (UINT i=0; i<c; i++) {
		ht_data *key=s->get_object(NULL);
		ht_data *value=s->get_object(NULL);
		if (!key) return 1;
		if (!value) return 1;
		insert(key, value);
	}
	balance();
#else
	root=NULL;
	node_count=0;

	stree_load(s, &root, &node_count, 0, c-1);
	assert(node_count == c);
	
#endif
	return s->get_error();
}

void ht_stree::store(ht_object_stream *s)
{
	s->putIntHex(find_atom_rev((void*)compare_keys), 4, NULL);
	
	s->putIntDec(count(), 4, NULL);
	
	ht_data *key=NULL;
	ht_data *value;
	while ((key=enum_next(&value, key))) {
		if (value) {
			s->putObject(key, NULL);
			s->putObject(value, NULL);
		}
	}
}

OBJECT_ID ht_stree::object_id() const
{
	return ATOM_HT_STREE;
}

void ht_stree::populate_ltable(ht_tree_node ***ltable, ht_tree_node *node)
{
	if (node->left) populate_ltable(ltable, node->left);
	if (node->value) {
		**ltable=node;
		(*ltable)++;
	}
	if (node->right) populate_ltable(ltable, node->right);
}

/*
 *	CLASS ht_dtree (rebalancing and dead node tree)
 */

void ht_dtree::init(compare_keys_func_ptr compare_keys, UINT _max_ub_delete, UINT _max_ub_insert)
{
	ht_stree::init(compare_keys);
	dead_node_count=0;
	ub_delete=0;
	ub_insert=0;
	max_ub_delete=_max_ub_delete;
	max_ub_insert=_max_ub_insert;
}

void ht_dtree::done()
{
	ht_stree::done();
}

void ht_dtree::set_compare_keys(compare_keys_func_ptr new_compare_keys)
{
	ub_delete=0;
	ub_insert=0;
	if (node_count-dead_node_count) {
		/* create ltable from tree */
		UINT new_node_count = node_count-dead_node_count;
		ht_tree_node **ltable = (ht_tree_node**)malloc(sizeof (ht_tree_node*) * new_node_count);
		assert(ltable);
		ht_tree_node **l = ltable;
		populate_ltable(&l, root);

		/* save old tree, empty tree */
		UINT old_node_count = node_count;
		ht_tree_node *old_root  =root;
		node_count = 0;
		root = NULL;
		if (compare_keys != new_compare_keys) {
			/* set new keying method */
			compare_keys = new_compare_keys;
			/* re-sort ltable */
			qsort_compare_compare_keys = compare_keys;
			qsort(ltable, new_node_count, sizeof *ltable, qsort_compare);
		}
		/* rebuild tree from ltable and old_tree */
		insert_ltable(ltable, ltable+new_node_count-1);
		/* destroy old_tree */
		node_count = old_node_count;
		free_skeleton(old_root);
		/* destroy ltable */
		free(ltable);
		/**/
		node_count = new_node_count;
		dead_node_count = 0;
	}
}

UINT ht_dtree::count()
{
	return node_count-dead_node_count;
}

bool ht_dtree::del(ht_data *key)
{
	ht_tree_node *n=get_node_i(key);
	if ((n) && (n->value)) {
		n->value->done();
		delete n->value;
		n->value=NULL;
		dead_node_count++;
		if (max_ub_delete) {
			if (++ub_delete>max_ub_delete) balance();
		}
		return true;
	}
	return false;
}

ht_data *ht_dtree::enum_next(ht_data **value, ht_data *prevkey)
{
	ht_tree_node *n;
	ht_data *k = prevkey;
	while (1) {
		n = NULL;
		if (root) enum_next_i(root, k, &n);
		if (!n) break;
		if (n->value) {
			*value = n->value;
			return n->key;
		}
		k = n->key;
	}
	return NULL;
}

ht_data *ht_dtree::enum_prev(ht_data **value, ht_data *nextkey)
{
	ht_tree_node *n;
	ht_data *k = nextkey;
	while (1) {
		n = NULL;
		if (root) enum_prev_i(root, k, &n);
		if (!n) break;
		if (n->value) {
			*value = n->value;
			return n->key;
		}
		k = n->key;
	}
	return NULL;
}

bool ht_dtree::insert(ht_data *key, ht_data *value)
{
	if ((!key) || (!value)) return false;
	ht_tree_node **n=&root;
	while (*n) {
		int c=compare_keys(key, (*n)->key);
		if (c>0) n=&(*n)->right; else
		if (c<0) n=&(*n)->left; else
			break;
	}
	if (*n) {
		if ((*n)->value) return false;
		(*n)->key->done();
		delete (*n)->key;
		(*n)->key=key;
		(*n)->value=value;
		dead_node_count--;
	} else {
		*n=new ht_tree_node();
		(*n)->key=key;
		(*n)->value=value;
		(*n)->left=NULL;
		(*n)->right=NULL;
		node_count++;
	}
	if (max_ub_insert) {
		if (++ub_insert>max_ub_insert) balance();
	}
	return true;
}

/*
 *	CLASS ht_list
 */

void ht_list::init(compare_keys_func_ptr _compare_keys)
{
	ht_data::init();
	compare_keys=_compare_keys;
}

void ht_list::done()
{
	ht_data::done();
}

void ht_list::destroy()
{
	ht_list::done();
}

void ht_list::append(ht_data *data)
{
}

UINT ht_list::count()
{
	return 0;
}

void ht_list::copy_to(UINT i, UINT count, ht_list *destlist)
{
	UINT j=0;
	while (count--) {
		ht_data *w=get(i+j);
		if (j) {
			destlist->insert_after(w, j-1);
		} else {
			destlist->insert(w);
		}
		j++;
	}
}

ht_list *ht_list::cut(UINT i, UINT count)
{
	return NULL;
}

bool ht_list::del(UINT i)
{
	return false;
}

bool ht_list::del_multiple(UINT i, UINT count)
{
	bool b=true;
	while (count--) {
		b&=del(i);
	}
	return b;
}

void ht_list::empty()
{
}

UINT ht_list::find(ht_data *data)
{
	return LIST_UNDEFINED;
}

ht_data *ht_list::get(UINT i)
{
	return NULL;
}

void ht_list::insert(ht_data *data)
{
}

void ht_list::insert_after(ht_data *data, UINT i)
{
}

void ht_list::insert_before(ht_data *data, UINT i)
{
}

void ht_list::move(UINT source, UINT dest)
{
}

void ht_list::move_multiple(UINT source, UINT dest, UINT count)
{
}

void ht_list::prepend(ht_data *data)
{
}

ht_data *ht_list::remove(UINT i)
{
	return NULL;
}

bool ht_list::remove_multiple(UINT i, UINT count)
{
	bool b=true;
	while (count--) {
		b&=(remove(i)!=NULL);
	}
	return b;
}

bool ht_list::set(UINT i, ht_data *data)
{
	return false;
}

bool ht_list::sort()
{
	return false;
}

/*
 *	CLASS ht_clist
 */

#define HT_CLIST_ENTRY_COUNT_START		32

// exponential growth factor: num/den
#define HT_CLIST_ENTRY_COUNT_EXT_NUM	3
#define HT_CLIST_ENTRY_COUNT_EXT_DEN	2

void ht_clist::init(compare_keys_func_ptr compare_keys)
{
	ht_list::init(compare_keys);
	c_size=HT_CLIST_ENTRY_COUNT_START;
	items=(ht_data**)malloc(c_size * sizeof (ht_data*));
	c_entry_count=0;
}

void ht_clist::done()
{
	free(items);
	ht_list::done();
}

void ht_clist::destroy()
{
	empty();
	ht_clist::done();
}

void ht_clist::append(ht_data *data)
{
	insert_before(data, c_entry_count);
}

UINT ht_clist::count()
{
	return c_entry_count;
}

ht_list *ht_clist::cut(UINT start, UINT count)
{
	ht_clist *c=new ht_clist();
	c->init(compare_keys);
	for (UINT i=0; i<count; i++) {
		c->insert(items[start+i]);
	}
	remove_multiple(start, count);
	return c;
}

bool ht_clist::del(UINT i)
{
	if (i<c_entry_count) {
		do_free(i);
		do_remove(i);
		return true;
	}
	return false;
}

void ht_clist::do_free(UINT i)
{
	if (items[i]) {
		items[i]->done();
		delete items[i];
	}
}

void ht_clist::do_remove(UINT i)
{
	memmove(&items[i], &items[i+1], sizeof items[i] * (c_entry_count-i-1));
	c_entry_count--;
}

Object *ht_clist::duplicate()
{
	ht_clist *d=new ht_clist();
	d->init(compare_keys);
	for (UINT i=0; i<c_entry_count; i++) {
		d->insert(items[i]->duplicate());
	}
	return d;
}

void ht_clist::empty()
{
	for (UINT i=0; i<c_entry_count; i++) {
		do_free(i);
	}
	c_entry_count=0;
}

void ht_clist::extend_list()
{
	c_size *= HT_CLIST_ENTRY_COUNT_EXT_NUM;
	c_size /= HT_CLIST_ENTRY_COUNT_EXT_DEN;
	ht_data **new_items=(ht_data**)malloc(c_size * sizeof (ht_data*));
	memmove(new_items, items, c_entry_count * sizeof (ht_data*));
	free(items);
	items=new_items;
}

UINT ht_clist::find(ht_data *data)
{
	if (compare_keys) {
		for (UINT i=0; i<c_entry_count; i++) {
			if (compare_keys(data, items[i])==0) return i;
		}
	}
	return LIST_UNDEFINED;
}

ht_data *ht_clist::get(UINT i)
{
	if (i<c_entry_count) return items[i];
	return NULL;
}

void ht_clist::insert(ht_data *data)
{
	append(data);
}

void ht_clist::insert_after(ht_data *data, UINT i)
{
	insert_before(data, i+1);
}

void ht_clist::insert_before(ht_data *data, UINT i)
{
	UINT hi= i>c_entry_count ? i+1 : c_entry_count+1;
	while (hi>=c_size) extend_list();
	if (i>c_entry_count) {
		c_entry_count=i+1;
	} else {
		memmove(items+i+1, items+i, sizeof items[0] * (c_entry_count-i));
		c_entry_count++;
	}		
	items[i]=data;
}

int  ht_clist::load(ht_object_stream *s)
{
	c_size=HT_CLIST_ENTRY_COUNT_START;
	items=(ht_data**)malloc(c_size * sizeof (ht_data*));
	c_entry_count=0;
	
	void *d=find_atom(s->getIntHex(4, NULL));
//	if (!d) return 1;
	compare_keys=(compare_keys_func_ptr)d;
	
	int c=s->getIntDec(4, "item_count");
	for (int i=0; i<c; i++) {
		ht_data *d=s->getObject("item");
		if (s->get_error()) break;
		prepend(d);
	}
	return s->get_error();
}

void ht_clist::move(UINT source, UINT dest)
{
	if (dest<=c_entry_count) {
		ht_data *src=get(source);
		memmove(items+source, items+source+1, sizeof items[0] * (c_entry_count-source-1));
		memmove(items+dest+1, items+dest, sizeof items[0] * (c_entry_count-dest-1));
		items[dest]=src;
	}
}

void ht_clist::move_multiple(UINT source, UINT dest, UINT count)
{
	if (dest<=c_entry_count) {
		for (UINT i=0; i<count; i++) {
			move(source, dest+count-1);
		}
	}
}

OBJECT_ID ht_clist::object_id() const
{
	return ATOM_HT_CLIST;
}

void ht_clist::prepend(ht_data *data)
{
	insert_before(data, 0);
}

ht_data *ht_clist::remove(UINT i)
{
	if (i<c_entry_count) {
		ht_data *d=items[i];
		do_remove(i);
		return d;
	}
	return NULL;
}

bool ht_clist::set(UINT i, ht_data *data)
{
	while (i>=c_entry_count) append(NULL);
	do_free(i);
	items[i] = data;
	return true;
}

bool ht_clist::sort()
{
	if (compare_keys && (c_entry_count>1)) {
		qsort_compare_compare_keys = compare_keys;
		qsort(items, c_entry_count, sizeof *items, qsort_compare);
	}
	return false;
}

void ht_clist::store(ht_object_stream *s)
{
	s->putIntHex(find_atom_rev((void*)compare_keys), 4, NULL);
	s->putIntDec(c_entry_count, 4, "item_count");
	for (int i=c_entry_count-1; i>=0; i--) {
		s->putObject(items[i], "item");
	}
}

/*

!!! THERE'S A BUG IN HERE !!!

bool ht_clist::qsort_i(UINT _l, UINT _r)
{
	int xchg_count=0;
	int l=(int)_l;
	int r=(int)_r;
	int m=(l+r) / 2;
	int origl=l;
	int origr=r;
	ht_data *c;
	c=items[m];
	do {
		while ((l<m) && compare_keys(items[l], c) < 0) l++;
		while ((r>m) && compare_keys(items[r], c) > 0) r--;
		if (l < r) {
			ht_data *t;
			t=items[l];
			items[l]=items[r];
			items[r]=t;
			l++;
			r--;
			xchg_count++;
		} else if (l==r) {
			l++;
			r--;
		}
	} while (l < r);
	if (origl < r) qsort_i(origl, r);
	if (l < origr) qsort_i(l, origr);
	return (xchg_count!=0);
}*/

/*
 *	CLASS ht_sorted_list
 */

void ht_sorted_list::init(compare_keys_func_ptr compare_keys)
{
	ht_clist::init(compare_keys);
}

void ht_sorted_list::done()
{
	ht_clist::done();
}

void ht_sorted_list::append(ht_data *data)
{
	insert(data);
}

UINT ht_sorted_list::find(ht_data *data)
{
	if (compare_keys) {
		HT_ERROR("function untested !");
		UINT w=(c_entry_count-1)/2, ow=0;
		while (w!=ow) {
			ow=w;
			int ck=compare_keys(data, items[w]);
			if (ck==0) return w;
			if (ck>0) w+=c_entry_count-1;
			w>>=1;
		}
	}
	return LIST_UNDEFINED;
}

void ht_sorted_list::insert(ht_data *data)
{
	if (c_entry_count) {
		int l=0, r=c_entry_count-1;
		int m=(l+r+1)/2;
		do {
			int cmp=compare_keys(data, items[m]);
			if (cmp>0) l=m+1; else
			if (cmp<0) r=m-1; else break;
			m=(l+r+1)/2;
		} while (l<=r);
		ht_clist::insert_before(data, m);
	} else {
		ht_clist::insert_before(data, 0);
	}
}

void ht_sorted_list::insert_after(ht_data *data, UINT i)
{
	insert(data);
}

void ht_sorted_list::insert_before(ht_data *data, UINT i)
{
	insert(data);
}

void ht_sorted_list::move(UINT source, UINT dest)
{
	// do nothing
}

void ht_sorted_list::move_multiple(UINT source, UINT dest, UINT count)
{
	// do nothing
}

void ht_sorted_list::prepend(ht_data *data)
{
	insert(data);
}

bool ht_sorted_list::set(UINT i, ht_data *data)
{
	insert(data);
	return true;
}

/*
 *	CLASS ht_stack
 */

ht_data *ht_stack::pop()
{
	if (c_entry_count) {
		ht_data *d=get(c_entry_count-1);
		do_remove(c_entry_count-1);
		return d;
	}
	return NULL;
}

void ht_stack::push(ht_data *data)
{
	append(data);
}

/*
 *	CLASS ht_queue
 */

void	ht_queue::enqueue(ht_data *data)
{
	append(data);
}

ht_data *ht_queue::dequeue()
{
	if (c_entry_count) {
		ht_data *d=get(0);
		do_remove(0);
		return d;
	}
	return NULL;
}

ht_data *ht_queue::pop()
{
	return dequeue();
}

void	ht_queue::push(ht_data *data)
{
	enqueue(data);
}

/*
 *   matchhash
 */

char *matchhash(int value, int_hash *hash_table)
{
	if (hash_table) {
		while (hash_table->desc) {
			if (hash_table->value==value) return hash_table->desc;
			hash_table++;
		}
	}
	return NULL;
}

/*
 *	compare procedures
 */

int compare_keys_ht_data(ht_data *key_a, ht_data *key_b)
{
	return key_a->compareTo(key_b);
}

int compare_keys_int(ht_data *key_a, ht_data *key_b)
{
	int a=((ht_data_uint*)key_a)->value;
	int b=((ht_data_uint*)key_b)->value;
	if (a>b) return 1; else if (a<b) return -1;
	return 0;
}

int compare_keys_uint(ht_data *key_a, ht_data *key_b)
{
	UINT a=((ht_data_uint*)key_a)->value;
	UINT b=((ht_data_uint*)key_b)->value;
	if (a>b) return 1; else if (a<b) return -1;
	return 0;
}

/*
 *	INIT
 */
 
BUILDER(ATOM_HT_DATA_UINT, ht_data_uint);
BUILDER(ATOM_HT_DATA_DWORD, ht_data_dword);
BUILDER(ATOM_HT_DATA_MEM, ht_data_mem);
BUILDER(ATOM_HT_STREE, ht_stree);
BUILDER(ATOM_HT_CLIST, ht_clist);

bool init_data()
{
	REGISTER(ATOM_HT_DATA_UINT, ht_data_uint);
	REGISTER(ATOM_HT_DATA_DWORD, ht_data_dword);
	REGISTER(ATOM_HT_DATA_MEM, ht_data_mem);
	REGISTER(ATOM_HT_STREE, ht_stree);
	REGISTER(ATOM_HT_CLIST, ht_clist);
	
	register_atom(ATOM_COMPARE_KEYS_HT_DATA, (void*)compare_keys_ht_data);
	register_atom(ATOM_COMPARE_KEYS_INT, (void*)compare_keys_int);
	register_atom(ATOM_COMPARE_KEYS_UINT, (void*)compare_keys_uint);
	
	return true;
}

/*
 *	DONE
 */

void done_data()
{
	unregister_atom(ATOM_COMPARE_KEYS_HT_DATA);
	unregister_atom(ATOM_COMPARE_KEYS_INT);
	unregister_atom(ATOM_COMPARE_KEYS_UINT);
	
	UNREGISTER(ATOM_HT_DATA_UINT, ht_data_uint);
	UNREGISTER(ATOM_HT_DATA_DWORD, ht_data_dword);
	UNREGISTER(ATOM_HT_DATA_MEM, ht_data_mem);
	UNREGISTER(ATOM_HT_STREE, ht_stree);
	UNREGISTER(ATOM_HT_CLIST, ht_clist);
}

