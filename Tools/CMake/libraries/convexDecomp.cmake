project(convexDecomp)

if(UNIX)
	addDef(LINUX)
endif()

finishLibrary("${libDir}/convexDecomp")
