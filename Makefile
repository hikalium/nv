
SRCS=nv.c nv_array.c nv_dict.c nv_driver.c nv_fix.c nv_id.c nv_node.c nv_static.c nv_variable.c
HEADERS=nv.h
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
	wc  $(SRCS) $(HEADERS)
	ls -la nv
