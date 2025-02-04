#--------------------------------------------------------------------------
# gscept laboration environment
#--------------------------------------------------------------------------
CMAKE_MINIMUM_REQUIRED(VERSION 3.15)
cmake_policy(SET CMP0048 NEW) # set policy for older versions
PROJECT(EvolvingCreaturesEngine VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)

# WHY THIS THIS IS HORRIBLE FIX IT
# FIX IT
list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}vcpkg\\installed\\x64-windows\\share\\unofficial-omniverse-physx-sdk")
# FIX IT

# VCPKG SETUP STUFF
# DYNAMIC LINKING
# set(VCPKG_TARGET_TRIPLET x64-windows)
# set(VCPKG_CRT_LINKAGE dynamic)

# STATIC LINKING
set(VCPKG_TARGET_TRIPLET x64-windows-static)
set(VCPKG_CRT_LINKAGE static)
set(CMAKE_TOOLCHAIN_FILE "$ENV{HOME}vcpkg\\scripts\\buildsystems\\vcpkg.cmake")
message("--------------------------------------------------------------------------------")
message("CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")
message("--------------------------------------------------------------------------------")

# FINDING THE PACKAGES

find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)
# END OF VCPKG

OPTION(CI_MODE "Run in CI mode" OFF) # Disabled by default

IF(CI_MODE)
    ADD_DEFINITIONS(-DCI_TEST)
ENDIF(CI_MODE)

MACRO(TARGET_PCH target path)
IF(WIN32)
    IF(MSVC)
        SET_TARGET_PROPERTIES(${target}
            PROPERTIES 
            COMPILE_FLAGS "/Yuconfig.h /Fp${CMAKE_CFG_INTDIR}/${target}.pch")
        SET_SOURCE_FILES_PROPERTIES(${path}config.cc
            PROPERTIES
            COMPILE_FLAGS "/Ycconfig.h")
    ENDIF()
ENDIF()
ENDMACRO(TARGET_PCH)

IF(CMAKE_COMPILER_IS_GNUCXX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
ENDIF()

IF(MSVC)
    SET(OPENGL_LIBS opengl32.lib)
ELSE()
    SET(OPENGL_LIBS GL X11 Xxf86vm pthread Xrandr Xi Xinerama Xcursor)
ENDIF()

SET(GSCEPT_LAB_ENV_ROOT ${CMAKE_CURRENT_DIR})

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<$<CONFIG:Debug>:${CMAKE_SOURCE_DIR}/bin>)

SET_PROPERTY(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS GLEW_STATIC)
ADD_SUBDIRECTORY(exts)
ADD_SUBDIRECTORY(engine)
ADD_SUBDIRECTORY(projects)


# target_link_libraries(main PRIVATE unofficial::omniverse-physx-sdk::sdk)
