include(basics.cmake)

setupVersionNumbers()

#the libs
include(libraries/lmng.cmake)
include(libraries/lpng.cmake)
include(libraries/lungif.cmake)
include(libraries/zlib.cmake)
include(libraries/ljpeg.cmake)
include(libraries/tinyxml.cmake)
include(libraries/opcode.cmake)
include(libraries/squish.cmake)
include(libraries/collada.cmake)
include(libraries/pcre.cmake)
include(libraries/convexDecomp.cmake)
if(TORQUE_SFX_VORBIS)
    include(libraries/libvorbis.cmake)
    include(libraries/libogg.cmake)
endif()
if(TORQUE_THEORA)
    include(libraries/libtheora.cmake)
endif()

# the main engine, should come last
include(torque3d.cmake)

#setupPackaging()
