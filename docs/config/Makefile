
TBD_PROJECT_DIR    ?= $(PWD)

SOURCEDIR          =  $(TBD_PROJECT_DIR)/docs
BUILDDIR           =  $(TBD_PROJECT_DIR)/build/docs
CONFDIR            =  "$(SOURCEDIR)/config"

SPHINXOPTS         ?=
SPHINXBUILD        ?= sphinx-build
SPHINXCONF         = --conf-dir "$(CONFDIR)"

CODE_DIRS          =  $(TBD_PROJECT_DIR)/tbd/tbd_sound_processor
CPP_SOURCES        := $(shell find $(CODE_DIRS) -type f -name \*.cpp -or -name \*.c -or -name \*.hpp -or -name \*.h)

builddir:
	mkdir -p "$(BUILDDIR)"

doxygen: builddir $(CPP_SOURCES)
	(cat $(SOURCEDIR)/config/static_doxyfile; echo "OUTPUT_DIRECTORY = $(BUILDDIR)"; echo "INPUT = $(CODE_DIRS)") > $(SOURCEDIR)/config/Doxyfile
	doxygen "$(SOURCEDIR)/config/Doxyfile"

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(SPHINXCONF) $(O)

.PHONY: help Makefile

html: Makefile
	
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(SPHINXCONF) $(O)


clean:
	@$(SPHINXBUILD) -M clean "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(SPHINXCONF) $(O)

