#include "Logger.hpp"

namespace MMPEngine::Core
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
}