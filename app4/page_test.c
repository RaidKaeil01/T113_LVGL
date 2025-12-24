#include "lvgl.h"
#include "font_conf.h"
#include <stdio.h>
#include "image_conf.h"

//extern const lv_font_t test_font;
extern const lv_img_dsc_t favicon;//图片资源声明

//按钮事件回调函数
bool is_long_click = false;
void btn_event_callback_func(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * btn = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        if(is_long_click) {
            is_long_click = false;
            return;
        }
        printf("Button clicked\n");
    } 
    if(code == LV_EVENT_LONG_PRESSED) {
    printf("Button LONG_PRESSED\n");
    is_long_click = true;
    } 
}

//开关事件返回函数
static void sw_event_callback_func(lv_event_t * e){
    lv_obj_t * sw= lv_event_get_target(e);//获取事件作用对象
    bool state = lv_obj_has_state(sw, LV_STATE_CHECKED); //获取开关状态
    lv_event_code_t code = lv_event_get_code(e); //获取事件代码  

    // //打印开关状态
    // if(state){
    //     printf("Switch is ON\n");
    // }else{
    //     printf("Switch is OFF\n");
    // }

    //根据不同事件代码执行不同操作
    switch (code)
    {
    case LV_EVENT_VALUE_CHANGED:
        printf("State: %s\n", lv_obj_has_state(sw, LV_STATE_CHECKED) ? "On" : "Off");
        break; 
    default:
        break;
    }
}



/**
 * @brief 文本框事件回调函数
 * @note 当用户在文本框中按下回车键时触发 LV_EVENT_READY 事件
 * @param e 事件对象，包含事件类型和触发对象信息
 */
static void ta_event_callback_func(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);  // 获取事件代码
    lv_obj_t * ta = lv_event_get_target(e);       // 获取触发事件的文本框对象
    
    // 判断是否为 READY 事件（用户按下回车键）
    if (code == LV_EVENT_READY) {
        const char * text = lv_textarea_get_text(ta);  // 读取文本框内容
        if (text != NULL) {
            printf("current text: %s\n", text);  // 打印文本内容
        } else {
            printf("current text: (empty)\n");   // 文本为空
        }
    }
}


/**
 * @brief 按钮点击事件回调函数
 * @note 每次点击按钮时都会读取并打印文本框的当前内容
 * @param e 事件对象，user_data 中存储了文本框对象指针
 */
static void btn_click_event_cb_func(lv_event_t * e){
    printf("btn click\n");  // 打印按钮点击提示
    
    // 从事件的 user_data 中获取文本框对象（在创建事件时传入）
    lv_obj_t * ta = lv_event_get_user_data(e);
    
    // 读取文本框当前内容
    const char * text = lv_textarea_get_text(ta);
    
    // 判断文本是否为空（检查指针和首字符）
    if (text != NULL && text[0] != '\0') {
        printf("current text: %s\n", text);  // 打印文本内容
    } else {
        printf("current text: (empty)\n");   // 文本为空
    }
}


/**
 * @brief 动画回调函数 - 图片旋转
 * @param img 图片对象指针
 * @param value 动画当前值（角度值，范围0-3600，对应0-360度，精度0.1度）
 * @note 每次动画更新时调用此函数，设置图片的旋转角度
 */
static void anim_cb(void * img, int32_t value) {
    // 每次动画回调时，设置对象的角度
    // LVGL的角度值：0-3600表示0-360度（乘以10存储，提高精度）
    lv_img_set_angle(img, value);
}


// /**
//  * @brief 进度条动画回调函数
//  * @param bar 进度条对象指针
//  * @param value 动画当前值（0-100）
//  * @note 每次动画更新时调用此函数，同步更新进度条的值和标签文本
//  */
// static void set_temp(void * bar, int32_t value) {
//     // 设置进度条的值（0-100）
//     lv_bar_set_value(bar, value, LV_ANIM_OFF);  // LV_ANIM_OFF表示直接设置，无动画过渡
    
//     // 更新标签显示内容，格式化显示百分比
//     lv_label_set_text_fmt(label_bar, "%d%%", value);  // %% 转义为单个 %
// }


// 全局变量：滑动条对应的标签对象（用于在回调函数中更新显示）
static lv_obj_t * slider_label;
/**
 * @brief 滑动条值变化事件回调函数
 * @param e 事件对象，包含事件类型和触发对象信息
 * @note 每次滑动条的值改变时都会调用此函数，同步更新标签显示
 */
static void slider_event_cb(lv_event_t * e)
{
    // 1. 获取触发事件的滑动条对象
    lv_obj_t * slider = lv_event_get_target(e);
    
    // 2. 读取滑动条当前值（默认范围0-100）
    int value = (int)lv_slider_get_value(slider);
    
    // 3. 更新标签显示内容，格式化显示百分比
    lv_label_set_text_fmt(slider_label, "%d%%", value);  // %% 转义为单个 %
    
    // 4. 重新对齐标签位置
    // 原因：标签文本长度改变（如 "9%" → "10%"）时，为保持居中对齐，需重新计算位置
    lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}


// 全局变量：圆弧进度条对应的标签对象（用于在回调函数中更新显示）
static lv_obj_t * arc_label;
/**
 * @brief 圆弧进度条动画回调函数
 * @param obj 圆弧对象指针
 * @param v 动画当前值（0-150）
 * @note 每次动画更新时调用此函数，将0-150映射为0-360度的圆弧角度，标签显示0-150的值
 */
static void set_angle(void * obj, int32_t v)
{
    // 将0-150的值映射为0-360度：角度 = v * 360 / 150 = v * 2.4
    // 由于lv_arc使用百分比（0-100），需要先设置范围为0-360
    int32_t angle = (v * 360) / 150;  // 计算对应的角度值（0-360）
    lv_arc_set_value(obj, angle);      // 设置圆弧角度
    
    // 更新标签显示内容，显示原始值0-150
    lv_label_set_text_fmt(arc_label, "%d", v);  // 显示0-150的数值
}


/**
 * @brief 滚轮值改变事件回调函数
 * @param e 事件对象，包含事件类型和触发对象信息
 * @note 当用户滚动滚轮并选择新选项时调用此函数
 *       打印选中项的索引ID和文本内容
 */
static void event_handler(lv_event_t * e)
{
    // 1. 获取触发事件的滚轮对象
    lv_obj_t * roller = lv_event_get_target(e);
    
    // 2. 获取当前选中项的索引ID（从0开始计数）
    // 例如: Monday=0, Tuesday=1, Wednesday=2, ...
    int id = lv_roller_get_selected(roller);
    
    // 3. 创建缓冲区用于存储选中项的文本内容
    char buf[32];
    
    // 4. 获取当前选中项的文本字符串
    // 参数说明：
    //   - roller: 滚轮对象
    //   - buf: 存储文本的缓冲区
    //   - sizeof(buf): 缓冲区大小（防止溢出）
    lv_roller_get_selected_str(roller, buf, sizeof(buf));
    
    // 5. 打印选中项的索引和文本内容
    printf("Selected id:%d day: %s\n", id, buf);
}


/* ========== 定时器回调函数 ========== */
// 全局变量：定时器关联的标签对象（用于在回调函数中更新显示）
static lv_obj_t * label_Timer;

// 全局变量：计数器，记录定时器执行的次数
static int count = 0;

/**
 * @brief 定时器回调函数
 * @param timer 定时器对象指针
 * @note 每次定时器触发时调用此函数，更新标签显示当前计数值
 *       定时器按设定的周期（如1000ms）自动调用此函数
 */
void timer_cb_func(lv_timer_t * timer)
{
    // 1. 计数器自增（每次定时器触发时加1）
    count++;
    
    // 2. 更新标签显示内容，格式化显示当前计数值
    lv_label_set_text_fmt(label_Timer, "Count = %d", count);
}  


/* ========== Grid网格布局配置 ========== */
/**
 * @brief 网格布局描述数组 - 定义3x3网格结构
 * @note 必须以 LV_GRID_TEMPLATE_LAST 结尾标识数组结束
 *       这些数组定义了网格的行列数量和每个格子的尺寸
 */

// 列描述数组：定义网格的列数和每列的宽度
// 格式：{第1列宽度, 第2列宽度, ..., LV_GRID_TEMPLATE_LAST}
// 这里定义3列，每列70像素宽
static lv_coord_t col_dsc[] = {70, 70, 70, LV_GRID_TEMPLATE_LAST};

// 行描述数组：定义网格的行数和每行的高度
// 格式：{第1行高度, 第2行高度, ..., LV_GRID_TEMPLATE_LAST}
// 这里定义3行，每行70像素高
static lv_coord_t row_dsc[] = {70, 70, 70, LV_GRID_TEMPLATE_LAST};


void lv_example_hello_world(void) {
// /* ========== Flex弹性布局示例(线性布局) ========== */
// /**
//  * @brief 创建弹性布局容器 - 自动排列子对象
//  * @note Flex布局是LVGL 8.x的核心布局方式，类似CSS Flexbox
//  *       适用于需要自动排列的场景：菜单、按钮组、列表等
//  */
    
//     // 1. 创建弹性布局容器
//     lv_obj_t * cont_row = lv_obj_create(lv_scr_act());  // 在当前活动屏幕上创建容器
    
//     // 2. 设置容器大小
//     // 参数：宽度1400像素，高度400像素
//     lv_obj_set_size(cont_row, 1400, 400);
    
//     // 3. 设置弹性布局流向为行方向（水平排列）
//     // 参数说明：
//     //   - LV_FLEX_FLOW_ROW: 从左到右水平排列（默认）
//     //   - LV_FLEX_FLOW_COLUMN: 从上到下垂直排列
//     //   - LV_FLEX_FLOW_ROW_REVERSE: 从右到左排列
//     //   - LV_FLEX_FLOW_COLUMN_REVERSE: 从下到上排列
//     //   - LV_FLEX_FLOW_ROW_WRAP: 水平排列，超出换行
//     //   - LV_FLEX_FLOW_COLUMN_WRAP: 垂直排列，超出换列
//     lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_ROW);
//     //lv_obj_set_flex_flow(cont_row, LV_FLEX_FLOW_COLUMN);

//     // 4. 设置列间距（子对象之间的水平间距）
//     // 参数说明：
//     //   - cont_row: 容器对象
//     //   - 150: 列间距150像素（子对象之间的水平间隔）
//     //   - LV_PART_MAIN: 应用到主部件
//     // 注意：对于行布局(ROW)使用pad_column，对于列布局(COLUMN)使用pad_row
//     lv_obj_set_style_pad_column(cont_row, 150, LV_PART_MAIN);

//     // 5. 循环创建5个按钮作为容器的子对象
//     // Flex布局会自动按照设定的流向排列这些按钮
//     for(int i = 0; i < 5; i++) {
//         // 5.1 创建按钮对象，父对象为弹性容器
//         // 按钮会自动按照Flex规则排列，无需手动设置位置
//         lv_obj_t * obj = lv_btn_create(cont_row);
        
//         // 5.2 设置按钮大小：宽100像素，高50像素
//         lv_obj_set_size(obj, 100, 50);
        
//         // 5.3 在按钮上创建标签显示序号
//         lv_obj_t * label = lv_label_create(obj);
        
//         // 5.4 设置标签文本，显示"Item: 0"到"Item: 4"
//         lv_label_set_text_fmt(label, "Item: %u", i);
        
//         // 5.5 将标签在按钮中居中显示
//         lv_obj_center(label);
//     }
    
/* ========== Grid网格布局示例 ========== */
/**
 * @brief 创建网格布局容器 - 精确定位子对象
 * @note Grid布局类似CSS Grid，适用于需要精确控制位置的场景
 *       如：计算器键盘、棋盘、相册网格、仪表盘等
 */

    // 1. 创建网格布局容器
    lv_obj_t * cont = lv_obj_create(lv_scr_act());  // 在当前活动屏幕上创建容器
    
    // 2. 设置容器大小
    // 计算：3列 × 70px = 210px宽，3行 × 70px = 210px高
    lv_obj_set_size(cont, 210, 210);
    
    // 3. 将容器在屏幕中居中显示
    lv_obj_center(cont);
    
    // 4. 设置容器内边距为0（去除容器内部的留白）
    // 这样网格可以紧贴容器边缘
    lv_obj_set_style_pad_all(cont, 0, LV_PART_MAIN);
    
    // 5. 设置容器为网格布局模式
    // LV_LAYOUT_GRID 启用网格布局引擎
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    
    // 6. 设置网格列描述（定义列数和每列宽度）
    // 参数说明：
    //   - cont: 容器对象
    //   - col_dsc: 列描述数组 {70, 70, 70, LV_GRID_TEMPLATE_LAST}
    //   - LV_PART_MAIN: 应用到主部件
    lv_obj_set_style_grid_column_dsc_array(cont, col_dsc, LV_PART_MAIN);
    
    // 7. 设置网格行描述（定义行数和每行高度）
    // 参数说明：
    //   - cont: 容器对象
    //   - row_dsc: 行描述数组 {70, 70, 70, LV_GRID_TEMPLATE_LAST}
    //   - LV_PART_MAIN: 应用到主部件
    lv_obj_set_style_grid_row_dsc_array(cont, row_dsc, LV_PART_MAIN);
    
    // 8. 设置网格列间距为0（列与列之间无间隙）
    lv_obj_set_style_pad_column(cont, 0, LV_PART_MAIN);
    
    // 9. 设置网格行间距为0（行与行之间无间隙）
    lv_obj_set_style_pad_row(cont, 0, LV_PART_MAIN);
    
    // 10. 使用循环创建9个按钮填充3x3网格
    for(int i = 0; i < 9; i++) {
        // 10.1 计算当前按钮的列索引（0, 1, 2循环）
        // i % 3: 0→0, 1→1, 2→2, 3→0, 4→1, 5→2...
        uint8_t col = i % 3;
        
        // 10.2 计算当前按钮的行索引（每3个按钮换一行）
        // i / 3: 0-2→0, 3-5→1, 6-8→2
        uint8_t row = i / 3;
        
        // 10.3 创建按钮对象，父对象为网格容器
        lv_obj_t * btn = lv_btn_create(cont);
        
        // 10.4 设置按钮大小：60x60像素
        // 注意：按钮大小(60)小于格子大小(70)，会在格子内留有空隙
        lv_obj_set_size(btn, 60, 60);
        
        // 10.5 在按钮上创建标签显示位置信息
        lv_obj_t * label = lv_label_create(btn);
        
        // 10.6 设置标签文本，显示列和行坐标
        // 格式："c列号, r行号"，例如："c0, r0", "c1, r2"等
        lv_label_set_text_fmt(label, "c%d, r%d", col, row);
        
        // 10.7 将标签在按钮中居中显示
        lv_obj_center(label);
        
        // 10.8 设置按钮在网格中的位置
        // 参数说明：
        //   - btn: 要定位的按钮对象
        //   - LV_GRID_ALIGN_START: 列对齐方式（起始对齐，靠左）
        //   - col: 列索引（0-2）
        //   - 1: 列跨度（占据1列）
        //   - LV_GRID_ALIGN_START: 行对齐方式（起始对齐，靠上）
        //   - row: 行索引（0-2）
        //   - 1: 行跨度（占据1行）
        // 其他对齐选项：
        //   - LV_GRID_ALIGN_CENTER: 居中对齐
        //   - LV_GRID_ALIGN_END: 末端对齐
        //   - LV_GRID_ALIGN_STRETCH: 拉伸填充整个格子
        lv_obj_set_grid_cell(btn, LV_GRID_ALIGN_START, col, 1, LV_GRID_ALIGN_START, row, 1);
    }
    
    // 执行效果：创建3x3网格，共9个按钮
    // 布局结构：
    // ┌─────────┬─────────┬─────────┐
    // │ c0, r0  │ c1, r0  │ c2, r0  │  第0行
    // ├─────────┼─────────┼─────────┤
    // │ c0, r1  │ c1, r1  │ c2, r1  │  第1行
    // ├─────────┼─────────┼─────────┤
    // │ c0, r2  │ c1, r2  │ c2, r2  │  第2行
    // └─────────┴─────────┴─────────┘


} 


/*
 * ============================================================================
 * LVGL 常用函数参考手册
 * ============================================================================
 */

/*
 * 一、基础对象创建
 * ============================================================================
 */

// 1. 创建基础对象（容器）
// lv_obj_t *obj = lv_obj_create(parent);
// 参数：parent - 父对象，通常是 lv_scr_act() 获取当前屏幕

// 2. 创建标签（文本显示）
// lv_obj_t *label = lv_label_create(parent);
// lv_label_set_text(label, "文本内容");              // 设置文本
// lv_label_set_text_fmt(label, "数字: %d", 100);    // 格式化文本

// 3. 创建按钮
// lv_obj_t *btn = lv_btn_create(parent);
// lv_obj_t *label = lv_label_create(btn);           // 在按钮上创建标签
// lv_label_set_text(label, "按钮");

// 4. 创建图片
// lv_obj_t *img = lv_img_create(parent);
// lv_img_set_src(img, "路径/图片.png");              // 设置图片源
// lv_img_set_src(img, &img_variable);               // 使用变量

// 5. 创建滑动条
// lv_obj_t *slider = lv_slider_create(parent);
// lv_slider_set_value(slider, 50, LV_ANIM_OFF);    // 设置值 (0-100)
// lv_slider_set_range(slider, 0, 200);             // 设置范围

// 6. 创建开关
// lv_obj_t *sw = lv_switch_create(parent);
// lv_obj_add_state(sw, LV_STATE_CHECKED);          // 设置为开启状态

// 7. 创建复选框
// lv_obj_t *cb = lv_checkbox_create(parent);
// lv_checkbox_set_text(cb, "同意协议");

// 8. 创建下拉列表
// lv_obj_t *dd = lv_dropdown_create(parent);
// lv_dropdown_set_options(dd, "选项1\n选项2\n选项3");

// 9. 创建文本输入框
// lv_obj_t *ta = lv_textarea_create(parent);
// lv_textarea_set_text(ta, "初始文本");
// lv_textarea_set_placeholder_text(ta, "请输入...");

// 10. 创建弧形进度条
// lv_obj_t *arc = lv_arc_create(parent);
// lv_arc_set_value(arc, 75);                       // 设置值 (0-100)
// lv_arc_set_range(arc, 0, 360);                   // 设置范围

/*
 * 二、位置和大小设置
 * ============================================================================
 */

// 1. 设置位置（绝对坐标）
// lv_obj_set_pos(obj, x, y);                       // x, y 是像素坐标
// lv_obj_set_x(obj, x);                            // 只设置 x 坐标
// lv_obj_set_y(obj, y);                            // 只设置 y 坐标

// 2. 设置大小
// lv_obj_set_size(obj, width, height);             // 设置宽度和高度
// lv_obj_set_width(obj, width);                    // 只设置宽度
// lv_obj_set_height(obj, height);                  // 只设置高度
// lv_obj_set_content_width(obj, width);            // 自动适应内容宽度
// lv_obj_set_content_height(obj, height);          // 自动适应内容高度

// 3. 对齐方式
// lv_obj_align(obj, LV_ALIGN_CENTER, x_ofs, y_ofs);           // 相对父对象对齐
// lv_obj_align_to(obj, target, align, x_ofs, y_ofs);         // 相对其他对象对齐

// 常用对齐参数：
// LV_ALIGN_CENTER          - 居中
// LV_ALIGN_TOP_LEFT        - 左上角
// LV_ALIGN_TOP_MID         - 顶部居中
// LV_ALIGN_TOP_RIGHT       - 右上角
// LV_ALIGN_BOTTOM_LEFT     - 左下角
// LV_ALIGN_BOTTOM_MID      - 底部居中
// LV_ALIGN_BOTTOM_RIGHT    - 右下角
// LV_ALIGN_LEFT_MID        - 左侧居中
// LV_ALIGN_RIGHT_MID       - 右侧居中
// LV_ALIGN_OUT_TOP_LEFT    - 外部左上
// LV_ALIGN_OUT_BOTTOM_MID  - 外部底部居中

// 示例：
// lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);               // 屏幕居中
// lv_obj_align(label, LV_ALIGN_TOP_LEFT, 10, 10);           // 左上角，偏移(10,10)
// lv_obj_align_to(btn2, btn1, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // btn2在btn1下方10像素

/*
 * 三、样式设置
 * ============================================================================
 */

// 1. 背景颜色
// lv_obj_set_style_bg_color(obj, lv_color_hex(0xFF0000), 0);     // 红色
// lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);                    // 透明度50%

// 2. 边框
// lv_obj_set_style_border_width(obj, 2, 0);                      // 边框宽度
// lv_obj_set_style_border_color(obj, lv_color_hex(0x000000), 0);// 边框颜色

// 3. 圆角
// lv_obj_set_style_radius(obj, 10, 0);                           // 圆角半径

// 4. 内边距
// lv_obj_set_style_pad_all(obj, 10, 0);                          // 所有方向
// lv_obj_set_style_pad_left(obj, 5, 0);                          // 左侧
// lv_obj_set_style_pad_top(obj, 5, 0);                           // 顶部

// 5. 文本样式
// lv_obj_set_style_text_color(label, lv_color_hex(0x0000FF), 0);// 文本颜色
// lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0); // 字体

// 6. 阴影
// lv_obj_set_style_shadow_width(obj, 10, 0);                     // 阴影宽度
// lv_obj_set_style_shadow_color(obj, lv_color_hex(0x000000), 0);// 阴影颜色

/*
 * 四、事件处理
 * ============================================================================
 */

// 1. 添加事件回调
// lv_obj_add_event_cb(obj, event_handler, LV_EVENT_CLICKED, NULL);

// 事件回调函数示例：
// void event_handler(lv_event_t *e) {
//     lv_event_code_t code = lv_event_get_code(e);           // 获取事件类型
//     lv_obj_t *obj = lv_event_get_target(e);               // 获取触发对象
//     
//     if(code == LV_EVENT_CLICKED) {
//         printf("对象被点击\n");
//     }
// }

// 常用事件类型：
// LV_EVENT_PRESSED         - 按下
// LV_EVENT_RELEASED        - 释放
// LV_EVENT_CLICKED         - 点击
// LV_EVENT_VALUE_CHANGED   - 值改变
// LV_EVENT_FOCUSED         - 获得焦点
// LV_EVENT_READY           - 准备完成

/*
 * 五、动画
 * ============================================================================
 */

// 1. 创建动画
// lv_anim_t a;
// lv_anim_init(&a);
// lv_anim_set_var(&a, obj);                              // 设置动画对象
// lv_anim_set_time(&a, 1000);                            // 持续时间(毫秒)
// lv_anim_set_values(&a, 0, 100);                        // 起始值和结束值
// lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_x); // 执行回调
// lv_anim_start(&a);                                     // 启动动画

// 2. 动画路径
// lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);    // 缓动效果

/*
 * 六、屏幕管理
 * ============================================================================
 */

// 1. 获取当前屏幕
// lv_obj_t *scr = lv_scr_act();

// 2. 创建新屏幕
// lv_obj_t *new_scr = lv_obj_create(NULL);

// 3. 加载屏幕
// lv_scr_load(new_scr);                                  // 立即切换
// lv_scr_load_anim(new_scr, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, false); // 动画切换

/*
 * 七、颜色定义
 * ============================================================================
 */

// lv_color_hex(0xFF0000)           // 红色
// lv_color_hex(0x00FF00)           // 绿色
// lv_color_hex(0x0000FF)           // 蓝色
// lv_color_hex(0xFFFFFF)           // 白色
// lv_color_hex(0x000000)           // 黑色
// lv_color_hex(0xFFFF00)           // 黄色
// lv_palette_main(LV_PALETTE_RED)  // 使用调色板

/*
 * 八、常用宏和常量
 * ============================================================================
 */

// 透明度：
// LV_OPA_TRANSP    - 完全透明
// LV_OPA_0         - 0%
// LV_OPA_50        - 50%
// LV_OPA_COVER     - 完全不透明
// LV_OPA_100       - 100%

// 动画选项：
// LV_ANIM_OFF      - 无动画
// LV_ANIM_ON       - 有动画

/*
 * 九、实用工具函数
 * ============================================================================
 */

// 1. 删除对象
// lv_obj_del(obj);                                       // 删除对象
// lv_obj_clean(parent);                                  // 删除所有子对象

// 2. 隐藏/显示
// lv_obj_add_flag(obj, LV_OBJ_FLAG_HIDDEN);             // 隐藏
// lv_obj_clear_flag(obj, LV_OBJ_FLAG_HIDDEN);           // 显示

// 3. 启用/禁用点击
// lv_obj_add_flag(obj, LV_OBJ_FLAG_CLICKABLE);          // 可点击
// lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);        // 不可点击

// 4. 滚动控制
// lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLLABLE);         // 可滚动
// lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);       // 不可滚动

/*
 * 十、完整示例
 * ============================================================================
 */

/*
void create_ui_example(void) {
    // 1. 创建一个容器
    lv_obj_t *cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 300, 200);
    lv_obj_align(cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0xF0F0F0), 0);
    
    // 2. 创建标题标签
    lv_obj_t *title = lv_label_create(cont);
    lv_label_set_text(title, "示例界面");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(title, lv_color_hex(0x000000), 0);
    
    // 3. 创建按钮
    lv_obj_t *btn = lv_btn_create(cont);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn, button_event_handler, LV_EVENT_CLICKED, NULL);
    
    lv_obj_t *btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "点击我");
    lv_obj_center(btn_label);
    
    // 4. 创建滑动条
    lv_obj_t *slider = lv_slider_create(cont);
    lv_obj_set_width(slider, 200);
    lv_obj_align(slider, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_slider_set_value(slider, 50, LV_ANIM_OFF);
}

void button_event_handler(lv_event_t *e) {
    printf("按钮被点击了！\n");
}
*/


