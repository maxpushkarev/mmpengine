#pragma once
#include <optional>
#include <Core/Base.hpp>
#include <Core/Entity.hpp>
#include <Core/Shader.hpp>

namespace MMPEngine::Core
{
	class BaseMaterial : public IInitializationTaskSource, public std::enable_shared_from_this<BaseMaterial>
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

			explicit Parameters(std::vector<Entry>&& entries);
			Parameters();
			~Parameters();

			Parameters(const Parameters&);
			Parameters(Parameters&&) noexcept;

			Parameters& operator=(const Parameters&);
			Parameters& operator=(Parameters&&) noexcept;

			std::optional<EntryView> TryGet(std::string_view name) const;
			const std::vector<Entry>& GetAll() const;


		private:

			void Build();

			std::vector<Entry> _entries;
			std::unordered_map<std::string_view, EntryView> _viewMap;
		};

		std::shared_ptr<BaseTask> CreateInitializationTask() override;
		std::shared_ptr<BaseTask> CreateTaskForUpdateParameters(Parameters&& parameters);
		virtual std::shared_ptr<BaseTask> CreateTaskForApply() = 0;
		const Parameters& GetParameters() const;
	protected:

		BaseMaterial();
		virtual std::shared_ptr<BaseTask> CreateTaskForUpdateParametersInternal() = 0;
	protected:
		Parameters _params;
	};

	class RenderingMaterial : public BaseMaterial
	{
	public:
		struct Settings
		{
		};
	protected:
		RenderingMaterial(const Settings& settings);
		Settings _settings;
	};

	class MeshMaterial : public RenderingMaterial
	{
	protected:
		MeshMaterial(const Settings& settings);
	};

	class ComputeMaterial : public BaseMaterial
	{
	protected:
		ComputeMaterial(const std::shared_ptr<ComputeShader>& computeShader);
		std::shared_ptr<ComputeShader> _shader;
	public:
		std::shared_ptr<ComputeShader> GetShader() const;
	};
}
