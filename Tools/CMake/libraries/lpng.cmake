project(lpng)

addStaticLib("${libDir}/${PROJECT_NAME}")

# addDef(PNG_NO_ASSEMBLER_CODE)

addInclude(${libDir}/zlib)
