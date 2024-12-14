#pragma once
#include <memory>
#include <unordered_set>
#include <Core/Base.hpp>
#include <Core/Context.hpp>

namespace MMPEngine::Core
{
	class Node final : public std::enable_shared_from_this<Node>
	{
	public:
		Node(const std::shared_ptr<AppContext>& appContext);
		Node(const std::shared_ptr<AppContext>& appContext, std::string_view);
		Node(const Node&);
		Node(Node&&) noexcept = delete;
		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) noexcept = delete;
		~Node() = default;

		void SetParent(const std::shared_ptr<Node>&);
		void AddChild(const std::shared_ptr<Node>&);
		std::shared_ptr<Node> GetParent() const;
		const std::unordered_set<std::shared_ptr<Node>>& GetChildren() const;
		std::string_view GetName() const;

		std::shared_ptr<Node> FindNodeInSubtree(std::string_view) const;
	private:
		std::shared_ptr<AppContext> _appContext;
		std::string _name;
		std::shared_ptr<Node> _parent;
		std::unordered_set<std::shared_ptr<Node>> _children;
		void ValidateSubtree() const;
		static void CloneSubtree(const Node*, Node*);
	public:
		Transform localTransform;
	};
}
