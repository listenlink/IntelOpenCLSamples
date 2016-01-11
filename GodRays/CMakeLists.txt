include_directories(${COMMON_DIR})

add_executable(godrays GodRays.cpp GodRaysNative.cpp ${BASIC_FILES} ${COMMON_DIR}/utils.cpp)
target_link_libraries(godrays ${OPENCL_LIBRARIES})

configure_file(GodRays.cl ${CMAKE_CURRENT_BINARY_DIR}/GodRays.cl COPYONLY)
