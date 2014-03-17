cmake_minimum_required (VERSION 2.6)

project("Torque3D")

# output folders
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

SET(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/My Projects/Empty" CACHE INTERNAL "Prefix prepended to install directories" FORCE)

# the projects
add_subdirectory(Engine)
