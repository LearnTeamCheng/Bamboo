namespace Bamboo {
    class ISystem {
        public:
            virtual void Update(float deltaTime) = 0;
            ~ISystem() = default;
    };
}