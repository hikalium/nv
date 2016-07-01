
SRCS=nv.c nv_term.c nv_fix.c nv_lang.c

nv : $(SRCS)
	cc -Wall -o nv $(SRCS)
