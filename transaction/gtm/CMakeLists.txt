cmake_minimum_required(VERSION 2.8)
project(gtm)

add_executable(gtm gtmmain.cc gtm.cc ../../base/configManager.cc)
target_link_libraries(gtm muduo_net muduo_base gflags glog)