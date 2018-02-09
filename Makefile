# Copyright (C) 2017 Daniel Page <csdsp@bristol.ac.uk>
#
# Use of this source code is restricted per the CC BY-NC-ND license, a copy of 
# which can be found via http://creativecommons.org (and should be included as 
# LICENSE.txt within the associated archive or repository).

modmul : $(wildcard *.[ch])
	@gcc -Wall -std=gnu99 -O3 -o ${@} $(filter %.c, ${^}) -lgmp

.DEFAULT_GOAL = all

all   : modmul

clean : 
	@rm -f core modmul

STAGES:=1 2 3 4

$(STAGES) :
	@printf "Running Stage $@ ... "
	@./modmul stage$@ < test_data/stage$@.input > stage$@.test
	@if diff stage$@.test test_data/stage$@.output > /dev/null; then \
		printf "\\e[32mPASSED\\e[39m\n";                         \
	else                                                             \
		printf "\\e[31mFAILED\\e[39m\n";                         \
	fi
	@rm stage$@.test

test : $(STAGES) modmul
