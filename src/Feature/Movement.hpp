#pragma once
#include <Feature/Input.hpp>
#include <Core/Node.hpp>
#include <Core/Context.hpp>

namespace MMPEngine::Feature
{
	class Movement
	{
	public:
		class Controller
		{
		protected:
			Controller(const std::shared_ptr<Core::GlobalContext>& globalContext, const IInputController* inputController, const std::shared_ptr<Core::Node>& node);
		public:
			Controller(const Controller&) = delete;
			Controller(Controller&&) noexcept = delete;
			Controller& operator=(const Controller&) = delete;
			Controller& operator=(Controller&&) noexcept = delete;
			virtual ~Controller();
			virtual void Update(std::float_t dt);
		protected:
			std::shared_ptr<Core::GlobalContext> _globalContext;
			const IInputController* _inputController;
			std::shared_ptr<Core::Node> _node;
		};

		class FreeController final : public Controller
		{
		public:
			FreeController(const std::shared_ptr<Core::GlobalContext>& globalContext, const IInputController* inputController, const std::shared_ptr<Core::Node>& node);
		};

	};
}