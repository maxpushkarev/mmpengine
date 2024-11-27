#pragma once
#include <mutex>
#include <string>
#include <Core/Task.hpp>

namespace MMPEngine::Core
{
	class BaseEntity
	{
	protected:
		BaseEntity(std::string_view name);
		BaseEntity();
		virtual ~BaseEntity();
	public:
		BaseEntity(const BaseEntity&) = delete;
		BaseEntity(BaseEntity&&) noexcept = delete;
		BaseEntity& operator=(const BaseEntity&) = delete;
		BaseEntity& operator=(BaseEntity&&) noexcept = delete;
		std::string_view GetName() const;
		std::uint64_t GetId() const;
	private:
		std::uint64_t _id;
		std::string _name;
		static std::mutex _mutex;
		static std::uint64_t _idCounter;
	};
}
