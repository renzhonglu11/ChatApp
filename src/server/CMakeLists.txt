
aux_source_directory(. SRC_LIST)

add_executable(ChatServer ${SRC_LIST})

# Header search paths (Muduo + your project include)
target_include_directories(ChatServer
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../../muduo/include
)


# Link Muduo static libraries
target_link_libraries(ChatServer
    ${CMAKE_CURRENT_SOURCE_DIR}/../../muduo/lib/libmuduo_net.a
    ${CMAKE_CURRENT_SOURCE_DIR}/../../muduo/lib/libmuduo_base.a
    pthread
)