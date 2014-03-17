project(pcre)

include_directories(.)

add_definitions(-DPCRE_STATIC -DHAVE_CONFIG_H)
addLibrary(/ pcre GLOB)
