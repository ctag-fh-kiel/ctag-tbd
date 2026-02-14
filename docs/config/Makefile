TBD_CODE           ?= $(PWD)

SOURCEDIR          =  $(TBD_CODE)/docs

TBD_BUILD          ?= $(TBD_CODE)/build
BUILDDIR           =  $(TBD_BUILD)/docs

CONFDIR            =  "$(SOURCEDIR)/config"

SPHINXOPTS         ?=
SPHINXBUILD        ?= sphinx-build
SPHINXCONF         = --conf-dir "$(CONFDIR)"

CODEDIRS          =  $(TBD_CODE)/components
CPP_SOURCES        := $(shell find $(CODE_DIRS) -type f -name \*.cpp -or -name \*.c -or -name \*.hpp -or -name \*.h)

builddir:
	mkdir -p "$(BUILDDIR)"

doxygen: builddir $(CPP_SOURCES)
	(cat $(SOURCEDIR)/config/static_doxyfile; echo "OUTPUT_DIRECTORY = $(BUILDDIR)"; echo "INPUT = $(CODEDIRS)") > $(SOURCEDIR)/config/Doxyfile
	doxygen "$(SOURCEDIR)/config/Doxyfile"

help:
	@$(SPHINXBUILD) -M help "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(SPHINXCONF) $(O)

.PHONY: help Makefile

html: Makefile
	@$(SPHINXBUILD) -M $@ "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(SPHINXCONF) $(O)


clean:
	@$(SPHINXBUILD) -M clean "$(SOURCEDIR)" "$(BUILDDIR)" $(SPHINXOPTS) $(SPHINXCONF) $(O)

