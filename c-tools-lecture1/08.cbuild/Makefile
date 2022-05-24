#TOOLDIR	=	$(HOME)/c-tools
INSTDIR	=	$(TOOLDIR)
BINDIR	=	$(INSTDIR)/bin
MANFILE	=	$(INSTDIR)/man/man1/cb.1
DEFS	=	$(INSTDIR)/lib/cbuilddefs

install:	cb
	install -c -m 0755 cb $(BINDIR)/cb
	install -c -m 0644 Rules.pm $(BINDIR)/Rules.pm
	install -c -m 0644 cb.man $(MANFILE)
	install -c -m 0644 cbuilddefs $(DEFS)
