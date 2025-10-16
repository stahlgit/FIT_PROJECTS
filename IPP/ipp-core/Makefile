##
# IPP Project Task 2 Makefile
# Author: Zbynek Krivka, v2025-02-17
#

# Unlike Merlin, dev-container with VSC gives username as "vscode", so change LOGIN to your login explicitly (be aware of no additional spaces)
LOGIN=xstahl01
TEMP_DIR=temp
TASK=2
STUDENT_DIR=student
SCRIPT=interpret.php

all: check

pack: student/*
	cd $(STUDENT_DIR) && zip -r $(LOGIN).zip  * -x __MACOSX/* .git/* && mv $(LOGIN).zip ../

check: pack vendor
	./is_it_ok.sh $(LOGIN).zip $(TEMP_DIR) $(TASK) 

check-code: vendor
	if [ "${HOSTNAME}" = "merlin.fit.vutbr.cz" ]; then php8.4 vendor/bin/phpstan analyze --level=6 ; else php vendor/bin/phpstan analyze --level=6; fi
	if [ "${HOSTNAME}" = "merlin.fit.vutbr.cz" ]; then php8.4 vendor/bin/phpcs ; else php vendor/bin/phpcs; fi

run-help: interpret.php
	if [ "${HOSTNAME}" = "merlin.fit.vutbr.cz" ]; then php8.4 $(SCRIPT) --help; else php $(SCRIPT) --help; fi

vendor: composer.phar
	if [ "${HOSTNAME}" = "merlin.fit.vutbr.cz" ]; then php8.4 $< install; else php $< install; fi

clean:
	$(RM) *.zip is_it_ok.log
	$(RM) -r $(TEMP_DIR)

