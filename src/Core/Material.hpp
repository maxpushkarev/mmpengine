#pragma once
#include <optional>
#include <variant>
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

			struct Buffer final
			{
				enum class Type : std::uint8_t
				{
					Uniform,
					ReadonlyAccess,
					UnorderedAccess
				};

				Type type;
			};

			struct Texture final
			{
				enum class Type : std::uint8_t
				{
					ReadonlyAccess,
					UnorderedAccess
				};

				Type type;
			};


			struct Entry final
			{
				std::string name;
				std::shared_ptr<BaseEntity> entity;
				std::variant<Buffer,Texture> settings;
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
		virtual std::shared_ptr<Core::BaseMaterial> GetUnderlyingMaterial();
		std::shared_ptr<BaseTask> CreateTaskForBakeParameters();
		virtual std::shared_ptr<BaseTask> CreateTaskForApply() = 0;

		virtual const Parameters& GetParameters() const;
		virtual void SetParameters(Parameters&& params);
	protected:
		BaseMaterial();
		virtual std::shared_ptr<BaseTask> CreateTaskForBakeParametersInternal() = 0;
	protected:
		Parameters _params;
		bool _bakedParams = false;
	};

	class RenderingMaterial : public BaseMaterial
	{
	public:
		struct Settings
		{
			enum class FillMode : std::uint8_t
			{
				Solid,
				WireFrame
			};

			enum class Comparision : std::uint8_t
			{
				LessEqual,
				GreaterEqual,
				Greater,
				Less,
				Equal,
				NotEqual,
				Always,
				Never
			};

			struct Depth final
			{
				enum class Write : std::uint8_t
				{
					On,
					Off
				};

				enum class Test : std::uint8_t
				{
					On,
					Off
				};

				Test test = Test::On;
				Write write = Write::On;
				Comparision comparision = Comparision::LessEqual;
			};

			struct Stencil final
			{
				enum class Op : std::uint8_t
				{
					Keep,
					Zero,
					Replace,
					IncrementAndSaturate,
					DecrementAndSaturate,
					Invert,
					IncrementAndWrap,
					DecrementAndWrap
				};

				struct Face final
				{
					Comparision comparision = Comparision::Always;
					Op stencilFail = Op::Keep;
					Op stencilPass = Op::Keep;
					Op depthFail = Op::Keep;
				};

				std::uint8_t readMask = (std::numeric_limits<std::uint8_t>::max)();
				std::uint8_t writeMask = (std::numeric_limits<std::uint8_t>::max)();
				Face front {};
				Face back {};
			};

			enum class AlphaToCoverage
			{
				On,
				Off
			};

			struct Blend
			{
				static constexpr std::size_t kMaxRenderTargets = 8;

				enum class Op
				{
					None,
					Add,
					Sub,
					RevSub,
					Min,
					Max
				};

				enum class Factor
				{
					One,
					Zero,
					SrcColor,
					SrcAlpha,
					DstColor,
					DstAlpha,
					OneMinusSrcAlpha,
					OneMinusSrcColor,
					OneMinusDstAlpha,
					OneMinusDstColor,
				};

				struct Target final
				{
					Factor src = Factor::One;
					Op op = Op::None;
					Factor dst = Factor::One;
					
					bool operator==(const Target& rhs) const;
					bool operator!=(const Target& rhs) const;
				};

				Target targets[kMaxRenderTargets];
			};

			FillMode fillMode = FillMode::Solid;
			Depth depth = {};
			std::optional<Stencil> stencil = std::nullopt;
			AlphaToCoverage alphaToCoverage = AlphaToCoverage::Off;
			Blend blend;
		};
	protected:
		RenderingMaterial(const Settings& settings);
		Settings _settings;
	};

	class MeshMaterial : public RenderingMaterial
	{
	protected:
		MeshMaterial(const Settings& settings, const std::shared_ptr<VertexShader>& vs, const std::shared_ptr<PixelShader>& ps);
		std::shared_ptr<VertexShader> _vs;
		std::shared_ptr<PixelShader> _ps;
	};

	class ComputeMaterial : public BaseMaterial
	{
	protected:
		ComputeMaterial(const std::shared_ptr<ComputeShader>& computeShader);
		std::shared_ptr<ComputeShader> _shader;
	public:
		const std::shared_ptr<ComputeShader>& GetShader() const;
	};
}
