cmake_minimum_required(VERSION 2.8)
project(dbtl)

add_executable(dbtl logplayer.cc dbtl.cc dbtlmain.cc )
target_link_libraries(dbtl base muduo_net muduo_base gflags glog pthread rocksdb)