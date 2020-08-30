programs=main_master main_master_spi main main_spi main_csma main_csma_spi

File_main_master=main_node eeprom_layout RNet IO uart .circularbuffer
args_main_master=-D RNET_MASTER -I ../shared/lib

File_main_master_spi=$(File_main_master)
args_main_master_spi=-D IO_SPI -D RNET_MASTER -I ../shared/lib

File_main=main_node eeprom_layout RNet IO uart .circularbuffer
args_main=-I ../shared/lib

File_main_spi=$(File_main)
args_main_spi=-D IO_SPI -I ../shared/lib

File_main_csma=main_node_csma eeprom_layout RNet_csma IO uart .circularbuffer
args_main_csma=-D RNET_CSMA -I ../shared/lib

File_main_csma_spi=$(File_main_csma)
args_main_csma_spi=-D IO_SPI -D RNET_CSMA -I ../shared/lib

dev=atmega328p# atmega64a atmega2560

.PHONY: all clean

define compile

build/$(1)/$(2)/$(2).flash:
	@echo "(  AVR  ) $(1)\t$(2)"
	@+$(MAKE) -f make-avr all DEVICE=$(1) args="$$(args_$(2))" output=$(2) files="$$(File_$(2))" --no-print-directory

buildall: build/$(1)/$(2)/$(2).flash
endef

$(foreach d,$(dev), $(foreach p,$(programs), $(eval $(call compile,$(d),$(p)))))

copyshared:
	cp ../shared/src/circularbuffer.cpp .circularbuffer.cpp

all: copyshared buildall
	@:

clean:
	rm -rf build
