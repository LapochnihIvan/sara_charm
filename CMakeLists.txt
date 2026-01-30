cmake_minimum_required(VERSION 4.2.1)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(MINIMAL_BUILD ON)

project(sara_charm)

set(EXTRA_COMPONENT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/components)

execute_process(
    COMMAND "${CMAKE_SOURCE_DIR}/frontend/build_compressed.sh"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/frontend"
    RESULT_VARIABLE FRONTEND_BUILD_RES
)
if(NOT FRONTEND_BUILD_RES EQUAL 0)
    message(FATAL_ERROR "Frontend build failed!")
endif()
