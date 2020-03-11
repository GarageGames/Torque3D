cmake_minimum_required(VERSION 3.0.2)

project(native-tools)

include(CheckLibraryExists)

set(CPP_DEFS )
if(WIN32)
    set(CPP_DEFS ${CPP_DEFS} _WIN32)
endif(WIN32)

check_library_exists(m pow "" HAVE_LIBM)

add_executable(bin2h bin2h.c)
# Enforce no dressing for executable names, so the main script can find it
set_target_properties(bin2h PROPERTIES OUTPUT_NAME bin2h)
# Avoid configuration-dependent subdirectories while building with Visual Studio
set_target_properties(bin2h PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}")
set_target_properties(bin2h PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(bin2h PRIVATE ${CPP_DEFS})

add_executable(bsincgen bsincgen.c)
set_target_properties(bsincgen PROPERTIES OUTPUT_NAME bsincgen)
set_target_properties(bsincgen PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}")
set_target_properties(bsincgen PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}")
target_compile_definitions(bsincgen PRIVATE ${CPP_DEFS})
if(HAVE_LIBM)
    target_link_libraries(bsincgen m)
endif(HAVE_LIBM)
