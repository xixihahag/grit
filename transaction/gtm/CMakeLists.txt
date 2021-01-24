cmake_minimum_required(VERSION 2.8)
project(gtm)

# add_executable(gtm gtmmain.cc gtm.cc ../../base/configManager.cc)
add_executable(gtm gtmmain.cc gtm.cc)
target_link_libraries(gtm base muduo_net muduo_base gflags glog)