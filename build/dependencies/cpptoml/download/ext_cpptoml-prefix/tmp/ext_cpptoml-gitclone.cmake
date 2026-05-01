# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

if(EXISTS "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp/ext_cpptoml-gitclone-lastrun.txt" AND EXISTS "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp/ext_cpptoml-gitinfo.txt" AND
  "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp/ext_cpptoml-gitclone-lastrun.txt" IS_NEWER_THAN "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp/ext_cpptoml-gitinfo.txt")
  message(VERBOSE
    "Avoiding repeated git clone, stamp file is up to date: "
    "'/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp/ext_cpptoml-gitclone-lastrun.txt'"
  )
  return()
endif()

# Even at VERBOSE level, we don't want to see the commands executed, but
# enabling them to be shown for DEBUG may be useful to help diagnose problems.
cmake_language(GET_MESSAGE_LOG_LEVEL active_log_level)
if(active_log_level MATCHES "DEBUG|TRACE")
  set(maybe_show_command COMMAND_ECHO STDOUT)
else()
  set(maybe_show_command "")
endif()

execute_process(
  COMMAND ${CMAKE_COMMAND} -E rm -rf "/workspace/build/dependencies/cpptoml/src"
  RESULT_VARIABLE error_code
  ${maybe_show_command}
)
if(error_code)
  message(FATAL_ERROR "Failed to remove directory: '/workspace/build/dependencies/cpptoml/src'")
endif()

# try the clone 3 times in case there is an odd git clone issue
set(error_code 1)
set(number_of_tries 0)
while(error_code AND number_of_tries LESS 3)
  execute_process(
    COMMAND "/usr/bin/git"
            clone --no-checkout --config "advice.detachedHead=false" "https://github.com/skystrife/cpptoml.git" "src"
    WORKING_DIRECTORY "/workspace/build/dependencies/cpptoml"
    RESULT_VARIABLE error_code
    ${maybe_show_command}
  )
  math(EXPR number_of_tries "${number_of_tries} + 1")
endwhile()
if(number_of_tries GREATER 1)
  message(NOTICE "Had to git clone more than once: ${number_of_tries} times.")
endif()
if(error_code)
  message(FATAL_ERROR "Failed to clone repository: 'https://github.com/skystrife/cpptoml.git'")
endif()

execute_process(
  COMMAND "/usr/bin/git"
          checkout "v0.1.1" --
  WORKING_DIRECTORY "/workspace/build/dependencies/cpptoml/src"
  RESULT_VARIABLE error_code
  ${maybe_show_command}
)
if(error_code)
  message(FATAL_ERROR "Failed to checkout tag: 'v0.1.1'")
endif()

set(init_submodules TRUE)
if(init_submodules)
  execute_process(
    COMMAND "/usr/bin/git" 
            submodule update --recursive --init 
    WORKING_DIRECTORY "/workspace/build/dependencies/cpptoml/src"
    RESULT_VARIABLE error_code
    ${maybe_show_command}
  )
endif()
if(error_code)
  message(FATAL_ERROR "Failed to update submodules in: '/workspace/build/dependencies/cpptoml/src'")
endif()

# Complete success, update the script-last-run stamp file:
#
execute_process(
  COMMAND ${CMAKE_COMMAND} -E copy "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp/ext_cpptoml-gitinfo.txt" "/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp/ext_cpptoml-gitclone-lastrun.txt"
  RESULT_VARIABLE error_code
  ${maybe_show_command}
)
if(error_code)
  message(FATAL_ERROR "Failed to copy script-last-run stamp file: '/workspace/build/dependencies/cpptoml/download/ext_cpptoml-prefix/src/ext_cpptoml-stamp/ext_cpptoml-gitclone-lastrun.txt'")
endif()
