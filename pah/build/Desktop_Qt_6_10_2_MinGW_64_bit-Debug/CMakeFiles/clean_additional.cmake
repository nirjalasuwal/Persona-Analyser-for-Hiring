# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\apppah_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\apppah_autogen.dir\\ParseCache.txt"
  "apppah_autogen"
  )
endif()
