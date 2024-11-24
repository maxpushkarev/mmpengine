#include <Core/Entity.hpp>
#include <Core/Text.hpp>

namespace MMPEngine::Core
{
	std::mutex BaseEntity::_mutex = {};
	std::uint64_t BaseEntity::_idCounter = 0;

	BaseEntity::BaseEntity(std::string_view name)
	{
		const std::lock_guard lock(_mutex);
		_id = ++_idCounter;
		_name = std::string {name};
	}

	BaseEntity::BaseEntity()
	{
		const std::lock_guard lock(_mutex);
		_id = ++_idCounter;
		_name = Text::CombineToString("Entity", _id);
	}

	BaseEntity::~BaseEntity() = default;

	std::string_view BaseEntity::GetName() const
	{
		return _name;
	}
	std::uint64_t BaseEntity::GetId() const
	{
		return _id;
	}
}
