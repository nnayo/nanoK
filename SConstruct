import os

drivers	= [
	'drivers/rs.c',		\
	'drivers/timer0.c',	\
	'drivers/timer1.c',	\
	'drivers/timer2.c',	\
	'drivers/twi.c',	\
	'drivers/sleep.c',	\
	'drivers/spi.c',	\
	'drivers/eeprom.c',	\
]

utils	= [
	'utils/fifo.c',		\
	'utils/time.c',		\
	'utils/state_machine.c',		\
]

externals	= [
	'externals/w5100.c',	\
	'externals/adxl345.c',	\
	'externals/sdcard.c',	\
	'externals/sdcardMgr.c',	\
]

MCU_TARGET      = 'atmega328p'
OPTIMIZE        = '-Os -mcall-prologues -fshort-enums -std=c99 '
includes	= ['.', 'utils', 'drivers', 'externals']
CFLAGS		= '-g -Wall ' + OPTIMIZE + '-mmcu=' + MCU_TARGET

env = Environment(
        ENV = os.environ,       \
	CC = 'avr-gcc',		\
	AR = 'avr-ar',		\
	CFLAGS = CFLAGS,	\
	CPPPATH = includes,	\
)

env.Library('nanoK', drivers + utils + externals)

# suppress reliquat files
env.Alias('clean', '', 'rm -f *~ *o */*.o *.a *.lis')
env.AlwaysBuild('clean')

# display sections size
env.Alias('size', 'libnanoK.a', 'avr-size -t libnanoK.a')
env.AlwaysBuild('size')

# disassemble library
env.Alias('lis', 'libnanoK.a', 'avr-objdump -S -x libnanoK.a > libnanoK.lis')
env.AlwaysBuild('lis')
