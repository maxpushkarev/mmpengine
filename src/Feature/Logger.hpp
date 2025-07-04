#pragma once
#include <cstdint>
#include <string>
#include <Core/Text.hpp>

namespace MMPEngine::Feature
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
	public:
		BaseLogger(const BaseLogger&) = delete;
		BaseLogger(BaseLogger&&) noexcept = delete;
		BaseLogger& operator=(const BaseLogger&) = delete;
		BaseLogger& operator=(BaseLogger&&) noexcept = delete;
		virtual ~BaseLogger();
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
		LogInternal(logType, Core::Text::CombineToString(std::forward<TArgs>(args)...).c_str());
	}

#ifdef MMPENGINE_WIN
	namespace Win
	{
		class OutputLogger : public BaseLogger
		{
		public:
			OutputLogger(std::string&&);
		protected:
			void LogInternal(const char*) const override;
		};
	}
#endif

#ifdef MMPENGINE_MAC
    namespace Mac
    {
        class OutputLogger : public BaseLogger
        {
        public:
            OutputLogger(std::string&&);
        protected:
            void LogInternal(const char*) const override;
        };
    }
#endif

}
