#ifndef RBSPH_BENCHMARK_H
#define RBSPH_BENCHMARK_H
#include <map>
#include <string>
#include <iostream>

#include "../TestApplication.h"
namespace rtps
{
    class RBSPHBenchmark : public TestApplication
    {
        public:
            RBSPHBenchmark(std::istream& is,std::string path,GLuint w, GLuint h, unsigned int maxIterations=1000);
            ~RBSPHBenchmark();
            void KeyboardCallback(unsigned char key, int x, int y);
            void MouseCallback(int button, int state, int x, int y);
            void MouseMotionCallback(int x, int y);
            void TimerCallback(int ms);
            //void readParamFile(std::istream& is, std::string path);
	    virtual void initScenes();
	protected:
	    unsigned int maxIterations;
	    unsigned int iterations;
    };
};
#endif
