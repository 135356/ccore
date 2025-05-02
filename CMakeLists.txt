get_filename_component(project_name_ ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(TOLOWER ${project_name_} project_name_)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 20)
#版本号
project(${project_name_} VERSION 0.0.1)
ADD_DEFINITIONS(-D_PROJECT_VERSION=\"${PROJECT_VERSION}\")

#Debug｜Release模式: -g表示带调试信息，-Wall表示输出所有的警告信息
set(CMAKE_BUILD_TYPE Release) #cmake根据编译器加-g参数(调试)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#库文件输出目录
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
#可执行文件输出目录
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

#指定源文件目录
file(GLOB SRC_FILE "${CMAKE_SOURCE_DIR}/src/*.cpp" "${CMAKE_SOURCE_DIR}/src/*/*.cpp" "${CMAKE_SOURCE_DIR}/src/*/*/*.cpp")
#排除掉与平台相关的源文件
list(FILTER SRC_FILE EXCLUDE REGEX "\\..+.cpp$")
#平台相关
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    file(GLOB SRC_FILE_platform "${CMAKE_SOURCE_DIR}/src/*.Linux.cpp" "${CMAKE_SOURCE_DIR}/src/*/*.Linux.cpp" "${CMAKE_SOURCE_DIR}/src/*/*/*.Linux.cpp")
    add_library(${PROJECT_NAME} SHARED ${SRC_FILE} ${SRC_FILE_platform})

    set(install_dir /usr/local/cc13535)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    file(GLOB SRC_FILE_platform "${CMAKE_SOURCE_DIR}/src/*.Darwin.cpp" "${CMAKE_SOURCE_DIR}/src/*/*.Darwin.cpp" "${CMAKE_SOURCE_DIR}/src/*/*/*.Darwin.cpp")
    add_library(${PROJECT_NAME} STATIC ${SRC_FILE} ${SRC_FILE_platform})

    set(install_dir ${CMAKE_SOURCE_DIR}/build)
elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    file(GLOB SRC_FILE_platform "${CMAKE_SOURCE_DIR}/src/*.Windows.cpp" "${CMAKE_SOURCE_DIR}/src/*/*.Windows.cpp" "${CMAKE_SOURCE_DIR}/src/*/*/*.Windows.cpp")
    add_library(${PROJECT_NAME} STATIC ${SRC_FILE} ${SRC_FILE_platform})
    
    set(install_dir ${CMAKE_SOURCE_DIR}/build)
else()
    message(STATUS "不支持当前平台下编译")
endif()

#指定目标包含的头文件路径
target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
#生成可执行文件
#add_executable(${PROJECT_NAME}_run tcp_client.cpp)
add_executable(${PROJECT_NAME}_run main.cpp)
#可执行文件依赖bb
add_dependencies(${PROJECT_NAME}_run ${PROJECT_NAME})
#连接库文件
target_link_libraries(${PROJECT_NAME}_run ${PROJECT_NAME} -lpthread)

#指定安装位置(默认为/usr/local 与 c:/Program Files/${PROJECT_NAME})
set(CMAKE_INSTALL_PREFIX ${install_dir}/${PROJECT_NAME})
#安装头文件
install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/${PROJECT_NAME}" TYPE INCLUDE)
#安装:库文件、可执行文件、头文件(运行make install时执行)
install(TARGETS ${PROJECT_NAME}
    EXPORT Targets #安装目标信息写入这个变量
    LIBRARY DESTINATION lib #动态库
    ARCHIVE DESTINATION lib #静态库
)
#将前面EXPORT Targets的信息写入到aaaTargets.cmake文件中
install(EXPORT Targets
    FILE ${PROJECT_NAME}Targets.cmake #输出文件
    DESTINATION cmake #输出的路径
)

#设置变量(对应Config.cmake.in)
#set(BIN bin)
set(CMAKE cmake)
set(INCLUDES include)
set(LIBS lib)
#写入配置文件
include(CMakePackageConfigHelpers)
configure_package_config_file(
    Config.cmake.in #输入文件(config.cmake.in)
    ${install_dir}/cmake/modules/${PROJECT_NAME}Config.cmake #输出文件
    INSTALL_DESTINATION ${install_dir}/cmake/modules #输出的路径
    PATH_VARS CMAKE INCLUDES LIBS #命名方式为`PACKAKE_VAR`(对应Config.cmake.in)
)
#写入版本文件
write_basic_package_version_file(
    ${install_dir}/cmake/modules/${PROJECT_NAME}ConfigVersion.cmake
    COMPATIBILITY SameMajorVersion  #AnyNewerVersion向后兼容，SameMajorVersion只对同一个大版本号生效
)