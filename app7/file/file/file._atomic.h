#ifndef __FILE_STORE_H__
#define __FILE_STORE_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ================= 错误码定义 ================= */

typedef enum {
    FILE_OK = 0,
    FILE_ERR_OPEN,
    FILE_ERR_READ,
    FILE_ERR_WRITE,
    FILE_ERR_FSYNC,
    FILE_ERR_RENAME,
    FILE_ERR_CRC,
    FILE_ERR_FORMAT,
    FILE_ERR_PARAM,
} file_err_t;

/* ================= 通用原子读写接口 ================= */

/**
 * @brief 原子写入二进制数据（带掉电保护）
 */
file_err_t file_atomic_write(const char *path,
                             const void *data,
                             size_t len);

/**
 * @brief 原子读取二进制数据（自动备份恢复）
 */
file_err_t file_atomic_read(const char *path,
                            void *data,
                            size_t len);

/* ================= 类型化接口（推荐业务使用） ================= */

file_err_t file_write_int(const char *path, int value);
file_err_t file_read_int(const char *path, int *value);

file_err_t file_write_string(const char *path, const char *str,
                             size_t max_len);
file_err_t file_read_string(const char *path, char *buf,
                            size_t buf_len);

/* ================= 工具接口 ================= */

int file_exists(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* __FILE_STORE_H__ */
