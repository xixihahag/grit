cmake_minimum_required(VERSION 2.8)
project(dbs)

add_executable(dbs dbtm.cc dbservicemain.cc dbservice.cc)
target_link_libraries(dbs base muduo_net muduo_base gflags glog)