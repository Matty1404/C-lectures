LIBDIR = $(TOOLDIR)/lib/$(ARCH)
INCDIR = $(TOOLDIR)/include
MAN1DIR = $(TOOLDIR)/man/man1
MAN3DIR = $(TOOLDIR)/man/man3
DESTDIR	= $(HOME)/public_html/c-tools-2022/lecture2
TARBALLNAME	=	c-tools-lecture2

all:
	echo 'Choose a target'

mkdir:
	mkdir -p $(LIBDIR) $(INCDIR) $(MAN1DIR) $(MAN3DIR)

clean:
	-for i in */; do \
		(cd $$i; test -f Makefile && make clean); \
	done

install:
	-mkdir -p $(DESTDIR)
	install -m0644 index.html $(DESTDIR)
	install -m0755 $(TARBALLNAME).tgz $(DESTDIR)
	install -m0644 lecture.pdf $(DESTDIR)
	install -m0644 lecture-handout.pdf $(DESTDIR)

uninstall:
	rm -f $(DESTDIR)/index.html
	rm -f $(DESTDIR)/$(TARBALLNAME).tgz
	rm -f $(DESTDIR)/lecture.pdf
	rm -f $(DESTDIR)/lecture-handout.pdf

tarball:	clean
	-mkdir -p tarball/$(TARBALLNAME)
	cp -pr [01]* lecture Makefile README tarball/$(TARBALLNAME)
	cd tarball; tar -czf ../$(TARBALLNAME).tgz --exclude .git --exclude '.nfs*' $(TARBALLNAME)
	rm -rf tarball
