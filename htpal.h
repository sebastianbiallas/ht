/* 
 *	HT Editor
 *	htpal.h
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

#ifndef __HTPAL_H__
#define __HTPAL_H__

#include "data.h"
#include "htobj.h"
#include "htreg.h"

#define rnt_palette_name		"palette"

struct defpal {
	int idx;
	const char *name;
	int color;
};

/*
 *	CLASS palette_entry
 */

class palette_entry: public ht_registry_data {
public:
	uint idx;
	vcp color;

		palette_entry(uint idx=0, vcp color=0);
		palette_entry(BuildCtorArg&a): ht_registry_data(a) {};
/* overwritten */
	virtual	bool editdialog(const char *keyname);
	virtual void strvalue(char *buf32bytes);
	virtual	void load(ObjectStream &f);
	virtual	ObjectID getObjectID() const;
	virtual	void store(ObjectStream &f) const;
};

/*
 *	palette_key
 */

#define palettekey "palette"

/*
 *   palette class keys
 */

#define palclasskey_generic					"generic"
#define palclasskey_tags	     				"tags"
#define palclasskey_syntax	     				"syntax"

/*
 *	generic palette
 */

#define palkey_generic_window_default				"window"
#define palkey_generic_dialog_default				"dialog"
#define palkey_generic_menu_default				"menu"
#define palkey_generic_keys_default				"keyline"
#define palkey_generic_desktop_default				"desktop"
#define palkey_generic_help_default				"help"

#define palkey_generic_cyan					"cyan"
#define palkey_generic_black					"black"
#define palkey_generic_blue					"blue"
#define palkey_generic_gray					"gray"
#define palkey_generic_special					"special"

#define palidx_generic_body					0
#define palidx_generic_text_focused				1
#define palidx_generic_text_unfocused				2
#define palidx_generic_text_shortcut				3
#define palidx_generic_text_shortcut_selected			4
#define palidx_generic_text_selected				5
#define palidx_generic_text_disabled				6
#define palidx_generic_frame_focused				7
#define palidx_generic_frame_unfocused				8
#define palidx_generic_frame_move_resize			9
#define palidx_generic_frame_killer				10
#define palidx_generic_scrollbar				11
#define palidx_generic_input_focused				12
#define palidx_generic_input_unfocused				13
#define palidx_generic_input_selected				14
#define palidx_generic_input_clip 				15
#define palidx_generic_button_focused				16
#define palidx_generic_button_unfocused				17
#define palidx_generic_button_shadow 				18
#define palidx_generic_button_shortcut				19
#define palidx_generic_list_focused_selected			20
#define palidx_generic_list_focused_unselected			21
#define palidx_generic_list_unfocused_selected			22
#define palidx_generic_list_unfocused_unselected		23
#define palidx_generic_cluster_focused				24
#define palidx_generic_cluster_unfocused			25
#define palidx_generic_cluster_shortcut				26

/*
 *   reg/pal management
 */

vcp getcolorv(palette *pal, uint index);
bool load_pal(const char *pal_class, const char *pal_flavour, palette *p);

/*
 *	INIT
 */

bool init_pal();

/*
 *	DONE
 */

void done_pal();

#endif /* !__HTPAL_H__ */
