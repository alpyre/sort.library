// This file is a part of Sort Library

#define VERSION		0
#define REVISION	15
#define RSTRING   "0.15"
#define LIBNAME   "sort.library"
#ifdef __SASC
  #define DATE __AMIGADATE__
#elif  _DCC
  #define DATE __COMMODORE_DATE__
#else
  #define DATE __DATE__
#endif
#define VERS		  LIBNAME " " RSTRING
#define VSTRING		VERS " (" DATE ")\r\n"
#define VERSTAG		"\0$VER: " VERS " (" DATE ")"
