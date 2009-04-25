ROOTDIR = $(CURDIR)/..

MAKEFILESDIR = $(ROOTDIR)/makefiles

include $(MAKEFILESDIR)/config.mk

TARGET = FBReader
target = fbreader

ALL_SUBDIRS = src src/migration src/options src/description src/collection src/bookmodel src/formats src/formats/fb2  src/formats/html src/formats/msword src/formats/pdb src/formats/txt src/formats/tcr src/formats/chm src/formats/xhtml src/formats/oeb src/formats/rtf src/formats/openreader src/formats/dummy src/formats/util src/external src/optionsDialog src/fbreader src/encodingOption
ALL_ARCHSUBDIRS = desktop pdaxrom opie zaurus maemo openzaurus pma400 win32

SUBDIRS = src/migration src/options src/description src/collection src/bookmodel src/formats src/formats/fb2 src/formats/html src/formats/msword src/formats/pdb src/formats/txt src/formats/tcr src/formats/chm src/formats/xhtml src/formats/oeb src/formats/rtf src/formats/openreader src/formats/util src/external src/optionsDialog src/fbreader src/encodingOption

all: .resources
	@for subdir in $(SUBDIRS); do \
		if ! $(MAKE) -C $$subdir -f $(MAKEFILESDIR)/subdir.mk; then \
			exit 1; \
		fi; \
	done;
	@echo -n "Linking $(TARGET) ..."
#	@$(LD) $(LDFLAGS) -o $(TARGET) `find src -name *.o` -L$(ROOTDIR)/zlibrary/text -lzltext $(CORE_LIBS)
#	@$(LD) $(LDFLAGS) -o $(TARGET) `find src -name *.o` -L$(ROOTDIR)/zlibrary/text -lzltext $(CORE_LIBS)
#	$(LD) $(LDFLAGS) -Wl,-soname,$(TARGET).so -o $(TARGET).so `find src -name *.o` -L$(ROOTDIR)/zlibrary/text -lzltext $(CORE_LIBS)

ifeq "$(EMULATOR)" "1" 
	$(LD) $(LDFLAGS) -Wl,-soname,$(TARGET).exe -o $(TARGET).exe `find src -name *.o` $(ROOTDIR)/images/c/images.o -L$(ROOTDIR)/zlibrary/text -lzltext $(CORE_LIBS) -linkview -lfreetype -lgdi32 -ljpeg -lrt -lz #-liconv
else
	$(LD) $(LDFLAGS) -Wl,-soname,$(target).app -o $(target).app `find src -name *.o` $(ROOTDIR)/images/c/images.o -L$(ROOTDIR)/zlibrary/text -lzltext -L/arm-linux/lib $(CORE_LIBS) -linkview -lfreetype -ljpeg -lrt -lz
endif
	@echo " OK"

FBSHAREDIR = $(DESTDIR)$(subst %APPLICATION_PATH%\\\\,/,$(SHAREDIR))/FBReader
VARIANT = $(TARGET_ARCH)
ifneq "$(RESOLUTION)" "" 
  VARIANT = $(TARGET_ARCH)_$(RESOLUTION)
endif

APPIMAGEDIR_REAL = $(subst \\\\,/,$(subst %APPLICATION_PATH%\\\\,/,$(subst %application_name%,$(target),$(subst %APPLICATION_NAME%,$(TARGET),$(APPIMAGEDIR)))))

do_install:
	@install -d $(DESTDIR)$(BINDIR)
	@install $(TARGET) $(DESTDIR)$(BINDIR)/FBReader
	@install -d $(FBSHAREDIR)
	@install -d $(FBSHAREDIR)/help
	@./scripts/install_help.sh $(VARIANT) $(FBSHAREDIR)/help
	@install -d $(FBSHAREDIR)/formats/html
	@install -m 0644 data/formats/html/html.ent $(FBSHAREDIR)/formats/html
	@install -d $(FBSHAREDIR)/formats/xhtml
	@install -m 0644 $(wildcard data/formats/xhtml/*.ent) $(FBSHAREDIR)/formats/xhtml
	@install -d $(FBSHAREDIR)/default
	@./scripts/install_toolbar_and_menu.sh $(VARIANT) $(FBSHAREDIR)/default
	@install -m 0644 data/default/external.$(TARGET_ARCH).xml $(FBSHAREDIR)/default/external.xml
	@if [ -f data/default/messages.$(TARGET_ARCH).xml ]; then \
		install -m 0644 data/default/messages.$(TARGET_ARCH).xml $(FBSHAREDIR)/default/messages.xml; \
	fi
	@install -d $(FBSHAREDIR)/resources
	@install -m 0644 $(wildcard data/resources/*.xml) $(FBSHAREDIR)/resources
	@install -d $(DESTDIR)$(APPIMAGEDIR_REAL)
	@install -m 0644 $(wildcard icons/toolbar/$(VARIANT)/*.*) $(DESTDIR)$(APPIMAGEDIR_REAL)
	@install -m 0644 $(wildcard icons/filetree/$(VARIANT)/*.*) $(DESTDIR)$(APPIMAGEDIR_REAL)
	@install -m 0644 $(wildcard icons/booktree/$(VARIANT)/*.*) $(DESTDIR)$(APPIMAGEDIR_REAL)
	@make -C $(TARGET_ARCH) RESOLUTION=$(RESOLUTION) install

clean:
	@for subdir in $(ALL_SUBDIRS); do \
		$(MAKE) -C $$subdir -f $(MAKEFILESDIR)/subdir.mk clean; \
	done;
	@for subdir in $(ALL_ARCHSUBDIRS); do \
		cd $$subdir; make clean; cd ..; \
	done;
	@$(RM) $(TARGET) err
