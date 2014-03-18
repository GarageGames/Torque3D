project(lmng)

addLibrary("${libDir}/lmng" lmng GLOB)

set_property(TARGET lmng PROPERTY COMPILE_DEFINITIONS MNG_OPTIMIZE_OBJCLEANUP)
set_property(TARGET lmng PROPERTY INCLUDE_DIRECTORIES ${libDir}/lpng ${libDir}/zlib ${libDir}/ljpeg)
