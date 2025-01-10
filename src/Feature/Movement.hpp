#pragma once
#include <optional>
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
			Controller(const std::shared_ptr<Core::GlobalContext>& globalContext, const Input* input, const std::shared_ptr<Core::Node>& node);
		public:
			Controller(const Controller&) = delete;
			Controller(Controller&&) noexcept = delete;
			Controller& operator=(const Controller&) = delete;
			Controller& operator=(Controller&&) noexcept = delete;
			virtual ~Controller();
			virtual void Update(std::float_t dt);
		protected:
			std::shared_ptr<Core::GlobalContext> _globalContext;
			const Input* _input;
			std::shared_ptr<Core::Node> _node;
		};

		class FreeController final : public Controller
		{
		public:
			struct Settings final
			{
				std::float_t movementSpeed = 10.0f;
				std::float_t rotationSpeedCoeff = 2.5f;
			};
			FreeController(const Settings& settings, const std::shared_ptr<Core::GlobalContext>& globalContext, const Input* input, const std::shared_ptr<Core::Node>& node);
			void Update(std::float_t dt) override;
		private:
			Settings _settings;
			std::optional<Core::Vector2Float> _previousFrameMousePosition;
			Core::Quaternion _initialRotation = Core::Math::kQuaternionIdentity;
			Core::Vector2Float _relativeAngles = {0.0f, 0.0f };
		};

	};
}