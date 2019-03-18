cmake_minimum_required(VERSION 3.8)
project(fxcalc)

set(PROJECT_VERSION 1.4)

# Set this to your Qt installation
set(CMAKE_PREFIX_PATH ~/sourcecode/sdk/QtLatest/5.12.0/clang_64)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(PROJECT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(ICON_NAME AppIcon)

set(RESOURCE_FILES 
	res/${PROJECT_NAME}.qrc
	res/${ICON_NAME}.icns
	res/${ICON_NAME}.png
	res/instruments.txt
)

# Qt
set(CMAKE_AUTOMOC ON) # For meta object compiler
set(CMAKE_AUTORCC ON) # Resource files
set(CMAKE_AUTOUIC ON) # UI files

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)

set(OS_BUNDLE "")
if(CMAKE_BUILD_TYPE STREQUAL "Release")
# Identify MacOS bundle
set(OS_BUNDLE "MACOSX_BUNDLE")
set(COMPANY "Arne Gockeln")
set(IDENTIFIER "com.arnegockeln.fxcalc")
set(COPYRIGHT "Arne Gockeln")
set(MACOSX_BUNDLE_BUNDLE_NAME FXCalc)
set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
set(MACOSX_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
set(MACOSX_BUNDLE_COPYRIGHT ${COPYRIGHT})
set(MACOSX_BUNDLE_GUI_IDENTIFIER ${IDENTIFIER})
set(MACOSX_BUNDLE_ICON_FILE ${ICON_NAME})
else(CMAKE_BUILD_TYPE STREQUAL "Release")
set(CMAKE_BUILD_TYPE Debug)
endif()

include_directories(${PROJECT_SOURCE_DIR})

# source files
file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)

# executable
add_executable(${PROJECT_NAME} ${OS_BUNDLE} ${SOURCE_FILES} ${RESOURCE_FILES})
# make values available in c++ files
target_compile_definitions(${PROJECT_NAME} PRIVATE 
	PROJECT_VERSION="${PROJECT_VERSION}"
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
set_target_properties(${PROJECT_NAME} PROPERTIES 
	MACOSX_BUNDLE TRUE 
	RESOURCE "${RESOURCE_FILES}"
	OUTPUT_NAME "${MACOSX_BUNDLE_BUNDLE_NAME}"
)

endif()
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Network)