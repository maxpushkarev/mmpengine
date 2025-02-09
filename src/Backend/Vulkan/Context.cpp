#include <Backend/Vulkan/Context.hpp>

namespace MMPEngine::Backend::Vulkan
{
	GlobalContext::GlobalContext(const Core::GlobalContext::Settings& s, std::unique_ptr<Core::Math>&& m) : Core::GlobalContext(s, std::move(m)),
		instance(nullptr), device(nullptr)
	{
	}

	StreamContext::StreamContext(
		const std::shared_ptr<Wrapper::Queue>& queue, 
		const std::shared_ptr<Wrapper::CommandAllocator>& allocator, 
		const std::shared_ptr<Wrapper::CommandBuffer>& cmdBuffer,
		const std::shared_ptr<Wrapper::Fence>& fence)
			: Shared::StreamContext<
				std::shared_ptr<MMPEngine::Backend::Vulkan::Wrapper::Queue>,
				std::shared_ptr<MMPEngine::Backend::Vulkan::Wrapper::CommandAllocator>,
				std::shared_ptr<MMPEngine::Backend::Vulkan::Wrapper::CommandBuffer>,
				std::shared_ptr<MMPEngine::Backend::Vulkan::Wrapper::Fence>>(queue, allocator, cmdBuffer, fence)
	{
	}
}