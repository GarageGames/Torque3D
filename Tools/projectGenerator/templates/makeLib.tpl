# I release this sample under the MIT license: free for any use, provided 
# you hold me harmless from any such use you make, and you retain my 
# copyright on the actual sources.
# Copyright 2005 Jon Watte.

# Static Library Template

# If this errors out, you didn't specify the right variable value, or 
# you're including the wrong template makefile.

LIBNAME := {$projName}
{assign var="dirWalk" value=$fileArray}
SOURCES := {foreach from=$dirWalk item=file key=key}
{include file="make_fileRecurse.tpl" dirWalk=$file}
{/foreach}

LDFLAGS_{$projName} := -g -m32
#LDLIBS_{$projName} := -lstdc++
CFLAGS_{$projName} := -MMD -I. -m32 -msse -mmmx -march=i686

{foreach item=def from=$projIncludes}CFLAGS_{$projName} += -I{$def}
{/foreach}

CFLAGS_{$projName} += -DUNICODE
CFLAGS_{$projName} += -DLINUX

{foreach item=def from=$projDefines}CFLAGS_{$projName} += -D{$def}
{/foreach}

CFLAGS_DEBUG_{$projName} := $(CFLAGS_{$projName}) -ggdb
CFLAGS_DEBUG_{$projName} += -DTORQUE_DEBUG
CFLAGS_DEBUG_{$projName} += -DTORQUE_DEBUG_GUARD
CFLAGS_DEBUG_{$projName} += -DTORQUE_NET_STATS

CFLAGS_{$projName} += -O3

CC := gcc
LD := gcc

TARGET_{$projName} := {$libDir}/compiled/Make/{$projName}.a
TARGET_{$projName}_DEBUG := {$libDir}/compiled/Make/{$projName}_DEBUG.a

LIB_TARGETS += $(TARGET_{$projName})
LIB_TARGETS_DEBUG += $(TARGET_{$projName}_DEBUG)

OBJS_{$projName} := $(patsubst {$libDir}%,Release/{$projName}/%.o,$(SOURCES))
OBJS_{$projName}_DEBUG := $(patsubst {$libDir}%,Debug/{$projName}/%.o,$(SOURCES))

# Deriving the variable name from the target name is the secret sauce 
# of the build system.
#
$(TARGET_{$projName}):	$(OBJS_{$projName})
	@mkdir -p $(dir $@)
	ar cr $@ $(OBJS_{$projName})
   
$(TARGET_{$projName}_DEBUG):	$(OBJS_{$projName}_DEBUG)
	@mkdir -p $(dir $@)
	ar cr $@ $(OBJS_{$projName}_DEBUG)

Release/{$projName}/%.o:	{$libDir}%
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS_{$projName}) $< -o $@
   
Debug/{$projName}/%.o:	{$libDir}%
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS_DEBUG_{$projName}) $< -o $@
   
release_{$projName}: $(TARGET_{$projName})
debug_{$projName}: $(TARGET_{$projName}_DEBUG)

.PHONY: debug_{$projName} release_{$projName}

DEPS += $(patsubst %.o,%.d,$(OBJS_{$projName}))
DEPS += $(patsubst %.o,%.d,$(OBJS_{$projName}_DEBUG))

