#include <stdexcept>
#include <Core/Stream.hpp>
#include <Core/Task.hpp>
#include <Core/Text.hpp>

namespace MMPEngine::Core
{
	BaseStream::BaseStream() = default;
	BaseStream::~BaseStream()
	{
		Wait();
	}

	std::unordered_map<BaseStream::State, std::unordered_set<BaseStream::State>> BaseStream::_validTransitionsMap {
		{ State::Idle, {State::Idle, State::Execution, State::Await}},
		{ State::Execution, {State::Await} },
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


    void BaseStream::Restart()
    {
		SwitchState(State::Idle);
		RestartInternal();
    }

	void BaseStream::Schedule(const std::shared_ptr<BaseTask>& task)
	{
		SwitchState(State::Execution);
		ScheduleInternal(task);
		task->Run(shared_from_this());
	}

	void BaseStream::Wait()
	{
		SwitchState(State::Await);
		WaitInternal();
		SwitchState(State::Complete);
	}

	void BaseStream::SwitchState(State targetState)
	{
		const auto& nextStates = _validTransitionsMap.at(_state);
		if(nextStates.find(targetState) == nextStates.cend())
		{
			throw std::logic_error(Text::CombineToString("Unable to switch stream state from ", _state, " to ", targetState));
		}
		_state = targetState;
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
			case BaseStream::State::Execution:
				stream << "Execution";
				break;
			case BaseStream::State::Complete:
				stream << "Complete";
				break;
		}
		return stream;
	}

	BaseStream::Executor::Executor(const std::shared_ptr<BaseStream>& stream)
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
			s->Wait();
		}
	}

}
