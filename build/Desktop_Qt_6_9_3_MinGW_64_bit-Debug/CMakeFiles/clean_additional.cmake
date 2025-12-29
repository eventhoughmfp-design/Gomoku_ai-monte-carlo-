# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\Gomoku_ai_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\Gomoku_ai_autogen.dir\\ParseCache.txt"
  "Gomoku_ai_autogen"
  )
endif()
