/*
 *	HT Editor
 *	htpal.cc
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

#include "global.h"
#include "htatom.h"
#include "htcurses.h"
#include "htctrl.h"
#include "htdialog.h"
#include "htpal.h"
#include "htreg.h"
#include "htstring.h"
#include "snprintf.h"
#include "tools.h"

// include indices
#include "httag.h"
#include "syntax.h"
#include "out_ht.h"
//

#include <stdlib.h>
#include <string.h>

#define ATOM_PALETTE_ENTRY			MAGICD("PAL\x00")
#define ATOM_HT_CREATE_PALETTE_ENTRY	MAGICD("PAL\x10")

struct pal_layout {
	int idx;
	char *name;
};

/* palette layout: tags */
pal_layout pal_layout_tags[]={
{palidx_tags_edit_tag_cursor_select,	"edit-tag cursor select"},
{palidx_tags_edit_tag_cursor_edit,		"edit-tag cursor edit"},
{palidx_tags_edit_tag_cursor_unfocused,	"edit-tag cursor unfocused"},
{palidx_tags_edit_tag_selected,		"edit-tag selected"},
{palidx_tags_edit_tag_modified,		"edit-tag modified"},
{palidx_tags_edit_tag,				"edit-tag"},
{palidx_tags_sel_tag_cursor_focused,	"sel-tag cursor focused"},
{palidx_tags_sel_tag_cursor_unfocused,	"sel-tag cursor unfocused"},
{palidx_tags_sel_tag,				"sel-tag"},
{0, NULL}
};

/* palette layout: generic */
pal_layout pal_layout_generic[] = {
{palidx_generic_body,				"body"},

{palidx_generic_text_focused,			"text focused"},
{palidx_generic_text_unfocused,		"text unfocused"},
{palidx_generic_text_shortcut,		"text shortcut"},
{palidx_generic_text_shortcut_selected,	"text shortcut selected"},
{palidx_generic_text_selected,		"text selected"},
{palidx_generic_text_disabled,		"text disabled"},

{palidx_generic_frame_focused,		"frame focused"},
{palidx_generic_frame_unfocused,		"frame unfocused"},
{palidx_generic_frame_move_resize,		"frame move-resize"},
{palidx_generic_frame_killer,			"frame killer"},

{palidx_generic_scrollbar,			"scrollbar"},

{palidx_generic_input_focused,		"input focused"},
{palidx_generic_input_unfocused,		"input unfocused"},
{palidx_generic_input_selected,		"input selected"},
{palidx_generic_input_clip,			"input clip-chars"},

{palidx_generic_button_focused,		"button focused"},
{palidx_generic_button_unfocused,		"button unfocused"},
{palidx_generic_button_shadow,		"button shadow"},
{palidx_generic_button_shortcut,		"button shortcut"},

{palidx_generic_list_focused_selected, 	"list focused & selected"},
{palidx_generic_list_focused_unselected, "list focused & unselected"},
{palidx_generic_list_unfocused_selected, "list unfocused & selected"},
{palidx_generic_list_unfocused_unselected, "list unfocused & unselected"},

{palidx_generic_cluster_focused,		"cluster focused"},
{palidx_generic_cluster_unfocused,		"cluster unfocused"},
{palidx_generic_cluster_shortcut,		"cluster shortcut"},

{0, NULL}
};

/* palette layout: syntax */
pal_layout pal_layout_syntax[] = {
{palidx_syntax_whitespace,			"whitespace"},
{palidx_syntax_comment,				"comment"},
{palidx_syntax_identifier,			"identifier"},
{palidx_syntax_reserved,				"reserved"},
{palidx_syntax_intnum,				"integer number"},
{palidx_syntax_floatnum,				"float number"},
{palidx_syntax_string,				"string"},
{palidx_syntax_char,				"character"},
{palidx_syntax_symbol,				"symbol"},
{palidx_syntax_preprocess,			"preprocess"},
{palidx_syntax_meta, 				"meta"}
};

/* palette layout: analyser */
pal_layout pal_layout_analyser[] = {
{palidx_analyser_default,			"default"},
{palidx_analyser_comment,			"comment"},
{palidx_analyser_label,				"label"},
{palidx_analyser_number,				"number"},
{palidx_analyser_string,				"string"},
{palidx_analyser_symbol,				"symbol-character"},
};

/* all layouts */

struct pal_class {
	pal_layout *layout;
	char *name;
};

pal_class pal_layouts[] =
{
{pal_layout_generic,	"generic"},
{pal_layout_tags,		"tags"},
{pal_layout_syntax,		"syntax"},
{pal_layout_analyser,	"analyser"},
{NULL, NULL}
};

/*
 *   reg/pal management
 */

vcp getcolorv(palette *pal, uint index)
{
	if ((index<pal->size) && (pal->data)) return pal->data[index];
	return VCP(VC_WHITE, VC_RED);
}

pal_layout *find_pal_layout(pal_class *layouts, char *pal_class, int *lsize)
{
	pal_layout *pl = NULL;
	while (layouts->layout && layouts->name) {
		if (strcmp(layouts->name, pal_class) == 0) {
			pl = layouts->layout;
			break;
		}
		layouts++;
	}
	int s = 0;
	if (pl) {
		pal_layout *p = pl;
		while (p->name) {
			p++;
			s++;
		}
	}
	if (pl) *lsize = s;
	return pl;
}

int find_pal_entry_idx(pal_layout *layout, const char *name)
{
	while (layout->name) {
		if (strcmp(layout->name, name) == 0) return layout->idx;
		layout++;
	}
	return -1;
}

bool load_pal(char *pal_class, char *pal_flavour, palette *p)
{
	if ((!pal_flavour) || (!pal_class)) return false;
	char dir[256];		/* secure */
	ht_snprintf(dir, sizeof dir, "%s/%s/%s", palettekey, pal_class, pal_flavour);

	int psize = 0;
	pal_layout *pl = find_pal_layout(pal_layouts, pal_class, &psize);
	if (!pl) return false;
	p->size = psize;
	p->data = (vcp*)malloc(sizeof *p->data * psize);

	for (int i=0; i < psize; i++) p->data[i] = VCP(VC_WHITE, VC_RED);

	ht_registry_node_type t;
	palette_entry *d;
	const char *n = NULL;
	ht_registry_node_type rnt_pal = registry->lookup_node_type(rnt_palette_name);
	while ((n = registry->enum_next((ht_registry_data **)&d, &t, dir, n))) {
		if (t == rnt_pal) {
			int idx = find_pal_entry_idx(pl, n);
			if ((idx != -1) && (idx < psize)) {
				p->data[idx] = d->color;
			}
		}
	}
	return true;
}

/*
 *	CLASS palette_entry
 */
 
palette_entry::palette_entry(uint _idx, vcp _color)
{
	idx=_idx;
	color=_color;
}

bool palette_entry::editdialog(const char *keyname)
{
	bool r=0;
	bounds b;
	b.w=50;
	b.h=15;
	b.x=(screen->size.w-b.w)/2;
	b.y=(screen->size.h-b.h)/2;
	
	ht_dialog *d=new ht_dialog();
	d->init(&b, "edit palette entry", FS_TITLE | FS_KILLER);
	
	ht_color_block *fgc, *bgc;
	ht_label *l1, *l2;
	
	BOUNDS_ASSIGN(b, 2, 1, 16, 5);
	fgc=new ht_color_block();
	fgc->init(&b, VCP_FOREGROUND(color), cf_transparent | cf_light);
	d->insert(fgc);

	BOUNDS_ASSIGN(b, 2, 0, 16, 1);
	l1 = new ht_label();
	l1->init(&b, "~foreground", fgc);
	d->insert(l1);
	
	BOUNDS_ASSIGN(b, 20, 1, 16, 5);
	bgc=new ht_color_block();
	bgc->init(&b, VCP_BACKGROUND(color), cf_transparent | cf_light);
	d->insert(bgc);
	
	BOUNDS_ASSIGN(b, 20, 0, 16, 1);
	l2 = new ht_label();
	l2->init(&b, "~background", bgc);
	d->insert(l2);

	if (d->run(false)) {
		ht_color_block_data fgd, bgd;
		fgc->databuf_get(&fgd, sizeof fgd);
		bgc->databuf_get(&bgd, sizeof bgd);
		color=VCP(fgd.color, bgd.color);
		r=1;
	}
	
	d->done();
	delete d;
	return r;
}

int palette_entry::load(ObjectStream &f)
{
	idx=f->getIntHex(4, 0);
	color=f->getIntHex(4, 0);
	return f->get_error();
}

ObjectID palette_entry::getObjectID() const
{
	return ATOM_PALETTE_ENTRY;
}

void palette_entry::store(ObjectStream &f)
{
	f->putIntHex(idx, 4, 0);
	f->putIntHex(color, 4, 0);
}

void palette_entry::strvalue(char *buf32bytes)
{
	char *p=buf32bytes;
	char *text;
	int fg=VCP_FOREGROUND(color);
	int bg=VCP_BACKGROUND(color);
	if ((fg==VC_TRANSPARENT) && (bg==VC_TRANSPARENT)) {
		text="transparent";
		fg=VC_WHITE;
		bg=VC_BLACK;
	} else if (fg==VC_TRANSPARENT) {
		text="fgtrans";
		fg=VC_WHITE;
		if (bg==fg) fg=VC_BLACK;
	} else if (bg==VC_TRANSPARENT) {
		text="bgtrans";
		bg=VC_BLACK;
		if (bg==fg) fg=VC_WHITE;
	} else {
		text="normal";
	}
	p=tag_make_color(p, VCP(fg, bg));
	p+=sprintf(p, text);
	p=tag_make_default_color(p);
	*p=0;
}

ht_registry_data *create_empty_palette_entry()
{
	return new palette_entry();
}

/*
 *	INIT
 */

BUILDER(ATOM_PALETTE_ENTRY, palette_entry);

bool init_pal()
{
	REGISTER(ATOM_PALETTE_ENTRY, palette_entry);
	register_atom(ATOM_HT_CREATE_PALETTE_ENTRY, (void*)create_empty_palette_entry);

	return true;
}

/*
 *	DONE
 */

void done_pal()
{
	UNREGISTER(ATOM_PALETTE_ENTRY, palette_entry);
	unregister_atom(ATOM_HT_CREATE_PALETTE_ENTRY);
}

