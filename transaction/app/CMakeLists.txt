cmake_minimum_required(VERSION 2.8)
project(app)

add_executable(app appmain.cc app.cc)
target_link_libraries(app base muduo_net muduo_base gflags glog)