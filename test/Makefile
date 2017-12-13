NV = ../nv

diff: simple_loop.diff 99table.diff
	@echo "All test passed."

%.diff: %.in %.out
	$(NV) $*.in | diff -u $*.out -
