#pragma once
#include <Core/Texture.hpp>
#include <Backend/Dx12/Entity.hpp>

namespace MMPEngine::Backend::Dx12
{
	class DepthStencilTargetTexture final : public Core::DepthStencilTargetTexture, public ResourceEntity
	{
	private:
		class InitTaskContext final : public Core::EntityTaskContext<DepthStencilTargetTexture>
		{
		};
		class InitTask final : public Task<InitTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	public:
		DepthStencilTargetTexture(const Settings& settings);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	private:
		DXGI_FORMAT GetResourceFormat() const;
		DXGI_FORMAT GetDSVFormat() const;
	};
}