#pragma once
#include <Core/Logger.hpp>

#ifdef MMPENGINE_WIN
namespace MMPEngine::Frontend::Win
{
	class OutputLogger : public Core::BaseLogger
	{
	public:
		OutputLogger(std::string&&);
	protected:
		void LogInternal(const char*) const override;
	};
}
#endif
