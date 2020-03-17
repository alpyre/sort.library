// This file is a part of Sort Library

extern LIBPROTO(SortA, ULONG, REG(a6, UNUSED __BASE_OR_IFACE), REG(a0, APTR), REG(d0, ULONG), REG(a1, struct TagItem *));
#ifdef __amigaos4__
extern LIBPROTOVA(Sort, ULONG, REG(a6, UNUSED __BASE_OR_IFACE), REG(a0, APTR), REG(d0, ULONG), REG(d1, ULONG), ...);
#endif

#define libvector LFUNC_FAS(SortA)\
                  LFUNC_VA_(Sort)
