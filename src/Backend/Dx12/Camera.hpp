#pragma once
#include <Core/Camera.hpp>
#include <Backend/Dx12/Task.hpp>

namespace MMPEngine::Backend::Dx12
{
	class Camera
	{
	protected:
		class CameraTaskContext final : Core::EntityTaskContext<Core::Camera>
		{
		};

		class InitTask final : public Task<CameraTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<CameraTaskContext>& ctx);
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	};

	class PerspectiveCamera final : public Core::PerspectiveCamera
	{
	public:
		PerspectiveCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	protected:
		void FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data) override;
	};

	class OrthographicCamera final : public Core::OrthographicCamera
	{
	public:
		OrthographicCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	protected:
		void FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data) override;
	};
}
