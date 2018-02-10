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

TEST_SUFFIX=

STAGES:=1 2 3 4

$(STAGES) :
	@printf "Running Stage $@ ... "
	@if [[ "$@${TEST_SUFFIX}" == "3-test" ]]; then                                   \
		./modmul stage$@${TEST_SUFFIX} < test_data/stage$@.input > stage$@.test; \
	elif [[ "$@${TEST_SUFFIX}" == "4-self-test" ]]; then                             \
		rm -f s4in.test;                                                         \
		for I in {1..60..6};                                                     \
		do                                                                       \
			sed "$$I,$$(($$I+3))! d;" test_data/stage4.input >> s4in.test;   \
			head -n2 stage3.test >> s4in.test;                               \
			sed -i -e 1,2d stage3.test;                                      \
		done;                                                                    \
		./modmul stage$@ < s4in.test > stage$@.test;                             \
	else                                                                             \
		./modmul stage$@ < test_data/stage$@.input > stage$@.test;               \
	fi
	@if [[ "$@${TEST_SUFFIX}" == "3-self-test" ]]; then                     \
		printf "\\e[33mSELF TESTING\\e[39m\n";                          \
	else                                                                    \
		if diff stage$@.test test_data/stage$@.output > /dev/null; then \
			printf "\\e[32mPASSED\\e[39m\n";                        \
		else                                                            \
			printf "\\e[31mFAILED\\e[39m\n";                        \
		fi                                                              \
	fi

_test : modmul $(STAGES)
	@rm s*.test

test:
	@$(MAKE) _test --no-print-directory TEST_SUFFIX=-test

self-test:
	@$(MAKE) _test --no-print-directory TEST_SUFFIX=-self-test
