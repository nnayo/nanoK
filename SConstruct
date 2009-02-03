drivers	= [
	'drivers/rs.c',		\
	'drivers/timer0.c',	\
	'drivers/timer2.c',	\
	'drivers/twi.c',	\
	'drivers/sleep.c',	\
]

utils	= [
	'utils/fifo.c',		\
	'utils/time.c',		\
]


MCU_TARGET      = 'atmega32 '
OPTIMIZE        = '-Os -mcall-prologues -fshort-enums '
includes	= ['.', 'utils', 'drivers']
CFLAGS		= '-g -Wall ' + OPTIMIZE + '-mmcu=' + MCU_TARGET

env = Environment(
	CC = 'avr-gcc',		\
	AR = 'avr-ar',		\
	CFLAGS = CFLAGS,	\
	CPPPATH = includes,	\
)

env.Library('nanoK', drivers + utils)

# suppress reliquat files
env.Alias('clean', '', 'rm -f *~ *o */*.o *.a')
env.AlwaysBuild('clean')
