#ifndef _SORT_SDI_MACROS_H
#define _SORT_SDI_MACROS_H

// This file is a part of Sort Library

#ifdef __amigaos4__
#define __BASE_OR_IFACE_TYPE	struct SortIFace *
#define __BASE_OR_IFACE_VAR		ISort
#else
#define __BASE_OR_IFACE_TYPE	struct SortBase *
#define __BASE_OR_IFACE_VAR		SortBase
#endif

#define __BASE_OR_IFACE __BASE_OR_IFACE_TYPE __BASE_OR_IFACE_VAR

#endif /* _SORT_SDI_MACROS_H */
