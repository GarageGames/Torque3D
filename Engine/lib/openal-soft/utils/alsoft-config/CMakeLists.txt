project(alsoft-config)

include_directories("${alsoft-config_BINARY_DIR}")

# Need Qt 4.8.0 or newer for the iconset theme attribute to work
find_package(Qt4 4.8.0 COMPONENTS QtCore QtGui)
if(QT4_FOUND)
    include(${QT_USE_FILE})

    set(alsoft-config_SRCS  main.cpp
                            mainwindow.cpp
        )

    set(alsoft-config_UIS  mainwindow.ui)
    QT4_WRAP_UI(UIS ${alsoft-config_UIS})

    set(alsoft-config_MOCS  mainwindow.h)
    QT4_WRAP_CPP(MOCS ${alsoft-config_MOCS})

    add_executable(alsoft-config ${alsoft-config_SRCS} ${UIS} ${RSCS} ${TRS} ${MOCS})
    target_link_libraries(alsoft-config ${QT_LIBRARIES})
    set_property(TARGET alsoft-config APPEND PROPERTY COMPILE_FLAGS ${EXTRA_CFLAGS})
    set_target_properties(alsoft-config PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OpenAL_BINARY_DIR})

    install(TARGETS alsoft-config
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()
