namespace Bamboo
{
    class Quaternion
    {
    public:
        union
        {
            struct 
            {
                float x, y, z, w;
            };
            
        };
    
        public:
            Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
            Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
            ~Quaternion() {}

            void FromAngleAxis(const Vector3& axis, float angle);
            void Normalize();
            Quaternion GetNormalized() const;

        public:
            static const Quaternion  Identity;
            static const Quaternion  Zero;
    };


    


}