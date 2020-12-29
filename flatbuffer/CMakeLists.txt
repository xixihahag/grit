cmake_minimum_required(VERSION 2.8)
project(gtm)

file(GLOB_RECURSE FBS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.fbs)

foreach(FBS_FILE ${FBS_FILES})

execute_process(
    COMMAND ${FLATC} -c -b ${FBS_FILE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

endforeach(FBS_FILE)
