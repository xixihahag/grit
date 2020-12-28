cmake_minimum_required(VERSION 2.8)
project(gtm)

file(GLOB_RECURSE FBS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.fbs)

foreach(FBS_FILE ${FBS_FILES})
# get_filename_component(FBS_NAME ${FBS_FILES} NAME)

# message(${FLATC})

set(FBS_SRC gen.h)

# add_custom_target(BUILD_FLAT)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/
    COMMAND ${FLATC} -c -b ${FBS_FILE}
    DEPENDS ${FBS_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

endforeach(FBS_FILE)
