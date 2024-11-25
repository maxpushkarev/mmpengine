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
		{ State::Idle, {State::Idle, State::Scheduling, State::Execution, State::Await}},
		{ State::Scheduling, {State::Scheduling, State::Execution} },
		{ State::Execution, {State::Await, State::Idle}},
		{ State::Await, { State::Complete } },
		{ State::Complete, {State::Idle} }
	};

	void BaseStream::RestartInternal()
	{
		if(!_scheduledTasks.empty())
		{
			throw std::logic_error("some pending tasks found on stream restart");
		}
	}

	void BaseStream::ScheduleInternal(const std::shared_ptr<BaseTask>& task)
	{
		_scheduledTasks.push(task);
	}

	void BaseStream::WaitInternal()
	{
		const auto thisPtr = shared_from_this();

		while(!_scheduledTasks.empty())
		{
			_scheduledTasks.front()->Finalize(thisPtr);
			_scheduledTasks.pop();
		}
	}

    void BaseStream::SubmitInternal()
    {
    }


    void BaseStream::Restart()
    {
		SwitchState(State::Idle);
		RestartInternal();
    }

	void BaseStream::Schedule(const std::shared_ptr<BaseTask>& task)
	{
		SwitchState(State::Scheduling);
		ScheduleInternal(task);
		task->Run(shared_from_this());
	}

	void BaseStream::Submit()
	{
		SwitchState(State::Execution);
		SubmitInternal();
	}

	void BaseStream::Wait()
	{
		SwitchState(State::Await);
		WaitInternal();
		SwitchState(State::Complete);
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
		const auto& nextStates = _validTransitionsMap.at(_sourceState);
		if(nextStates.find(targetState) == nextStates.cend())
		{
			throw InvalidStateException(_sourceState, targetState);
		}
		_sourceState = targetState;
	}

	std::ostream& operator<< (std::ostream& stream, BaseStream::State state)
	{
		switch (state)
		{
			case BaseStream::State::Await:
				stream << "Await";
				break;
			case BaseStream::State::Idle:
				stream << "Idle";
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

	BaseStream::Executor::Executor(const std::shared_ptr<BaseStream>& stream, bool waitAfterSubmit) : _waitAfterSubmit(waitAfterSubmit)
	{
		if(const auto s = _stream.lock())
		{
			s->Restart();
		}
	}

	BaseStream::Executor::~Executor()
	{
		if (const auto s = _stream.lock())
		{
			s->Submit();
			if(_waitAfterSubmit)
			{
				s->Wait();
			}
		}
	}

	BaseStream::InvalidStateException::InvalidStateException(State from, State to)
		: std::logic_error(Text::CombineToString("Unable to switch stream state from ", from, " to ", to))
	{
	}
}
