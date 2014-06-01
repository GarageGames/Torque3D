project(pcre)

addDef(PCRE_STATIC)
addDef(HAVE_CONFIG_H)

finishLibrary("${libDir}/pcre")

if(WIN32)
    set_property(TARGET pcre PROPERTY COMPILE_FLAGS /TP) #/TP = compile as C++
endif()