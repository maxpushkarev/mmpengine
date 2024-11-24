#pragma once

namespace MMPEngine::Core
{
	class BaseTask
	{
	protected:
		BaseTask();
		virtual ~BaseTask();
	public:
		BaseTask(const BaseTask&) = delete;
		BaseTask(BaseTask&&) noexcept = default;
		BaseTask& operator=(const BaseTask&) = delete;
		BaseTask& operator=(BaseTask&&) noexcept = default;
	};
}