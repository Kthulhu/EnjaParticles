/* 
 * File:   debug.h
 * Author: andrew
 *
 * Created on January 31, 2012, 1:18 PM
 */

#ifndef DEBUG_H
#define	DEBUG_H
#include <iostream>
#include <string.h>
#ifndef dout 
        //#ifdef DEBUG
            #define FLE (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__ )
            #define dout std::cout<<"\033[35m"<<FLE<<": \033[34m"<<__LINE__<<": \033[33m"<<__func__<<": \033[0m"
        /*#else
            #define dout 0 && std::cout
        #endif*/
    #endif

#endif	/* DEBUG_H */

