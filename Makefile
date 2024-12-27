

.PHONY:	all release arm9

all: boot.firm

release: boot.firm

boot.firm: arm9
	@firmtool build boot.firm -D arm9/arm9.elf -A 0x18180000 -C NDMA
	
arm9:
	@$(MAKE) -C arm9 all