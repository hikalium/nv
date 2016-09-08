
SRCS=nv.c nv_blob.c nv_builtin.c nv_dict.c nv_driver.c nv_element.c nv_fix.c nv_integer.c nv_lang.c nv_lang00.c nv_list.c nv_operator.c nv_string.c nv_var.c
HEADERS=nv.h nv_rawelem.h
CFLAGS=-Wall -Wextra -lncurses -Wunused-function

nv : $(SRCS) $(HEADERS) Makefile
	cc $(CFLAGS) -Os -o nv  $(SRCS)
	strip nv
	upx -9 nv

fastbin : $(SRCS) $(HEADERS) Makefile
	cc $(CFLAGS) -Ofast -O3 -o nv  $(SRCS)

debugbin : $(SRCS) $(HEADERS) Makefile
	cc $(CFLAGS) -g3 -o nv  $(SRCS)

debug: $(SRCS) $(HEADERS) Makefile
	make debugbin
	lldb -f ./nv

clean:
	-rm nv
	-rm -rf ./nv.dSYM

log:
	git log --pretty=format:" - %s %n   http://github.com/hikalium/nv/commit/%H" --since=10hour

sc:
	~/Desktop/LocalProjects/hdp/scsc/scsc $(SRCS) $(HEADERS)
	wc -l $(SRCS) $(HEADERS)
	ls -la nv
