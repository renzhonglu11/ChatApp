#pragma once

/**
 * common include file for server and client
 */

enum EnMsgType
{
    LOGIN_MSG = 1, // 登录消息
    REG_MSG,       // 注册消息
    REG_MSG_ACK,   // 注册响应消息
    LOGIN_MSG_ACK,
    ONE_CHAT_MSG,
    ADD_FRIEND_MSG,
};

// {"msgid":1,"id":1,"password":"1234"}
// {"msgid":2,"name":"jason","password":"1234"}

// {"msgid":1,"id":2,"password":"12345a"}
// {"msgid":2,"name":"ben","password":"12345a"}

// {"msgid":1,"id":3,"password":"123456"}
// {"msgid":2,"name":"foo","password":"123456"}

// {"msgid":5,"from":"jason","to":2,"msg":"hello!!"}
// {"msgid":5,"from":"ben","to":1,"msg":"你好!"}

// Add friend request
// {"msgid":6,"id":3,"friendid":1}
// {"msgid":6,"id":3,"friendid":2}

// msgid
// id : 1
// from : "name"
// to: 3
// msg : ""