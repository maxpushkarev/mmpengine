#include <Core/Node.hpp>
#include <cassert>
#include <queue>
#include <string>

namespace MMPEngine::Core
{
	Node::Node(const std::shared_ptr<AppContext>& appContext) :
		_appContext(appContext),
		_parent(nullptr),
		localTransform{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} }
	{
		static std::size_t counter = 0;
		_name = "Node" + std::to_string(++counter);
	}

	Node::Node(const std::shared_ptr<AppContext>& appContext, std::string_view name) :
		_appContext(appContext),
		_name({ name.cbegin(), name.cend() }),
		_parent(nullptr),
		localTransform{ {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f} }
	{
	}

	Node::Node(const Node& origin) :
		enable_shared_from_this(origin),
		_appContext(origin._appContext),
		_name({ origin.GetName().cbegin(), origin.GetName().cend() })
	{
		CloneSubtree(&origin, this);
	}

	void Node::SetParent(const std::shared_ptr<Node>& newParent)
	{
		const auto thisPtr = shared_from_this();

		if (_parent)
		{
			_parent->_children.erase(thisPtr);
		}

		if (newParent)
		{
			newParent->_children.emplace(thisPtr);
		}

		const auto oldParent = _parent;
		_parent = newParent;

		if (newParent)
		{
			newParent->ValidateSubtree();
		}

		if (oldParent)
		{
			oldParent->ValidateSubtree();
		}
	}

	void Node::AddChild(const std::shared_ptr<Node>& newChild)
	{
		const auto thisPtr = shared_from_this();

		if (newChild->_parent)
		{
			newChild->_parent->_children.erase(newChild);
		}
		_children.emplace(newChild);

		const auto oldParent = newChild->_parent;
		newChild->_parent = thisPtr;

		if (oldParent)
		{
			oldParent->ValidateSubtree();
		}

		thisPtr->ValidateSubtree();
	}

	std::shared_ptr<Node> Node::GetParent() const
	{
		return _parent;
	}

	const std::unordered_set<std::shared_ptr<Node>>& Node::GetChildren() const
	{
		return  _children;
	}

	std::string_view Node::GetName() const
	{
		return _name;
	}

	std::shared_ptr<Node> Node::FindNodeInSubtree(std::string_view name) const
	{
		if (_name == name)
		{
			return std::const_pointer_cast<Node>(shared_from_this());
		}

		for (auto& c : _children)
		{
			if (const auto node = c->FindNodeInSubtree(name))
			{
				return node;
			}
		}

		return nullptr;
	}

	void Node::ValidateSubtree() const
	{
		std::queue<std::shared_ptr<const Node>> bfs;
		std::unordered_set<std::shared_ptr<const Node>> visited;
		const auto sharedThis = shared_from_this();
		bfs.emplace(sharedThis);

		while (!bfs.empty())
		{
			const auto currentNode = bfs.front();
			bfs.pop();

			assert(visited.find(currentNode) == visited.cend());
			visited.emplace(currentNode);

			for (const auto& c : currentNode->GetChildren())
			{
				assert(c->GetParent() == currentNode);
				bfs.emplace(c);
			}
		}
	}
	void Node::CloneSubtree(const Node* origin, Node* target)
	{
		target->localTransform = origin->localTransform;
		target->SetParent(nullptr);

		for (const auto& originChild : origin->GetChildren())
		{
			const auto targetChild = std::make_shared<Node>(origin->_appContext, originChild->GetName());
			CloneSubtree(originChild.get(), targetChild.get());
			target->AddChild(targetChild);
		}
	}
}