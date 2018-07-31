programs=main
File_main=main_node.o
dev=atmega328 atmega2560

TARGETS=

# 1 = program, 2 = device
define compile
$(1)/$(2).bin.d/stamp:
	@echo "  [  DIR  ] $1/$2.bin.d"
	@mkdir -p $(1)/$(2).bin.d
	@touch $(1)/$(2).bin.d/stamp

$(1)/$(2).bin.d/%.o: %.cpp $(1)/$(2).bin.d/stamp
	@echo "  [AVR-GCC] $$@"
	@avr-gcc -g -Wall -Os -Werror -Wextra -mmcu=$(2) -Wa,-ahlmns=$$(basename $$@).lst -c -o $$@ $$(word 1,$$^)

$(1)/$(2).elf: $$(addprefix $(1)/$(2).bin.d/,$$(File_$(1)))
	@echo "  [  AVR  ] $(2) $(1)"
	@make -f make-avr hex dev=$(2) dir=$(1) --no-print-directory

endef

.PHONY: all

$(foreach d, $(dev), $(foreach p, $(programs), $(eval TARGETS+= $(p)/$(d).elf)))
$(foreach d, $(dev), $(foreach p, $(programs), $(eval $(call compile,$(p),$(d)))))

all: $(TARGETS)

%_clean:
	@echo "  [ CLEAN ] $(subst _clean,,$@)"
	@rm -rf $(subst _clean,,$@)

clean: $(addsuffix _clean,$(programs))
