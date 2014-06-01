project(lpng)

# addDef(PNG_NO_ASSEMBLER_CODE)

addInclude(${libDir}/zlib)

finishLibrary("${libDir}/${PROJECT_NAME}")
