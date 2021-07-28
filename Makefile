
# Eievui's boilerplate GBDK makefile.

.SUFFIXES:

################################################
#                                              #
#             CONSTANT DEFINITIONS             #
#                                              #
################################################

LCC	= lcc -Wa-l -Wl-m -debug -Isrc -Wb-ext=.rel -autobank

ifeq ($(OS),Windows_NT)
	ROMUSAGE := ./tools/romusage.exe
	SUPERFAMICONV := ./tools/superfamiconv.exe
	RGB2SDAS := ./tools/rgb2sdas.exe
else
	ROMUSAGE := ./tools/romusage
	SUPERFAMICONV := ./tools/superfamiconv
	RGB2SDAS := ./tools/rgb2sdas
endif

SRCDIR := src
OBJDIR := obj
RESDIR := res
BINDIR := bin

ROM 	= $(BINDIR)/$(ROMNAME).$(ROMEXT)

GFXS	= $(patsubst $(SRCDIR)%.png, $(RESDIR)%.2bpp, $(GFXSRC))

OBJS	= $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(CSRC)) \
		$(patsubst $(SRCDIR)/%.s, $(OBJDIR)/%.o, $(SSRC))
#		$(OBJDIR)/libs/hUGEDriver.obj.o

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

map: $(MAPS)

###############################################
#                                             #
#                 COMPILATION                 #
#                                             #
###############################################

# Special rules

$(OBJDIR)/libs/hUGEDriver.obj.o:
	@mkdir -p $(@D)
	rgbasm -o $(OBJDIR)/libs/hUGEDriver.obj $(SRCDIR)/libs/hUGEDriver.asm
	$(RGB2SDAS) $(OBJDIR)/libs/hUGEDriver.obj

# Compile source code.
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(LCC) -c -o $@ $<

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	@mkdir -p $(@D)
	$(LCC) -c -o $@ $<

$(OBJDIR)/%.b.o: $(SRCDIR)/%.b.c
	@mkdir -p $(@D)
	$(LCC) -c -Wf-ba0 -o $@ $<

# Convert .png files to .2bpp graphics.
$(RESDIR)/%.1bpp.2bpp: $(SRCDIR)/%.1bpp.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) tiles -i $< -d $@ -M gbc -R -D -F -B 1

$(RESDIR)/%.2bpp: $(SRCDIR)/%.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) tiles -i $< -d $@ -M gbc -R -D -F

$(RESDIR)/%.h.2bpp: $(SRCDIR)/%.h.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) tiles -i $< -d $@ -M gbc -H 16 -R -D -F

# Convert .png files to tilemaps.
$(RESDIR)/%.map: $(SRCDIR)/%.1bpp.png
	$(SUPERFAMICONV) -i $< -t $(patsubst %.map, %.2bpp, $@) -m $@ -R -M gb -B 1

$(RESDIR)/%.map: $(SRCDIR)/%.png
	$(SUPERFAMICONV) -i $< -t $(patsubst %.map, %.2bpp, $@) -m $@ -M gb

$(ROM): $(GFXS) $(MAPS) $(OBJS)
	@mkdir -p $(@D)
	$(LCC) -Wm-yn$(TITLE) $(COMPAT) -Wl-yt$(MBCTYPE) -Wl-yo$(ROMBANKS) -Wl-ya$(RAMBANKS) -o $(ROM) $(OBJS)
	@$(ROMUSAGE) $(BINDIR)/$(ROMNAME).map -g -e:STACK:DEFF:100 -e:SHADOW_OAM:C000:A0 -e:HEADER:0083:17D
