cmake_minimum_required(VERSION 3.10)

project(ctp_trade)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

aux_source_directory(. SRC_LIST)

add_executable(ctp_cmd ${SRC_LIST})

target_link_libraries(ctp_cmd PRIVATE
    thosttraderapi_se
    Threads::Threads
)