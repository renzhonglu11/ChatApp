cmake_minimum_required(VERSION 3.10)

project(ChatApp)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# Include Muduo headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/server)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/server/db)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/server/model)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/muduo/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/server/redis)
include_directories(/usr/local/include)

# Link directories
link_directories(/usr/local/lib)



# load source files
add_subdirectory(src)

