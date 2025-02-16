#pragma once
#include <Core/Entity.hpp>
#include <Backend/Vulkan/Task.hpp>

namespace MMPEngine::Backend::Vulkan
{
	class DeviceMemoryBlock final : public Core::BaseEntity
	{
	public:
		struct Settings final
		{
			std::size_t byteSize;
		};

		DeviceMemoryBlock(const Settings&);
		std::shared_ptr<Core::BaseTask> CreateInitializationTask() override;
	private:
		Settings _settings;

		class InitTaskContext final : public Core::EntityTaskContext<DeviceMemoryBlock>
		{
		};

		class InitTask final : public Task<InitTaskContext>
		{
		public:
			InitTask(const std::shared_ptr<InitTaskContext>& ctx);
		protected:
			void Run(const std::shared_ptr<Core::BaseStream>& stream) override;
		};
	};
}
