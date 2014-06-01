project(lmng)


addDef(MNG_OPTIMIZE_OBJCLEANUP)

addInclude(${libDir}/lpng)
addInclude(${libDir}/zlib)
addInclude(${libDir}/ljpeg)

finishLibrary("${libDir}/${PROJECT_NAME}")
