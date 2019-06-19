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

set (LIB_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR})
set (BIN_INSTALL_DIR ${CMAKE_INSTALL_BINDIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
	Core Gui Network Widgets
)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

add_subdirectory(res)
add_subdirectory(src)

add_executable(divvydroid ${divvydroid_SRCS} ${divvydroid_RES})
target_include_directories(divvydroid PUBLIC ${divvydroid_INC})
target_link_libraries(divvydroid ${divvydroid_LIBS})

install(TARGETS divvydroid DESTINATION ${BIN_INSTALL_DIR})
#install(FILES divvydroid.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
#install(FILES pkg/misc/divvydroid.desktop.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
