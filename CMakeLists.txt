CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( RiftTrackerDisplay )

INCLUDE(cmake_modules/InvokePython.cmake)
INVOKEPYTHON( "tools/hardcode_shaders.py" )
# Python script will dump generated headers to autogen/
INCLUDE_DIRECTORIES("autogen/")

# GLFW, SDL, SFML
SET( GL_FRAMEWORK "GLFW" CACHE STRING "Which OpenGL windowing framework to use" )
SET( USE_OCULUSSDK TRUE CACHE BOOL "Use Oculus SDK" )
SET( OVRSDK_VERSION "06" CACHE STRING "Which version of the OVR SDK to use - 05 or 06" )
SET( USE_CORE_CONTEXT TRUE CACHE BOOL "Use Core Context" )
IF( USE_CORE_CONTEXT )
    ADD_DEFINITIONS(-DUSE_CORE_CONTEXT)
ENDIF()

ADD_DEFINITIONS( -DPROJECT_NAME="${PROJECT_NAME}" )

MESSAGE( "CMAKE_SYSTEM_NAME: ${CMAKE_SYSTEM_NAME}" )

#
# Platform-dependent section
#
IF( WIN32 )
    MESSAGE(STATUS "MSVC_VERSION=${MSVC_VERSION}")
    # 1600 = VS 10.0
    # 1700 = VS 11.0
    # 1800 = VS 12.0

    #
    # Custom Windows include and link dirs for my machine:
    #
    SET( LIBS_HOME "C:/lib" CACHE STRING "Location of support libraries Glut, Glew and others"  )

    SET( GLM_ROOT "${LIBS_HOME}/glm-0.9.5.4/glm" CACHE STRING "glm root" )
    INCLUDE_DIRECTORIES( "${GLM_ROOT}" )
    ADD_DEFINITIONS( -DGLM_FORCE_RADIANS )

    IF( GL_FRAMEWORK STREQUAL "GLFW" )
        MESSAGE("Using GLFW Framework.")
        IF( MSVC_VERSION STREQUAL "1600" ) # VS 2010
            SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.1.bin.WIN32" CACHE STRING "glfw root" )
            LINK_DIRECTORIES( "${GLFW3_ROOT}/lib-vc2010" )
        ELSEIF( MSVC_VERSION STREQUAL "1700" )
            SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.1.bin.WIN32" CACHE STRING "glfw root" )
            LINK_DIRECTORIES( "${GLFW3_ROOT}/lib-vc2012" )
        ELSEIF( MSVC_VERSION STREQUAL "1800" )
            SET( GLFW3_ROOT "${LIBS_HOME}/glfw-3.1.bin.WIN32" CACHE STRING "glfw root" )
            LINK_DIRECTORIES( "${GLFW3_ROOT}/lib-vc2013" )
        ENDIF()
        INCLUDE_DIRECTORIES( "${GLFW3_ROOT}/include" )
        SET( FRAMEWORK_LIBS glfw3.lib )
    ELSEIF( GL_FRAMEWORK STREQUAL "SDL" )
    ELSEIF( GL_FRAMEWORK STREQUAL "SFML" )
    ELSE()
        MESSAGE("Unknown GL_FRAMEWORK: choose GLFW, SDL or SFML.")
    ENDIF()

    SET( GLEW_ROOT "${LIBS_HOME}/glew-1.12.0" CACHE STRING "glew root" )
    INCLUDE_DIRECTORIES( "${GLEW_ROOT}/include" )
    LINK_DIRECTORIES   ( "${GLEW_ROOT}/lib/Release/Win32" )
    ADD_DEFINITIONS( -DGLEW_STATIC )

    IF( USE_OCULUSSDK )
        # TODO: the logic here needs work, OVR SDK dir is kept in cache until manually changed
        IF( OVRSDK_VERSION STREQUAL "05" )
            SET( OCULUSSDK_ROOT "${LIBS_HOME}/OculusSDK-0.5.0.1" CACHE STRING "OculusSDK root" )
            ADD_DEFINITIONS( -DOVRSDK05 )
        ELSEIF( OVRSDK_VERSION STREQUAL "06" )
            SET( OCULUSSDK_ROOT "${LIBS_HOME}/OculusSDK" CACHE STRING "OculusSDK root" )
            ADD_DEFINITIONS( -DOVRSDK06 )
        ELSE()
            MESSAGE("Unknown OVR SDK version.")
        ENDIF()

        INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Include" )
        INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Src" )
        INCLUDE_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVRKernel/Src" )
        IF( MSVC_VERSION STREQUAL "1600" )
            IF( DEBUG )
                LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Debug/VS2010" )
            ELSE()
                LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Release/VS2010" )
            ENDIF()
        ELSEIF( MSVC_VERSION STREQUAL "1700" )
            IF( DEBUG )
                LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Debug/VS2012" )
            ELSE()
                LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Release/VS2012" )
            ENDIF()
        ELSEIF( MSVC_VERSION STREQUAL "1800" )
            IF( DEBUG )
                LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Debug/VS2013" )
            ELSE()
                LINK_DIRECTORIES( "${OCULUSSDK_ROOT}/LibOVR/Lib/Windows/Win32/Release/VS2013" )
            ENDIF()
        ENDIF()
        SET( OVR_LIBS LibOVR.lib )
        ADD_DEFINITIONS( -DUSE_OCULUSSDK )
    ENDIF( USE_OCULUSSDK )

    SET( PLATFORM_LIBS
        ${FRAMEWORK_LIBS}
        opengl32.lib glu32.lib glew32s.lib
        ${OVR_LIBS}
        Winmm.lib
        ws2_32.lib
        )

    ADD_DEFINITIONS( /W4 -D_CRT_SECURE_NO_WARNINGS )

    # These lines will silence some linker warnings on VS2010,
    # but also remove debug info from the debug build.
    #SET( CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:\"libcmt.lib\" )
    #SET( CMAKE_EXE_LINKER_FLAGS_DEBUG /NODEFAULTLIB:\"msvcrt.lib\" )

    # Copy DLLs of the included libraries into output directories.
    INVOKEPYTHON( "tools/copyDLLs.py"
        ${PROJECT_SOURCE_DIR}
        ${PROJECT_BINARY_DIR}
        "OCULUSSDK_ROOT=${OCULUSSDK_ROOT}"
        "GLEW_ROOT=${GLEW_ROOT}"
        )
ELSEIF( APPLE )
ELSEIF( UNIX )
ENDIF()

#
# Platform-independent section
#
FILE( GLOB_RECURSE UTIL_SOURCE_FILES
    src/Util/*.cpp
    src/Util/*.h
    )

INCLUDE_DIRECTORIES("src/Util")
INCLUDE_DIRECTORIES("src/Util/GL")

ADD_LIBRARY( Util        ${UTIL_SOURCE_FILES} )

#
# Some convoluted logic here to choose one of several main source files:
# Since the OVR 06 SDK changes the way main handles the AppSkeleton, there is a main
# file for each windowing system(GLFW,SDL) and SDK version(05,06).
#
IF( GL_FRAMEWORK STREQUAL "GLFW" )
    ADD_EXECUTABLE( ${PROJECT_NAME} src/glfw_main.cpp )
ENDIF()

TARGET_LINK_LIBRARIES( ${PROJECT_NAME}
    Util
    ${PLATFORM_LIBS}
    )
