
# module openvr

option(TORQUE_OPENVR "Enable openvr module" OFF)
mark_as_advanced(TORQUE_OPENVR)
if(TORQUE_OPENVR)
	if(TORQUE_OPENVR_SDK_PATH STREQUAL "")
		set(TORQUE_OPENVR_SDK_PATH "" CACHE PATH "openvr library path" FORCE)
	endif()
else() # hide variable
	set(TORQUE_OPENVR_SDK_PATH "" CACHE INTERNAL "" FORCE) 
endif() 
 
if(TORQUE_OPENVR)
	# Source
	addPathRec( "${srcDir}/platform/input/openvr" )

	# Includes
	addInclude( "${TORQUE_OPENVR_SDK_PATH}/headers" )
	 
	# Libs
	if( WIN32 ) 
		if( TORQUE_CPU_X64 )
		link_directories( "${TORQUE_OPENVR_SDK_PATH}/lib/win64" )
		else()
		link_directories( "${TORQUE_OPENVR_SDK_PATH}/lib/win32" )
		endif()
		addLib( "openvr_api" )
	endif()

    addDef(TORQUE_OPENVR)
endif()
