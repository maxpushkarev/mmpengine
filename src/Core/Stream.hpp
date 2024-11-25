#pragma once
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <Core/Context.hpp>

namespace MMPEngine::Core
{
	class BaseTask;

	class BaseStream : public std::enable_shared_from_this<BaseStream>
	{
	protected:

		enum class State : std::uint8_t
		{
			Idle,
			Scheduling,
			Execution,
			Await,
			Complete
		};

		friend std::ostream& operator<< (std::ostream& stream, State state);

		BaseStream(const std::shared_ptr<AppContext>& appContext, const std::shared_ptr<StreamContext>& streamContext);
		virtual ~BaseStream();

		virtual void RestartInternal();
		virtual void ScheduleInternal(const std::shared_ptr<BaseTask>& task);
		virtual void WaitInternal();
		virtual void SubmitInternal();

	public:
		
		class Executor final
		{
		public:
			Executor(const std::shared_ptr<BaseStream>& stream, bool wait = true);
			Executor(const Executor&) = delete;
			Executor(Executor&&) noexcept = delete;
			Executor& operator=(const Executor&) = delete;
			Executor& operator=(Executor&&) noexcept = delete;
			~Executor();
		private:
			std::weak_ptr<BaseStream> _stream;
			bool _waitAfterSubmit;
		};

		BaseStream(const BaseStream&) = delete;
		BaseStream(BaseStream&&) noexcept = delete;
		BaseStream& operator=(const BaseStream&) = delete;
		BaseStream& operator=(BaseStream&&) noexcept = delete;

		void Restart();
		void Schedule(const std::shared_ptr<BaseTask>& task);
		void Submit();
		void Wait();
		void SubmitAndWait();

		std::shared_ptr<AppContext> GetAppContext() const;
		std::shared_ptr<StreamContext> GetStreamContext() const;
	private:
		void SwitchState(State targetState);
	private:
		std::queue<std::shared_ptr<BaseTask>> _scheduledTasks;
		State _sourceState = State::Idle;
		std::shared_ptr<AppContext> _appContext;
		std::shared_ptr<StreamContext> _streamContext;

		static std::unordered_map<State, std::unordered_set<State>> _validTransitionsMap;

		class InvalidStateException : public std::logic_error
		{
		public:
			InvalidStateException(State from, State to);
		};
	};

	template<typename TAppContext, typename TStreamContext>
	class Stream : public Core::BaseStream
	{
	public:
		Stream(const std::shared_ptr<TAppContext>& appContext, const std::shared_ptr<TStreamContext>& streamContext);
	protected:
		std::shared_ptr<TAppContext> _specificAppContext;
		std::shared_ptr<TStreamContext> _specificStreamContext;
	};

	template<typename TAppContext, typename TStreamContext>
	Stream<TAppContext, TStreamContext>::Stream(const std::shared_ptr<TAppContext>& appContext, const std::shared_ptr<TStreamContext>& streamContext)
		: Core::BaseStream(appContext, streamContext), _specificAppContext(appContext), _specificStreamContext(streamContext)
	{
	}
}
