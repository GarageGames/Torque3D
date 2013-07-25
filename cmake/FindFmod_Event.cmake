#Dushan - changed in order to find automatically files on Windows x86 and Windows x64

FIND_PATH( FMOD_INCLUDE_DIR
    NAMES fmod.h
    PATHS
    /opt/fmodex/api/inc
    /usr/include/fmodex
    "/Developer/FMOD Programmers API Mac/api/inc"
    "C:/Program Files/FMOD SoundSystem/FMOD Programmers API Windows/api/inc"
	"C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Windows/api/inc"
    )

FIND_PATH( FMOD_EVENT_INCLUDE_DIR
    NAMES fmod_event.h
    PATHS
    /opt/fmodex/fmoddesignerapi/api/inc
    "/Developer/FMOD Programmers API Mac/fmoddesignerapi/api/inc"
    "C:/Program Files/FMOD SoundSystem/FMOD Programmers API Windows/fmoddesignerapi/api/inc"
	"C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Windows/fmoddesignerapi/api/inc"
    )

IF( UNIX )
    FIND_LIBRARY( FMOD_LIBRARY
        NAMES fmodex64 fmodex
        PATHS
        /opt/fmodex/api/lib
        "/Developer/FMOD Programmers API Mac/api/lib"
        )
    FIND_LIBRARY( FMOD_EVENT_LIBRARY
        NAMES fmodevent64 fmodevent
        PATHS
        /opt/fmodex/fmoddesignerapi/api/lib
        "/Developer/FMOD Programmers API Mac/fmoddesignerapi/api/lib"
        )
ELSEIF( MSVC )
    FIND_LIBRARY( FMOD_LIBRARY
        NAMES fmodex_vc
        PATHS
        "C:/Program Files/FMOD SoundSystem/FMOD Programmers API Windows/api/lib"
		"C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Windows/api/lib"
        )
    FIND_LIBRARY( FMOD_EVENT_LIBRARY
        NAMES fmod_event
        PATHS
        "C:/Program Files/FMOD SoundSystem/FMOD Programmers API Windows/fmoddesignerapi/api/lib"
		"C:/Program Files (x86)/FMOD SoundSystem/FMOD Programmers API Windows/fmoddesignerapi/api/lib"
        )
ENDIF( )

SET( FMOD_INCLUDE_DIRS
    ${FMOD_INCLUDE_DIR}
    ${FMOD_EVENT_INCLUDE_DIR}
    )
SET( FMOD_LIBRARIES
    ${FMOD_EVENT_NET_LIBRARY}
    ${FMOD_EVENT_LIBRARY}
    ${FMOD_LIBRARY}
    )

IF (FMOD_LIBRARY AND FMOD_INCLUDE_DIR)
    set(FMOD_FOUND TRUE)
ELSE (FMOD_LIBRARY AND FMOD_INCLUDE_DIR)
    set(FMOD_FOUND FALSE)
ENDIF (FMOD_LIBRARY AND FMOD_INCLUDE_DIR)