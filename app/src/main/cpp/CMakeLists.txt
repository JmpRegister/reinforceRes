# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

#set(FA_STATIC on)
SET(ONE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/FAInHook)
SET(TWO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/native-lib)
ADD_SUBDIRECTORY(${ONE_PATH})
ADD_SUBDIRECTORY(${TWO_PATH})
