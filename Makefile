# Compiler
CC = g++
OPTS = -c -Wall --std=c++17

# Project name
PROJECT = luthien

# Directories
OBJDIR = obj
SRCDIR = src

# Libraries
LIBS = -lpthread -lstdc++fs

# Files and folders
SRCS    = $(shell find $(SRCDIR) -name '*.cpp')
SRCDIRS = $(shell find . -name '*.cpp' | dirname {} | sort | uniq | sed 's/\/$(SRCDIR)//g' )
OBJS    = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# Targets
$(PROJECT): buildrepo $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(OPTS) -c $< -o $@

clean:
	rm $(PROJECT) $(OBJDIR) -Rf

test_single1:
	./luthien -i1 test/test.fastq -o1 test/output/test.fastq \
	-t 2 -c 16 -b 256

test_single2:
	./luthien -i1 test/big/SRR941557.fastq -o1 test/output/test2.fastq \
	-t 2 -c 16 -b 256

test_paired1:
	./luthien -i1 test/test.f.fastq -i2 test/test.r.fastq \
	-o1 test/test.f.fastq -o2 test/output/test.r.fastq \
	-t 2 -c 17 -b 512

test_paired2:
	./luthien -i1 test/big/SRR3309317_1.mini.fastq -i2 test/big/SRR3309317_2.mini.fastq \
	-o1 test/output/SRR3309317_1.mini.fastq -o2 test/output/SRR3309317_2.mini.fastq \
	-t 2 -c 17 -b 512

buildrepo:
	@$(call make-repo)

# Create obj directory structure
define make-repo
	mkdir -p $(OBJDIR)
	for dir in $(SRCDIRS); \
	do \
		mkdir -p $(OBJDIR)/$$dir; \
	done
endef