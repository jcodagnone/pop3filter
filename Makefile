.PHONY : clean Changelog

VERSION = 0.0.0

all:	sub-src

sub-src: sub-lib
	$(MAKE) -C src 

sub-lib:
	$(MAKE) -C lib

clean-src:
	$(MAKE) -C src clean

clean-lib:
	$(MAKE) -C lib clean
	
clean:	clean-src clean-lib
	$(MAKE) -C src clean

Changelog:
	bin/cvs2cl.pl --distributed --ignore ChangeLog --revisions --tags \
	-U ./cvs2cl.ufile
