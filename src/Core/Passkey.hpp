#pragma once
#include <type_traits>

namespace MMPEngine::Core
{
	template<typename T>
	class PasskeyRequest
	{
		friend T;
	private:
		explicit PasskeyRequest(const T*)
		{
		}
	};

	template<bool AllowInheritance, typename...Types>
	class PasskeyControl
	{
	private:
		template<typename TRequest>
		static constexpr bool IsRequestedTypeAllowed()
		{
			return false;
		}

		template<typename TRequest, typename TAllowedType, typename...OtherAllowedTypes>
		static constexpr bool IsRequestedTypeAllowed()
		{
			return (AllowInheritance ? std::is_base_of_v<TAllowedType, TRequest> : std::is_same_v<TAllowedType, TRequest>)
				|| IsRequestedTypeAllowed<TRequest, OtherAllowedTypes...>();
		}

	public:
		template<typename T, typename = std::enable_if_t<IsRequestedTypeAllowed<T, Types...>(), void>>
		PasskeyControl(const PasskeyRequest<T>&)
		{
		}
	};
}
