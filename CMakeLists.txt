cmake_minimum_required(VERSION 3.16)

project(OpenSearch VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})


find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick )
find_package(KF6 REQUIRED COMPONENTS IconThemes)
find_package(rapidfuzz REQUIRED)


qt_standard_project_setup(REQUIRES 6.10.0)

set(CMAKE_AUTORCC ON)
qt_add_executable(appOpenSearch
    main.cpp
)

qt_add_qml_module(appOpenSearch
    URI OpenSearch
    VERSION 1.0
    QML_FILES
        Main.qml
    SOURCES
        structs/ResultItem.cpp
        models/ResultsModel.h
        models/ResultsModel.cpp

    RESOURCES resources.qrc
)
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appOpenSearch PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appOpenSearch
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appOpenSearch
    PRIVATE
        KF6::IconThemes
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        rapidfuzz::rapidfuzz
)

include(GNUInstallDirs)
install(TARGETS appOpenSearch
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
