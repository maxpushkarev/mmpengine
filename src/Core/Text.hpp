#pragma once
#include <sstream>
#include <Core/Base.hpp>

namespace MMPEngine::Core
{
	class Text
	{
	private:
		Text();
	public:
		template<class... TArgs>
		static std::string CombineToString(TArgs&&... args);

		static std::string ToString(BackendType backend);

	private:
		template<class TArg, class... TArgs>
		static std::string CombineToStringInternal(std::stringstream& ss, TArg&& arg, TArgs&&... args);
		static std::string CombineToStringInternal(const std::stringstream& ss);

	};

	template<class ...TArgs>
	inline std::string Text::CombineToString(TArgs&&... args)
	{
		std::stringstream ss;
		return CombineToStringInternal(ss, std::forward<TArgs>(args)...);
	}

	template<class TArg, class ...TArgs>
	inline std::string Text::CombineToStringInternal(std::stringstream& ss, TArg&& arg, TArgs&&... args)
	{
		ss << arg;
		return CombineToStringInternal(ss, std::forward<TArgs>(args)...);
	}
}