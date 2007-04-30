/*
 *	HT Editor
 *	htxex.cc
 *
 *	Copyright (C) 2006 Sebastian Biallas (sb@biallas.net)
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

#include <cstdlib>
#include <cstring>

#include "log.h"
#include "endianess.h"
#include "htxex.h"
#include "htxexhead.h"
#include "htxeximg.h"
#include "stream.h"
#include "tools.h"

static format_viewer_if *htxex_ifs[] = {
	&htxexheader_if,
	&htxeximage_if,
	0
};

static ht_view *htxex_init(Bounds *b, File *file, ht_format_group *format_group)
{
	byte xexmagic[4];

	file->seek(0);
	file->read(xexmagic, 4);
	if (xexmagic[0]!=XEX_MAGIC0 || xexmagic[1]!=XEX_MAGIC1 ||
	    xexmagic[2]!=XEX_MAGIC2 || xexmagic[3]!=XEX_MAGIC3) return 0;

	ht_xex *g=new ht_xex();
	g->init(b, file, htxex_ifs, format_group, 0);
	return g;
}

format_viewer_if htxex_if = {
	htxex_init,
	0
};

/*
 *	CLASS ht_xex
 */
void ht_xex::init(Bounds *b, File *file, format_viewer_if **ifs, ht_format_group *format_group, FileOfs header_ofs)
{
	ht_format_group::init(b, VO_BROWSABLE | VO_SELECTABLE | VO_RESIZE, DESC_XEX, file, false, true, 0, format_group);
	VIEW_DEBUG_NAME("ht_xex");

	ht_xex_shared_data *xex_shared = ht_malloc(sizeof (ht_xex_shared_data));
	shared_data = xex_shared;

	xex_shared->v_header = NULL;
	xex_shared->image = NULL;
	xex_shared->loader_info.type = XEX_LOADER_NONE;
	xex_shared->imports.ofs = 0;

	/* read header */
	file->seek(0);
	file->read(&xex_shared->header, sizeof xex_shared->header);
	createHostStruct(&xex_shared->header, XEX_IMAGE_HEADER_struct, big_endian);
	xex_shared->info_table = ht_malloc(xex_shared->header.number_of_sections * sizeof *xex_shared->info_table);
	xex_shared->info_table_cooked = ht_malloc(xex_shared->header.number_of_sections * sizeof *xex_shared->info_table_cooked);
	for (uint i=0; i < xex_shared->header.number_of_sections; i++) {
		file->read(xex_shared->info_table+i, sizeof *xex_shared->info_table);
		createHostStruct(xex_shared->info_table+i, XEX_IMAGE_HEADER_INFO_ENTRY_struct, big_endian);
		xex_shared->info_table_cooked[i].type = createHostInt(&xex_shared->info_table[i].type, 4, big_endian);
		xex_shared->info_table_cooked[i].start = 0;
		xex_shared->info_table_cooked[i].size = xex_shared->info_table[i].b.size;
		if (xex_shared->info_table[i].b.size == 0xff) {
			FileOfs ofs = file->tell();
			file->seek(xex_shared->info_table[i].value);
			uint32 s;
			if (file->read(&s, 4) == 4) {
				xex_shared->info_table_cooked[i].start = xex_shared->info_table[i].value + 4;
				xex_shared->info_table_cooked[i].size = createHostInt(&s, 4, big_endian) - 4;
			}
			file->seek(ofs);
		} else if (xex_shared->info_table[i].b.size > 1) {
			xex_shared->info_table_cooked[i].start = xex_shared->info_table[i].value;
			xex_shared->info_table_cooked[i].size = xex_shared->info_table[i].b.size * 4;
		}
	}

	for (uint i=0; i < xex_shared->header.number_of_sections; i++) {
		switch (xex_shared->info_table_cooked[i].type) {
		case XEX_HEADER_FIELD_LOADERINFO: {
			uint32 size = xex_shared->info_table_cooked[i].size;
			if (size > 4) {
				XEX_LOADER_INFO_HEADER ldhdr;
				file->seek(xex_shared->info_table_cooked[i].start);
				if (file->read(&ldhdr, sizeof ldhdr) == sizeof ldhdr) {
					size -= 4;
					createHostStruct(&ldhdr, XEX_LOADER_INFO_HEADER_struct, big_endian);
					if (!ldhdr.crypted && ldhdr.type == XEX_LOADER_RAW) {
						int entries = size / 8;
						xex_shared->loader_info.type = XEX_LOADER_RAW;
						xex_shared->loader_info.raw.sections = new Array(true, entries);
						for (int j=0; j < entries; j++) {
							XEX_RAW_LOADER_ENTRY e;
							if (file->read(&e, sizeof e) != sizeof e) break;
							createHostStruct(&e, XEX_RAW_LOADER_ENTRY_struct, big_endian);
							xex_shared->loader_info.raw.sections->insert(
								new XEXLoaderRawSection(e.raw, e.pad));
						}
					}
				}
			}
			break;
		}
		case XEX_HEADER_FIELD_IMPORT:
			xex_shared->imports.ofs = xex_shared->info_table[i].value;
			break;
		case XEX_HEADER_FIELD_ENTRY:
			xex_shared->entrypoint = xex_shared->info_table[i].value;
			break;
		case XEX_HEADER_FIELD_BASE:
			xex_shared->image_base = xex_shared->info_table[i].value;
			break;
		}
	}

	file->seek(xex_shared->header.file_header_offset);
	if (file->read(&xex_shared->file_header, sizeof xex_shared->file_header) == sizeof xex_shared->file_header) {
		createHostStruct(&xex_shared->file_header, XEX_FILE_HEADER_struct, big_endian);
		xex_shared->image_base = xex_shared->file_header.load_address;
		xex_shared->image_size = xex_shared->file_header.image_size;
		file->seek(xex_shared->header.file_header_offset+0x184);
		xex_shared->pages.page = new XexPage[xex_shared->file_header.pages];
		for (int i=0; i < xex_shared->file_header.pages; i++) {
			uint32 flags;
			file->read(&flags, 4);
			flags = createHostInt(&flags, 4, big_endian);
			xex_shared->pages.page[i].flags = flags;
			file->seek(file->tell()+20);
		}
		xex_shared->pages.page_shift = 16; // FIXME: can also be 12
	} else {
		xex_shared->pages.page = NULL;
		xex_shared->file_header.hdr_size = 0;
		xex_shared->image_size = 0;
		xex_shared->image_base = 0;
	}
	
	xex_shared->imports.lib_count = 0;
	xex_shared->imports.libs = NULL;
	try {
		if (xex_shared->imports.ofs) {
			file->seek(xex_shared->imports.ofs);
			uint32 size, sizen, l;
			file->readx(&size, 4);
	    		file->readx(&sizen, 4);
			file->readx(&l, 4);
			size = createHostInt(&size, 4, big_endian);
			sizen = createHostInt(&sizen, 4, big_endian);
			l = createHostInt(&l, 4, big_endian);
			FileOfs ofs = xex_shared->imports.ofs + 3*4 + sizen;
			XexImportLib *libs = new XexImportLib[l];
			if (l) {
				String s;
				for (uint i=0; i < l; i++) {
					s.clear();
					file->readStringz(s);
					libs[i].name = strdup(s.contentChar());
				}
				
				// patch table
				for (uint i=0; i < l; i++) {				
					file->seek(ofs);
					file->readx(&sizen, 4);
					sizen = createHostInt(&sizen, 4, big_endian);
					ofs += sizen;
					
					file->seek(36 + file->tell()); // skip garbage
					int count = (sizen - 40) / 4;
					libs[i].func_count = count;
					if (count) {
						libs[i].funcs = new XexImportFunc[count];
					} else {
						libs[i].funcs = NULL;
					}
					for (int j=0; j < count; j++) {
						uint32 patch;
						file->readx(&patch, 4);
						libs[i].funcs[j].patch = createHostInt(&patch, 4, big_endian);
						libs[i].funcs[j].ord = libs[i].funcs[j].ia = 0;
					}
				}				
			}
			xex_shared->imports.libs = libs;
			xex_shared->imports.lib_count = l;
		}
	} catch (...) {}
	
	xex_shared->image = new MemoryFile(0, xex_shared->image_size);

	try {
		if (xex_shared->loader_info.type == XEX_LOADER_RAW) {
			file->seek(xex_shared->header.size);
			FileOfs ofs = 0;
			foreach(XEXLoaderRawSection, section, *xex_shared->loader_info.raw.sections, {
				xex_shared->image->seek(ofs);
				file->copyTo(xex_shared->image, section->raw);
				ofs += section->raw + section->pad;
			});
		}
	} catch (...) {}

	uint32 *iat = new uint32[0x8000];
	byte *image_ptr = xex_shared->image->getBufPtr();
	for (int i=0; i < xex_shared->imports.lib_count; i++) {
		memset(iat, 0, 4*0x8000);
		// get import ordinals
		XexImportLib *lib = xex_shared->imports.libs+i;
		for (int j=0; j < lib->func_count; j++) {
			uint32 ord;
			ord = lib->funcs[j].ord = createHostInt(&image_ptr[lib->funcs[j].patch - xex_shared->image_base], 4, big_endian);
			if ((ord & 0xff000000) == 0) {
				ord &= 0x7fff;
				if (iat[ord]) {
//					fprintf(stderr, "%s: warning: duplicate import %d from library '%s'\n", infn, ord, imports[i].lib);
				} else {
					iat[ord] = lib->funcs[j].patch;
				}
			}
		}

//		printf("\n; Library '%s':\n", lib.name);
		// resolve imports
		for (int j=0; j < lib->func_count; j++) {
			uint32 ord = lib->funcs[j].ord;
			if (ord & 0xff000000) {
				int libidx = (ord & 0x00ff0000) >> 16;
				ord &= 0x7fff;
				if (libidx != i) {
/*					if (libidx < import_count) {
						fprintf(stderr, "%s: import %d from library '%s', but in section of library '%s'\n", infn, ord, imports[libidx].lib, imports[i].lib);
					} else {
						fprintf(stderr, "%s: import %d from unknown library %d, but in section of library '%s'\n", infn, ord, libidx, imports[i].lib);
					}
					return 1;*/
					continue;
				}
				if (!iat[ord]) {
/*					fprintf(stderr, "%s: import %d from library '%s' need to be resolved but is not imported\n", infn, ord, imports[i].lib);
					return 1;*/
					continue;
				}
				uint32 ia = iat[ord];
				
				// patch it
				uint32 lis_r11 = (0x3d600000 | (ia >> 16)) + !!(ia & 0x8000);
				uint32 lwz_r11 = (0x816b0000 | (ia & 0xffff));
				
				createForeignInt(&image_ptr[lib->funcs[j].patch-xex_shared->image_base], lis_r11, 4, big_endian);
				createForeignInt(&image_ptr[lib->funcs[j].patch-xex_shared->image_base+4], lwz_r11, 4, big_endian);
				
				lib->funcs[j].ia = ia; 
/*				char s[100];
				snprintf(s, sizeof s, "wrapper_import_%s_%d", imports[i].lib, ord);
				printf("function 0x%08x name '%s'\n", imports[i].func[j].patch, s);
				
				printf("xref 0x%08x -> 0x%08x\n", ia, imports[i].func[j].patch + 12);*/
				
			} else {
/*				ord &= 0x7fff;
				char s[100];
				snprintf(s, sizeof s, "import_%s_%d", imports[i].lib, ord);
				printf("function_ptr 0x%08x name '%s'\n", imports[i].func[j].patch, s);*/
			}
		}
	}
	delete[] iat;
	
	ht_format_group::init_ifs(ifs);
}

void ht_xex::done()
{
	ht_format_group::done();

	ht_xex_shared_data *xex_shared = (ht_xex_shared_data*)shared_data;

	delete[] xex_shared->pages.page;
	for (int i=0; i < xex_shared->imports.lib_count; i++) {
		free(xex_shared->imports.libs[i].name);
		delete[] xex_shared->imports.libs[i].funcs;
	}
	delete[] xex_shared->imports.libs;
	free(xex_shared->info_table);
	free(xex_shared->info_table_cooked);
	delete xex_shared->image;
	free(xex_shared);
}

bool xex_ofs_to_rva(ht_xex_shared_data *xex_shared, FileOfs ofs, RVA &rva)
{
#if 0
	FileOfs xex_ofs = 0;
	RVA xex_va = 0;
	for (int i = 0; i < g_entry_count; i++) {
		if (xex_ofs <= ofs && xex_ofs+g_entries[i].raw > ofs) {
			res = xex_va + (ofs - xex_ofs);
			return true;
		}
		xex_va += g_entries[i].raw + g_entries[i].pad;
		xex_ofs += g_entries[i].raw;
		if (xex_ofs > ofs) return false;
	}
	return false;
#else
	if (ofs < xex_shared->image_size) {
		rva = ofs;
		return true;
	} else {
		return false;
	}
#endif
}

bool xex_rva_to_ofs(ht_xex_shared_data *xex_shared, RVA rva, FileOfs &ofs)
{
#if 0
	FileOfs xex_ofs = 0;
	RVA xex_va = 0;
	for (int i = 0; i < g_entry_count; i++) {
		if (xex_va <= va && xex_va + g_entries[i].read > va) {
			res = xex_ofs + (va - xex_va);
			return true;
		}
		xex_va += g_entries[i].read + g_entries[i].pad;
		xex_ofs += g_entries[i].read;
		if (xex_va > va) return false;
	}
	return false;
#else
	if (rva < xex_shared->image_size) {
		ofs = rva;
		return true;
	} else {
		return false;
	}
#endif
}

uint32 xex_get_rva_flags(ht_xex_shared_data *xex_shared, RVA rva)
{
	uint pagen = rva >> xex_shared->pages.page_shift;
	XexPage *p = xex_shared->pages.page + pagen;
	return p->flags;
}
