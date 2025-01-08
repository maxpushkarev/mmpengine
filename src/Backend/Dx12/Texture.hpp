#pragma once
#include <Core/Texture.hpp>
#include <Backend/Dx12/Entity.hpp>

namespace MMPEngine::Backend::Dx12
{
	class ITargetTexture
	{
	public:
		ITargetTexture();
		ITargetTexture(const ITargetTexture&) = delete;
		ITargetTexture(ITargetTexture&&) noexcept = delete;
		ITargetTexture& operator=(const ITargetTexture&) = delete;
		ITargetTexture& operator=(ITargetTexture&&) noexcept = delete;
		virtual ~ITargetTexture();
		virtual DXGI_SAMPLE_DESC GetSampleDesc() const = 0;
	};

	class IDepthStencilTexture : public ITargetTexture
	{
	public:
		virtual DXGI_FORMAT GetDSVFormat() const = 0;
	};

	class IColorTargetTexture : public ITargetTexture
	{
	public:
		virtual DXGI_FORMAT GetRTVFormat() const = 0;
	};

	class DepthStencilTargetTexture final : public Core::DepthStencilTargetTexture, public ResourceEntity, public IDepthStencilTexture
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
		const BaseDescriptorPool::Handle* GetDSVDescriptorHandle() const override;
		DXGI_FORMAT GetDSVFormat() const override;
		DXGI_SAMPLE_DESC GetSampleDesc() const override;
	private:
		BaseDescriptorPool::Handle _dsvHandle;

		DXGI_FORMAT GetResourceFormat() const;
	};
}