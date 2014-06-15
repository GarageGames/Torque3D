project(libtheora)

addPath( "${libDir}/libtheora" )
addPathRec( "${libDir}/libtheora/include" )
addPath( "${libDir}/libtheora/lib" )
addPath( "${libDir}/libtheora/lib/dec" )
addPath( "${libDir}/libtheora/lib/enc" )

if(WIN32)
	addPath( "${libDir}/libtheora/lib/dec/x86_vc" )
	addPath( "${libDir}/libtheora/lib/enc/x86_32_vs" )
else()
	addPath( "${libDir}/libtheora/lib/dec/x86" )
	addPath( "${libDir}/libtheora/lib/enc/x86_32" )
endif()

addDef(TORQUE_OGGTHEORA)
addDef(TORQUE_OGGVORIBS)
addInclude(${libDir}/libogg/include)
addInclude(${libDir}/libtheora/include)

finishLibrary()
