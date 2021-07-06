
# Eievui's boilerplate GBDK makefile.

.SUFFIXES:

################################################
#                                              #
#             CONSTANT DEFINITIONS             #
#                                              #
################################################

LCC	= lcc -Wa-l -Wa--vc -Wl-m -Isrc -Wb-ext=.rel -autobank -debug

ifeq ($(OS),Windows_NT)
	ROMUSAGE := ./tools/romusage.exe
	SUPERFAMICONV := ./tools/superfamiconv.exe
else
	ROMUSAGE := ./tools/romusage
	SUPERFAMICONV := ./tools/superfamiconv
endif

SRCDIR := src
OBJDIR := obj
RESDIR := res
BINDIR := bin

ROM 	= $(BINDIR)/$(ROMNAME).$(ROMEXT)

GFXS	= $(patsubst $(SRCDIR)%.png, $(RESDIR)%.2bpp, $(GFXSRC))

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
	lcc -c -o $@ $<

$(OBJDIR)/%.o: $(RESDIR)/%.c
	@mkdir -p $(@D)
	$(LCC) -c -o $@ $<

$(OBJDIR)/%.b.o: $(SRCDIR)/%.b.c
	@mkdir -p $(@D)
	$(LCC) -c -Wf-ba0 -o $@ $<

# Convert .png files to .2bpp graphics.
$(RESDIR)/%.1bpp.2bpp: $(SRCDIR)/%.1bpp.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) tiles -i $< -d $@ -M gbc -R -D -F -B 1

# Convert .png files to .2bpp graphics.
$(RESDIR)/%.2bpp: $(SRCDIR)/%.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) tiles -i $< -d $@ -M gbc -R -D -F

# Convert .png files to .2bpp graphics.
$(RESDIR)/%.h.2bpp: $(SRCDIR)/%.h.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) tiles -i $< -d $@ -M gbc -H 16 -R -D -F

$(ROM): $(GFXS) $(OBJS)
	@mkdir -p $(@D)
	$(LCC) -Wm-yn$(TITLE) $(COMPAT) -Wl-yt$(MBCTYPE) -Wl-yo$(ROMBANKS) -Wl-ya$(RAMBANKS) -o $(ROM) $(OBJS)
	$(ROMUSAGE) $(BINDIR)/$(ROMNAME).map -g -e:STACK:DEFF:100 -e:SHADOW_OAM:C000:A0
