#pragma once
#include <memory>

namespace MMPEngine::Core
{
	class BaseStream;

	class BaseTask
	{
		friend class BaseStream;
	protected:
		BaseTask();
		virtual ~BaseTask();

		virtual void Run(const std::shared_ptr<BaseStream>& stream);
		virtual void Finalize(const std::shared_ptr<BaseStream>& stream);

	public:
		BaseTask(const BaseTask&) = delete;
		BaseTask(BaseTask&&) noexcept = default;
		BaseTask& operator=(const BaseTask&) = delete;
		BaseTask& operator=(BaseTask&&) noexcept = default;
	};
}
