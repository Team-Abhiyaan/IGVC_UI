set(SOURCES
    main.cpp
    mainwindow.cpp
    execbox.cpp
    parameter.cpp
    spoiler.cpp
)

set(HEADERS
    mainwindow.h
    execbox.h
    parameter.h
    spoiler.h
)

set(UIS
    mainwindow.ui
)

set(RESOURCES
    resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(IGVC_UI
        MANUAL_FINALIZATION
        ${SOURCES}
        ${HEADERS}
        ${UIS}
        ${RESOURCES}
    )
else()
    add_executable(IGVC_UI
        ${SOURCES}
        ${HEADERS}
        ${UIS}
        ${RESOURCES}
    )
endif()

# Link libraries
target_link_libraries(IGVC_UI PRIVATE Qt${QT_VERSION_MAJOR}::Widgets yaml-cpp::yaml-cpp)

# Bundle settings for macOS and Windows packaging
set_target_properties(IGVC_UI PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.IGVC_UI
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

# Install rules for packaging
include(GNUInstallDirs)
install(TARGETS IGVC_UI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Finalize Qt 6 app
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_finalize_executable(IGVC_UI)
endif()

