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




void lv_example_hello_world(void) {
//     // 获取活动屏幕对象


//     lv_obj_t *obj = lv_obj_create(lv_scr_act());//父对象：lv_scr_act()-->活动的屏幕
//     lv_obj_set_size(obj,200, 200);             // 设置宽度和高度
//     lv_obj_set_pos(obj,200,50);                       // x, y 是像素坐标
//     lv_obj_set_style_bg_color(obj, lv_color_hex(0xFF0000), 0);     // 红色
//     lv_obj_set_style_bg_opa(obj, LV_OPA_50, 0);                    // 透明度50%
//     lv_obj_set_style_radius(obj, 50, 0);                           // 圆角半径
//     lv_obj_set_style_pad_all(obj,0,LV_PART_MAIN);                  //内边距    
//     lv_obj_set_style_border_width(obj,0,LV_PART_MAIN);              //边框宽度                         
//     lv_obj_set_style_outline_width(obj,0,LV_PART_MAIN);           //轮廓宽度  


//     lv_obj_t *obj1 = lv_obj_create(lv_scr_act());
//     lv_obj_set_size(obj1,200, 200);             // 设置宽度和高度
//     lv_obj_set_pos(obj1,400,50);                       // x, y 是像素坐标
//     lv_obj_set_style_bg_color(obj1, lv_color_hex(0x000000), 0);     // 白色
//     lv_obj_set_style_bg_opa(obj1, LV_OPA_50, 0);                    // 透明度50%
//     lv_obj_set_style_radius(obj1, 50, 0);                           // 圆角半径
//     lv_obj_set_style_pad_all(obj1,0,LV_PART_MAIN);                  //内边距
//     lv_obj_set_style_border_width(obj1,0,LV_PART_MAIN);             //边框宽度
//     lv_obj_set_style_outline_width(obj1,0,LV_PART_MAIN);            //轮廓宽度

//     //点击事件
//     lv_obj_t * obj2 = lv_obj_create(lv_scr_act());
//     lv_obj_set_size(obj2,100, 100);             // 设置宽度和高度
//     lv_obj_set_pos(obj2,800,50);                       // x, y 是像素坐标
//     lv_obj_add_flag(obj2,LV_OBJ_FLAG_CLICKABLE);
//     lv_obj_set_style_bg_color(obj2,lv_color_hex(0x000000),LV_PART_MAIN);
//     lv_obj_set_style_bg_color(obj2,lv_color_hex(0xFF0000),LV_PART_MAIN | LV_STATE_PRESSED);
   
//     //标签
//     lv_obj_t * label0 = lv_label_create(lv_scr_act());
//     lv_obj_set_pos(label0,800,150);                       // x, y 是像素坐标
//     lv_label_set_recolor(label0,true);//表示标签可以变换颜色
//     lv_label_set_text(label0,"#0000ff Hello# #ff00ff nihao#");
//     lv_obj_t * label1 = lv_label_create(lv_scr_act());
//     lv_obj_set_pos(label1,800,200);  
//     int data = 10;
//     lv_label_set_text_fmt(label1, "Value: %d", data);//显示变量

//     //标签汉字（自定义配置汉字）
//     lv_obj_t * label2 = lv_label_create(lv_scr_act());
//     lv_obj_set_pos(label2,1000,150);                       // x, y 是像素坐标
//    // lv_label_set_recolor(label0,true);//表示标签可以变换颜色
//    lv_obj_set_style_text_font(label2,&test_font,LV_PART_MAIN);//设置中文字体,lv_font_simsun_16_cjk->16像素常用汉字库（只能显示已保存的汉字）
//     lv_label_set_text(label2,"西江月");

    // //字库引擎使用（直接调用，但是占用空间大）
    // lv_font_t * font = get_font(FONT_TYPE_CN, 20);  
    // lv_obj_t * label3 = lv_label_create(lv_scr_act());
    // lv_obj_set_pos(label3, 100, 100);  // 调整到屏幕可见位置
    // lv_obj_set_style_text_font(label3, font, LV_PART_MAIN);
    // lv_label_set_text(label3, "西江月");

// //按钮的使用
//     lv_obj_t * btn = lv_btn_create(lv_scr_act());
//     lv_obj_set_size(btn,120,50);
//     lv_obj_set_pos(btn,200,200);
//     lv_obj_set_style_radius(btn,10,LV_PART_MAIN);//设置圆角半径
//     lv_obj_set_style_bg_opa(btn,LV_OPA_50,LV_PART_MAIN);//设置背景不透明
//     lv_obj_set_style_bg_color(btn,lv_color_hex(0x00FF00),LV_PART_MAIN);//设置背景颜色绿色
//     lv_obj_set_style_bg_color(btn,lv_color_hex(0x0000FF),LV_PART_MAIN | LV_STATE_PRESSED);//设置按下背景颜色蓝色

//     lv_obj_t * btn_label = lv_label_create(btn);
//     lv_label_set_text(btn_label,"Buttun");
//     lv_obj_center(btn_label);//标签居中

// //添加点击事件
//     lv_obj_add_flag(btn,LV_OBJ_FLAG_CLICKABLE);//设置对象可点击
//     lv_obj_add_event_cb(btn,btn_event_callback_func,LV_EVENT_ALL, NULL);


// //开关的使用
//     lv_obj_t * sw = lv_switch_create(lv_scr_act());
//     lv_obj_set_pos(sw,400,200);
//     lv_obj_set_size(sw,60,30);
//     lv_obj_set_style_bg_color(sw, lv_color_hex(0x00FF00), LV_PART_INDICATOR | LV_STATE_CHECKED); //开启状态背景颜色绿色

//     lv_obj_add_event_cb(sw,sw_event_callback_func,LV_EVENT_ALL,NULL);

    // /* ========== 文本框使用示例 ========== */
    // // 1. 创建文本框对象
    // lv_obj_t * ta = lv_textarea_create(lv_scr_act());  // 在当前活动屏幕上创建文本框
    
    // // 2. 设置文本框位置和大小
    // lv_obj_set_pos(ta, 500, 150);      // 设置位置：x=500, y=150
    // lv_obj_set_size(ta, 200, 50);      // 设置大小：宽200像素，高50像素
    
    // // 3. 设置文本框模式
    // lv_textarea_set_one_line(ta, true);  // 设置为单行模式（禁止换行）
    
    // // 4. 设置文本框样式
    // lv_obj_set_style_bg_color(ta, lv_color_hex(0xFFFFFF), LV_PART_MAIN);  // 背景颜色：白色
    // // lv_obj_set_style_border_color(ta, lv_color_hex(0x000000), LV_PART_MAIN);  // 边框颜色：黑色（已注释）
    // lv_obj_set_style_text_color(ta, lv_color_hex(0xFF0000), LV_PART_MAIN);  // 文字颜色：红色
    // lv_obj_set_style_border_color(ta, lv_color_hex(0x00FF00), LV_PART_CURSOR | LV_STATE_FOCUSED);  // 光标颜色：绿色（获得焦点时）
    
    // // 5. 设置文本框功能选项
    // // lv_textarea_set_password_mode(ta, true);  // 密码模式（显示为*号）（已注释）
    // lv_textarea_set_placeholder_text(ta, "please");  // 占位符文本（文本框为空时显示）
    
    // // 6. 为文本框添加事件监听
    // // 当用户按下回车键时触发 LV_EVENT_READY 事件，调用 ta_event_callback_func 函数
    // lv_obj_add_event_cb(ta, ta_event_callback_func, LV_EVENT_READY, NULL);

    // /* ========== 创建按钮用于读取文本框内容 ========== */
    // // 1. 创建按钮
    // lv_obj_t * test_btn = lv_btn_create(lv_scr_act());  // 在当前活动屏幕上创建按钮
    
    // // 2. 为按钮添加点击事件
    // // 参数说明：
    // //   - test_btn: 按钮对象
    // //   - btn_click_event_cb_func: 点击时调用的回调函数
    // //   - LV_EVENT_CLICKED: 监听点击事件
    // //   - ta: 用户数据，将文本框对象传递给回调函数（重要！）
    // lv_obj_add_event_cb(test_btn, btn_click_event_cb_func, LV_EVENT_CLICKED, ta);
    
    // // 3. 在按钮上创建标签显示文字
    // lv_obj_t * label = lv_label_create(test_btn);  // 创建标签作为按钮的子对象
    // lv_label_set_text(label, "Button");            // 设置按钮上的文字
    // lv_obj_center(label);                          // 将标签在按钮中居中显示
     
    // // 4. 设置按钮位置（相对文本框对齐）
    // // 参数说明：
    // //   - test_btn: 要对齐的按钮
    // //   - ta: 参考对象（文本框）
    // //   - LV_ALIGN_OUT_BOTTOM_MID: 在文本框外部底部中间位置
    // //   - 0, 0: x和y方向的偏移量（像素）
    // lv_obj_align_to(test_btn, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

// //键盘控件
//     //创建文本框
//     lv_obj_t * ta = lv_textarea_create(lv_scr_act());
//     lv_textarea_set_one_line(ta, true);
//     lv_obj_center(ta);

//     //创建键盘控件
//     lv_obj_t *kb = lv_keyboard_create(lv_scr_act());
//     //设置键盘大小
//     lv_obj_set_size(kb,  500, 280);
//     //绑定键盘和输入框
//     lv_keyboard_set_textarea(kb, ta);
//     //对齐，使得键盘在文本框右侧
//     lv_obj_align_to(kb,ta,LV_ALIGN_OUT_RIGHT_MID,0,0);
//
//图片显示
//方法一：此方法适合无文件系统环境，图片数据以数组形式存储在代码中
    lv_obj_t * img = lv_img_create(lv_scr_act());//创建图片对象
    lv_img_set_src(img, &favicon);//设置图片源（使用编译到代码中的图片数据）
    lv_obj_center(img);//图片居中

// //方法二：此方法适合有文件系统环境，图片以文件形式存储在文件系统中
//     lv_obj_t * img1 = lv_img_create(lv_scr_act());  // 创建图片对象
//     lv_obj_t * img2 = lv_img_create(lv_scr_act());  // 创建图片对象
//     lv_obj_t * img3 = lv_img_create(lv_scr_act());  // 创建图片对象
//     lv_obj_t * img_phone = lv_img_create(lv_scr_act());  // 创建图片对象
//     lv_obj_t * img_yun = lv_img_create(lv_scr_act());  // 创建图片对象
//     // 注意：LVGL文件系统驱动配置
//     // X86 Linux模拟器：使用 LV_USE_FS_STDIO，需要 "A:" 驱动器前缀
//     // lv_conf.h 中定义：LV_FS_STDIO_LETTER 'A'，LV_FS_STDIO_PATH ""
//     // 所以路径格式为：A:相对路径
//     lv_img_set_src(img1,"A:res/image/favicon.png");  // 设置图片源（注意A:前缀）
//     lv_obj_set_pos(img1, 300,100);                       // x, y 是像素坐标
//     lv_img_set_src(img2,"A:res/image/PINGMU.png");  // 设置图片源（注意A:前缀）
//     lv_obj_set_pos(img2, 500,0);                       // x, y 是像素坐标

//     //简化路径书写
//     lv_img_set_src(img3,GET_IMAGE_PATH("watch.png"));  // 设置图片源
//     lv_obj_set_pos(img3,10,15);

// //动画
//     //创建img对象
//     lv_obj_t *img_douhua = lv_img_create(lv_scr_act());
//     lv_img_set_src(img_douhua,GET_IMAGE_PATH("icon_loading1.png"));
//     lv_obj_center(img_douhua);
    
//     //创建动画
//     lv_anim_t anim;
//     lv_anim_init(&anim);
//     //绑定对象
//     lv_anim_set_var(&anim, img_douhua);
//     //设置动画值在0-3600变化
//     lv_anim_set_values(&anim, 0, 3600);
//     //设置动画时间为1500ms，也就是从0增加到3600时间为1500ms
//     lv_anim_set_time(&anim, 1500);
//     //设置回调函数
//     lv_anim_set_exec_cb(&anim, anim_cb);
//     //设置重复次数，LV_ANIM_REPEAT_INFINITE为一直重复
//     lv_anim_set_repeat_count(&anim,LV_ANIM_REPEAT_INFINITE);

// /*lv_anim_path_linear 线性动画
// lv_anim_path_step最后一步改变
// lv_anim_path_ease_in 开始时很慢
// lv_anim_path_ease_out 最后慢
// lv_anim_path_ease_in_out 开始和结束都很慢
// lv_anim_path_overshoot 超过结束值
// lv_anim_path_bounce 从最终值反弹一点（比如撞墙）*/

//     //设置动画算法，快进快出
//     lv_anim_set_path_cb(&anim,lv_anim_path_ease_in_out);
//     //最后启动动画
//     lv_anim_start(&anim);


// /* ========== 进度条示例 ========== */
//     // 1. 创建进度条对象
//     lv_obj_t * bar = lv_bar_create(lv_scr_act());  // 在当前活动屏幕上创建进度条
//     lv_obj_set_size(bar, 200, 20);                 // 设置进度条大小：宽200像素，高20像素
//     lv_obj_center(bar);                            // 将进度条在屏幕中心显示

//     // 2. 创建标签显示进度百分比
//     label_bar = lv_label_create(lv_scr_act());     // 创建全局标签对象（在回调函数中更新）
//     lv_obj_align_to(label_bar, bar, LV_ALIGN_OUT_RIGHT_MID, 10, 0);  // 标签在进度条右侧10像素处

//     // 3. 创建动画控制进度条
//     lv_anim_t a;                                   // 定义动画结构体
//     lv_anim_init(&a);                              // 初始化动画
//     lv_anim_set_var(&a, bar);                      // 绑定动画对象为进度条
//     lv_anim_set_values(&a, 0, 100);                // 设置动画值从0变化到100
//     lv_anim_set_time(&a, 3000);                    // 设置动画持续时间3000ms（3秒）
//     lv_anim_set_exec_cb(&a, set_temp);             // 设置动画执行回调函数（每帧调用）
//     lv_anim_set_repeat_count(&a, 1);               // 设置重复次数为1次（执行2次：初始+重复1次）
//     lv_anim_start(&a);                             // 启动动画

// /* ========== 滑动条示例 ========== */
//     // 1. 创建滑动条对象
//     lv_obj_t * slider = lv_slider_create(lv_scr_act());  // 在当前活动屏幕上创建滑动条
//     lv_obj_center(slider);  // 将滑动条在屏幕中心显示
    
//     // 2. 设置滑动条范围（必须在设置值之前设置范围）
//     lv_slider_set_range(slider, 0, 200);  // 设置滑动条范围：0-200

//     // 3. 设置滑动条初始值（在绑定事件和创建标签之前设置）
//     lv_slider_set_value(slider, 50, LV_ANIM_OFF);  // 设置滑动条当前值为50

//     // 4. 创建标签显示滑动条当前值
//     slider_label = lv_label_create(lv_scr_act());  // 创建全局标签对象（在回调函数中更新）
//     // 手动设置标签初始值，与滑动条当前值同步（避免依赖事件触发）
//     lv_label_set_text_fmt(slider_label, "%d%%", (int)lv_slider_get_value(slider));
//     lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // 设置位置
    
//     // 5. 为滑动条添加值变化事件监听
//     // 当滑动条的值发生改变时（用户拖动滑块），触发 LV_EVENT_VALUE_CHANGED 事件
//     // 调用 slider_event_cb 回调函数来更新标签显示
//     lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);


// /* ========== 圆弧进度条示例 ========== */
// /**
//  * @brief 初始化页面 - 创建带动画的圆弧进度条
//  * @note 圆弧会循环显示从0到100的进度动画，常用于加载指示器
//  */
//     // 1. 创建圆弧对象
//     lv_obj_t * arc = lv_arc_create(lv_scr_act());  // 在当前活动屏幕上创建圆弧
    
//     // 2. 设置圆弧旋转角度（起始位置）
//     // 270度表示从顶部（12点钟方向）开始，默认从右侧（3点钟方向）开始
//     lv_arc_set_rotation(arc, 270);
    
//     // 3. 设置圆弧背景角度范围
//     // 参数：起始角度0度，结束角度360度（完整的圆）
//     lv_arc_set_bg_angles(arc, 0, 360);
    
//     // 设置圆弧值的范围为0-360（对应0-360度）
//     lv_arc_set_range(arc, 0, 360);
    
//     // 4. 移除圆弧的旋钮样式（让圆弧看起来更简洁）
//     // 参数：NULL表示移除所有样式，LV_PART_KNOB表示旋钮部分
//    // lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   
    
//     // 5. 清除可点击标志（禁止用户交互，仅用于显示）
//     lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE); 
    
//     // 6. 将圆弧在屏幕中心显示
//     lv_obj_center(arc);

//     // 7. 创建标签显示圆弧当前进度
//     arc_label = lv_label_create(lv_scr_act());  // 创建全局标签对象（在回调函数中更新）
//     lv_label_set_text_fmt(arc_label, "%d%%", 0);  // 初始显示 "0%"
//     lv_obj_align_to(arc_label, arc, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);  // 标签在圆弧下方10像素处居中

//     // 8. 创建动画控制圆弧进度
//     lv_anim_t a_arc;                                   // 定义动画结构体
//     lv_anim_init(&a_arc);                              // 初始化动画
//     lv_anim_set_var(&a_arc, arc);                      // 绑定动画对象为圆弧
//     lv_anim_set_exec_cb(&a_arc, set_angle);            // 设置动画执行回调函数
//     lv_anim_set_time(&a_arc,1000);                    // 设置动画持续时间1000ms（1秒）
//     lv_anim_set_repeat_count(&a_arc, LV_ANIM_REPEAT_INFINITE);  // 设置无限循环重复
//     lv_anim_set_repeat_delay(&a_arc, 500);             // 设置每次重复之间的延迟500ms
//     lv_anim_set_values(&a_arc, 0, 150);              // 设置动画值从0变化到150（对应0-360度）
//     lv_anim_start(&a_arc);                             // 启动动画


// /* ========== 滚轮(Roller)示例 ========== */
// /**
//  * @brief 创建滚轮控件 - 用于从列表中选择选项
//  * @note 滚轮是一个可滚动的选项列表,常用于日期、时间选择等场景
//  */
//     // 1. 创建滚轮对象
//     lv_obj_t *roller1 = lv_roller_create(lv_scr_act());  // 在当前活动屏幕上创建滚轮
    
//     // 2. 设置滚轮选项内容
//     // 参数说明：
//     //   - roller1: 滚轮对象
//     //   - 选项字符串: 使用 \n 分隔每个选项
//     //   - LV_ROLLER_MODE_INFINITE: 无限循环模式（滚动到底部后可继续滚动到顶部）
//     //   - LV_ROLLER_MODE_NORMAL: 普通模式（滚动到底部后停止）
//     lv_roller_set_options(roller1,
//                         "Monday\n"
//                         "Tuesday\n"
//                         "Wednesday\n"
//                         "Thursday\n"
//                         "Friday\n"
//                         "Saturday\n"
//                         "Sunday",
//                         LV_ROLLER_MODE_INFINITE);  // 设置为无限循环模式
    
//     // 3. 将滚轮在屏幕中居中显示
//     lv_obj_center(roller1);

//     // 4. 设置可见行数（同时显示4行选项）
//     // 其他选项通过滚动可见，中间行为选中行
//     lv_roller_set_visible_row_count(roller1, 4);  
    
//     // 5. 设置默认选中项
//     // 参数说明：
//     //   - roller1: 滚轮对象（错误2: 变量名要一致）
//     //   - 0: 选中的行索引（0表示第一行Monday，错误3: Monday不是数字）
//     //   - LV_ANIM_ON: 带动画效果滚动到选中项
//     lv_roller_set_selected(roller1, 0, LV_ANIM_ON);  // 默认选中第一行(Monday)
    
//     /* ========== 样式设置 ========== */
//     // 6. 设置主部件(未选中项)的背景颜色为黑色
//     // LV_PART_MAIN 表示滚轮的主要部分（未选中的选项区域）
//     lv_obj_set_style_bg_color(roller1, lv_color_hex(0x000000), LV_PART_MAIN);
    
//     // 7. 设置主部件的文字颜色为白色
//     lv_obj_set_style_text_color(roller1, lv_color_hex(0xffffff), LV_PART_MAIN);
    
//     // 8. 设置主部件边框宽度为0（去除边框）
//     lv_obj_set_style_border_width(roller1, 0, LV_PART_MAIN);
    
//     // 9. 设置选中部件(当前选中项)的背景颜色为黑色
//     // LV_PART_SELECTED 表示当前选中行的样式
//     lv_obj_set_style_bg_color(roller1, lv_color_hex(0x000000), LV_PART_SELECTED);
    
//     // 10. 设置选中部件的文字颜色为白色
//     // 注意: 如果想让选中项更明显，可以设置不同颜色，如0x00FF00(绿色)
//     lv_obj_set_style_text_color(roller1, lv_color_hex(0xffffff), LV_PART_SELECTED);
    
//     // 11. 清除焦点状态（去除键盘焦点时的高亮效果）
//     // 这样滚轮在非触摸操作时不会显示焦点框
//     lv_obj_clear_state(roller1, LV_STATE_FOCUS_KEY);

//     // 12. 添加值改变事件监听
//     // 当用户滚动并选择新选项时，触发 LV_EVENT_VALUE_CHANGED 事件
//     // 调用 event_handler 回调函数来处理选中事件
//     lv_obj_add_event_cb(roller1, event_handler, LV_EVENT_VALUE_CHANGED, NULL);  // 变量名一致


// /* ========== 定时器(Timer)示例 ========== */
// /**
//  * @brief 创建定时器 - 周期性执行任务
//  * @note 定时器常用于定期更新UI、执行周期性任务等场景
//  *       LVGL定时器在主循环中自动调用，无需手动管理线程
//  */
//     // 1. 创建标签用于显示定时器计数
//     label_Timer = lv_label_create(lv_scr_act());  // 在当前活动屏幕上创建标签
    
//     // 2. 将标签在屏幕中居中显示
//     lv_obj_center(label_Timer);
    
//     // 3. 设置标签初始文本
//     lv_label_set_text(label_Timer, "Hello");
    
//     // 4. 创建定时器
//     // 参数说明：
//     //   - timer_cb_func: 定时器回调函数（每次触发时调用）
//     //   - 1000: 定时周期1000毫秒（1秒）
//     //   - NULL: 用户数据（传递给回调函数，这里不需要）
//     lv_timer_t * timer = lv_timer_create(timer_cb_func, 1000, NULL);

//     // 5. 设置定时器重复次数
//     // 参数说明：
//     //   - timer: 定时器对象
//     //   - 10: 执行10次后自动删除定时器
//     //   - 如果不设置此项，定时器默认无限重复执行
//     lv_timer_set_repeat_count(timer, 10);
    
//     // 6. 立即触发定时器（可选）
//     // 不等待第一个周期，立即执行一次回调函数
//     // 注意：这会导致定时器立即执行，通常在需要立即更新UI时使用
//     //lv_timer_ready(timer);
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


