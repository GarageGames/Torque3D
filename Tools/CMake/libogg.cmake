project(libogg)

addLibrary("${libDir}/libogg" libogg GLOB_RECURSE)

set_property(TARGET libogg PROPERTY INCLUDE_DIRECTORIES ${libDir}/libogg/include )
