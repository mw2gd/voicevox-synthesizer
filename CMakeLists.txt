cmake_minimum_required(VERSION 3.10)
project(VvSynth)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(EXT_DIR ${CMAKE_SOURCE_DIR}/ext)

# ------------------------------
# Source files
# ------------------------------
file(GLOB_RECURSE SRC_FILES
    ${SRC_DIR}/*.[ch]pp
    ${SRC_DIR}/*.c
)
add_executable(vvsynth ${SRC_FILES})

# ------------------------------
# Include directories
# ------------------------------
target_include_directories(vvsynth PRIVATE ${SRC_DIR})

# Add all "include" dirs under ./ext
file(GLOB_RECURSE EXT_DIRS LIST_DIRECTORIES true "${EXT_DIR}/*")
set(EXT_INCLUDE_DIRS "")
foreach(path IN LISTS EXT_DIRS)
    if(IS_DIRECTORY "${path}" AND path MATCHES "/include$")
        list(APPEND EXT_INCLUDE_DIRS "${path}")
    endif()
endforeach()
list(REMOVE_DUPLICATES EXT_INCLUDE_DIRS)
target_include_directories(vvsynth PRIVATE ${EXT_INCLUDE_DIRS})

# ------------------------------
# Libraries used directly.
# ------------------------------
set(REQUIRED_LIBS voicevox_core)
set(LINK_LIBS)

foreach(LIB_NAME IN LISTS REQUIRED_LIBS)
    file(GLOB_RECURSE FOUND_LIBS
        "${EXT_DIR}/lib${LIB_NAME}.*"
        "${EXT_DIR}/${LIB_NAME}.*"
    )
    list(FILTER FOUND_LIBS INCLUDE REGEX "\\.(a|so|dylib)$")
    if(NOT FOUND_LIBS)
        message(FATAL_ERROR "Library '${LIB_NAME}' not found under: ${EXT_DIR}")
    endif()
    list(GET FOUND_LIBS 0 LIB_PATH)
    list(APPEND LINK_LIBS "${LIB_PATH}")
    message(STATUS "Library: ${LIB_NAME} -> ${LIB_PATH}")
endforeach()

target_link_libraries(vvsynth PRIVATE ${LINK_LIBS})

set_target_properties(vvsynth PROPERTIES
    BUILD_RPATH "@executable_path"
    INSTALL_RPATH "@executable_path"
)
