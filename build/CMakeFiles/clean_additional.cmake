# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\PetApp_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\PetApp_autogen.dir\\ParseCache.txt"
  "PetApp_autogen"
  )
endif()
