# I release this sample under the MIT license: free for any use, provided 
# you hold me harmless from any such use you make, and you retain my 
# copyright on the actual sources.
# Copyright 2005 Jon Watte.

# This sets the option for how many instances of gcc we'll run at the
# same time, one per CPU core in this case. This speeds up build time.
# Adjust to your amount of cores.
OPTIONS := --jobs=4

DEPS := 
LIB_TARGETS :=
LIB_TARGETS_DEBUG := 
SHARED_LIB_TARGETS := 
SHARED_LIB_TARGETS_DEBUG := 
APP_TARGETS := 
APP_TARGETS_DEBUG := 

all: debug release

clean:
	rm -rf Release
	rm -rf Debug
	rm -rf {$libDir}/compiled/Make
	rm -f $(SHARED_LIB_TARGETS) $(SHARED_LIB_TARGETS_DEBUG)
	rm -f $(APP_TARGETS) $(APP_TARGETS_DEBUG)
	rm -f *.d

.PHONY:	all debug release clean

{foreach item=project from=$projects}
-include x {$project->name}
{/foreach}
-include x $(DEPS)

# it's important that I specify the libs and apps targets
# after they've actually been defined by the includes above.
#
release: $(LIB_TARGETS) $(SHARED_LIB_TARGETS) $(APP_TARGETS)
	@echo Built libraries: $(LIB_TARGETS)
	@echo Built shared libraries: $(SHARED_LIB_TARGETS)
	@echo Build apps: $(APP_TARGETS)

debug: $(LIB_TARGETS_DEBUG) $(SHARED_LIB_TARGETS_DEBUG) $(APP_TARGETS_DEBUG)
	@echo Built libraries: $(LIB_TARGETS_DEBUG)
	@echo Built shared libraries: $(SHARED_LIB_TARGETS_DEBUG)
	@echo Build apps: $(APP_TARGETS_DEBUG)
