#include <Feature/Input.hpp>

namespace MMPEngine::Feature
{
	IInputController::IInputController() = default;
	IInputController::~IInputController() = default;

	void Input::ClearInstantEvents()
	{
		_keyUpEvents.clear();
		_keyDownEvents.clear();

		_mouseBtnDownEvents.clear();
		_mouseBtnUpEvents.clear();
	}

	void Input::ClearAll()
	{
		_pressedKeyButtons.clear();
		_pressedMouseButtons.clear();

		ClearInstantEvents();
	}

	void Input::UpdateMouseNormalizedPosition(const Core::Vector2Float& newPosition)
	{
		_currentFrameMousePosition = newPosition;
	}

	void Input::SetButtonPressedStatus(KeyButton btn, bool status)
	{
		SetButtonPressedStatusInternal(btn, status);
	}

	void Input::SetButtonPressedStatus(MouseButton btn, bool status)
	{
		SetButtonPressedStatusInternal(btn, status);
	}

	Core::Vector2Float Input::GetMouseNormalizedPosition() const
	{
		return  _currentFrameMousePosition;
	}
}