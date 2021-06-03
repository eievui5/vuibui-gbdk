
# Eievui's boilerplate GBDK makefile.

.SUFFIXES:

################################################
#                                              #
#             CONSTANT DEFINITIONS             #
#                                              #
################################################

LCC	= lcc -Wa-l -Wl-m -Wb-ext=.rel -autobank

SRCDIR := src
OBJDIR := obj
BINDIR := bin

ROM 	= $(BINDIR)/$(ROMNAME).$(ROMEXT)

OBJS	= $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(CSRC)) \
	  $(patsubst $(SRCDIR)/%.s, $(OBJDIR)/%.o, $(SSRC))

include project.mk

################################################
#                                              #
#                    TARGETS                   #
#                                              #
################################################

all: $(ROM)

clean:
	rm -rf $(BINDIR)
	rm -rf $(OBJDIR)

rebuild:
	$(MAKE) clean
	$(MAKE) all

###############################################
#                                             #
#                 COMPILATION                 #
#                                             #
###############################################

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(LCC) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	@mkdir -p $(@D)
	$(LCC) -c -o $@ $<

$(ROM): $(OBJS)
	@mkdir -p $(@D)
	$(LCC) -Wm-yn$(TITLE) $(COMPAT) -Wl-yt$(MBCTYPE) -Wl-yo$(ROMBANKS) -Wl-ya$(RAMBANKS) -o $(ROM) $(OBJS)
ifneq ($(OS),Windows_NT)
	./tools/romusage $(BINDIR)/$(ROMNAME).map -g
else
	./tools/romusage.exe $(BINDIR)/$(ROMNAME).map -g
endif
