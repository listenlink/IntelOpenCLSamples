include_directories(${COMMON_DIR})
#message(STATUS "COMMON DIRECTORY: ${COMMON_DIR}")

add_executable(bitonic_sort BitonicSort.cpp ${BASIC_FILES})
target_link_libraries(bitonic_sort ${OPENCL_LIBRARIES})

configure_file(BitonicSort.cl ${CMAKE_CURRENT_BINARY_DIR}/BitonicSort.cl COPYONLY)
