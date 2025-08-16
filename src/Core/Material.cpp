#include <Core/Material.hpp>
#include <cassert>
#include <memory>
#include <unordered_set>

namespace MMPEngine::Core
{
	StencilRef::StencilRef() = default;
	StencilRef::StencilRef(std::string_view name) : BaseEntity(name)
	{
	}

	bool RenderingMaterial::Settings::Blend::Target::operator==(const Target& rhs) const
	{
		return src == rhs.src && dst == rhs.dst && op == rhs.op;
	}

	bool RenderingMaterial::Settings::Blend::Target::operator!=(const Target& rhs) const
	{
		return !operator==(rhs);
	}


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
		std::unordered_set<std::string> knownTags {};

		std::uint32_t stencilRefCount = 0;

		for (std::size_t i = 0; i < _entries.size(); ++i)
		{
			const auto& entry = _entries[i];
			const auto& name = entry.name;
			const auto& tag = entry.tag;

			assert(!name.empty());
			assert(!tag.empty());
			assert(_viewMap.find(name) == _viewMap.cend());

			if (i > 0)
			{
				assert((_entries[i - 1].tag == tag) || (knownTags.find(tag) == knownTags.cend()));
			}

			knownTags.insert(tag);

			EntryView entryView{};
			entryView.entryPtr = &entry;
			entryView.index = static_cast<decltype(entryView.index)>(i);

			_viewMap[name] = entryView;

			if (std::holds_alternative<StencilRef>(entry.settings))
			{
				++stencilRefCount;
			}
		}

		assert(stencilRefCount <= 1);
	}

	BaseMaterial::BaseMaterial(Parameters&& params) : _params(std::move(params))
	{
	}

	BaseMaterial::~BaseMaterial() = default;

	const BaseMaterial::Parameters& BaseMaterial::GetParameters() const
	{
		return _params;
	}

	RenderingMaterial::RenderingMaterial(const Settings& settings, Parameters&& params) : BaseMaterial(std::move(params)), _settings(settings)
	{
	}

	const RenderingMaterial::Settings& RenderingMaterial::GetSettings() const
	{
		return _settings;
	}

	MeshMaterial::MeshMaterial(const Settings& settings, Parameters&& params, const std::shared_ptr<Shader>& vs, const std::shared_ptr<Shader>& ps)
		: RenderingMaterial(settings, std::move(params)), _vs(vs), _ps(ps)
	{
		assert(_vs->GetInfo().type == Core::Shader::Info::Type::Vertex);
		assert(_ps->GetInfo().type == Core::Shader::Info::Type::Pixel);
	}

	const std::shared_ptr<Shader>& MeshMaterial::GetPixelShader() const
	{
		return _ps;
	}

	const std::shared_ptr<Shader>& MeshMaterial::GetVertexShader() const
	{
		return _vs;
	}

	ComputeMaterial::ComputeMaterial(Parameters&& params, const std::shared_ptr<Shader>& computeShader) : BaseMaterial(std::move(params)), _shader(computeShader)
	{
		assert(_shader->GetInfo().type == Core::Shader::Info::Type::Compute);
	}

	const std::shared_ptr<Shader>& ComputeMaterial::GetShader() const
    {
        return _shader;
    }

}
