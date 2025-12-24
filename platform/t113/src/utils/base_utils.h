#ifndef _BASE_UTILS_H_
#define _BASE_UTILS_H_

int base_utils_shell_exec(const char* exe, char* buf, int len);

int base_utils_system(char *cmd);

int base_utils_get_process_state(const char *name,int length);

#endif
