#pragma once
namespace BambooEditor
{
    class Application
    {
    public:
        static Application& GetInstance();
    private:
        Application(/* args */);
        ~Application();
        bool Initialize();
        void Update();
        void Renderer();
        
        void Shoudown();
        void Run();

    private:
        bool m_IsRunning;
    };

}
