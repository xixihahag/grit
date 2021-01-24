cmake_minimum_required(VERSION 2.8)
project(es)

add_executable(es es.cc esmain.cc)
target_link_libraries(es base muduo_net muduo_base gflags glog)