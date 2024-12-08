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

	BaseMaterial::Parameters& BaseMaterial::Parameters::operator=(Parameters&& other) noexcept
	{
		if (this != &other)
		{
			_entries = std::move(other._entries);

			other._viewMap.clear();
			_viewMap.clear();

			Build();
		}

		return *this;
	}

	BaseMaterial::Parameters& BaseMaterial::Parameters::operator=(const Parameters& other)
	{
		if (this != &other)
		{
			_entries = other._entries;
			_viewMap.clear();

			Build();
		}

		return *this;
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

	BaseMaterial::BaseMaterial(Parameters&& params) : _params(std::move(params))
	{
	}

	std::shared_ptr<BaseTask> BaseMaterial::CreateInitializationTask()
	{
		return UpdateParametersInternal();
	}

	std::shared_ptr<BaseTask> BaseMaterial::UpdateParameters(Parameters&& parameters)
	{
		_params = std::move(parameters);
		return UpdateParametersInternal();
	}

	RenderingMaterial::RenderingMaterial(const Settings& settings, Parameters&& params) : BaseMaterial(std::move(params)), _settings(settings)
	{
	}

	MeshMaterial::MeshMaterial(const Settings& settings, Parameters&& params) : RenderingMaterial(settings, std::move(params))
	{
	}

	ComputeMaterial::ComputeMaterial(Parameters&& params) : BaseMaterial(std::move(params))
	{
	}

}