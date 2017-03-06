
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

SRCS_PATH	=$(addprefix ../, $(SRCS))
HEADERS_PATH=$(addprefix ../, $(HEADERS))

testbin : $(SRCS_PATH) $(HEADERS_PATH) test.c Makefile
	cc $(CFLAGS) -Os -o testbin  $(SRCS_PATH) test.c

