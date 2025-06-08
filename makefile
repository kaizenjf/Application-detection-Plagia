.PHONY: clean dist

dist: clean
	tar -hzcf "$(CURDIR).tar.gz" hashtable/* jdis_test/* holdall/* makefile

clean:
	$(MAKE) -C hashtable_test clean
