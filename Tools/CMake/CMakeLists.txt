include(basics.cmake)

setupVersionNumbers()

#the libs
include(lmng.cmake)
include(lpng.cmake)
include(lungif.cmake)
include(zlib.cmake)
include(ljpeg.cmake)
include(tinyxml.cmake)
include(opcode.cmake)
include(squish.cmake)
include(collada.cmake)
include(pcre.cmake)
include(convexDecomp.cmake)
if(TORQUE_SFX_VORBIS)
    include(libvorbis.cmake)
    include(libogg.cmake)
endif()
if(TORQUE_THEORA)
    include(libtheora.cmake)
endif()

# the main engine, should come last
include(torque3d.cmake)

#setupPackaging()