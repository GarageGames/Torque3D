project(lpng)

addLibrary("${libDir}/lpng" lpng GLOB)

#set_property(TARGET lpng PROPERTY COMPILE_DEFINITIONS PNG_NO_ASSEMBLER_CODE)
set_property(TARGET lpng PROPERTY INCLUDE_DIRECTORIES ${libDir}/zlib)
