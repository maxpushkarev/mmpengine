#include <cassert>
#include <Feature/Movement.hpp>

namespace MMPEngine::Feature
{
	Movement::Controller::Controller(const std::shared_ptr<Core::GlobalContext>& globalContext, const Input* input, const std::shared_ptr<Core::Node>& node)
		: _globalContext(globalContext), _input(input), _node(node)
	{
	}
	Movement::Controller::~Controller() = default;

	void Movement::Controller::Update(std::float_t dt)
	{
	}

	Movement::FreeController::FreeController(const Settings& settings, const std::shared_ptr<Core::GlobalContext>& globalContext, const Input* input, const std::shared_ptr<Core::Node>& node)
		: Controller(globalContext, input, node), _settings(settings)
	{
		assert(_node->GetParent() == nullptr);
		_initialRotation = _node->localTransform.rotation;
	}

	void Movement::FreeController::Update(std::float_t dt)
	{
		Controller::Update(dt);
		assert(_node->GetParent() == nullptr);

		auto& transform = _node->localTransform;

		Core::Vector4Float cameraPos {};
		Core::Vector4Float cameraFwd {};
		Core::Vector4Float cameraUp {};
		Core::Vector4Float cameraRight {};

		Core::Matrix4x4 l2w {};
		_globalContext->math->CalculateLocalToWorldSpaceMatrix(l2w, _node);
		_globalContext->math->GetColumn(cameraRight, 0, l2w);
		_globalContext->math->GetColumn(cameraUp, 1, l2w);
		_globalContext->math->GetColumn(cameraFwd, 2, l2w);
		_globalContext->math->GetColumn(cameraPos, 3, l2w);

		const auto movementDistance = dt * _settings.movementSpeed;


		if (_input->IsButtonPressed(KeyButton::A) || _input->IsButtonPressed(KeyButton::LeftArrow))
		{
			cameraPos.x -= cameraRight.x * movementDistance;
			cameraPos.y -= cameraRight.y * movementDistance;
			cameraPos.z -= cameraRight.z * movementDistance;
		}

		if (_input->IsButtonPressed(KeyButton::D) || _input->IsButtonPressed(KeyButton::RightArrow))
		{
			cameraPos.x += cameraRight.x * movementDistance;
			cameraPos.y += cameraRight.y * movementDistance;
			cameraPos.z += cameraRight.z * movementDistance;
		}

		if (_input->IsButtonPressed(KeyButton::W) || _input->IsButtonPressed(KeyButton::UpArrow))
		{
			cameraPos.x += cameraFwd.x * movementDistance;
			cameraPos.y += cameraFwd.y * movementDistance;
			cameraPos.z += cameraFwd.z * movementDistance;
		}

		if (_input->IsButtonPressed(KeyButton::S) || _input->IsButtonPressed(KeyButton::DownArrow))
		{
			cameraPos.x -= cameraFwd.x * movementDistance;
			cameraPos.y -= cameraFwd.y * movementDistance;
			cameraPos.z -= cameraFwd.z * movementDistance;
		}

		if (_input->IsButtonPressed(KeyButton::Q))
		{
			cameraPos.x += cameraUp.x * movementDistance;
			cameraPos.y += cameraUp.y * movementDistance;
			cameraPos.z += cameraUp.z * movementDistance;
		}

		if (_input->IsButtonPressed(KeyButton::E))
		{
			cameraPos.x -= cameraUp.x * movementDistance;
			cameraPos.y -= cameraUp.y * movementDistance;
			cameraPos.z -= cameraUp.z * movementDistance;
		}

		transform.position.x = cameraPos.x;
		transform.position.y = cameraPos.y;
		transform.position.z = cameraPos.z;



		if (_input->IsButtonPressed(MouseButton::Left))
		{
			const auto currPos = _input->GetMouseNormalizedPosition();
			const auto prevPos = _previousFrameMousePosition.value_or(currPos);

			Core::Vector2Float delta = {0.0f, 0.0f};
			delta.x = currPos.x - prevPos.x;
			delta.y = currPos.y - prevPos.y;

			_previousFrameMousePosition = currPos;

			_relativeAngles.x += delta.x * _settings.rotationSpeedCoeff;
			_relativeAngles.y += delta.y * _settings.rotationSpeedCoeff;

			Core::Quaternion q {};
			_globalContext->math->RotationFromEuler(q, { _relativeAngles.y, _relativeAngles.x, 0.0f });

			_globalContext->math->Multiply(transform.rotation, _initialRotation, q);
		}
		else
		{
			_previousFrameMousePosition = std::nullopt;
		}

	}
}