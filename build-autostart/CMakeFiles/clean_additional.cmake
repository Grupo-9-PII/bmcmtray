# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "BMCMTrayLauncherQt_autogen"
  [[CMakeFiles\BMCMTrayLauncherQt_autogen.dir\AutogenUsed.txt]]
  [[CMakeFiles\BMCMTrayLauncherQt_autogen.dir\ParseCache.txt]]
  [[CMakeFiles\launcher_runtime_test_autogen.dir\AutogenUsed.txt]]
  [[CMakeFiles\launcher_runtime_test_autogen.dir\ParseCache.txt]]
  "launcher_runtime_test_autogen"
  )
endif()
