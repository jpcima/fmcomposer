cmake_minimum_required(VERSION 3.5)
# TODO: Determine actual required CMake version

# Set default build type to Release
if(NOT DEFINED CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)")
endif()

project(FMComposer)

# Detect and enable C++11 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(STATUS "FMComposer requires C++11, but ${CMAKE_CXX_COMPILER} does not appear to support C++11. Please use a different compiler or update ${CMAKE_CXX_COMPILER}.")
endif()

include_directories(src)

# Add source files
set(SOURCES
	"src/fmengine/fmlib.c"
	"src/fmengine/lz4/lz4.c"
	"src/fmengine/lz4/lz4hc.c"
	"src/globalFunctions.cpp"
	"src/gui/button/button.cpp"
	"src/gui/button/buttonlist.cpp"
	"src/gui/channelHead.cpp"
	"src/gui/checkbox/checkbox.cpp"
	"src/gui/contextmenu/contextmenu.cpp"
	"src/gui/drawBatcher.cpp"
	"src/gui/gui.cpp"
	"src/gui/list/list.cpp"
	"src/gui/mainmenu.cpp"
	"src/gui/operator/operator.cpp"
	"src/gui/operator/operatorparams.cpp"
	"src/gui/popup/popup_build.cpp"
	"src/gui/popup/popup_close.cpp"
	"src/gui/popup/popup.cpp"
	"src/gui/popup/popup_effectInfos.cpp"
	"src/gui/scrollbar/scrollbar.cpp"
	"src/gui/sidebar.cpp"
	"src/gui/slider/dataslider.cpp"
	"src/gui/textfield/textfield.cpp"
	"src/gui/vumeter/stereovumeter.cpp"
	"src/gui/vumeter/vumeter.cpp"
	"src/input/input.cpp"
	"src/input/noteInput.cpp"
	"src/lang.cpp"
	"src/libs/simpleini/ConvertUTF.c"
	"src/libs/tinyfiledialogs/tinyfiledialogs.c"
	"src/main.cpp"
	"src/midi/midi_export.cpp"
	"src/midi/midi_import.cpp"
	"src/midi/midiInstrNames.cpp"
	"src/midi/midi_realtime.cpp"
	"src/streamed/streamedExport.cpp"
	"src/views/general/generalEditor.cpp"
	"src/views/instrument/algoPresets.cpp"
	"src/views/instrument/instrEditor_algorithms.cpp"
	"src/views/instrument/instrEditor.cpp"
	"src/views/instrument/instrEditor_fileActions.cpp"
	"src/views/instrument/instrEditor_fmSync.cpp"
	"src/views/instrument/instrEditor_global.cpp"
	"src/views/instrument/instrEditor_history.cpp"
	"src/views/pattern/patternSelection.cpp"
	"src/views/pattern/songEditor_channelHeads.cpp"
	"src/views/pattern/songEditor_contextMenu.cpp"
	"src/views/pattern/songEditor.cpp"
	"src/views/pattern/songEditor_history.cpp"
	"src/views/pattern/songEditor_mouseEvents.cpp"
	"src/views/pattern/songEditor_patternEdit.cpp"
	"src/views/pattern/songEditor_patternList.cpp"
	"src/views/pattern/songEditor_search.cpp"
	"src/views/pattern/songEditor_shortcuts.cpp"
	"src/views/pattern/songEditor_updateDisplay.cpp"
	"src/views/pattern/songFileActions.cpp"
	"src/views/pianoroll/pianoRoll_contextMenu.cpp"
	"src/views/pianoroll/pianoRoll.cpp"
	"src/views/pianoroll/pianoRoll_draw.cpp"
	"src/views/settings/configEditor.cpp"
	"src/views/settings/configEditor_keyboard.cpp"
	"src/views/settings/configEditor_recentSongs.cpp"
	"src/views/settings/configEditor_soundDevice.cpp")

# Set executable output path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Set executable name
set(EXECUTABLE_NAME "FMComposer")
add_executable(${EXECUTABLE_NAME} ${SOURCES})

# Add CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

# Add the portable clipboard library
add_subdirectory(thirdparty/clip EXCLUDE_FROM_ALL)
target_link_libraries(${EXECUTABLE_NAME} clip)
target_include_directories(${EXECUTABLE_NAME} PRIVATE "thirdparty/clip") # :-(

# Add the portable "whereami" library
add_library(whereami STATIC "thirdparty/whereami/src/whereami.c")
target_include_directories(whereami PUBLIC "thirdparty/whereami/src")
target_link_libraries(${EXECUTABLE_NAME} whereami)

# Detect and add SFML
find_package(SFML 2 REQUIRED graphics window system)
if(SFML_FOUND)
	include_directories(${SFML_INCLUDE_DIR})
	target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES} ${SFML_DEPENDENCIES})
endif()

# Detect and add OpenGL
find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
	include_directories(${OPENGL_INCLUDE_DIRS})
	target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
endif()

# Detect and add FLAC
find_package(FLAC REQUIRED)
if(FLAC_FOUND)
	include_directories(${FLAC_INCLUDE_DIR})
	target_link_libraries(${EXECUTABLE_NAME} ${FLAC_LIBRARY})
endif()

# Detect and add PortAudio
find_package(PortAudio REQUIRED)
if(PORTAUDIO_FOUND)
	include_directories(${PORTAUDIO_INCLUDE_DIRS})
	target_link_libraries(${EXECUTABLE_NAME} ${PORTAUDIO_LIBRARIES})
endif()

# Detect and add PortMidi
find_library(PORTMIDI_LIBRARY "portmidi")
if(NOT PORTMIDI_LIBRARY)
	message(FATAL_ERROR "Cannot find the PortMidi library")
else()
	message(STATUS "PortMidi library: ${PORTMIDI_LIBRARY}")
endif()
find_path(PORTMIDI_INCLUDE_DIR "portmidi.h")
if(NOT PORTMIDI_INCLUDE_DIR)
	message(FATAL_ERROR "Cannot find the PortMidi headers")
else()
	message(STATUS "PortMidi headers: ${PORTMIDI_INCLUDE_DIR}")
endif()
include_directories(${PORTMIDI_INCLUDE_DIR})
target_link_libraries(${EXECUTABLE_NAME} ${PORTMIDI_LIBRARY})

# Detect and add Lame
find_library(LAME_LIBRARY "mp3lame")
find_path(LAME_INCLUDE_DIR "lame.h" PATH_SUFFIXES "lame")
if(NOT LAME_LIBRARY)
	message(STATUS "Cannot find mp3lame library")
elseif(NOT LAME_INCLUDE_DIR)
	message(STATUS "Cannot find mp3lame includes")
else()
	message(STATUS "Found mp3lame includes: ${LAME_INCLUDE_DIR}")
	message(STATUS "Found mp3lame library: ${LAME_LIBRARY}")
	include_directories(${LAME_INCLUDE_DIR})
	target_link_libraries(${EXECUTABLE_NAME} ${LAME_LIBRARY})
endif()

# Link POSIX Thread library if necessary
if(CMAKE_COMPILER_IS_GNUCC)
	target_link_libraries(${EXECUTABLE_NAME} pthread)
endif(CMAKE_COMPILER_IS_GNUCC)

# Install
include(GNUInstallDirs)
install(TARGETS
  ${EXECUTABLE_NAME}
  DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY
  "resources/instruments"
  "resources/songs"
  "resources/themes"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/FMComposer)
install(FILES
  "resources/defaultkeyboards.ini"
  "resources/gmlist.ini"
  "resources/readme.txt"
  DESTINATION ${CMAKE_INSTALL_DATADIR}/FMComposer)
