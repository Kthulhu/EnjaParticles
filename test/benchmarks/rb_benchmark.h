#ifndef RB_BENCHMARK_H
#define RB_BENCHMARK_H
#include <map>
#include <string>
#include <iostream>

#include "../TestApplication.h"
namespace rtps
{
    class RB_Benchmark : public TestApplication
    {
        public:
            RB_Benchmark(std::istream& is,std::string path,GLuint w, GLuint h);
            ~RB_Benchmark();
            void KeyboardCallback(unsigned char key, int x, int y);
            void MouseCallback(int button, int state, int x, int y);
            void MouseMotionCallback(int x, int y);
            void TimerCallback(int ms);
            void readParamFile(std::istream& is, std::string path);
	    virtual void initScenes();
    };
};
#endif
