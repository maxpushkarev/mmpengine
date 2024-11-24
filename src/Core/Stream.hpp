#pragma once

namespace MMPEngine::Core
{
	class BaseStream
	{
	protected:
		BaseStream();
		virtual ~BaseStream();
	public:
		BaseStream(const BaseStream&) = delete;
		BaseStream(BaseStream&&) noexcept = delete;
		BaseStream& operator=(const BaseStream&) = delete;
		BaseStream& operator=(BaseStream&&) noexcept = delete;
	};
}