#pragma once
#include <cstdint>
#include <unordered_set>
#include <Core/Base.hpp>

namespace MMPEngine::Core
{
	enum class MouseButton : std::uint8_t
	{
		Left = 0,
		Middle = 1,
		Right = 2
	};

	enum class KeyButton : std::uint16_t
	{
		W,
		A,
		S,
		D,
		LeftArrow,
		RightArrow,
		UpArrow,
		DownArrow,
		Q,
		E,
		R,
		F,
		PageUp,
		PageDown,
		LeftShift,
		RightShift,
		Enter,
		Space
	};


	class AppInputController;

	class IInputController
	{
	protected:
		IInputController();
	public:
		IInputController(const IInputController&) = delete;
		IInputController(IInputController&&) noexcept = delete;
		IInputController& operator=(const IInputController&) = delete;
		IInputController& operator=(IInputController&&) noexcept = delete;
		virtual ~IInputController();
	protected:
		virtual void ClearEvents() = 0;
		virtual void ClearAll() = 0;
		virtual void UpdateMouseNormalizedPosition(const Vector2Float&) = 0;

		virtual void SetButtonPressedStatus(KeyButton, bool) = 0;
		virtual void SetButtonPressedStatus(MouseButton, bool) = 0;
	};

	class Input final : public IInputController
	{
		friend class AppInputController;
	public:
		template<class TButton>
		bool IsButtonPressed(TButton) const;
		template<class TButton>
		bool IsButtonDown(TButton) const;
		template<class TButton>
		bool IsButtonUp(TButton) const;

		Vector2Float GetMouseNormalizedPosition() const;

	protected:
		void ClearEvents() override;
		void ClearAll() override;
		void UpdateMouseNormalizedPosition(const Vector2Float&) override;

		void SetButtonPressedStatus(KeyButton, bool) override;
		void SetButtonPressedStatus(MouseButton, bool) override;
	private:
		template<class TButton>
		void SetButtonPressedStatusInternal(TButton, bool);

		Vector2Float _currentFrameMousePosition;

		std::unordered_set<KeyButton> _pressedKeyButtons;
		std::unordered_set<KeyButton> _keyDownEvents;
		std::unordered_set<KeyButton> _keyUpEvents;

		std::unordered_set<MouseButton> _pressedMouseButtons;
		std::unordered_set<MouseButton> _mouseBtnDownEvents;
		std::unordered_set<MouseButton> _mouseBtnUpEvents;

		template<class TButton>
		std::enable_if_t<std::is_same_v<KeyButton, TButton>, const std::unordered_set<TButton>*> GetPressedMap() const;
		template<class TButton>
		std::enable_if_t<std::is_same_v<MouseButton, TButton>, const std::unordered_set<TButton>*> GetPressedMap() const;

		template<class TButton>
		std::enable_if_t<std::is_same_v<KeyButton, TButton>, const std::unordered_set<TButton>*> GetDownMap() const;
		template<class TButton>
		std::enable_if_t<std::is_same_v<MouseButton, TButton>, const std::unordered_set<TButton>*> GetDownMap() const;

		template<class TButton>
		std::enable_if_t<std::is_same_v<KeyButton, TButton>, const std::unordered_set<TButton>*> GetUpMap() const;
		template<class TButton>
		std::enable_if_t<std::is_same_v<MouseButton, TButton>, const std::unordered_set<TButton>*> GetUpMap() const;
	};


	template<class TButton>
	inline void Input::SetButtonPressedStatusInternal(TButton btn, bool value)
	{
		std::unordered_set<TButton>* pressed = nullptr;
		std::unordered_set<TButton>* down = nullptr;
		std::unordered_set<TButton>* up = nullptr;

		if constexpr (std::is_same_v<TButton, KeyButton>)
		{
			pressed = &_pressedKeyButtons;
			down = &_keyDownEvents;
			up = &_keyUpEvents;
		}

		if constexpr (std::is_same_v<TButton, MouseButton>)
		{
			pressed = &_pressedMouseButtons;
			down = &_mouseBtnDownEvents;
			up = &_mouseBtnUpEvents;
		}

		const auto isPressedCurrently = pressed->find(btn) != pressed->cend();

		if (value != isPressedCurrently)
		{
			(value ? down : up)->emplace(btn);
		}

		if (value)
		{
			pressed->emplace(btn);
		}
		else
		{
			pressed->erase(btn);
		}
	}

	template<class TButton>
	inline std::enable_if_t<std::is_same_v<MouseButton, TButton>, const std::unordered_set<TButton>*> Input::GetPressedMap() const
	{
		return &_pressedMouseButtons;
	}

	template<class TButton>
	inline std::enable_if_t<std::is_same_v<KeyButton, TButton>, const std::unordered_set<TButton>*> Input::GetPressedMap() const
	{
		return &_pressedKeyButtons;
	}

	template<class TButton>
	inline std::enable_if_t<std::is_same_v<MouseButton, TButton>, const std::unordered_set<TButton>*> Input::GetDownMap() const
	{
		return &_mouseBtnDownEvents;
	}

	template<class TButton>
	inline std::enable_if_t<std::is_same_v<KeyButton, TButton>, const std::unordered_set<TButton>*> Input::GetDownMap() const
	{
		return &_keyDownEvents;
	}

	template<class TButton>
	inline std::enable_if_t<std::is_same_v<MouseButton, TButton>, const std::unordered_set<TButton>*> Input::GetUpMap() const
	{
		return &_mouseBtnUpEvents;
	}

	template<class TButton>
	inline std::enable_if_t<std::is_same_v<KeyButton, TButton>, const std::unordered_set<TButton>*> Input::GetUpMap() const
	{
		return &_keyUpEvents;
	}

	template<class TButton>
	inline bool Input::IsButtonPressed(TButton btn) const
	{
		return GetPressedMap<TButton>()->find(btn) != GetPressedMap<TButton>()->cend();
	}

	template<class TButton>
	inline bool Input::IsButtonDown(TButton btn) const
	{
		return GetDownMap<TButton>()->find(btn) != GetDownMap<TButton>()->cend();
	}

	template<class TButton>
	inline bool Input::IsButtonUp(TButton btn) const
	{
		return GetUpMap<TButton>()->find(btn) != GetUpMap<TButton>()->cend();
	}
}