# CMakeLists.txt --- CMake project settings
##############################################################################

# CMake minimum version
cmake_minimum_required(VERSION 3.6)

# project name and languages
project(NotifPIDL CXX)

# NotifPIDL.exe
add_executable(NotifPIDL NotifPIDL.cpp)
target_link_libraries(NotifPIDL shell32 comctl32)
