#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

/* ================= 内部定义 ================= */

#define FILE_MAGIC   0x46535452  /* 'FSTR' */
#define FILE_VERSION 1
#define TMP_SUFFIX   ".tmp"
#define BAK_SUFFIX   ".bak"

typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t length;
    uint32_t crc32;
} file_header_t;

/* 全局文件锁（防止多线程并发写） */
static pthread_mutex_t g_file_mutex = PTHREAD_MUTEX_INITIALIZER;

/* ================= CRC32 ================= */

static uint32_t crc32_calc(const void *data, size_t len)
{
    const uint8_t *p = data;
    uint32_t crc = 0xFFFFFFFF;

    while (len--) {
        crc ^= *p++;
        for (int i = 0; i < 8; i++)
            crc = (crc >> 1) ^ (0xEDB88320 & -(crc & 1));
    }
    return ~crc;
}

/* ================= 内部工具 ================= */

static int fsync_dir(const char *path)
{
    char dir[256];
    strncpy(dir, path, sizeof(dir));
    char *p = strrchr(dir, '/');
    if (!p) return 0;

    *p = '\0';
    int dfd = open(dir, O_DIRECTORY | O_RDONLY);
    if (dfd < 0) return -1;

    fsync(dfd);
    close(dfd);
    return 0;
}

/* ================= 原子写 ================= */

file_err_t file_atomic_write(const char *path,
                             const void *data,
                             size_t len)
{
    if (!path || !data || len == 0)
        return FILE_ERR_PARAM;

    pthread_mutex_lock(&g_file_mutex);

    char tmp_path[256];
    char bak_path[256];
    snprintf(tmp_path, sizeof(tmp_path), "%s%s", path, TMP_SUFFIX);
    snprintf(bak_path, sizeof(bak_path), "%s%s", path, BAK_SUFFIX);

    file_header_t hdr = {
        .magic = FILE_MAGIC,
        .version = FILE_VERSION,
        .length = len,
        .crc32 = crc32_calc(data, len),
    };

    int fd = open(tmp_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (fd < 0) {
        pthread_mutex_unlock(&g_file_mutex);
        return FILE_ERR_OPEN;
    }

    if (write(fd, &hdr, sizeof(hdr)) != sizeof(hdr) ||
        write(fd, data, len) != (ssize_t)len) {
        close(fd);
        pthread_mutex_unlock(&g_file_mutex);
        return FILE_ERR_WRITE;
    }

    fsync(fd);
    close(fd);

    /* 旧文件备份 */
    if (access(path, F_OK) == 0) {
        rename(path, bak_path);
    }

    if (rename(tmp_path, path) < 0) {
        pthread_mutex_unlock(&g_file_mutex);
        return FILE_ERR_RENAME;
    }

    fsync_dir(path);

    pthread_mutex_unlock(&g_file_mutex);
    return FILE_OK;
}

/* ================= 原子读 ================= */

static file_err_t file_read_internal(const char *path,
                                     void *data,
                                     size_t len)
{
    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return FILE_ERR_OPEN;

    file_header_t hdr;
    if (read(fd, &hdr, sizeof(hdr)) != sizeof(hdr)) {
        close(fd);
        return FILE_ERR_READ;
    }

    if (hdr.magic != FILE_MAGIC ||
        hdr.length > len) {  // 修复：改为范围检查，数据长度不能超过缓冲区
        close(fd);
        return FILE_ERR_FORMAT;
    }

    if (read(fd, data, hdr.length) != (ssize_t)hdr.length) {  // 修复：读取实际长度而非缓冲区长度
        close(fd);
        return FILE_ERR_READ;
    }

    close(fd);

    if (crc32_calc(data, hdr.length) != hdr.crc32)  // 修复：使用实际长度计算CRC
        return FILE_ERR_CRC;

    return FILE_OK;
}

file_err_t file_atomic_read(const char *path,
                            void *data,
                            size_t len)
{
    if (!path || !data)
        return FILE_ERR_PARAM;

    pthread_mutex_lock(&g_file_mutex);

    file_err_t ret = file_read_internal(path, data, len);
    if (ret == FILE_OK) {
        pthread_mutex_unlock(&g_file_mutex);
        return FILE_OK;
    }

    /* 主文件失败，尝试备份 */
    char bak_path[256];
    snprintf(bak_path, sizeof(bak_path), "%s%s", path, BAK_SUFFIX);

    ret = file_read_internal(bak_path, data, len);

    pthread_mutex_unlock(&g_file_mutex);
    return ret;
}

/* ================= 类型化接口 ================= */

file_err_t file_write_int(const char *path, int value)
{
    return file_atomic_write(path, &value, sizeof(value));
}

file_err_t file_read_int(const char *path, int *value)
{
    return file_atomic_read(path, value, sizeof(*value));
}

file_err_t file_write_string(const char *path,
                             const char *str,
                             size_t max_len)
{
    if (!str) return FILE_ERR_PARAM;
    size_t len = strnlen(str, max_len);
    return file_atomic_write(path, str, len + 1);
}

file_err_t file_read_string(const char *path,
                            char *buf,
                            size_t buf_len)
{
    return file_atomic_read(path, buf, buf_len);
}

/* ================= 工具 ================= */

int file_exists(const char *path)
{
    return access(path, F_OK) == 0;
}
