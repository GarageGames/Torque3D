# this is a template file that should help you write a new cmake build script for a new library


# 1st thing: the project name
project(pcre)

# add the paths where the source code is

addPath("${libDir}/pcre")
addPathRec("${libDir}/pcre")

# then add definitions
addDef(PCRE_STATIC)
addDef(HAVE_CONFIG_H)

# and maybe more include paths
addInclude(${libDir}/libvorbis/include)
addInclude(${libDir}/libogg/include)

# finally: add finishLibrary()
finishLibrary()
