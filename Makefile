
# Eievui's boilerplate GBDK makefile.

.SUFFIXES:

################################################
#                                              #
#             CONSTANT DEFINITIONS             #
#                                              #
################################################

LCC	= lcc -Wa-l -Wl-m -Wb-ext=.rel -autobank

ifeq ($(OS),Windows_NT)
	ROMUSAGE := ./tools/romusage.exe
	SUPERFAMICONV := ./tools/superfamiconv.exe
else
	ROM_USAGE := ./tools/romusage
	SUPERFAMICONV := ./tools/superfamiconv
endif

SRCDIR := src
OBJDIR := obj
RESDIR := res
BINDIR := bin

ROM 	= $(BINDIR)/$(ROMNAME).$(ROMEXT)

GFXS	= $(patsubst $(SRCDIR)%.png, $(RESDIR)%.2bpp, $(GFXSRC)) \
	  $(patsubst $(SRCDIR)%.png, $(RESDIR)%.tall, $(TALLSRC))

BINS	= $(patsubst $(SRCDIR)%.bin, $(RESDIR)%.c, $(BINSRC))

OBJS	= $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(CSRC)) \
	  $(patsubst $(SRCDIR)/%.s, $(OBJDIR)/%.o, $(SSRC)) \
	  $(patsubst $(RESDIR)/%.c, $(OBJDIR)/%.o, $(BINS)) \
	  $(patsubst $(RESDIR)/%.2bpp, $(OBJDIR)/%.o, $(GFXS)) \
	  $(patsubst $(RESDIR)/%.tall, $(OBJDIR)/%.o, $(GFXS))


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
	rm -rf $(RESDIR)

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

$(OBJDIR)/%.o: $(RESDIR)/%.c
	@mkdir -p $(@D)
	$(LCC) -c -o $@ $<

# Convert .png files to .2bpp graphics and C source files.
$(RESDIR)/%.2bpp: $(SRCDIR)/%.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) tiles -i $< -d $@ -M gbc -R -D -F
	py tools/incbin.py -o $(patsubst $(RESDIR)%.2bpp, $(RESDIR)%.c, $@) -i $@ -s

# Convert .png files to .2bpp graphics and C source files.
$(RESDIR)/%.tall: $(SRCDIR)/%.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) tiles -i $< -d $@ -M gbc -H 16 -R -D -F
	py tools/incbin.py -o $(patsubst $(RESDIR)%.tall, $(RESDIR)%.c, $@) -i $@ -s -a

$(ROM): $(GFXS) $(BINS) $(OBJS)
	@mkdir -p $(@D)
	$(LCC) -Wm-yn$(TITLE) $(COMPAT) -Wl-yt$(MBCTYPE) -Wl-yo$(ROMBANKS) -Wl-ya$(RAMBANKS) -o $(ROM) $(OBJS)
	$(ROMUSAGE) $(BINDIR)/$(ROMNAME).map -g
