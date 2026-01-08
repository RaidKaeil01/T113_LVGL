/**
 * @file audio_player_async.h
 * @brief 异步音频播放器头文件
 * 
 * 提供基于线程和消息队列的异步音频播放功能，
 * 避免音频播放阻塞主线程，提升系统响应性能。
 * 
 * 工作原理：
 * 1. 使用独立线程处理音频播放任务
 * 2. 通过消息队列接收播放命令
 * 3. 支持异步启动、停止等操作
 * 
 * @date 2026年1月6日
 */

#ifndef _AUDIO_PLAY_ASYNC_H_
#define _AUDIO_PLAY_ASYNC_H_

/**
 * @brief 异步启动音频播放
 * 
 * 通过消息队列发送播放命令到音频处理线程，
 * 不会阻塞调用线程，立即返回。
 * 
 * @param url 音频文件路径（支持绝对路径和相对路径）
 *            例如："./res/music/audio_start.wav"
 *                 "/usr/res/music/audio_warn2.wav"
 * 
 * @note 如果当前有音频正在播放，会先停止再播放新音频
 * @note 最大路径长度为255字符
 */
void start_play_audio_async(const char *url);

/**
 * @brief 初始化异步音频播放器
 * 
 * 创建音频处理所需的消息队列和工作线程，
 * 必须在调用start_play_audio_async之前调用。
 * 
 * @return 0  初始化成功
 * @return -1 初始化失败（队列创建失败或线程创建失败）
 * 
 * @note 该函数只需调用一次，重复调用会创建多个线程和队列
 * @note 初始化失败时会自动清理已创建的资源
 */
int init_async_audio_player(void);

#endif