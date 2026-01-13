#pragma once
#include <optional>
#include <variant>
#include <filesystem>
#include <array>
#include <Core/Base.hpp>
#include <Core/Entity.hpp>
#include <Core/Shader.hpp>

namespace MMPEngine::Core
{
	class StencilRef final : public Core::BaseEntity
	{
	public:
		StencilRef(std::string_view name);
		StencilRef();
		std::uint8_t value = std::numeric_limits<std::uint8_t>::lowest();
	};

	class BaseMaterial : public IInitializationTaskSource, public std::enable_shared_from_this<BaseMaterial>
	{
	public:

		BaseMaterial(const BaseMaterial&) = delete;
		BaseMaterial(BaseMaterial&&) noexcept = delete;

		BaseMaterial& operator=(const BaseMaterial&) = delete;
		BaseMaterial& operator=(BaseMaterial&&) noexcept = delete;

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

			struct StencilRef final
			{
			};

			struct Entry final
			{
				std::string name;
				std::shared_ptr<const BaseEntity> entity;
				std::variant<Buffer,Texture,StencilRef> settings;
			};

			struct EntryView final
			{
				const Entry* entryPtr;
				std::uint32_t index;
			};

            struct Bindings final
            {
                std::variant<std::string, std::filesystem::path> source;
            };
            
			explicit Parameters(std::vector<Entry>&& entries, Bindings&& bindings);
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
            Bindings _bindings;
			std::unordered_map<std::string_view, EntryView> _viewMap;
		};

		const Parameters& GetParameters() const;
	protected:
		BaseMaterial(Parameters&& params);
		virtual ~BaseMaterial();
	protected:
		Parameters _params;
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

			enum class CullMode : std::uint8_t
			{
				Back,
				Front,
				None
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

				enum class ColorMask : std::uint8_t
				{
					None = 0,
					Red = 1,
					Green = 2,
					Blue = 4,
					Alpha = 8,
					All = (Red | Green | Blue | Alpha)
				};

				struct Target final
				{
					ColorMask colorMask = ColorMask::All;
					Factor src = Factor::One;
					Op op = Op::None;
					Factor dst = Factor::One;
					
					bool operator==(const Target& rhs) const;
					bool operator!=(const Target& rhs) const;
				};

				std::array<Target, kMaxRenderTargets> targets;
			};

			FillMode fillMode = FillMode::Solid;
			CullMode cullMode = CullMode::Back;
			Depth depth = {};
			std::optional<Stencil> stencil = std::nullopt;
			AlphaToCoverage alphaToCoverage = AlphaToCoverage::Off;
			Blend blend;
		};
	protected:
		RenderingMaterial(const Settings& settings, Parameters&& params);
		Settings _settings;
	public:
		const Settings& GetSettings() const;
	};

	class MeshMaterial : public RenderingMaterial
	{
	public:
		MeshMaterial(const Settings& settings, Parameters&& params, const std::shared_ptr<Shader>& vs, const std::shared_ptr<Shader>& ps);
		const std::shared_ptr<Shader>& GetVertexShader() const;
		const std::shared_ptr<Shader>& GetPixelShader() const;
    private:
        std::shared_ptr<Shader> _vs;
        std::shared_ptr<Shader> _ps;
	};

	class ComputeMaterial : public BaseMaterial
	{
	public:
		ComputeMaterial(Parameters&& params, const std::shared_ptr<Shader>& computeShader);
		const std::shared_ptr<Shader>& GetShader() const;
    private:
        std::shared_ptr<Shader> _shader;
	};
}
