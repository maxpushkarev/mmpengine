#include <stdexcept>
#include <Core/Stream.hpp>
#include <Core/Task.hpp>
#include <Core/Text.hpp>
#include <Core/Context.hpp>
#include <cassert>

namespace MMPEngine::Core
{
	BaseStream::BaseStream(const std::shared_ptr<GlobalContext>& globalContext, const std::shared_ptr<StreamContext>& streamContext)
		: _globalContext(globalContext), _streamContext(streamContext)
	{
	}

	BaseStream::~BaseStream() = default;

	BaseStream::State BaseStream::GetCurrentState() const
	{
		return _currentState;
	}


	std::unordered_map<BaseStream::State, std::unordered_set<BaseStream::State>> BaseStream::_validTransitionsMap {
		{ State::Initial, {State::Start}},
		{ State::Start, {State::Scheduling, State::Execution, State::Sync}},
		{ State::Scheduling, {State::Scheduling, State::Execution} },
		{ State::Execution, {State::Sync, State::Start}},
		{ State::Sync, { State::Complete } },
		{ State::Complete, {State::Start} }
	};

	void BaseStream::RestartInternal()
	{
		++_syncCounter;
	}

	void BaseStream::ScheduleInternal(const std::shared_ptr<BaseTask>& task)
	{
	}

	void BaseStream::SyncInternal()
	{
	}

    void BaseStream::SubmitInternal()
    {
    }

	std::uint64_t BaseStream::GetSyncCounterValue() const
	{
		return _syncCounter;
	}

	std::uint64_t BaseStream::GetLastCompletedSyncCounterValue() const
	{
		return _lastCompletedSyncCounter;
	}

	bool BaseStream::IsSyncCounterValueCompleted(std::uint64_t counterValue) const
	{
		return GetLastCompletedSyncCounterValue() >= counterValue;
	}

    void BaseStream::Restart()
    {
		if (_currentState == State::Start)
		{
			return;
		}

		assert(_finalizedTasks.empty());
		assert(_scheduledTasks.empty());

		SwitchState(State::Start);
		RestartInternal();
    }

	void BaseStream::Schedule(const std::shared_ptr<BaseTask>& task)
	{
		SwitchState(State::Scheduling);
		_scheduledTasks.push(task);
		ScheduleInternal(task);
		task->OnScheduled(shared_from_this());
	}

	void BaseStream::Submit()
	{
		if (_currentState == State::Complete)
		{
			return;
		}

		SwitchState(State::Execution);

		const auto thisPtr = shared_from_this();
		while (!_scheduledTasks.empty())
		{
			const auto currentTask = _scheduledTasks.front();
			_scheduledTasks.pop();

			_finalizedTasks.push(currentTask);
			currentTask->Run(thisPtr);
		}

		SubmitInternal();
	}

	void BaseStream::Wait()
	{
		if(_currentState == State::Sync || _currentState == State::Complete || _currentState == State::Start)
		{
			return;
		}

		SwitchState(State::Sync);
		WaitInternal();
		SwitchState(State::Complete);
		_lastCompletedSyncCounter = _syncCounter;
	}

	void BaseStream::WaitInternal()
	{
		SyncInternal();
		FinalizeTasks();
	}

	void BaseStream::FinalizeTasks()
	{
		const auto thisPtr = shared_from_this();
		while (!_finalizedTasks.empty())
		{
			_finalizedTasks.front()->OnComplete(thisPtr);
			_finalizedTasks.pop();
		}
	}


	void BaseStream::SubmitAndWait()
	{
		Submit();
		Wait();
	}

	std::shared_ptr<GlobalContext> BaseStream::GetGlobalContext() const
	{
		return _globalContext;
	}

	std::shared_ptr<StreamContext> BaseStream::GetStreamContext() const
	{
		return _streamContext;
	}

	BaseStream::Executor BaseStream::CreateExecutor(bool waitAfterSubmit)
	{
		return Executor{shared_from_this(), waitAfterSubmit};
	}

	void BaseStream::SwitchState(State targetState)
	{
		const auto& nextStates = _validTransitionsMap.at(_currentState);
		if(nextStates.find(targetState) == nextStates.cend())
		{
			throw InvalidStateException(_currentState, targetState);
		}
		_currentState = targetState;
	}

	std::ostream& operator<< (std::ostream& stream, BaseStream::State state)
	{
		switch (state)
		{
			case BaseStream::State::Initial:
				stream << "Initial";
				break;
			case BaseStream::State::Sync:
				stream << "Sync";
				break;
			case BaseStream::State::Start:
				stream << "Start";
				break;
			case BaseStream::State::Scheduling:
				stream << "Scheduling";
				break;
			case BaseStream::State::Execution:
				stream << "Execution";
				break;
			case BaseStream::State::Complete:
				stream << "Complete";
				break;
		}
		return stream;
	}

	BaseStream::Executor::Executor(const std::shared_ptr<BaseStream>& stream, bool waitAfterSubmit) : _stream(stream), _waitAfterSubmit(waitAfterSubmit)
	{
		_stream->Restart();
	}

	BaseStream::Executor::~Executor()
	{
		_stream->Submit();
		if(_waitAfterSubmit)
		{
			_stream->Wait();
		}
	}

	BaseStream::InvalidStateException::InvalidStateException(State from, State to)
		: std::logic_error(Text::CombineToString("Unable to switch stream state from ", from, " to ", to))
	{
	}
}
