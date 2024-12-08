#pragma once
#include <optional>
#include <Core/Base.hpp>
#include <Core/Entity.hpp>

namespace MMPEngine::Core
{
	class BaseMaterial : public IInitializationTaskSource
	{
	public:
		class Parameters final
		{
		public:
			struct Entry final
			{
				std::string name;
				std::shared_ptr<BaseEntity> entity;
			};

			struct EntryView final
			{
				const Entry* entryPtr;
				std::uint32_t index;
			};

			Parameters(std::vector<Entry>&& entries);
			Parameters();
			~Parameters();

			Parameters(const Parameters&);
			Parameters(Parameters&&) noexcept;

			Parameters& operator=(const Parameters&) = delete;
			Parameters& operator=(Parameters&&) noexcept = delete;

			std::optional<EntryView> TryGet(std::string_view name) const;
			const std::vector<Entry>& GetAll() const;
		private:

			void Build();

			std::vector<Entry> _entries;
			std::unordered_map<std::string_view, EntryView> _viewMap;
		};
	};

	class MeshMaterial : public BaseMaterial
	{
	};

	class ComputeMaterial : public BaseMaterial
	{
	};
}
