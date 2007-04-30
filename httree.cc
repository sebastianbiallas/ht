/* 
 *	HT Editor
 *	httree.cc
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

#include <stdlib.h>

#include "string.h"

#include "htdialog.h"
#include "htpal.h"
#include "keyb.h"
#include "strtools.h"
#include "httree.h"
#include "stream.h"

void ht_treeview::init(Bounds *b, const char *d)
{
	ht_view::init(b, VO_SELECTABLE | VO_BROWSABLE/* <- FIXME */ | VO_RESIZE, d);
	VIEW_DEBUG_NAME("ht_treeview");

	growmode = MK_GM(GMH_FIT, GMV_FIT);

	foc = 0;
	delta_x = delta_y = 0;
	maxsize_y = 5;
	selected = NULL;
}

void ht_treeview::done()
{
	ht_view::done();
}

void ht_treeview::adjust_focus(int Focus)
{
	if (Focus < 0) Focus = 0; else if (Focus >= maxsize_y) Focus = maxsize_y - 1;
	if (foc != Focus) has_focused(Focus);
	if (Focus < delta_y) scroll_to(delta_x, Focus);
		else if ((Focus - size.h) >= delta_y) scroll_to(delta_x, Focus - size.h + 1);
}

int  ht_treeview::create_graph(AbstractChar *s, void *node, int level, int lines, int width, int endwidth, const AbstractChar *Chars)
{
// Chars: space, vbar, T, last L,hbar,+,-,[,]
	for (int i=0; i < level; i++) {
		s[i*width] = (lines&(1<<i)) ? Chars[1] : Chars[0];
		for (int j=1; j<(width); j++) s[i*width+j] = Chars[0];
	}
	int p = level*width;
	s[p] = (get_next_node(node)) ? Chars[2]: Chars[3];
	if (has_children(node)) {
		s[p+1] = Chars[7];
		s[p+2] = (is_expanded(node)) ? Chars[6]: Chars[5];
		s[p+3] = Chars[8];
	} else {
		s[p+1] = Chars[4];
		s[p+2] = Chars[4];
		s[p+3] = Chars[4];
	}
	s[p+4].codepage = CP_DEVICE;
	s[p+4].chr = ' ';
	p += 5;
	char *text=get_text(node);
	while (*text) {
		s[p].codepage = CP_DEVICE;
		s[p++].chr = *(text++);
	}
	s[p].codepage = CP_INVALID;
	return p;
}

int	ht_treeview::count_children(void *node)
{
	int i = 0;
	if (!is_expanded(node)) return 0;
	void *p = get_child(node, 1);
	while (p) {
		i+=count_children(p);
		p = get_next_node(p);
		i++;
	}
	return i;
}

void ht_treeview::collapse_all(void *node)
{
	if (has_children(node)) {
		adjust(node, false);
		void *tmp = get_child(node, 1);
		while (tmp) {
			collapse_all(tmp);
			tmp = get_next_node(tmp);
		}
	}
}

void ht_treeview::draw_r(void *node, int level, int *pos, uint32 lines)
{
	vcp normal_color, sel_color, foc_color, color;
	normal_color = getcolor(palidx_generic_list_unfocused_unselected);
	foc_color = getcolor(palidx_generic_list_focused_selected);
	sel_color = getcolor(palidx_generic_list_unfocused_selected);
	AbstractChar s[1024];

	while (node) {
		if (*pos >= delta_y) {
			if (*pos >= (delta_y+size.h)) break;
			if (is_selected(*pos)) {
				selected = node; // only for getdata()
				if (focused) color=foc_color; else color=sel_color;
				fill(0, *pos-delta_y, size.w, 1, color, ' ');
			} else color = normal_color;
			if (get_graph(s, node, level, lines) > delta_x)
				buf->nprintW(0, *pos-delta_y, color, &s[delta_x], size.w);
		}
		(*pos)++;
		if (has_children(node) && is_expanded(node)) {
			draw_r(get_child(node, 1), level+1, pos, (get_next_node(node)) ? lines|(1<<level) : lines);
		}
		node = get_next_node(node);
	}
}

void ht_treeview::draw()
{
	clear(getcolor(palidx_generic_list_unfocused_unselected));

	int p = 0;
	draw_r(get_root(), 0, &p, 0);
}

void ht_treeview::expand_all(void *node)
{
	if (has_children(node)) {
		adjust(node, true);
		void *tmp = get_child(node, 1);
		while (tmp) {
			expand_all(tmp);
			tmp = get_next_node(tmp);
		}
	}
}

/*
 *	stub
 */
void ht_treeview::getdata(ObjectStream &s)
{
	PUT_INT32D(s, (long)selected);
}

/*
 *	void *ht_treeview::get_child(void *node, int i)
 *	get i. child of node
 */

int  ht_treeview::get_graph(AbstractChar *s, void *node, int level, int lines)
{
	static const AbstractChar graph[10] = {
		{CP_DEVICE, ' '},
		{CP_GRAPHICAL, GC_1VLINE},
		{CP_GRAPHICAL, GC_1RTEE},
		{CP_GRAPHICAL, GC_1CORNER2},
		{CP_GRAPHICAL, GC_1HLINE},
		{CP_DEVICE, '+'},
		{CP_DEVICE, '-'},
		{CP_DEVICE, '['},
		{CP_DEVICE, ']'},
	};
	return create_graph(s, node, level, lines, 5, 5, graph);
}

void *ht_treeview::get_node_r(void *node, int *i)
{
	while ((node) && (--(*i))) {
		if (has_children(node) && is_expanded(node)) {
			void *tmp = get_node_r(get_child(node, 1), i);
			if (!(*i)) return tmp;
		}
		node = get_next_node(node);
	}
	return node;
}

/*
 *   get i. node
 *	i==1 => 1. node
 */
void *ht_treeview::get_node(int i)
{
	if (i <= 0) return 0;
	void *Node = get_root();
	return get_node_r(Node, &i);
}

void	ht_treeview::handlemsg(htmsg *msg)
{
	ht_view::handlemsg(msg);
	if (msg->msg==msg_keypressed) {
		int Foc = foc;
		switch (msg->data1.integer) {
		case K_Up:
			Foc--;
			break;
		case K_Down:
			Foc++;
			break;
		case K_Control_Right:
			if (delta_x < maxsize_x-1) delta_x++;
			break;
		case K_Control_Left:
			if (delta_x > 0) delta_x--;
			break;
		case K_PageUp:
			Foc -= size.h-1;
			break;
		case K_PageDown:
			Foc += size.h-1;
			break;
		case K_Control_PageUp:
			Foc = 0;
			break;
		case K_Control_PageDown:
			Foc = maxsize_y - 1;
			break;
		case '+':
		case K_Right: {
			void *p = get_node(Foc+1);
			if (has_children(p)) adjust(p, true);
			break;
		}
		case '-':
			adjust(get_node(Foc+1), false);
			break;
		case K_Left: {
			void *n = get_node(Foc+1);
			if (is_expanded(n)) {
				adjust(n, false);
			} else {
				if (Foc) {
					do {
						Foc-=count_children(n)+1;
						n = get_prev_node(n);
					} while (n);
					adjust(get_node(Foc+1), false);
				}
			}
			break;
		}
		case '*':
			expand_all(get_node(Foc+1));
			break;
		case '/':
			collapse_all(get_node(Foc+1));
			break;
		case K_Return: {
			void *n = get_node(Foc+1);
			if (has_children(n)) {
				adjust(n, !is_expanded(n));
			} else {
				select_node(n);
			}
			break;
		}
		default:
			return;
		}
		update();
		adjust_focus(Foc);
		dirtyview();
		clearmsg(msg);
	}
}

void	ht_treeview::has_focused(int i)
{
	foc = i;
}

/*
 *	can be overwritten to handle mutiple selections
 */
bool	ht_treeview::is_selected(int i)
{
	return (foc == i);
}

void	ht_treeview::scroll_to(int x, int y)
{
	delta_x = x;
	delta_y = y;
}

/*
 *	stub
 */
void ht_treeview::setdata(ObjectStream &s)
{
}

/*
 *	called whenever a node is being selected
 *	can be overwritten
 */
void	ht_treeview::select_node(void *node)
{
}

void	ht_treeview::set_limit(int x, int y)
{
	maxsize_x = x;
	maxsize_y = y;
}

void	ht_treeview::update_r(void *node, int level, int *pos, int *x)
{
	AbstractChar s[2048];

	while (node) {
		int l = get_graph(s, node, level, 0);
		if (l > *x) *x = l;
		(*pos)++;
		if (has_children(node) && is_expanded(node)) {
			update_r(get_child(node, 1), level+1, pos, x);
		}
		node = get_next_node(node);
	}
}

/*
 *   must be called whenever the data of the tree has changed
 */
void	ht_treeview::update()
{
	int count = 0, maxx = 0;
	update_r(get_root(), 0, &count, &maxx);
	set_limit(maxx, count);
	adjust_focus(foc);
}

/****************************************************************************/

void ht_static_treeview::init(Bounds *b, const char *desc)
{
	ht_treeview::init(b, desc);
	VIEW_DEBUG_NAME("ht_static_treeview");
	root=0;
}

void ht_static_treeviewdone_r(static_node *node)
{
	while (node) {
		ht_static_treeviewdone_r(node->child);
		free(node->text);
		if (node->data) {
			node->data->done();
			delete node->data;
		}
		static_node *temp = node->next;
		free(node);
		node = temp;
	}
}

void ht_static_treeview::done()
{
	ht_static_treeviewdone_r(root);
	ht_treeview::done();
}

void *ht_static_treeview::add_child(void *node, const char *text, Object *Data)
{
	if (node) {
		return add_node(&((static_node *)node)->child, text, Data);
	} else {
		if (root) {
			return add_node(&root, text, Data);
		} else {
			root=create_node(text, NULL, Data);
			return root;
		}
	}
}

void	*ht_static_treeview::add_node(static_node **node, const char *text, Object *Data)
{
	static_node **p = node;
	static_node *prev = NULL;
	if (*p) {
		while (*p) {
			prev = *p;
			p = &(*p)->next;
		}
	}
	*p=create_node(text, prev, Data);
	return *p;
}

void	ht_static_treeview::adjust(void *node, bool expand)
{
	((static_node *)node)->expanded = expand;
}

static_node *ht_static_treeview::create_node(const char *text, static_node *prev, Object *Data)
{
	static_node *node = ht_malloc(sizeof(static_node));
	node->text = ht_strdup(text);
	node->next = NULL;
	node->prev = prev;
	node->child = NULL;
	node->expanded = false;
	node->data = Data;
	return node;
}

void *ht_static_treeview::get_child(void *node, int i)
{
	static_node *p;
	if (node) {
		p = ((static_node *)node)->child;
	} else {
		p = root;
	}
	while (p && (--i)) p = p->next;
	return p;
}

void	*ht_static_treeview::get_next_node(void *node)
{
	return ((static_node *)node)->next;
}

void	*ht_static_treeview::get_prev_node(void *node)
{
	return ((static_node *)node)->prev;
}

void	*ht_static_treeview::get_root()
{
	return root;
}

char	*ht_static_treeview::get_text(void *node)
{
	return ((static_node *)node)->text;
}

void	*ht_static_treeview::get_cursor_node()
{
// FIXME: said to be the wrong thing, works however
	return selected;
}

void	ht_static_treeview::goto_node(void *node)
{
}

bool	ht_static_treeview::has_children(void *node)
{
	return (((static_node *)node)->child);
}

bool	ht_static_treeview::is_expanded(void *node)
{
	return (((static_node *)node)->expanded);
}

void	ht_static_treeview::select_node(void *node)
{
}

