
GIT_COMMIT_ID := $(shell git log -1 --format='%H')
GIT_COMMIT_DATE := $(shell git log -1 --format='%ad')

SRCS= 	main.c \
		nv.c nv_array.c nv_dict.c nv_driver.c \
		nv_fix.c nv_id.c nv_node.c nv_op.c nv_static.c \
		nv_variable.c nv_term.c nv_signal.c \
		nv_path.c nv_integer.c nv_string.c nv_relation.c nv_context.c \
		lang/02/parse.c lang/02/eval.c
HEADERS=nv.h nv_node.h nv_func.h
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

perf:
	make debugbin
	iprofiler -timeprofiler ./nv < sample/perf.nv
	open nv.dtps

clean:
	-rm nv
	-rm -rf ./nv.dSYM
	-rm *.c-e
	-rm *.h-e

log:
	git log --pretty=format:" - %s %n   http://github.com/hikalium/nv/commit/%H" --since=10hour

sc:
	~/Desktop/LocalProjects/hdp/scsc/scsc $(SRCS) $(HEADERS)
	wc  $(SRCS) $(HEADERS)
	ls -la nv

id:
	@uuidgen | tr "[:lower:]" "[:upper:]" | sed -E "s/(.{8})-(.{4})-(.{4})-(.{4})-(.{4})(.{8})/\{{0x\1, 0x\2\3, 0x\4\5, 0x\6\}}/"

header:
	@for filename in $(SRCS); do \
		(./makeheadersub.sh $$filename) \
	done

nv_func.h: $(SRCS) Makefile makeheadersub.sh
	make header > nv_func.h

