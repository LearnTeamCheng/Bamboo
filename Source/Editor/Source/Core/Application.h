#pragma once
namespace BambooEditor
{
    class Application
    {
    public:
        Application(/* args */);
        ~Application()=default;

        static Application& GetInstance();
    public:
        void Run();

    private:
        bool Initialize();
        void Update();
        void Renderer();
        
        void Shoudown();

    private:
        bool m_IsRunning;
    };

}
