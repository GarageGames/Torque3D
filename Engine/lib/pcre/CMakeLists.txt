project(pcre)

include_directories(.)

add_definitions(-DPCRE_STATIC -DHAVE_CONFIG_H /TP) # /TP = compile as C++
addLibrary(/ pcre GLOB)
