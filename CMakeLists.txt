project(DivvyDroid)

cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
set(QT_MIN_VERSION "5.3.0")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0043 NEW)
cmake_policy(SET CMP0063 NEW)
if(POLICY CMP0071)
	cmake_policy(SET CMP0071 NEW)
endif()

include(FeatureSummary)
include(GNUInstallDirs)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
	Core Gui Network Widgets
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

add_subdirectory(res)
add_subdirectory(src)

install(FILES pkg/divvydroid.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES res/icons/android-debug-bridge.svg RENAME divvydroid.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)
install(FILES pkg/divvydroid.desktop.appdata.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
