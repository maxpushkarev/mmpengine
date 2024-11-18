#pragma once
#include <cstdint>
#include <string>
#include <Core/Text.hpp>

namespace MMPEngine::Core
{
	enum class ELogType : std::uint8_t
	{
		Debug,
		Info,
		Warning,
		Error
	};

	class BaseLogger
	{
	protected:
		BaseLogger(std::string&&);
		virtual ~BaseLogger();
	public:
		BaseLogger(const BaseLogger&) = delete;
		BaseLogger(BaseLogger&&) noexcept = delete;
		BaseLogger& operator=(const BaseLogger&) = delete;
		BaseLogger& operator=(BaseLogger&&) noexcept = delete;
	protected:
		virtual void LogInternal(const char*) const = 0;
	public:
		template<class... TArgs>
		void Log(ELogType logType, TArgs&&... args) const;
	private:
		std::string _name;
		void LogInternal(ELogType, const std::string&) const;
	};

	template<class ...TArgs>
	inline void BaseLogger::Log(ELogType logType, TArgs && ...args) const
	{
		LogInternal(logType, Text::CombineToString(std::forward<TArgs>(args)...).c_str());
	}
}
