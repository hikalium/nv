
SRCS=nv.c nv_dict.c nv_element.c nv_env.c nv_envdep.c nv_fix.c nv_integer.c nv_lang.c nv_lang00.c nv_list.c nv_operator.c nv_string.c nv_var.c
HEADERS=nv.h nv_rawelem.h

nv : $(SRCS) $(HEADERS) Makefile
	cc -Wall -lncurses -Wunused-function -ferror-limit=5 -Os -o nv  $(SRCS)
	strip nv
	upx -9 nv

fastbin : $(SRCS) $(HEADERS) Makefile
	cc -Wall -lncurses -Wunused-function -ferror-limit=5 -O3 -o nv  $(SRCS)

debugbin : $(SRCS) $(HEADERS) Makefile
	cc -g -Wall -lncurses -Wunused-function -ferror-limit=5 -o nv  $(SRCS)

debug: $(SRCS) $(HEADERS) Makefile
	make debugbin
	lldb -f ./nv

clean:
	-rm nv

log:
	git log --pretty=format:" - %s %n   http://github.com/hikalium/nv/commit/%H" --since=10hour

sc:
	~/Desktop/LocalProjects/hdp/scsc/scsc $(SRCS) $(HEADERS)
	wc -l $(SRCS) $(HEADERS)
	ls -la nv
