# Install script for directory: /workspace/external/iceoryx/iceoryx_hoofs

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/iceoryx_hoofs" TYPE FILE FILES "/workspace/external/iceoryx/iceoryx_hoofs/LICENSE-APACHE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/share/doc/iceoryx_hoofs" TYPE FILE FILES "/workspace/external/iceoryx/iceoryx_hoofs/LICENSE-MIT")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "bin" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/workspace/build/external/iceoryx/iceoryx_hoofs/libiceoryx_hoofs.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/iceoryx/v1.0.0" TYPE DIRECTORY FILES
    "/workspace/external/iceoryx/iceoryx_hoofs/buffer/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/cli/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/concurrent/sync/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/concurrent/buffer/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/container/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/design/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/functional/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/memory/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/primitives/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/reporting/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/time/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/utility/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/vocabulary/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/posix/design/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/posix/vocabulary/include/"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/iceoryx_hoofs" TYPE FILE FILES
    "/workspace/build/external/iceoryx/iceoryx_hoofs/iceoryx_hoofsConfigVersion.cmake"
    "/workspace/build/external/iceoryx/iceoryx_hoofs/iceoryx_hoofsConfig.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/iceoryx_hoofs/iceoryx_hoofsTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/iceoryx_hoofs/iceoryx_hoofsTargets.cmake"
         "/workspace/build/external/iceoryx/iceoryx_hoofs/CMakeFiles/Export/2df1df3350d90c929624816c3cc2d98f/iceoryx_hoofsTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/iceoryx_hoofs/iceoryx_hoofsTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/iceoryx_hoofs/iceoryx_hoofsTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/iceoryx_hoofs" TYPE FILE FILES "/workspace/build/external/iceoryx/iceoryx_hoofs/CMakeFiles/Export/2df1df3350d90c929624816c3cc2d98f/iceoryx_hoofsTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/iceoryx_hoofs" TYPE FILE FILES "/workspace/build/external/iceoryx/iceoryx_hoofs/CMakeFiles/Export/2df1df3350d90c929624816c3cc2d98f/iceoryx_hoofsTargets-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/iceoryx/v1.0.0" TYPE DIRECTORY FILES
    "/workspace/external/iceoryx/iceoryx_hoofs/concurrent/sync_extended/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/filesystem/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/posix/auth/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/posix/ipc/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/posix/filesystem/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/posix/sync/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/posix/time/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/posix/utility/include/"
    "/workspace/external/iceoryx/iceoryx_hoofs/legacy/include/"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "dev" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/iceoryx/v1.0.0/iox" TYPE FILE FILES "/workspace/build/generated/iceoryx_hoofs/include/iox/iceoryx_hoofs_deployment.hpp")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "/workspace/build/external/iceoryx/iceoryx_hoofs/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
