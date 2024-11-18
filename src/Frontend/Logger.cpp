#include <Frontend/Logger.hpp>
#ifdef MMPENGINE_WIN
#include <Windows.h>
#endif

#ifdef MMPENGINE_WIN
namespace MMPEngine::Frontend::Win
{
	OutputLogger::OutputLogger(std::string&& name) : BaseLogger(std::move(name))
	{
	}

	void OutputLogger::LogInternal(const char* log) const
	{
		OutputDebugString(log);
	}
}
#endif
