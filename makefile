PRG		= nanoK
OBJ		= nanoK.o
OBJS_UTILS	= utils/fifo.o utils/mutex.o utils/time.o
OBJS_DRIVERS	= drivers/rs.o drivers/timer0.o drivers/timer2.o drivers/twi.o

#MCU_TARGET     = atmega128
#MCU_TARGET     = atmega8
MCU_TARGET	= atmega32
OPTIMIZE	= -Os -mcall-prologues -fshort-enums -Wall

# NANOK_CONF_PATH shall set from the makefile calling this one by -e option
# it is the path to the nanoK_conf.h file
DEFS		=
LIBS		= -I. -I$(NANOK_CONF_PATH)


# You should not have to change anything below here.

CC		= avr-gcc
AR		= avr-ar

# Override is only needed by avr-lib build system.

override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) $(LIBS)
override LDFLAGS       = -Wl,-Map,$(PRG).map

OBJCOPY		= avr-objcopy
OBJDUMP		= avr-objdump

all: libnanoK.a
	avr-size -t libnanoK.a

libnanoK.a: $(OBJ) $(OBJS_DRIVERS) $(OBJS_UTILS)
	$(AR) rcv $@ $^

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(PRG).elf *.bak 
	rm -fr $(OBJ) $(OBJS_DRIVERS) $(OBJS_UTILS)
	rm -rf *.lst *.map
	rm -fr *~ */*~
	rm -f libnanoK.a

very_clean: clean
	rm -fr *.*~
	rm -fr *.*bak
	rm -fr *.*swp


lst:  $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

