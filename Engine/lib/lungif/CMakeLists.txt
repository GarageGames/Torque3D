project(ungif)

add_definitions(-D_GBA_NO_FILEIO)

include_directories(.)

addLibrary(/ ungif GLOB)
