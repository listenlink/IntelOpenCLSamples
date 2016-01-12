include_directories(${CMAKE_SOURCE_DIR} ${COMMON_DIR})

add_executable(simple_optimizations SimpleOptimizations.cpp ${BASIC_FILES})
target_link_libraries(simple_optimizations ${OPENCL_LIBRARIES})

configure_file(SimpleOptimizations.cl ${CMAKE_CURRENT_BINARY_DIR}/SimpleOptimizations.cl COPYONLY)
