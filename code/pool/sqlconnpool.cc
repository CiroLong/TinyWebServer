#include "sqlconnpool.h"

SqlConnPool::SqlConnPool()
{
    useCount_ = 0;
    freeCount_ = 0;
}

SqlConnPool::~SqlConnPool()
{
    ClosePool();
}
SqlConnPool *SqlConnPool::Instance()
{
    static SqlConnPool connPool;
    return &connPool;
}

void SqlConnPool::init(const char *host, int port, const char *user, const char *pwd, const char *dbName, int connSize)
{
    assert(connSize > 0);
    for (int i = 0; i < connSize; ++i)
    {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql)
        {
            LOG_ERROR("Mysql init error!");
            assert(sql);
        }
        sql =
            mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if (!sql)
        {
            LOG_ERROR("Mysql Connect error!");
        }

        connQue_.push(sql);
    }
    MAX_CONN_ = connSize;
    sem_init(&semId_, 0, MAX_CONN_);
}
MYSQL *SqlConnPool::GetConn()
{
    MYSQL *sql = nullptr;
    if (connQue_.empty())
    {
        LOG_WARN("SqlConnPool Busy!");
        return nullptr;
    }
    sem_wait(&semId_);
    {
        std::lock_guard<std::mutex> locker(mtx_);
        sql = connQue_.front();
        connQue_.pop();
    }
    return sql;
}

void SqlConnPool::FreeConn(MYSQL *sql)
{
    assert(sql);
    std::lock_guard<std::mutex> locker(mtx_);
    connQue_.push(sql);
    sem_post(&semId_);
}

int SqlConnPool::GetFreeConnCount()
{
    std::lock_guard<std::mutex> locker(mtx_);
    return connQue_.size();
}

void SqlConnPool::ClosePool()
{
    std::lock_guard<std::mutex> locker(mtx_);
    while (!connQue_.empty())
    {
        auto item = connQue_.front();
        connQue_.pop();
        mysql_close(item);
    }
    mysql_library_end();
}
