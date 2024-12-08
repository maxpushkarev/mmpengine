#include <Core/Material.hpp>
#include <cassert>

namespace MMPEngine::Core
{
	BaseMaterial::Parameters::Parameters()
	{
		Build();
	};

	BaseMaterial::Parameters::~Parameters() = default;


	BaseMaterial::Parameters::Parameters(std::vector<Entry>&& entries) : _entries(std::move(entries))
	{
		Build();
	}

	BaseMaterial::Parameters::Parameters(const Parameters& other) : _entries(other._entries)
	{
		Build();	
	}

	BaseMaterial::Parameters::Parameters(Parameters&& other) noexcept
	{
		if(this != &other)
		{
			_entries = std::move(other._entries);
			other._viewMap.clear();

			Build();
		}
	}

	const std::vector<BaseMaterial::Parameters::Entry>& BaseMaterial::Parameters::GetAll() const
	{
		return _entries;
	}

	std::optional<BaseMaterial::Parameters::EntryView> BaseMaterial::Parameters::TryGet(std::string_view name) const
	{
		if(_viewMap.find(name) != _viewMap.cend())
		{
			return _viewMap.at(name);
		}

		return std::nullopt;
	}

	void BaseMaterial::Parameters::Build()
	{
		for (std::size_t i = 0; i < _entries.size(); ++i)
		{
			const auto& entry = _entries[i];
			const auto& name = entry.name;

			assert(_viewMap.find(name) == _viewMap.cend());

			EntryView entryView{};
			entryView.entryPtr = &entry;
			entryView.index = static_cast<decltype(entryView.index)>(i);

			_viewMap.emplace(std::make_pair(name, entryView));
		}
	}

}