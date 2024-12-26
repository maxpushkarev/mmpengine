#pragma once
#include <Core/Geometry.hpp>
#include <filesystem>

class aiMesh;

namespace MMPEngine::Frontend
{
	class Geometry final
	{
	public:
		enum class PrimitiveType : std::uint8_t
		{
			Box
		};
		static Core::GeometryPrototype Generate(const aiMesh* aiMesh);

		template<PrimitiveType TPrimitiveType>
		static Core::GeometryPrototype Generate();
	};
}

namespace MMPEngine::Frontend
{
	template<>
	Core::GeometryPrototype Geometry::Generate<Geometry::PrimitiveType::Box>();
}
