#include <vector>
#include <memory>

namespace Bamboo
{
    class Component;

    /// @brief 节点类
    class Node
    {
    public:
        void AddChild(Node *child);
        void RemoveChild(Node *child);

        std::vector<std::unique_ptr<Node *>> &GetChildren() { return m_children; }

    private:
        std::vector<std::unique_ptr<Node *>> m_children;
    };

};