cmake_minimum_required(VERSION 3.16)

project(audio_player VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)

find_package(Qt5 5.15 REQUIRED COMPONENTS Quick multimedia)

#qt_standard_project_setup(REQUIRES 5.15)

add_executable(appaudio_player
    main.cpp
    ImageButton.qml Main.qml main.qrc
    myaudioplayer.h myaudioplayer.cpp
    AudioDeal.h
    spectrum3d.h spectrum3d.cpp
)

link_libraries(libfftw3-3.lib libfftw3l-3.lib libfftw3f-3.lib)

set(RESOURCES main.qrc)
set(QMLS Main.qml ImageButton.qml)
# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appaudio_player PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appaudio_player
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appaudio_player
    PRIVATE Qt5::Quick
    PRIVATE Qt5::Multimedia
    Opengl32
)

include(GNUInstallDirs)
install(TARGETS appaudio_player
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
