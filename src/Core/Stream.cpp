#include <stdexcept>
#include <Core/Stream.hpp>
#include <Core/Task.hpp>
#include <Core/Text.hpp>
#include <Core/Context.hpp>

namespace MMPEngine::Core
{
	BaseStream::BaseStream(const std::shared_ptr<AppContext>& appContext, const std::shared_ptr<StreamContext>& streamContext)
		: _appContext(appContext), _streamContext(streamContext)
	{
	}

	BaseStream::~BaseStream() = default;

	std::unordered_map<BaseStream::State, std::unordered_set<BaseStream::State>> BaseStream::_validTransitionsMap {
		{ State::Start, {State::Start, State::Scheduling, State::Execution, State::Sync}},
		{ State::Scheduling, {State::Scheduling, State::Execution} },
		{ State::Execution, {State::Sync, State::Start}},
		{ State::Sync, { State::Complete } },
		{ State::Complete, {State::Start} }
	};

	void BaseStream::RestartInternal()
	{
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


    void BaseStream::Restart()
    {
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
		if (_currentState == State::Start || _currentState == State::Complete)
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

	std::shared_ptr<AppContext> BaseStream::GetAppContext() const
	{
		return _appContext;
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
