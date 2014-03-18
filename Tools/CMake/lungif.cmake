project(lungif)

addLibrary("${libDir}/lungif" lungif GLOB)

set_property(TARGET lungif PROPERTY COMPILE_DEFINITIONS _GBA_NO_FILEIO)
