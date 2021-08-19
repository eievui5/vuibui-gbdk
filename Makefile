
# Eievui's boilerplate GBDK makefile.

.SUFFIXES:

################################################
#                                              #
#             CONSTANT DEFINITIONS             #
#                                              #
################################################

LCC	= lcc -Wa-l -Wl-m -Isrc -Wb-ext=.rel -autobank

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

res/gfx/ui/paw_mark.map res/gfx/ui/paw_mark.1bpp: src/gfx/ui/paw_mark.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) \
		-i $< \
		-t res/gfx/ui/paw_mark.1bpp \
		-m res/gfx/ui/paw_mark.map \
		-R -M gb -B 1

res/gfx/maps/crater.map res/gfx/maps/crater.2bpp: src/gfx/maps/crater.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) \
		-i $< \
		-t res/gfx/maps/crater.2bpp \
		-m res/gfx/maps/crater.map \
		-R -M gb

res/gfx/maps/worldmap_ui.map res/gfx/maps/worldmap_ui.2bpp: src/gfx/maps/worldmap_ui.png
	@mkdir -p $(@D)
	$(SUPERFAMICONV) \
		-i $< \
		-t res/gfx/maps/worldmap_ui.2bpp \
		-m res/gfx/maps/worldmap_ui.map \
		-W 8 -H 8 -T 128 -R -M gb

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

$(ROM): $(GFXS) $(DEPS) $(OBJS)
	@mkdir -p $(@D)
	$(LCC) -Wm-yn$(TITLE) $(COMPAT) -Wl-yt$(MBCTYPE) -Wl-yo$(ROMBANKS) -Wl-ya$(RAMBANKS) -o $(ROM) $(OBJS)
	@$(ROMUSAGE) $(BINDIR)/$(ROMNAME).map -g -e:STACK:DEFF:100 -e:SHADOW_OAM:C000:A0 -e:HEADER:0083:17D
