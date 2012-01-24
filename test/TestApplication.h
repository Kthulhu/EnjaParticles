#ifndef TEST_APPLICATION_H
#define TEST_APPLICATION_H

namespace rtps
{
    public class TestApplication
    {
        public:
            TestApplication();
            ~TestApplication();
            void KeyboardCallback(unsigned char key, int x, int y);
            void RenderCallback();
            void DestroyCallback();
            void MouseCallback(int button, int state, int x, int y);
            void MouseMotionCallback(int x, int y);
            void ResizeWindowCallback(int w, int h);
            void TimerCallback(int ms);
            void ResetSimulations();
            void drawString(const char *str, int x, int y, float color[4], void *font);
        private:
            GLuint windowWidth,windowHeight;
            std::map<std::string,RTPS*> systems;
            std::map<std::string,ParticleEffect*> effects;
            std::map<std::string,ParticleShape*> pShapes;
            std::map<std::string,GLuint> meshVBOs;
            std::map<std::string,GLuint> meshIBOs;
            std::string renderType;
            CL* cli;
            bool paused;
            bool renderVelocity = false;
    }
}
#endif