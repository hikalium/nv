
SRCS=nv.c nv_element.c nv_env.c nv_envdep.c nv_fix.c nv_lang.c nv_term.c nv_termlist.c nv_var.c nv_varset.c
HEADERS=nv.h

nv : $(SRCS) $(HEADERS) Makefile
	cc -Wall -lncurses -o nv  $(SRCS)

clean:
	-rm nv

