
SRCS=nv.c nv_dict.c nv_element.c nv_env.c nv_envdep.c nv_fix.c nv_integer.c nv_lang.c nv_list.c nv_operator.c nv_string.c nv_var.c
HEADERS=nv.h

nv : $(SRCS) $(HEADERS) Makefile
	cc -Wall -lncurses -Wunused-function -ferror-limit=5 -Os -o nv  $(SRCS)

clean:
	-rm nv

log:
	git log --pretty=format:" - %s %n   http://github.com/hikalium/nv/commit/%H" --since=10hour

