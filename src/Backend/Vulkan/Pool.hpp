#pragma once
#include <Core/Pool.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class DescriptorPool final : public Core::Pool
	{
	protected:
		struct NativeSettings final
		{
		};
		struct Settings final
		{
			Core::Pool::Settings base;
			NativeSettings native;
		};
		class Block final : public Core::Pool::Block
		{
		public:
			Block(std::uint32_t size, const NativeSettings& nativeSettings);
		};
	public:

		class Handle final : public Core::Pool::Handle
		{
			friend class DescriptorPool;
		public:
			Handle();
		protected:
			Handle(const std::shared_ptr<DescriptorPool>& descHeap, const Entry& entry);
		private:
			std::weak_ptr<DescriptorPool> _descHeap;
		};

		DescriptorPool(const Settings& settings);
		Handle Allocate();
		std::unique_ptr<Core::Pool::Block> InstantiateBlock(std::uint32_t size) override;

	private:
		NativeSettings _nativeSettings;
	};
}