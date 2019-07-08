# Top level makefile, the real shit is at src/Makefile

default: all

.DEFAULT:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@
	cp bin/addb_RR /usr/bin

clearlogs:
	rm -r *\:default
	rm -r *.aof
	rm -r *.rdb

.PHONY: install clearlogs
