/* 
 *	HT Editor
 *	formats.cc
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

#include "formats.h"

#include "htdebug.h"

#include "class.h"
#include "hthex.h"
#include "httext.h"
#include "htdisasm.h"
#include "htfinfo.h"
#include "htelf.h"
#include "htcoff.h"
#include "htle.h"
#include "htmacho.h"
#include "htmz.h"
#include "htne.h"
#include "htpe.h"
#include "htpef.h"
#include "htflt.h"
#include "htxbe.h"
#include "htxex.h"

format_viewer_if *format_viewer_ifs[] =
{
	&hthex_if,
	&httext_if,
	&htdisasm_if,
	&htfinfo_if,
	&htelf_if,
	&htmz_if,
	&htcls_if,
	&htcoff_if,
	&htpe_if,
	&htne_if,
	&htle_if,
	&htmacho_if,
	&htflt_if,
	&htxbe_if,
	&htxex_if,
	&htpef_if,
	NULL
};
