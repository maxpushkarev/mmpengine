#include <Core/Input.hpp>

namespace MMPEngine::Core
{
	IInputController::IInputController() = default;
	IInputController::~IInputController() = default;

	void Input::ClearEvents()
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

		ClearEvents();
	}

	void Input::UpdateMouseNormalizedPosition(const Vector2Float& newPosition)
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

	Vector2Float Input::GetMouseNormalizedPosition() const
	{
		return  _currentFrameMousePosition;
	}
}