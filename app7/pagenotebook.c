#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>  // 目录操作
#include <unistd.h>  // access函数
#include "lvgl.h"
#include "page_conf.h"
#include "image_conf.h"
#include "font_conf.h"
#include "file.h"  // 引入文件原子操作接口

/* ========== 文件路径定义 ========== */
#define NOTEBOOK_FILE_PATH "/usr/data/note/notebook.txt"
#define HISTORY_DIR "/usr/data/note"

/* ========== 全局变量 ========== */
static lv_style_t com_style;
static lv_obj_t *textarea = NULL;  // 文本编辑区
static lv_obj_t *keyboard = NULL;  // 虚拟键盘
static lv_obj_t *content_cont = NULL;  // 内容区容器
static lv_obj_t *history_msgbox = NULL;  // 历史记录弹窗
static lv_obj_t *saveas_msgbox = NULL;  // 另存弹窗
static char notebook_content[1024] = {0};  // 记事本内容缓存
static bool keyboard_visible = false;  // 键盘显示状态标志

/* ========== 函数前向声明 ========== */
static void switch_btn_event_cb(lv_event_t * e);

/* ========== 样式初始化 ========== */
static void com_style_init(void)
{
    lv_style_init(&com_style);
    lv_style_set_bg_color(&com_style, lv_color_hex(0xFFFFFF));
    lv_style_set_radius(&com_style, 0);
    lv_style_set_border_width(&com_style, 0);
    lv_style_set_pad_all(&com_style, 0);
    lv_style_set_outline_width(&com_style, 0);
}

/* ========== 字体设置 ========== */
static void obj_font_set(lv_obj_t *obj, int type, uint16_t weight)
{
    lv_font_t* font = get_font(type, weight);
    if(font != NULL)
        lv_obj_set_style_text_font(obj, font, LV_PART_MAIN);
}

/**
 * @brief 确保目录存在，如果不存在则创建
 */
static void ensure_directory_exists(const char *path)
{
    char dir_path[256];
    strncpy(dir_path, path, sizeof(dir_path) - 1);
    
    // 提取目录路径（去掉文件名）
    char *last_slash = strrchr(dir_path, '/');
    if (last_slash) {
        *last_slash = '\0';
        
        // 创建目录（如果不存在）
        struct stat st = {0};
        if (stat(dir_path, &st) == -1) {
            mkdir(dir_path, 0755);
            printf("Created directory: %s\n", dir_path);
        }
    }
}

/**
 * @brief 从文件加载记事本内容
 */
static void load_notebook_content(void)
{
    file_err_t ret = file_read_string(NOTEBOOK_FILE_PATH, 
                                       notebook_content, 
                                       sizeof(notebook_content));
    
    if (ret == FILE_OK) {
        printf("✅ Notebook content loaded successfully from %s\n", NOTEBOOK_FILE_PATH);
        printf("Content: %s\n", notebook_content);
    } else if (ret == FILE_ERR_OPEN) {
        printf("ℹ️  No saved notebook file found, starting with empty content\n");
        notebook_content[0] = '\0';
    } else {
        printf("⚠️  Failed to load notebook (error: %d), starting with empty content\n", ret);
        notebook_content[0] = '\0';
    }
}

/**
 * @brief 保存记事本内容到文件（原子写入 + 掉电保护）
 */
static file_err_t save_notebook_content(const char *content)
{
    // 确保目录存在
    ensure_directory_exists(NOTEBOOK_FILE_PATH);
    
    // 使用原子写入接口保存
    file_err_t ret = file_write_string(NOTEBOOK_FILE_PATH, 
                                        content, 
                                        sizeof(notebook_content));
    
    if (ret == FILE_OK) {
        printf("✅ Notebook saved successfully to %s\n", NOTEBOOK_FILE_PATH);
        printf("Content length: %zu bytes\n", strlen(content));
    } else {
        printf("❌ Failed to save notebook (error: %d)\n", ret);
    }
    
    return ret;
}

/**
 * @brief 清理记事本页面资源
 */
void cleanup_pageNotebook(void)
{
    printf("Cleaning up pageNotebook resources...\n");
    
    // 清理样式
    if(lv_style_is_empty(&com_style) == false) {
        lv_style_reset(&com_style);
        printf("Style reset\n");
    }
    
    // 移除事件回调
    lv_obj_remove_event_cb(lv_scr_act(), NULL);
    
    // 清理历史记录弹窗
    if(history_msgbox != NULL) {
        lv_obj_del(history_msgbox);
        history_msgbox = NULL;
    }
    
    // 清理另存弹窗
    if(saveas_msgbox != NULL) {
        lv_obj_del(saveas_msgbox);
        saveas_msgbox = NULL;
    }
    
    // 清空文本编辑区和键盘指针
    textarea = NULL;
    keyboard = NULL;
    content_cont = NULL;
    keyboard_visible = false;
    
    printf("pageNotebook cleanup completed\n");
}

/**
 * @brief 键盘拖动事件回调 - 实现键盘可移动
 */
static void keyboard_drag_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    
    // 只处理键盘本身的拖动，不处理按键点击
    if(obj != keyboard) return;
    
    lv_indev_t * indev = lv_indev_get_act();
    if(indev == NULL) return;
    
    lv_point_t vect;
    lv_indev_get_vect(indev, &vect);
    
    // 如果没有移动则不处理
    if(vect.x == 0 && vect.y == 0) return;
    
    lv_coord_t x = lv_obj_get_x(obj) + vect.x;
    lv_coord_t y = lv_obj_get_y(obj) + vect.y;
    
    // 限制键盘移动范围（在内容区域内）
    // X范围：0 到 content_cont宽度-键盘宽度
    // Y范围：0 到 content_cont高度-键盘高度
    lv_coord_t max_x = 1384 - lv_obj_get_width(obj);
    lv_coord_t max_y = 200 - lv_obj_get_height(obj);
    
    if(x < 0) x = 0;
    if(y < 0) y = 0;
    if(x > max_x) x = max_x;
    if(y > max_y) y = max_y;
    
    lv_obj_set_pos(obj, x, y);
}

/**
 * @brief 切换键盘显示/隐藏
 */
static void toggle_keyboard(void)
{
    if(keyboard == NULL) return;
    
    if(keyboard_visible) {
        lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        keyboard_visible = false;
        printf("Keyboard hidden\n");
    } else {
        lv_obj_clear_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        keyboard_visible = true;
        printf("Keyboard shown\n");
    }
}

/**
 * @brief 文本编辑区点击事件 - 切换键盘
 */
static void textarea_click_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    // 只处理SHORT_CLICKED事件，避免重复触发
    if(code == LV_EVENT_SHORT_CLICKED) {
        toggle_keyboard();
    }
}

/**
 * @brief 返回按钮点击回调 - 返回菜单页面
 */
static void back_btn_event_cb(lv_event_t * e)
{
    printf("Back button clicked, returning to Menu page\n");
    
    // 清理当前页面资源
    cleanup_pageNotebook();
    
    // 清空屏幕
    lv_obj_clean(lv_scr_act());
    
    // 返回菜单页面
    init_pageMenu();
}

/**
 * @brief 关闭历史记录弹窗回调
 */
static void close_history_msgbox_cb(lv_event_t * e)
{
    if(history_msgbox != NULL) {
        lv_obj_del(history_msgbox);
        history_msgbox = NULL;
        printf("History msgbox closed\n");
    }
}

/**
 * @brief 删除文件按钮回调
 */
static void delete_file_cb(lv_event_t * e)
{
    // 获取文件名（从user_data中）
    const char *filename = (const char *)lv_event_get_user_data(e);
    
    if (filename == NULL) {
        printf("❌ Filename is NULL\n");
        return;
    }
    
    // 构建完整文件路径
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/%s", HISTORY_DIR, filename);
    
    printf("🗑️  Attempting to delete: %s\n", file_path);
    
    // 删除文件及其备份文件
    int ret = remove(file_path);
    
    if (ret == 0) {
        printf("✅ File deleted successfully: %s\n", file_path);
        
        // 删除备份文件（如果存在）
        char bak_path[256];
        snprintf(bak_path, sizeof(bak_path), "%s.bak", file_path);
        remove(bak_path);  // 忽略返回值，备份文件可能不存在
        
        // 删除临时文件（如果存在）
        char tmp_path[256];
        snprintf(tmp_path, sizeof(tmp_path), "%s.tmp", file_path);
        remove(tmp_path);  // 忽略返回值，临时文件可能不存在
        
        // 刷新历史记录列表：关闭并重新打开弹窗
        if(history_msgbox != NULL) {
            lv_obj_del(history_msgbox);
            history_msgbox = NULL;
        }
        
        // 重新创建历史记录弹窗
        lv_event_t dummy_event;
        switch_btn_event_cb(&dummy_event);
        
    } else {
        printf("❌ Failed to delete file: %s (error: %d)\n", file_path, ret);
        // TODO: 可以添加删除失败的UI提示
    }
}

/**
 * @brief 编辑文件按钮回调
 */
static void edit_file_cb(lv_event_t * e)
{
    // 获取文件名（从user_data中）
    const char *filename = (const char *)lv_event_get_user_data(e);
    
    if (filename == NULL) {
        printf("❌ Filename is NULL\n");
        return;
    }
    
    // 构建完整文件路径
    char file_path[256];
    snprintf(file_path, sizeof(file_path), "%s/%s", HISTORY_DIR, filename);
    
    printf("📝 Editing file: %s\n", file_path);
    
    // 读取文件内容
    char file_content[1024] = {0};
    file_err_t ret = file_read_string(file_path, file_content, sizeof(file_content));
    
    if (ret == FILE_OK) {
        // 加载内容到文本编辑区
        if (textarea != NULL) {
            lv_textarea_set_text(textarea, file_content);
            printf("✅ File content loaded for editing: %s\n", filename);
        }
        
        // 关闭历史记录弹窗
        if(history_msgbox != NULL) {
            lv_obj_del(history_msgbox);
            history_msgbox = NULL;
        }
        
    } else {
        printf("❌ Failed to read file: %s (error: %d)\n", file_path, ret);
        // TODO: 可以添加读取失败的UI提示
    }
}

/**
 * @brief 另存弹窗-取消按钮回调
 */
static void saveas_cancel_cb(lv_event_t * e)
{
    if(saveas_msgbox != NULL) {
        lv_obj_del(saveas_msgbox);
        saveas_msgbox = NULL;
        printf("Save as cancelled\n");
    }
}

/**
 * @brief 另存弹窗-确认按钮回调
 */
static void saveas_confirm_cb(lv_event_t * e)
{
    printf("Save as confirmed\n");
    
    if(textarea == NULL) {
        printf("❌ Textarea is NULL\n");
        goto close_msgbox;
    }
    
    // 获取当前文本编辑区内容
    const char *text = lv_textarea_get_text(textarea);
    if (text == NULL || strlen(text) == 0) {
        printf("⚠️  Text is empty, nothing to save\n");
        goto close_msgbox;
    }
    
    // 确保目录存在
    ensure_directory_exists(HISTORY_DIR);
    
    // 查找可用的文件名（notebook1.txt, notebook2.txt, ...）
    char new_file_path[256];
    int file_index = 1;
    
    while (file_index < 1000) {  // 最多支持1000个文件
        snprintf(new_file_path, sizeof(new_file_path), 
                 "%s/notebook%d.txt", HISTORY_DIR, file_index);
        
        // 检查文件是否已存在
        if (access(new_file_path, F_OK) != 0) {
            // 文件不存在，使用这个文件名
            break;
        }
        
        file_index++;
    }
    
    if (file_index >= 1000) {
        printf("❌ Too many files, cannot create more\n");
        goto close_msgbox;
    }
    
    printf("📝 Saving as: %s\n", new_file_path);
    
    // 使用原子写入保存到新文件
    file_err_t ret = file_write_string(new_file_path, text, strlen(text) + 1);
    
    if (ret == FILE_OK) {
        printf("✅ File saved successfully as %s\n", new_file_path);
        printf("Content length: %zu bytes\n", strlen(text));
        
        // TODO: 可以添加保存成功的UI提示
    } else {
        printf("❌ Failed to save file (error: %d)\n", ret);
        // TODO: 可以添加保存失败的UI提示
    }
    
close_msgbox:
    if(saveas_msgbox != NULL) {
        lv_obj_del(saveas_msgbox);
        saveas_msgbox = NULL;
        printf("Save as msgbox closed\n");
    }
}

/**
 * @brief 另存按钮点击回调 - 显示另存弹窗
 */
static void saveas_btn_event_cb(lv_event_t * e)
{
    printf("Save as button clicked\n");
    
    // 如果弹窗已存在，先删除
    if(saveas_msgbox != NULL) {
        lv_obj_del(saveas_msgbox);
        saveas_msgbox = NULL;
    }
    
    // 创建另存弹窗容器
    saveas_msgbox = lv_obj_create(lv_scr_act());
    lv_obj_set_size(saveas_msgbox, 240, 160);
    lv_obj_center(saveas_msgbox);  // 居中显示
    lv_obj_set_style_bg_color(saveas_msgbox, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_color(saveas_msgbox, lv_color_hex(0x3498DB), LV_PART_MAIN);
    lv_obj_set_style_border_width(saveas_msgbox, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(saveas_msgbox, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(saveas_msgbox, 20, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(saveas_msgbox, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(saveas_msgbox, LV_OPA_50, LV_PART_MAIN);
    lv_obj_clear_flag(saveas_msgbox, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建标题
    lv_obj_t *title_label = lv_label_create(saveas_msgbox);
    obj_font_set(title_label, FONT_TYPE_CN, 18);
    lv_label_set_text(title_label, "另存为");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, -10);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x333333), LV_PART_MAIN);
    
    // 创建内容区域（可以添加文件名输入框等）
    lv_obj_t *content_label = lv_label_create(saveas_msgbox);
    obj_font_set(content_label, FONT_TYPE_CN, 14);
    lv_label_set_text(content_label, "确认另存为新文件？");
    lv_obj_align(content_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(content_label, lv_color_hex(0x666666), LV_PART_MAIN);
    
    // 左下角：取消按钮
    lv_obj_t *cancel_btn = lv_btn_create(saveas_msgbox);
    lv_obj_set_size(cancel_btn, 70, 30);
    lv_obj_align(cancel_btn, LV_ALIGN_BOTTOM_LEFT, 15, -10);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0x95A5A6), LV_PART_MAIN);
    lv_obj_set_style_radius(cancel_btn, 5, LV_PART_MAIN);
    
    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    obj_font_set(cancel_label, FONT_TYPE_CN, 14);
    lv_label_set_text(cancel_label, "取消");
    lv_obj_center(cancel_label);
    lv_obj_set_style_text_color(cancel_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_add_event_cb(cancel_btn, saveas_cancel_cb, LV_EVENT_CLICKED, NULL);
    
    // 右下角：确认按钮
    lv_obj_t *confirm_btn = lv_btn_create(saveas_msgbox);
    lv_obj_set_size(confirm_btn, 70, 30);
    lv_obj_align(confirm_btn, LV_ALIGN_BOTTOM_RIGHT, -15, -10);
    lv_obj_set_style_bg_color(confirm_btn, lv_color_hex(0x3498DB), LV_PART_MAIN);
    lv_obj_set_style_radius(confirm_btn, 5, LV_PART_MAIN);
    
    lv_obj_t *confirm_label = lv_label_create(confirm_btn);
    obj_font_set(confirm_label, FONT_TYPE_CN, 14);
    lv_label_set_text(confirm_label, "确认");
    lv_obj_center(confirm_label);
    lv_obj_set_style_text_color(confirm_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_add_event_cb(confirm_btn, saveas_confirm_cb, LV_EVENT_CLICKED, NULL);
    
    printf("Save as msgbox created and displayed\n");
}

/**
 * @brief 切换按钮点击回调 - 显示历史记录弹窗
 */
static void switch_btn_event_cb(lv_event_t * e)
{
    printf("Switch button clicked, showing history msgbox\n");
    
    // 如果弹窗已存在，先删除
    if(history_msgbox != NULL) {
        lv_obj_del(history_msgbox);
        history_msgbox = NULL;
    }
    
    // 创建弹窗容器
    history_msgbox = lv_obj_create(lv_scr_act());
    lv_obj_set_size(history_msgbox, 400, 260);
    lv_obj_center(history_msgbox);  // 居中显示
    lv_obj_set_style_bg_color(history_msgbox, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_color(history_msgbox, lv_color_hex(0x3498DB), LV_PART_MAIN);
    lv_obj_set_style_border_width(history_msgbox, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(history_msgbox, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(history_msgbox, 20, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(history_msgbox, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(history_msgbox, LV_OPA_50, LV_PART_MAIN);
    lv_obj_clear_flag(history_msgbox, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建标题
    lv_obj_t *title_label = lv_label_create(history_msgbox);
    obj_font_set(title_label, FONT_TYPE_CN, 20);
    lv_label_set_text(title_label, "历史记录");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x333333), LV_PART_MAIN);
    
    // 创建内容区域（可滚动列表）
    lv_obj_t *content_area = lv_obj_create(history_msgbox);
    lv_obj_set_size(content_area, 380, 180);
    lv_obj_align(content_area, LV_ALIGN_TOP_MID, 0, 35);
    lv_obj_set_style_bg_color(content_area, lv_color_hex(0xF5F5F5), LV_PART_MAIN);
    lv_obj_set_style_border_width(content_area, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(content_area, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_radius(content_area, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_all(content_area, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(content_area, LV_FLEX_FLOW_COLUMN);  // 垂直排列
    lv_obj_set_flex_align(content_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    
    // 读取目录下的所有文件
    DIR *dir = opendir(HISTORY_DIR);
    if (dir == NULL) {
        printf("⚠️  Failed to open directory: %s\n", HISTORY_DIR);
        
        // 显示错误信息
        lv_obj_t *error_label = lv_label_create(content_area);
        obj_font_set(error_label, FONT_TYPE_CN, 16);
        lv_label_set_text(error_label, "无法读取目录");
        lv_obj_set_style_text_color(error_label, lv_color_hex(0xFF0000), LV_PART_MAIN);
    } else {
        struct dirent *entry;
        int file_count = 0;
        
        while ((entry = readdir(dir)) != NULL && file_count < 20) {
            // 跳过 . 和 .. 以及备份文件
            if (strcmp(entry->d_name, ".") == 0 || 
                strcmp(entry->d_name, "..") == 0 ||
                strstr(entry->d_name, ".tmp") != NULL ||
                strstr(entry->d_name, ".bak") != NULL) {
                continue;
            }
            
            // 读取文件内容的前10个字符
            char file_path[256];
            snprintf(file_path, sizeof(file_path), "%s/%s", HISTORY_DIR, entry->d_name);
            
            char preview[32] = {0};
            FILE *fp = fopen(file_path, "r");
            if (fp != NULL) {
                // 跳过文件头（16字节的header）
                fseek(fp, 16, SEEK_SET);
                
                // 读取前10个字符
                size_t read_count = fread(preview, 1, 10, fp);
                preview[read_count] = '\0';
                fclose(fp);
                
                // 如果内容为空，显示提示
                if (strlen(preview) == 0) {
                    strcpy(preview, "(空文件)");
                }
            } else {
                // 无法读取文件，显示提示
                strcpy(preview, "(无法读取)");
            }
            
            // 为每个文件创建一个容器（包含3行信息）
            lv_obj_t *file_item = lv_obj_create(content_area);
            lv_obj_set_size(file_item, 360, 75);
            lv_obj_set_style_bg_color(file_item, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            lv_obj_set_style_border_width(file_item, 1, LV_PART_MAIN);
            lv_obj_set_style_border_color(file_item, lv_color_hex(0xDDDDDD), LV_PART_MAIN);
            lv_obj_set_style_radius(file_item, 5, LV_PART_MAIN);
            lv_obj_set_style_pad_all(file_item, 5, LV_PART_MAIN);
            lv_obj_clear_flag(file_item, LV_OBJ_FLAG_SCROLLABLE);
            
            // 第一行：文件名（带图标）
            lv_obj_t *filename_label = lv_label_create(file_item);
            obj_font_set(filename_label, FONT_TYPE_CN, 16);
            char filename_text[64];
            snprintf(filename_text, sizeof(filename_text), "📄 %s", entry->d_name);
            lv_label_set_text(filename_label, filename_text);
            lv_obj_align(filename_label, LV_ALIGN_TOP_LEFT, 0, 0);
            lv_obj_set_style_text_color(filename_label, lv_color_hex(0x2C3E50), LV_PART_MAIN);
            
            // 第二行：内容预览
            lv_obj_t *preview_label = lv_label_create(file_item);
            obj_font_set(preview_label, FONT_TYPE_CN, 14);
            char preview_text[64];
            snprintf(preview_text, sizeof(preview_text), "%s...", preview);
            lv_label_set_text(preview_label, preview_text);
            lv_obj_align(preview_label, LV_ALIGN_TOP_LEFT, 0, 22);
            lv_obj_set_style_text_color(preview_label, lv_color_hex(0x7F8C8D), LV_PART_MAIN);
            
            // 第三行：编辑和删除按钮
            // 编辑按钮
            lv_obj_t *edit_btn = lv_btn_create(file_item);
            lv_obj_set_size(edit_btn, 60, 25);
            lv_obj_align(edit_btn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
            lv_obj_set_style_bg_color(edit_btn, lv_color_hex(0x3498DB), LV_PART_MAIN);
            lv_obj_set_style_radius(edit_btn, 3, LV_PART_MAIN);
            
            lv_obj_t *edit_label = lv_label_create(edit_btn);
            obj_font_set(edit_label, FONT_TYPE_CN, 14);
            lv_label_set_text(edit_label, "编辑");
            lv_obj_center(edit_label);
            lv_obj_set_style_text_color(edit_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            
            // 复制文件名到动态内存（因为entry->d_name在循环后会失效）
            char *filename_copy_edit = strdup(entry->d_name);
            lv_obj_add_event_cb(edit_btn, edit_file_cb, LV_EVENT_CLICKED, filename_copy_edit);
            
            // 删除按钮
            lv_obj_t *delete_btn = lv_btn_create(file_item);
            lv_obj_set_size(delete_btn, 60, 25);
            lv_obj_align(delete_btn, LV_ALIGN_BOTTOM_LEFT, 70, 0);
            lv_obj_set_style_bg_color(delete_btn, lv_color_hex(0xE74C3C), LV_PART_MAIN);
            lv_obj_set_style_radius(delete_btn, 3, LV_PART_MAIN);
            
            lv_obj_t *delete_label = lv_label_create(delete_btn);
            obj_font_set(delete_label, FONT_TYPE_CN, 14);
            lv_label_set_text(delete_label, "删除");
            lv_obj_center(delete_label);
            lv_obj_set_style_text_color(delete_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
            
            // 复制文件名到动态内存（因为entry->d_name在循环后会失效）
            char *filename_copy_delete = strdup(entry->d_name);
            lv_obj_add_event_cb(delete_btn, delete_file_cb, LV_EVENT_CLICKED, filename_copy_delete);
            
            file_count++;
            printf("Found history: %s (preview: %s)\n", entry->d_name, preview);
        }
        
        closedir(dir);
        
        // 如果没有找到文件
        if (file_count == 0) {
            lv_obj_t *placeholder = lv_label_create(content_area);
            obj_font_set(placeholder, FONT_TYPE_CN, 16);
            lv_label_set_text(placeholder, "暂无历史记录");
            lv_obj_center(placeholder);
            lv_obj_set_style_text_color(placeholder, lv_color_hex(0x999999), LV_PART_MAIN);
        } else {
            printf("✅ Loaded %d history files\n", file_count);
        }
    }
    
    // 创建关闭按钮
    lv_obj_t *close_btn = lv_btn_create(history_msgbox);
    lv_obj_set_size(close_btn, 80, 35);
    lv_obj_align(close_btn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(close_btn, lv_color_hex(0xE74C3C), LV_PART_MAIN);
    lv_obj_set_style_radius(close_btn, 5, LV_PART_MAIN);
    
    lv_obj_t *close_label = lv_label_create(close_btn);
    obj_font_set(close_label, FONT_TYPE_CN, 16);
    lv_label_set_text(close_label, "关闭");
    lv_obj_center(close_label);
    lv_obj_set_style_text_color(close_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_add_event_cb(close_btn, close_history_msgbox_cb, LV_EVENT_CLICKED, NULL);
    
    printf("History msgbox created and displayed\n");
}

/**
 * @brief 取消按钮点击回调 - 清空文本内容
 */
static void cancel_btn_event_cb(lv_event_t * e)
{
    printf("Cancel button clicked, clearing text\n");
    
    // 一键清空文本编辑区内容
    if(textarea != NULL) {
        lv_textarea_set_text(textarea, "");
        printf("Text area cleared\n");
    }
}

/**
 * @brief 保存按钮点击回调
 */
static void save_btn_event_cb(lv_event_t * e)
{
    printf("Save button clicked\n");
    
    if(textarea != NULL) {
        // 获取当前文本编辑区内容
        const char *text = lv_textarea_get_text(textarea);
        
        // 保存到缓存
        strncpy(notebook_content, text, sizeof(notebook_content) - 1);
        notebook_content[sizeof(notebook_content) - 1] = '\0';
        
        printf("Content to save: %s\n", notebook_content);
        
        // 调用原子写入接口保存到文件（带掉电保护）
        file_err_t ret = save_notebook_content(notebook_content);
        
        if (ret == FILE_OK) {
            printf("✅ Notebook content saved successfully with power-loss protection\n");
            // TODO: 可以添加保存成功的UI提示
        } else {
            printf("❌ Failed to save notebook content (error: %d)\n", ret);
            // TODO: 可以添加保存失败的UI提示
        }
    }
}

/**
 * @brief 创建顶部功能栏
 */
static void init_header_view(lv_obj_t *parent)
{
    // 创建顶部容器
    lv_obj_t *header = lv_obj_create(parent);
    lv_obj_set_size(header, 1424, 60);
    lv_obj_align(header, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_add_style(header, &com_style, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(header, lv_color_hex(0xF5F5F5), LV_PART_MAIN);
    
    /* ========== 左侧：返回按钮 ========== */
    lv_obj_t *back_btn_cont = lv_obj_create(header);
    lv_obj_set_size(back_btn_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(back_btn_cont, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_add_style(back_btn_cont, &com_style, LV_PART_MAIN);
    lv_obj_add_flag(back_btn_cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(back_btn_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *back_img = lv_img_create(back_btn_cont);
    lv_img_set_src(back_img, GET_IMAGE_PATH("main/back.png"));
    lv_obj_align(back_img, LV_ALIGN_CENTER, 0, 0);
    
    lv_obj_add_event_cb(back_btn_cont, back_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    /* ========== 中间：标题 ========== */
    lv_obj_t *title_label = lv_label_create(header);
    obj_font_set(title_label, FONT_TYPE_CN, 24);
    lv_label_set_text(title_label, "记事本");
    lv_obj_align(title_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x333333), LV_PART_MAIN);
    
    /* ========== 右侧：切换、取消和保存按钮 ========== */
    // 切换按钮
    lv_obj_t *switch_btn = lv_btn_create(header);
    lv_obj_set_size(switch_btn, 80, 40);
    lv_obj_align(switch_btn, LV_ALIGN_RIGHT_MID, -400, 0);
    lv_obj_set_style_bg_color(switch_btn, lv_color_hex(0x9B59B6), LV_PART_MAIN);
    lv_obj_set_style_radius(switch_btn, 5, LV_PART_MAIN);
    
    lv_obj_t *switch_label = lv_label_create(switch_btn);
    obj_font_set(switch_label, FONT_TYPE_CN, 18);
    lv_label_set_text(switch_label, "切换");
    lv_obj_center(switch_label);
    lv_obj_set_style_text_color(switch_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_add_event_cb(switch_btn, switch_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 取消按钮（清空）
    lv_obj_t *cancel_btn = lv_btn_create(header);
    lv_obj_set_size(cancel_btn, 80, 40);
    lv_obj_align(cancel_btn, LV_ALIGN_RIGHT_MID, -320, 0);
    lv_obj_set_style_bg_color(cancel_btn, lv_color_hex(0xE74C3C), LV_PART_MAIN);
    lv_obj_set_style_radius(cancel_btn, 5, LV_PART_MAIN);
    
    lv_obj_t *cancel_label = lv_label_create(cancel_btn);
    obj_font_set(cancel_label, FONT_TYPE_CN, 18);
    lv_label_set_text(cancel_label, "清空");
    lv_obj_center(cancel_label);
    lv_obj_set_style_text_color(cancel_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_add_event_cb(cancel_btn, cancel_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 保存按钮
    lv_obj_t *save_btn = lv_btn_create(header);
    lv_obj_set_size(save_btn, 80, 40);
    lv_obj_align(save_btn, LV_ALIGN_RIGHT_MID, -150, 0);
    lv_obj_set_style_bg_color(save_btn, lv_color_hex(0x4CAF50), LV_PART_MAIN);
    lv_obj_set_style_radius(save_btn, 5, LV_PART_MAIN);
    
    lv_obj_t *save_label = lv_label_create(save_btn);
    obj_font_set(save_label, FONT_TYPE_CN, 18);
    lv_label_set_text(save_label, "保存");
    lv_obj_center(save_label);
    lv_obj_set_style_text_color(save_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_add_event_cb(save_btn, save_btn_event_cb, LV_EVENT_CLICKED, NULL);
    
    // 另存按钮
    lv_obj_t *saveas_btn = lv_btn_create(header);
    lv_obj_set_size(saveas_btn, 80, 40);
    lv_obj_align(saveas_btn, LV_ALIGN_RIGHT_MID, -60, 0);
    lv_obj_set_style_bg_color(saveas_btn, lv_color_hex(0x2ECC71), LV_PART_MAIN);
    lv_obj_set_style_radius(saveas_btn, 5, LV_PART_MAIN);
    
    lv_obj_t *saveas_label = lv_label_create(saveas_btn);
    obj_font_set(saveas_label, FONT_TYPE_CN, 18);
    lv_label_set_text(saveas_label, "另存");
    lv_obj_center(saveas_label);
    lv_obj_set_style_text_color(saveas_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    
    lv_obj_add_event_cb(saveas_btn, saveas_btn_event_cb, LV_EVENT_CLICKED, NULL);
}

/**
 * @brief 创建内容编辑区
 */
static void init_content_view(lv_obj_t *parent)
{
    // 创建文本编辑区容器
    content_cont = lv_obj_create(parent);
    lv_obj_set_size(content_cont, 1424, 220);  // 总高度280 - 顶部60 = 220
    lv_obj_align(content_cont, LV_ALIGN_TOP_LEFT, 0, 60);
    lv_obj_add_style(content_cont, &com_style, LV_PART_MAIN);
    lv_obj_clear_flag(content_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    // 创建文本编辑区（textarea）
    textarea = lv_textarea_create(content_cont);
    lv_obj_set_size(textarea, 1384, 200);  // 留出边距
    lv_obj_align(textarea, LV_ALIGN_CENTER, 0, 0);
    
    // 设置字体
    obj_font_set(textarea, FONT_TYPE_CN, 20);
    
    // 设置占位符文本
    lv_textarea_set_placeholder_text(textarea, "在这里输入你的记事内容...");
    
    // 设置文本内容（从缓存加载）
    lv_textarea_set_text(textarea, notebook_content);
    
    // 设置样式
    lv_obj_set_style_bg_color(textarea, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_color(textarea, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_set_style_border_width(textarea, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(textarea, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_all(textarea, 10, LV_PART_MAIN);
    lv_obj_set_style_text_color(textarea, lv_color_hex(0x333333), LV_PART_MAIN);
    
    // 设置为单行模式（如果需要多行，注释掉下面这行）
    // lv_textarea_set_one_line(textarea, true);
    
    // 设置最大长度
    lv_textarea_set_max_length(textarea, sizeof(notebook_content) - 1);
    
    // 添加点击事件，用于切换键盘显示
    lv_obj_add_event_cb(textarea, textarea_click_cb, LV_EVENT_SHORT_CLICKED, NULL);
    
    printf("Textarea created and initialized\n");
}

/**
 * @brief 创建虚拟键盘（浮动在文本区域内）
 */
static void init_keyboard_view(void)
{
    // 创建虚拟键盘，挂载到textarea上，显示在文本输入区域内
    keyboard = lv_keyboard_create(textarea);
    lv_obj_set_size(keyboard, 850, 150);  // 设置键盘大小
    
    // 初始位置：文本区域内居中偏下
    lv_obj_set_pos(keyboard, 180, 5);
    
    // 绑定到文本编辑区
    lv_keyboard_set_textarea(keyboard, textarea);
    
    // 设置键盘样式 - 浮动效果
    lv_obj_set_style_bg_color(keyboard, lv_color_hex(0xECECEC), LV_PART_MAIN);
    lv_obj_set_style_border_width(keyboard, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(keyboard, lv_color_hex(0x3498DB), LV_PART_MAIN);
    lv_obj_set_style_radius(keyboard, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(keyboard, 15, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(keyboard, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(keyboard, LV_OPA_40, LV_PART_MAIN);
    
    // 设置按键样式
    lv_obj_set_style_bg_color(keyboard, lv_color_hex(0xFFFFFF), LV_PART_ITEMS);
    lv_obj_set_style_text_color(keyboard, lv_color_hex(0x212529), LV_PART_ITEMS);
    lv_obj_set_style_radius(keyboard, 5, LV_PART_ITEMS);
    
    // 设置键盘按键字体为Montserrat 14（包含符号图标）
    // 这是解决退格键、回车键等特殊按键图标显示为矩形X的关键
    lv_obj_set_style_text_font(keyboard, &lv_font_montserrat_14, LV_PART_ITEMS);
    
    // 初始时隐藏键盘
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    keyboard_visible = false;
    
    printf("Virtual keyboard created (inside textarea, initially hidden)\n");
}

/**
 * @brief 初始化记事本页面
 */
void init_pageNotebook(void)
{
    printf("Initializing pageNotebook...\n");
    
    // 初始化样式
    com_style_init();
    
    // 从文件加载记事本内容（掉电恢复）
    load_notebook_content();
    
    // 创建主容器
    lv_obj_t *main_cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_cont, 1424, 280);
    lv_obj_add_style(main_cont, &com_style, LV_PART_MAIN);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(main_cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // 初始化顶部功能栏
    init_header_view(main_cont);
    
    // 初始化内容编辑区
    init_content_view(main_cont);
    
    // 初始化虚拟键盘（浮动在文本区域内）
    init_keyboard_view();
    
    printf("pageNotebook initialized successfully\n");
}
