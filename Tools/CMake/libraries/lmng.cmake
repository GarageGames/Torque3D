project(lmng)

addStaticLib("${libDir}/${PROJECT_NAME}")

addDef(MNG_OPTIMIZE_OBJCLEANUP)

addInclude(${libDir}/lpng)
addInclude(${libDir}/zlib)
addInclude(${libDir}/ljpeg)
