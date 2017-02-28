
GIT_COMMIT_ID := $(shell git log -1 --format='%H')
GIT_COMMIT_DATE := $(shell git log -1 --format='%ad')

SRCS= 	nv.c nv_array.c nv_dict.c nv_driver.c \
		nv_fix.c nv_id.c nv_node.c nv_op.c nv_static.c \
		nv_test.c nv_variable.c nv_graph.c nv_term.c nv_signal.c \
		nv_path.c
HEADERS=nv.h
CFLAGS=-Wall -Wextra -lncurses -Wunused-function
CFLAGS += -DGIT_COMMIT_ID="\"$(GIT_COMMIT_ID)\"" \
			-DGIT_COMMIT_DATE="\"$(GIT_COMMIT_DATE)\""

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

id:
	@uuidgen | tr "[:lower:]" "[:upper:]" | sed -E "s/(.{8})-(.{4})-(.{4})-(.{4})-(.{4})(.{8})/\{{0x\1, 0x\2\3, 0x\4\5, 0x\6\}}/"
