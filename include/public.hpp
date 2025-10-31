#pragma once

/**
 * common include file for server and client
 */

enum EnMsgType
{
    LOGIN_MSG = 1, // 登录消息
    REG_MSG,       // 注册消息
    REG_MSG_ACK    // 注册响应消息

};