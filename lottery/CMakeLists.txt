add_contract(lottery lottery ${CMAKE_CURRENT_SOURCE_DIR}/src/lottery.cpp)
target_include_directories(lottery.wasm
   PUBLIC
   ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(lottery.wasm
   PROPERTIES
   RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
