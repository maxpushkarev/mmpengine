#pragma once
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace MMPEngine::Core
{
	class BaseTask;

	class BaseStream : std::enable_shared_from_this<BaseStream>
	{
	protected:

		enum class State : std::uint8_t
		{
			Idle,
			Execution,
			Await,
			Complete
		};

		friend std::ostream& operator<< (std::ostream& stream, State state);

		BaseStream();
		virtual ~BaseStream();

		virtual void RestartInternal();
		virtual void ScheduleInternal(const std::shared_ptr<BaseTask>& task);
		virtual void WaitInternal();

	public:
		
		class Executor final
		{
		public:
			Executor(const std::shared_ptr<BaseStream>& stream);
			Executor(const Executor&) = delete;
			Executor(Executor&&) noexcept = delete;
			Executor& operator=(const Executor&) = delete;
			Executor& operator=(Executor&&) noexcept = delete;
			~Executor();
		private:
			std::weak_ptr<BaseStream> _stream;
		};

		BaseStream(const BaseStream&) = delete;
		BaseStream(BaseStream&&) noexcept = delete;
		BaseStream& operator=(const BaseStream&) = delete;
		BaseStream& operator=(BaseStream&&) noexcept = delete;

		void Restart();
		void Schedule(const std::shared_ptr<BaseTask>& task);
		void Wait();
	private:
		void SwitchState(State targetState);
	private:
		std::queue<std::shared_ptr<BaseTask>> _scheduledTasks;
		State _state = State::Idle;
		static std::unordered_map<State, std::unordered_set<State>> _validTransitionsMap;
	};
}
