#include <cstdlib>
#include <db.hpp>
#include <fstream>
#include <muduo/base/Logging.h>
#include <vector>

namespace {
string trim(const string& s)
{
    const string whitespace = " \t\n\r";
    const size_t start = s.find_first_not_of(whitespace);
    if (start == string::npos) {
        return "";
    }
    const size_t end = s.find_last_not_of(whitespace);
    return s.substr(start, end - start + 1);
}

bool loadDbConfigFromFile(const string& path, string& server, string& user, string& password, string& database, unsigned int& port)
{
    ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    string line;
    while (getline(in, line)) {
        const string content = trim(line);
        if (content.empty() || content[0] == '#') {
            continue;
        }

        const size_t eqPos = content.find('=');
        if (eqPos == string::npos) {
            continue;
        }

        const string key = trim(content.substr(0, eqPos));
        const string value = trim(content.substr(eqPos + 1));

        if (key == "server") {
            server = value;
        } else if (key == "user") {
            user = value;
        } else if (key == "password") {
            password = value;
        } else if (key == "database") {
            database = value;
        } else if (key == "port") {
            port = static_cast<unsigned int>(std::strtoul(value.c_str(), nullptr, 10));
        }
    }

    return true;
}
}

MySQL::MySQL()
{
    _conn = mysql_init(nullptr);

    // Safe defaults; password must come from config file.
    _server = "127.0.0.1";
    _user = "root";
    _password = "";
    _database = "chat";
    _port = 3306;

    const char* envPath = std::getenv("CHAT_DB_CONFIG");
    vector<string> candidates;
    if (envPath != nullptr && *envPath != '\0') {
        candidates.push_back(envPath);
    }
    candidates.push_back("config/db.conf");
    candidates.push_back("../config/db.conf");

    bool loaded = false;
    for (const auto& path : candidates) {
        if (loadDbConfigFromFile(path, _server, _user, _password, _database, _port)) {
            LOG_INFO << "Loaded DB config from: " << path;
            loaded = true;
            break;
        }
    }

    if (!loaded) {
        LOG_WARN << "DB config file not found. Using defaults except password (empty).";
    }
}
MySQL::~MySQL()
{
    if (_conn != nullptr) {
        mysql_close(_conn);
    }
}

MYSQL* MySQL::getConnection()
{
    return _conn;
}

// Connect to database
bool MySQL::connect()
{
    MYSQL* p = mysql_real_connect(_conn, _server.c_str(), _user.c_str(),
        _password.c_str(), _database.c_str(), _port, nullptr, 0);
    if (p != nullptr) {
        // Set character set to utf8
        mysql_set_character_set(_conn, "utf8");
        LOG_INFO << "MySQL Connect Success!";
        return true;
    }
    LOG_ERROR << "MySQL Connect Fail!";
    return false;
}

// Update operation
bool MySQL::update(string sql)
{
    if (mysql_query(_conn, sql.c_str())) {
        LOG_ERROR << __FILE__ << ":" << __LINE__ << " - " << mysql_error(_conn);
        return false;
    }
    return true;
}

// Query operation
MYSQL_RES* MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str())) {
        LOG_ERROR << __FILE__ << ":" << __LINE__ << " - " << mysql_error(_conn);
        return nullptr;
    }
    return mysql_store_result(_conn);
}