# 定義
CC			= gcc
TDDIR		= test
OBJDIR		= obj
TESTTARGET	= $(OBJDIR)/TestCutter.dll

FLAGS		= -fpic -DTDD
INCLUDES	= -I./$(TDDIR)
TDDINCLIBS	= -I/usr/local/include/cutter -L/usr/local/lib -lcutter -include/usr/local/include/cutter/cutter.h
DMYHEADER	= -include./$(TDDIR)/dmy.h

FILELIST		= $(shell ls *.c)
OBJLIST			= $(patsubst %.c,$(OBJDIR)/%.o,$(FILELIST))
TDDFILELIST		= $(shell ls $(TDDIR)/*.c)
TDDOBJLIST		= $(patsubst $(TDDIR)/%.c,$(OBJDIR)/%.co,$(TDDFILELIST))

.SUFFIXES: .o .c .co

all: callPath $(OBJDIR) $(TESTTARGET)



$(TESTTARGET): $(OBJLIST) $(TDDOBJLIST)
	$(CC) -shared $^ -o $@ $(TDDINCLIBS)

$(OBJDIR)/%.o: %.c
	$(CC) $(FLAGS) -o $@ -c $< $(DMYHEADER)

$(OBJDIR)/%.co: $(TDDIR)/%.c
	$(CC) $(FLAGS) -o $@ -c $< $(DMYHEADER) $(TDDINCLIBS)


tddexec:
	$(shell cutter .)

clean:
	rm -r $(OBJDIR)

$(OBJDIR):
	mkdir $@

callPath:
	@echo $(shell pwd);

tddver:
	cutter --version
