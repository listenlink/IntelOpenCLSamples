
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )
PROJECT( INTEL_OCL_SAMPLES )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package( OpenCL REQUIRED )

include_directories( ${OPENCL_INCLUDE_DIRS} )


set(COMMON_DIR ${PROJECT_SOURCE_DIR}/common)
set(BASIC_FILES "${COMMON_DIR}/basic.cpp"
                "${COMMON_DIR}/cmdparser.cpp"
                "${COMMON_DIR}/oclobject.cpp")

add_subdirectory(BitonicSort)
add_subdirectory(CapsBasic)
add_subdirectory(GEMM)
add_subdirectory(GodRays)
add_subdirectory(MedianFilter)
add_subdirectory(MonteCarlo)
add_subdirectory(MotionEstimation)
add_subdirectory(MotionEstimationAdvanced)
#add_subdirectory(MultiDeviceBasic)
#add_subdirectory(SimpleOptimizations)
#add_subdirectory(ToneMapping)
#add_subdirectory(ToneMappingMultiDevice)
