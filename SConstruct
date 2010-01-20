import os

drivers	= [
	'drivers/rs.c',		\
	'drivers/timer0.c',	\
	'drivers/timer2.c',	\
	'drivers/twi.c',	\
	'drivers/sleep.c',	\
	'drivers/spi.c',	\
	'drivers/eeprom.c',	\
]

utils	= [
	'utils/fifo.c',		\
	'utils/time.c',		\
]

externals	= [
	'externals/w5100.c',	\
	'externals/adxl345.c',	\
	'externals/sdcard.c',	\
	'externals/sdcardMgr.c',	\
]

MCU_TARGET      = 'atmega324p'
OPTIMIZE        = '-Os -mcall-prologues -fshort-enums '
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
env.Alias('clean', '', 'rm -f *~ *o */*.o *.a')
env.AlwaysBuild('clean')

# display sections size
env.Alias('size', 'libnanoK.a', 'avr-size -t libnanoK.a')
env.AlwaysBuild('size')
