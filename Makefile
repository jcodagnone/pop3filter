
VERSION = 0.0.0

all:	sub-src

sub-src: sub-lib
	$(MAKE) -C src

sub-lib:
	$(MAKE) -C lib
	
clean:
	$(MAKE) -C src
