#pragma once
#include <Core/Context.hpp>
#include <Backend/Vulkan/Wrapper.hpp>
#include <Backend/Shared/Context.hpp>
#include <vulkan/vulkan.h>

namespace MMPEngine::Backend::Vulkan
{
	class GlobalContext : public Core::GlobalContext
	{
	public:
		GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m);
	public:
		std::shared_ptr<Wrapper::Instance> instance;
		std::shared_ptr<Wrapper::Device> device;
	};


	class StreamContext : public Shared::StreamContext<
		std::shared_ptr<Wrapper::Queue>,
		std::shared_ptr<Wrapper::CommandAllocator>,
		std::shared_ptr<Wrapper::CommandBuffer>,
		std::shared_ptr<Wrapper::Fence>>
	{
	public:
		StreamContext(
			const std::shared_ptr<Wrapper::Queue>& queue,
			const std::shared_ptr<Wrapper::CommandAllocator>& allocator,
			const std::shared_ptr<Wrapper::CommandBuffer>& cmdBuffer,
			const std::shared_ptr<Wrapper::Fence>& fence
		);
	};
}