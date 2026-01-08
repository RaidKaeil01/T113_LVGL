#ifndef _RES_CONF_H_
#define _RES_CONF_H_

#ifdef SIMULATOR_LINUX
    #define FONT_PATH "./res/font/"
    #define IMAGE_PATH "./res/image/"
    #define MUSIC_PATH "./build/app7/res/music/"
#else
    #define FONT_PATH "/usr/res/font/"
    #define IMAGE_PATH "/usr/res/image/"
    #define MUSIC_PATH "/usr/res/music/"
#endif

#endif 