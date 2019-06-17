# CMakeList.txt: CMakeProject2 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

# 将源代码添加到此项目的可执行文件。SS
add_executable (ws_socket main.cc ws_socket.h ws_socket.cc thread_pool.h thread_pool.cc)

target_link_libraries(ws_socket pthread)

# TODO: 如有需要，请添加测试并安装目标。
