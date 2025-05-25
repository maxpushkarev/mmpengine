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
		friend class StreamBarrierTask;
		friend class StreamFlushTask;

	protected:

		enum class State : std::uint8_t
		{
			Initial,
			Start,
			Scheduling,
			Execution,
			Sync,
			Complete
		};

		friend std::ostream& operator<< (std::ostream& stream, State state);

		BaseStream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext);
		virtual ~BaseStream();

		virtual void RestartInternal();
		virtual void ScheduleInternal(const std::shared_ptr<BaseTask>& task);
		virtual void SyncInternal();
		virtual void SubmitInternal();
		virtual void Flush();

	public:
		
		class Executor final
		{
		public:
			Executor(const std::shared_ptr<BaseStream>& stream, bool waitAfterSubmit = true);
			Executor(const Executor&) = delete;
			Executor(Executor&&) noexcept = delete;
			Executor& operator=(const Executor&) = delete;
			Executor& operator=(Executor&&) noexcept = delete;
			~Executor();
		private:
			std::shared_ptr<BaseStream> _stream;
			bool _waitAfterSubmit;
		};

		BaseStream(const BaseStream&) = delete;
		BaseStream(BaseStream&&) noexcept = delete;
		BaseStream& operator=(const BaseStream&) = delete;
		BaseStream& operator=(BaseStream&&) noexcept = delete;

		std::uint64_t GetSyncCounterValue() const;
		std::uint64_t GetLastCompletedSyncCounterValue() const;
		virtual bool IsSyncCounterValueCompleted(std::uint64_t counterValue) const;

		void Restart();
		void Schedule(const std::shared_ptr<BaseTask>& task);
		void Submit();
		void Wait();
		void SubmitAndWait();

		std::shared_ptr<GlobalContext> GetGlobalContext() const;
		std::shared_ptr<StreamContext> GetStreamContext() const;
		Executor CreateExecutor(bool waitAfterSubmit = true);
	private:
		void SwitchState(State targetState);
		void WaitInternal();
		void FinalizeTasks();
	private:

		std::uint64_t _syncCounter = 0;
		std::uint64_t _lastCompletedSyncCounter = 0;

		std::queue<std::shared_ptr<BaseTask>> _scheduledTasks;
		std::queue<std::shared_ptr<BaseTask>> _finalizedTasks;
		State _currentState = State::Initial;
		std::shared_ptr<GlobalContext> _globalContext;
		std::shared_ptr<StreamContext> _streamContext;

		static std::unordered_map<State, std::unordered_set<State>> _validTransitionsMap;

		class InvalidStateException : public std::logic_error
		{
		public:
			InvalidStateException(State from, State to);
		};
	};

	template<typename TGlobalContext, typename TStreamContext>
	class Stream : public Core::BaseStream
	{
	public:
		Stream(const std::shared_ptr<TGlobalContext>& globalContext, const std::shared_ptr<TStreamContext>& streamContext);
	protected:
		std::shared_ptr<TGlobalContext> _specificGlobalContext;
		std::shared_ptr<TStreamContext> _specificStreamContext;
	};

	template<typename TGlobalContext, typename TStreamContext>
	class AppStreamContextCache
	{
		static_assert(std::is_base_of_v<GlobalContext, TGlobalContext>, "TGlobalContext must be derived from GlobalContext");
		static_assert(std::is_base_of_v<StreamContext, TStreamContext>, "TStreamContext must be derived from StreamContext");
	protected:
		void UpdateCache(const std::shared_ptr<BaseStream>& stream);
		std::shared_ptr<TGlobalContext> _specificGlobalContext;
		std::shared_ptr<TStreamContext> _specificStreamContext;
	};

	template<typename TGlobalContext, typename TStreamContext>
	Stream<TGlobalContext, TStreamContext>::Stream(const std::shared_ptr<TGlobalContext>& globalContext, const std::shared_ptr<TStreamContext>& streamContext)
		: Core::BaseStream(globalContext, streamContext), _specificGlobalContext(globalContext), _specificStreamContext(streamContext)
	{
	}

	template<typename TGlobalContext, typename TStreamContext>
	inline void AppStreamContextCache<TGlobalContext, TStreamContext>::UpdateCache(const std::shared_ptr<BaseStream>& stream)
	{
		_specificGlobalContext = std::static_pointer_cast<TGlobalContext>(stream->GetGlobalContext());
		_specificStreamContext = std::static_pointer_cast<TStreamContext>(stream->GetStreamContext());
	}
}
