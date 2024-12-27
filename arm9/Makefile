.SUFFIXES:

TOPDIR ?= $(CURDIR)
include $(DEVKITARM)/base_rules

TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH	:=	-marm -march=armv5te -mtune=arm946e-s
DEFINES :=	-DARM9 -D__3DS__

FALSEPOSITIVES := -Wno-array-bounds -Wno-stringop-overflow -Wno-stringop-overread
CFLAGS	:=	-g -std=gnu11 -Wall -Wextra -Werror -O2 -mword-relocations \
			-fomit-frame-pointer -ffunction-sections -fdata-sections \
			-Wno-main $(FALSEPOSITIVES) $(ARCH) $(DEFINES)

CFLAGS	+=	$(INCLUDE)
ASFLAGS	:=	-g $(ARCH)

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

export LD	:=	$(CC)

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 		:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile
	
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
all	:	$(OUTPUT).elf

$(OUTPUT).elf	:	$(OFILES)

%.elf: $(OFILES)
	@echo linking $(notdir $@)
	@$(LD) -specs=$(TOPDIR)/linker.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map) $(OFILES) $(LIBPATHS) -o $@
	@$(NM) -CSn $@ > $(notdir $*.lst)

$(OFILES_SRC)	: $(HFILES_BIN)
-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------