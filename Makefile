
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

Changelog-src:
	$(MAKE) -C src Changelog
Changelog-lib:
	$(MAKE) -C lib Changelog
Changelog:	Changelog-lib Changelog-src
	rcs2log > Changelog
