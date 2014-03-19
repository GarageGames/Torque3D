# this is a template file that should help you write a new cmake build script for a new library


# 1st thing: the project name
project(pcre)

# 2nd: add the paths where the source code is

addPath("${libDir}/pcre")
addPathRec("${libDir}/pcre")

# 3rd: add addStaticLib()
addStaticLib()

# then add definitions
addDef(PCRE_STATIC)
addDef(HAVE_CONFIG_H)

# and maybe more include paths
addInclude(${libDir}/libvorbis/include)
addInclude(${libDir}/libogg/include)
