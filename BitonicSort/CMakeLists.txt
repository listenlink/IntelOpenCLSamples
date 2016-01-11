set(COMMON_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../common")
set(BASIC_FILES "${COMMON_DIR}/basic.cpp"
		"${COMMON_DIR}/cmdparser.cpp"
		"${COMMON_DIR}/oclobject.cpp")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../common)
add_executable(bitonic_sort BitonicSort.cpp ${BASIC_FILES})
target_link_libraries(bitonic_sort ${OPENCL_LIBRARIES})

configure_file(BitonicSort.cl ${CMAKE_CURRENT_BINARY_DIR}/BitonicSort.cl COPYONLY)
