#pragma once
#include <Core/Pool.hpp>
#include <Backend/Vulkan/Wrapper.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class DescriptorPool final : public Core::Pool
	{
	public:
		struct NativeSettings final
		{
			VkDescriptorType type;
		};
		struct Settings final
		{
			Core::Pool::Settings base;
			NativeSettings native;
		};
	private:
		class Block final : public Core::Pool::Block
		{
		public:
			Block(const std::shared_ptr<Wrapper::Device>& device, std::uint32_t size, const NativeSettings& nativeSettings);
			~Block();
		private:
			VkDescriptorPool _nativeDescPool;
			std::shared_ptr<Wrapper::Device> _device;
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

		DescriptorPool(const std::shared_ptr<Wrapper::Device>& device, const Settings& settings);
		Handle Allocate();
		std::unique_ptr<Core::Pool::Block> InstantiateBlock(std::uint32_t size) override;

	private:
		NativeSettings _nativeSettings;
		std::shared_ptr<Wrapper::Device> _device;
	};
}