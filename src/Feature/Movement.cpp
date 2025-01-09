#include <Feature/Movement.hpp>

namespace MMPEngine::Feature
{
	Movement::Controller::Controller(const std::shared_ptr<Core::GlobalContext>& globalContext, const IInputController* inputController, const std::shared_ptr<Core::Node>& node)
		: _globalContext(globalContext), _inputController(inputController), _node(node)
	{
	}
	Movement::Controller::~Controller() = default;

	void Movement::Controller::Update(std::float_t dt)
	{
	}

	Movement::FreeController::FreeController(const std::shared_ptr<Core::GlobalContext>& globalContext, const IInputController* inputController, const std::shared_ptr<Core::Node>& node)
		: Controller(globalContext, inputController, node)
	{
	}

}