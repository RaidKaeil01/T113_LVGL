#include "event_log.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sqlite3.h>

/* ================= 全局变量 ================= */
static sqlite3 *g_db = NULL;
static pthread_mutex_t g_event_log_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_event_log_initialized = false;

/* ================= SQL语句 ================= */

// 创建事件日志表
static const char *SQL_CREATE_TABLE = 
    "CREATE TABLE IF NOT EXISTS event_log ("
    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
    "  timestamp INTEGER NOT NULL,"
    "  level INTEGER NOT NULL,"
    "  module INTEGER NOT NULL,"
    "  event INTEGER NOT NULL,"
    "  message TEXT"
    ");"
    "CREATE INDEX IF NOT EXISTS idx_timestamp ON event_log(timestamp);"
    "CREATE INDEX IF NOT EXISTS idx_level ON event_log(level);";

// 插入日志记录
static const char *SQL_INSERT =
    "INSERT INTO event_log (timestamp, level, module, event, message) "
    "VALUES (?, ?, ?, ?, ?);";

// 查询总记录数
static const char *SQL_COUNT = "SELECT COUNT(*) FROM event_log;";

// 删除最老的记录
static const char *SQL_DELETE_OLD =
    "DELETE FROM event_log WHERE id IN ("
    "  SELECT id FROM event_log ORDER BY timestamp ASC LIMIT ?"
    ");";

// 获取最新记录
static const char *SQL_GET_LATEST =
    "SELECT id, timestamp, level, module, event, message "
    "FROM event_log ORDER BY timestamp DESC LIMIT ?;";

// 获取统计信息
static const char *SQL_GET_STATS =
    "SELECT "
    "  COUNT(*) as total,"
    "  SUM(CASE WHEN level = 3 THEN 1 ELSE 0 END) as errors,"
    "  SUM(CASE WHEN level = 4 THEN 1 ELSE 0 END) as fatals,"
    "  SUM(CASE WHEN level = 5 THEN 1 ELSE 0 END) as events,"
    "  MIN(timestamp) as oldest,"
    "  MAX(timestamp) as newest "
    "FROM event_log;";

/* ================= 内部函数 ================= */

/**
 * @brief 确保数据库目录存在
 */
static int ensure_db_directory(void)
{
    struct stat st = {0};
    
    if (stat("/usr/data", &st) == -1) {
        if (mkdir("/usr/data", 0755) != 0) {
            printf("❌ Failed to create /usr/data\n");
            return -1;
        }
    }
    
    if (stat("/usr/data/log", &st) == -1) {
        if (mkdir("/usr/data/log", 0755) != 0) {
            printf("❌ Failed to create /usr/data/log\n");
            return -1;
        }
    }
    
    return 0;
}

/**
 * @brief 自动清理旧日志
 */
static int auto_cleanup(void)
{
    sqlite3_stmt *stmt = NULL;
    int rc;
    
    // 查询总记录数
    rc = sqlite3_prepare_v2(g_db, SQL_COUNT, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("❌ Failed to prepare count query: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        return -1;
    }
    
    int total_count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    
    // 检查是否需要清理
    if (total_count <= EVENT_LOG_CLEANUP_THRESHOLD) {
        return 0;  // 无需清理
    }
    
    // 删除最老的记录
    int delete_count = total_count - EVENT_LOG_MAX_RECORDS;
    
    rc = sqlite3_prepare_v2(g_db, SQL_DELETE_OLD, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("❌ Failed to prepare delete query: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, delete_count);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        printf("❌ Failed to delete old records: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    
    printf("✅ Event log cleanup: deleted %d old records\n", delete_count);
    return 0;
}

/* ================= 接口实现 ================= */

/**
 * @brief 初始化事件日志模块
 */
int event_log_init(void)
{
    pthread_mutex_lock(&g_event_log_mutex);
    
    if (g_event_log_initialized) {
        printf("⚠️  Event log already initialized\n");
        pthread_mutex_unlock(&g_event_log_mutex);
        return 0;
    }
    
    // 确保目录存在
    if (ensure_db_directory() != 0) {
        pthread_mutex_unlock(&g_event_log_mutex);
        return -1;
    }
    
    // 打开数据库
    int rc = sqlite3_open(EVENT_LOG_DB_PATH, &g_db);
    if (rc != SQLITE_OK) {
        printf("❌ Failed to open database: %s\n", sqlite3_errmsg(g_db));
        pthread_mutex_unlock(&g_event_log_mutex);
        return -1;
    }
    
    // 设置性能优化选项
    sqlite3_exec(g_db, "PRAGMA journal_mode=WAL;", NULL, NULL, NULL);
    sqlite3_exec(g_db, "PRAGMA synchronous=NORMAL;", NULL, NULL, NULL);
    
    // 创建表
    char *err_msg = NULL;
    rc = sqlite3_exec(g_db, SQL_CREATE_TABLE, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        printf("❌ Failed to create table: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(g_db);
        pthread_mutex_unlock(&g_event_log_mutex);
        return -1;
    }
    
    g_event_log_initialized = true;
    pthread_mutex_unlock(&g_event_log_mutex);
    
    printf("✅ Event log initialized: %s\n", EVENT_LOG_DB_PATH);
    return 0;
}

/**
 * @brief 反初始化事件日志模块
 */
void event_log_deinit(void)
{
    pthread_mutex_lock(&g_event_log_mutex);
    
    if (!g_event_log_initialized) {
        pthread_mutex_unlock(&g_event_log_mutex);
        return;
    }
    
    if (g_db != NULL) {
        sqlite3_close(g_db);
        g_db = NULL;
    }
    
    g_event_log_initialized = false;
    pthread_mutex_unlock(&g_event_log_mutex);
    
    printf("✅ Event log deinitialized\n");
}

/**
 * @brief 写入一条事件日志
 */
int event_log_write(log_level_t level, log_module_t module,
                   log_event_type_t event, const char *message)
{
    if (!g_event_log_initialized || g_db == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&g_event_log_mutex);
    
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(g_db, SQL_INSERT, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("❌ Failed to prepare insert: %s\n", sqlite3_errmsg(g_db));
        pthread_mutex_unlock(&g_event_log_mutex);
        return -1;
    }
    
    // 绑定参数
    sqlite3_bind_int(stmt, 1, (int)time(NULL));
    sqlite3_bind_int(stmt, 2, (int)level);
    sqlite3_bind_int(stmt, 3, (int)module);
    sqlite3_bind_int(stmt, 4, (int)event);
    sqlite3_bind_text(stmt, 5, message ? message : "", -1, SQLITE_TRANSIENT);
    
    // 执行插入
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc != SQLITE_DONE) {
        printf("❌ Failed to insert event log: %s\n", sqlite3_errmsg(g_db));
        pthread_mutex_unlock(&g_event_log_mutex);
        return -1;
    }
    
    // 自动清理旧日志
    auto_cleanup();
    
    pthread_mutex_unlock(&g_event_log_mutex);
    return 0;
}

/**
 * @brief 获取最新的N条事件日志
 */
int event_log_get_latest(event_log_record_t *records, uint32_t count)
{
    if (!g_event_log_initialized || g_db == NULL || records == NULL || count == 0) {
        return -1;
    }
    
    pthread_mutex_lock(&g_event_log_mutex);
    
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(g_db, SQL_GET_LATEST, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("❌ Failed to prepare query: %s\n", sqlite3_errmsg(g_db));
        pthread_mutex_unlock(&g_event_log_mutex);
        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, (int)count);
    
    int fetched = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW && fetched < count) {
        records[fetched].id = sqlite3_column_int64(stmt, 0);
        records[fetched].timestamp = sqlite3_column_int(stmt, 1);
        records[fetched].level = (log_level_t)sqlite3_column_int(stmt, 2);
        records[fetched].module = (log_module_t)sqlite3_column_int(stmt, 3);
        records[fetched].event = (log_event_type_t)sqlite3_column_int(stmt, 4);
        
        const char *msg = (const char *)sqlite3_column_text(stmt, 5);
        strncpy(records[fetched].message, msg ? msg : "", sizeof(records[fetched].message) - 1);
        records[fetched].message[sizeof(records[fetched].message) - 1] = '\0';
        
        fetched++;
    }
    
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&g_event_log_mutex);
    
    return fetched;
}

/**
 * @brief 获取事件日志统计信息
 */
int event_log_get_stats(event_log_stats_t *stats)
{
    if (!g_event_log_initialized || g_db == NULL || stats == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&g_event_log_mutex);
    
    sqlite3_stmt *stmt = NULL;
    int rc = sqlite3_prepare_v2(g_db, SQL_GET_STATS, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("❌ Failed to prepare stats query: %s\n", sqlite3_errmsg(g_db));
        pthread_mutex_unlock(&g_event_log_mutex);
        return -1;
    }
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        stats->total_count = sqlite3_column_int(stmt, 0);
        stats->error_count = sqlite3_column_int(stmt, 1);
        stats->fatal_count = sqlite3_column_int(stmt, 2);
        stats->event_count = sqlite3_column_int(stmt, 3);
        stats->oldest_timestamp = sqlite3_column_int(stmt, 4);
        stats->newest_timestamp = sqlite3_column_int(stmt, 5);
    } else {
        memset(stats, 0, sizeof(event_log_stats_t));
    }
    
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&g_event_log_mutex);
    
    return 0;
}

/**
 * @brief 清空所有事件日志
 */
int event_log_clear(void)
{
    if (!g_event_log_initialized || g_db == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&g_event_log_mutex);
    
    char *err_msg = NULL;
    int rc = sqlite3_exec(g_db, "DELETE FROM event_log;", NULL, NULL, &err_msg);
    
    if (rc != SQLITE_OK) {
        printf("❌ Failed to clear event log: %s\n", err_msg);
        sqlite3_free(err_msg);
        pthread_mutex_unlock(&g_event_log_mutex);
        return -1;
    }
    
    pthread_mutex_unlock(&g_event_log_mutex);
    printf("✅ Event log cleared\n");
    
    return 0;
}

/**
 * @brief 删除指定时间之前的事件日志
 */
int event_log_delete_before(uint32_t before_time)
{
    if (!g_event_log_initialized || g_db == NULL) {
        return -1;
    }
    
    pthread_mutex_lock(&g_event_log_mutex);
    
    sqlite3_stmt *stmt = NULL;
    const char *sql = "DELETE FROM event_log WHERE timestamp < ?;";
    
    int rc = sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("❌ Failed to prepare delete: %s\n", sqlite3_errmsg(g_db));
        pthread_mutex_unlock(&g_event_log_mutex);
        return -1;
    }
    
    sqlite3_bind_int(stmt, 1, (int)before_time);
    
    rc = sqlite3_step(stmt);
    int deleted = sqlite3_changes(g_db);
    
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&g_event_log_mutex);
    
    if (rc != SQLITE_DONE) {
        printf("❌ Failed to delete old events: %s\n", sqlite3_errmsg(g_db));
        return -1;
    }
    
    printf("✅ Deleted %d old event logs\n", deleted);
    return deleted;
}
