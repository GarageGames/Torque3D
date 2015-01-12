#!/bin/sh

find "`dirname "$0"`" -type f \( -name "prefs.cs" -or -name "config.cs" -or -name "banlist.cs" -or -name "prefs.cs.dso" -or -name "config.cs.dso" -or -name "banlist.cs.dso" \) -exec rm {} \;
