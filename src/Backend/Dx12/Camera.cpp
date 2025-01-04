#include <Backend/Dx12/Camera.hpp>

namespace MMPEngine::Backend::Dx12
{
	PerspectiveCamera::PerspectiveCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node) : Core::PerspectiveCamera(settings, node)
	{
	}

	std::shared_ptr<Core::BaseTask> PerspectiveCamera::CreateInitializationTask()
	{
		return Core::BaseTask::kEmpty;
	}

	void PerspectiveCamera::FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data)
	{
	}

	OrthographicCamera::OrthographicCamera(const Settings& settings, const std::shared_ptr<Core::Node>& node) : Core::OrthographicCamera(settings, node)
	{
	}

	std::shared_ptr<Core::BaseTask> OrthographicCamera::CreateInitializationTask()
	{
		return Core::BaseTask::kEmpty;
	}

	void OrthographicCamera::FillData(const std::shared_ptr<Core::GlobalContext>& globalContext, Data& data)
	{
	}
}