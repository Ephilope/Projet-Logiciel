CC=$(GCC_CROSS)
CCAS=$(GCC_CROSS)
AM_CFLAGS=-mbig-endian
AM_CCASFLAGS=-mbig-endian
AM_LDFLAGS=-nostdlib -T $(srcdir)/linker_script -n --entry main
AM_LDFLAGS+=-Wl,-EB

if HAVE_ARM_COMPILER
noinst_PROGRAMS=example1 example2 example3 example4 test_branch test_branch2 example_add example2_add example_mov_adds insertion_sort

all-am: $(PROGS)
endif

example1_SOURCES=example1.s
example2_SOURCES=example2.s
example3_SOURCES=example3.s
example4_SOURCES=example4.s
test_branch_SOURCES=test_branch.s
test_branch2_SOURCES=test_branch2.s
example_add_SOURCES=example_add.s
example2_add_SOURCES=example2_add.s
example_mov_adds_SOURCES=example_mov_adds.s


insertion_sort_SOURCES=insertion_sort.c

EXTRA_DIST=linker_script

clean-local:
	$(RM) $(PROGS)
