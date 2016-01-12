include_directories(${COMMON_DIR})

add_executable(tone_mapping ToneMapping.cpp ToneMappingNative.cpp ${BASIC_FILES} ${COMMON_DIR}/utils.cpp)
target_link_libraries(tone_mapping ${OPENCL_LIBRARIES})

configure_file(ToneMapping.cl ${CMAKE_CURRENT_BINARY_DIR}/ToneMapping.cl COPYONLY)
