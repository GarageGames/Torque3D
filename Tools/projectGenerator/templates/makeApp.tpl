# I release this sample under the MIT license: free for any use, provided 
# you hold me harmless from any such use you make, and you retain my 
# copyright on the actual sources.
# Copyright 2005 Jon Watte.

# App Template

APPNAME := {$projectOffset}../../{$gameFolder}/{$projOutName}
{assign var="dirWalk" value=$fileArray}
SOURCES := {foreach from=$dirWalk item=file key=key}
{include file="make_fileRecurse.tpl" dirWalk=$file}
{/foreach}

LDFLAGS := -g -m32
LDLIBS := -lstdc++ -lm -lpthread -lrt
{foreach item=def from=$projLibs}LDLIBS += -l{$def}
{/foreach}

CFLAGS := -MMD -I. -Wfatal-errors -m32 -msse -mmmx -march=i686 -pipe

{foreach item=def from=$projIncludes}CFLAGS += -I{$def}
{/foreach}

CFLAGS += -DUNICODE
CFLAGS += -DLINUX

{foreach item=def from=$projDefines}CFLAGS += -D{$def}
{/foreach}

CFLAGS_DEBUG := $(CFLAGS) -ggdb
CFLAGS_DEBUG += -DTORQUE_DEBUG
CFLAGS_DEBUG += -DTORQUE_DEBUG_GUARD
CFLAGS_DEBUG += -DTORQUE_NET_STATS

CFLAGS += -O0

CC := gcc
LD := gcc

APP_TARGETS += $(APPNAME)
APP_TARGETS_DEBUG += $(APPNAME)_DEBUG

OBJS_{$projName} := $(patsubst ../../../../Engine/source/%,Release/{$projName}/%.o,$(SOURCES))
OBJS_{$projName} += $(patsubst ../../source/%, Release/{$projName}/%.o,$(SOURCES))
OBJS_{$projName} := $(filter %.o, $(OBJS_{$projName}))
OBJS_{$projName}_DEBUG := $(patsubst ../../../../Engine/source/%,Debug/{$projName}/%.o,$(SOURCES))
OBJS_{$projName}_DEBUG += $(patsubst ../../source/%, Debug/{$projName}/%.o,$(SOURCES))
OBJS_{$projName}_DEBUG := $(filter %.o, $(OBJS_{$projName}_DEBUG))

# Deriving the actual prerequisite list name to use from the target 
# name in the shell command is the "secret sauce" that makes this all 
# work.
#
$(APPNAME):	$(OBJS_{$projName}) $(LIB_TARGETS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS_{$projName}) $(LIB_TARGETS) $(LDLIBS)
   
$(APPNAME)_DEBUG:	$(OBJS_{$projName}_DEBUG) $(LIB_TARGETS_DEBUG)
	$(LD) $(LDFLAGS) -o $@ $(OBJS_{$projName}_DEBUG) $(LIB_TARGETS_DEBUG) $(LDLIBS)

Release/{$projName}/%.asm.o:	../../../../Engine/source/%.asm
	@mkdir -p $(dir $@)
	nasm -f elf $< -o $@

Release/{$projName}/%.o:	../../../../Engine/source/%
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

Release/{$projName}/%.o:	../../source/%
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@
   
Debug/{$projName}/%.asm.o:	../../../../Engine/source/%.asm
	@mkdir -p $(dir $@)
	nasm -f elf $< -o $@

Debug/{$projName}/%.o:	../../../../Engine/source/%
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS_DEBUG) $< -o $@

Debug/{$projName}/%.o:	../../source/%
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS_DEBUG) $< -o $@
   
release_{$projName}: $(APPNAME)
debug_{$projName}: $(APPNAME)_DEBUG

.PHONY: debug_{$projName} release_{$projName}

DEPS += $(patsubst %.o,%.d,$(OBJS_{$projName}))
DEPS += $(patsubst %.o,%.d,$(OBJS_{$projName}_DEBUG))

APPNAME :=
SOURCES :=

