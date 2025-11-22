
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)



add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST})


# Link Muduo static libraries
target_link_libraries(ChatServer
    ${CMAKE_CURRENT_SOURCE_DIR}/../../muduo/lib/libmuduo_net.a
    ${CMAKE_CURRENT_SOURCE_DIR}/../../muduo/lib/libmuduo_base.a
    pthread
    mysqlclient
)