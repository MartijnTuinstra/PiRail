programs=main test
File_main=main_node.o eeprom_layout.o RNet.o IO.o
File_test=test.o
dev=atmega328 atmega328p atmega64a atmega2560
#dev=atmega2560

avr_gcc_args=-g -Wall -Os -Werror -Wextra

TARGETS=

# 1 = program, 2 = device
define compile
build/$(1)/$(2).bin.d/stamp:
	@echo "  [  DIR  ] build/$1/$2.bin.d"
	@mkdir -p build/$(1)/$(2).bin.d
	@touch build/$(1)/$(2).bin.d/stamp

build/$(1)/$(2).bin.d/%.o: %.cpp %.h build/$(1)/$(2).bin.d/stamp
	@echo "  [AVR-GCC] $$@ ($$^)"
	@avr-gcc -mmcu=$(2) -Wa,-ahlmns=$$(basename $$@).lst -c -o $$@ $$(word 1,$$^) $$(avr_gcc_args)

build/$(1)/$(2).elf: $$(addprefix build/$(1)/$(2).bin.d/,$$(File_$(1)))
	@echo "  [  AVR  ] $(2) $(1)"
	@make -f make-avr hex dev=$(2) dir=build/$(1) --no-print-directory

endef

.PHONY: all

$(foreach d, $(dev), $(foreach p, $(programs), $(eval TARGETS+= build/$(p)/$(d).elf)))
$(foreach d, $(dev), $(foreach p, $(programs), $(eval $(call compile,$(p),$(d)))))

all: $(TARGETS)

%_clean:
	@echo "  [ CLEAN ] build/$(subst _clean,,$@)"
	@rm -rf build/$(subst _clean,,$@)

clean: $(addsuffix _clean,$(programs))
