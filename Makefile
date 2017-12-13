
GIT_COMMIT_ID := $(shell git log -1 --format='%H')
GIT_COMMIT_DATE := $(shell git log -1 --format='%ad')

SRCS= 	main.c \
		nv.c nv_array.c nv_dict.c nv_driver.c \
		nv_fix.c nv_id.c nv_node.c nv_lang.c nv_static.c \
		nv_variable.c nv_term.c nv_signal.c nv_op.c \
		nv_integer.c nv_string.c nv_relation.c \
		fnv1.c \
		lang/02/parse.c lang/02/eval.c lang/02/table.c \
		lang/osecpu/parse.c lang/osecpu/eval.c
HEADERS=nv.h nv_func.h nv_static.h
CFLAGS=-Wall -Wpedantic -Wextra -lncurses -Wunused-function
CFLAGS += -DGIT_COMMIT_ID="\"$(GIT_COMMIT_ID)\"" \
			-DGIT_COMMIT_DATE="\"$(GIT_COMMIT_DATE)\""

.PHONY: test

nv : $(SRCS) $(HEADERS) Makefile
	cc $(CFLAGS) -Os -o nv  $(SRCS)
	strip nv
	-upx -9 nv

run : ./nv Makefile
	rlwrap ./nv $(NV_ARGS)

fastbin : $(SRCS) $(HEADERS) Makefile
	cc $(CFLAGS) -Ofast -O3 -o nv  $(SRCS)

debugbin : $(SRCS) $(HEADERS) Makefile
	cc $(CFLAGS) -g3 -o nv -DDEBUG  $(SRCS)

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

log:
	git log --pretty=format:" - %s %n   http://github.com/hikalium/nv/commit/%H" --since=10hour

test:
	make -C test/

sc:
	~/Desktop/LocalProjects/hdp/scsc/scsc $(SRCS) $(HEADERS)
	wc  $(SRCS) $(HEADERS)
	ls -la nv

id:
	@uuidgen | tr "[:lower:]" "[:upper:]" | sed -E "s/(.{8})-(.{4})-(.{4})-(.{4})-(.{4})(.{8})/\{{0x\1, 0x\2\3, 0x\4\5, 0x\6\}}/"

mkheader: Makefile tokenizer.c
	@echo "Generating $@..."
	@$(CC) $(CFLAGS) -Ofast -O3 -o mkheader tokenizer.c

nv_func.h: $(SRCS) Makefile mkheader
	@echo "Generating $@..."
	@echo "" > $@
	@for filename in $(SRCS); do \
		(./mkheader $$filename | grep -v Internal >> $@)\
	done

nv_static.h : nv_static.c Makefile
	@echo "Generating $@..."
	@cat nv_static.c | grep -e '^const NV_ID NODEID' -e '^const NV_ID RELID' | sed -e 's/$$/;/g' | sed -e 's/^/extern /g' > nv_static.h

