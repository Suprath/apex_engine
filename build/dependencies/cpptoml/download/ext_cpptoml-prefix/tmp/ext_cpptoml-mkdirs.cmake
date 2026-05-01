# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/workspace/build/dependencies/cpptoml/src")
  file(MAKE_DIRECTORY "/workspace/build/dependencies/cpptoml/src")
endif()
file(MAKE_DIRECTORY
  "/workspace/build/dependencies/cpptoml/build"
  "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix"
  "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/tmp"
  "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp"
  "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src"
  "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp${cfgdir}") # cfgdir has leading slash
endif()
