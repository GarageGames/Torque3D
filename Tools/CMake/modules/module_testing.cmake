option(TORQUE_TESTS_ENABLED "TORQUE_TESTS_ENABLED" OFF)

if(TORQUE_TESTS_ENABLED)

    # Project defines
    addDef( "TORQUE_TESTS_ENABLED" )
    addDef( "_VARIADIC_MAX" 10 )

    # Add source files
    addPathRec( "${srcDir}/testing" )

    # Add include paths
    addInclude( "${libDir}/gtest/fused-src/" )

endif()