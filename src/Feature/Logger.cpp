#include <Feature/Logger.hpp>
#include <iostream>
#ifdef MMPENGINE_WIN
#include <Windows.h>
#endif

namespace MMPEngine::Feature
{
	BaseLogger::BaseLogger(std::string&& name)
	{
		_name = std::move(name);
	}

	BaseLogger::~BaseLogger() = default;

	void BaseLogger::LogInternal(ELogType type, const std::string& str) const
	{
		std::string res {_name};
		res += "...";

		switch (type)
		{
		case ELogType::Debug:
			res += "Debug: ";
			break;
		case ELogType::Info:
			res += "Info: ";
			break;
		case ELogType::Warning:
			res += "Warning: ";
			break;
		case ELogType::Error:
			res += "Error: ";
			break;
		default:
			break;
		}

		res += str;
		res += '\n';

		LogInternal(res.c_str());
	}

#ifdef MMPENGINE_WIN
	namespace Win
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

#ifdef MMPENGINE_MAC
    namespace Mac
    {
        OutputLogger::OutputLogger(std::string&& name) : BaseLogger(std::move(name))
        {
        }

        void OutputLogger::LogInternal(const char* log) const
        {
            std::cout << log << std::endl;
        }
    }
#endif
}
