namespace Bamboo
{
 
    class Singleton
    {
        public:
            static Singleton* Instance()
            {
                static Singleton instance;
                return &instance;
            }
    };
};