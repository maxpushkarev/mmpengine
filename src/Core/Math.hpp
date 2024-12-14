#pragma once
#include <Core/Base.hpp>

namespace MMPEngine::Core
{
	class Math
	{
	protected:
		Math();
	public:
		Math(const Math&) = delete;
		Math(Math&&) noexcept = delete;
		Math& operator=(const Math&) = delete;
		Math& operator=(Math&&) noexcept = delete;
		virtual ~Math();

		static constexpr auto kPi = 3.1415926535f;

		static constexpr Matrix4x4 kMatrix4x4Identity = {
			{
		{1.0f, 0.0f, 0.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f, 1.0f}
			}
		};

		inline static std::float_t ConvertDegreesToRadians(std::float_t degrees);
	private:
		static constexpr auto _deg2Rad = kPi / 180.0f;
	};

	class DefaultMath final : public Math
	{
	};
}