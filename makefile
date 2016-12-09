SRCS = \
	drivers/rs.c \
	drivers/timer0.c \
	drivers/timer1.c \
	drivers/timer2.c \
	drivers/twi.c \
	drivers/sleep.c \
	drivers/spi.c \
	drivers/eeprom.c \
	utils/fifo.c \
	utils/time.c \
	utils/state_machine.c \
	utils/majority_voting.c
OBJS = $(patsubst %.c, %.o, $(SRCS))


CFLAGS = \
		 -g -std=c99 \
		 -Wall -Wextra -Werror \
		 -mmcu=atmega328p \
		 -Os -mcall-prologues -fshort-enums \
		 -I. \
		 -I$(TROLL_PROJECTS)/nanoK

CC = avr-gcc
AR = avr-ar

# for dependency autogeneration
# part #1
DEPDIR = .deps
DEPDIRS = $(addprefix .deps/, $(dir $(SRCS)))
$(shell mkdir -p $(DEPDIRS) >/dev/null)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.Td

COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) -c
POSTCOMPILE = mv -f $(DEPDIR)/$*.Td $(DEPDIR)/$*.d

%.o : $(%.o:.o=.c)
%.o : %.c $(DEPDIR)/%.d
	$(COMPILE.c) $(OUTPUT_OPTION) $<
	$(POSTCOMPILE)

$(DEPDIR)/%.d: ;
.PRECIOUS: $(DEPDIR)/%.d
# end part #1


.PHONY: all clean

all: libnanoK.a

libnanoK.a: $(OBJS)
	$(AR) -rs $@ $?

clean:
	rm -fr */*.o libnanoK.a

very_clean: clean
	rm -fr *~ */*~ */*.swp
	rm -fr $(DEPDIR)


# for dependency autogeneration
# part #2
-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))
# end part #2
