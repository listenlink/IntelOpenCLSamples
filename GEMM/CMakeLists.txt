include_directories(${CMAKE_SOURCE_DIR} ${COMMON_DIR})

add_executable(gemm cmdoptions.cpp gemm.cpp ${BASIC_FILES})
target_link_libraries(gemm ${OPENCL_LIBRARIES})

configure_file(gemm.cl ${CMAKE_CURRENT_BINARY_DIR}/gemm.cl COPYONLY)
