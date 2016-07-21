
SRCS=nv.c nv_envdep.c nv_fix.c nv_lang.c nv_term.c nv_var.c nv_varset.c
HEADERS=nv.h

nv : $(SRCS) $(HEADERS) Makefile
	cc -Wall -lncurses -o nv  $(SRCS)
