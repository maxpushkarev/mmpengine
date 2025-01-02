#include <Backend/Dx12/Texture.hpp>

namespace MMPEngine::Backend::Dx12
{
	DepthStencilTargetTexture::DepthStencilTargetTexture(const Settings& settings) : Core::DepthStencilTargetTexture(settings)
	{
	}

	std::shared_ptr<Core::BaseTask> DepthStencilTargetTexture::CreateInitializationTask()
	{
		const auto ctx = std::make_shared<InitTaskContext>();
		ctx->entity = std::dynamic_pointer_cast<DepthStencilTargetTexture>(shared_from_this());
		return std::make_shared<InitTask>(ctx);
	}

	DepthStencilTargetTexture::InitTask::InitTask(const std::shared_ptr<InitTaskContext>& ctx) : Task(ctx)
	{
	}

	void DepthStencilTargetTexture::InitTask::Run(const std::shared_ptr<Core::BaseStream>& stream)
	{
		Task::Run(stream);

		const auto dsTex = GetTaskContext()->entity;
		const auto gc = _specificGlobalContext;


	}

}