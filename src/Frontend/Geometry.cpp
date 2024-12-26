#include <Frontend/Geometry.hpp>

namespace MMPEngine::Frontend
{
	Core::GeometryPrototype Geometry::Generate(const aiMesh* aiMesh)
	{
		return {};
	}

	template<>
	Core::GeometryPrototype Geometry::Generate<Geometry::PrimitiveType::Box>()
	{
		Core::GeometryPrototype proto;

		auto vertexBuffer = std::make_unique<Core::VertexBufferPrototypeFloat3>(Core::VertexBufferPrototype::Settings {
			{ Core::VertexBufferPrototype::Semantics::Position },
			{}
		});
		auto indexBuffer = std::make_unique<Core::IndexBufferPrototype16>(Core::IndexBufferPrototype::Settings {});

		vertexBuffer->data = {
			Core::Vector3Float {-0.5f, -0.5f, -0.5f},
			Core::Vector3Float {-0.5f, +0.5f, -0.5f},
			Core::Vector3Float {+0.5f, +0.5f, -0.5f},
			Core::Vector3Float {+0.5f, -0.5f, -0.5f},
			Core::Vector3Float {-0.5f, -0.5f, +0.5f},
			Core::Vector3Float {-0.5f, +0.5f, +0.5f},
			Core::Vector3Float {+0.5f, +0.5f, +0.5f},
			Core::Vector3Float {+0.5f, -0.5f, +0.5f}
		};

		indexBuffer->data = {
			0, 1, 2,
			0, 2, 3,
			4, 6, 5,
			4, 7, 6,
			4, 5, 1,
			4, 1, 0,
			3, 2, 6,
			3, 6, 7,
			1, 5, 6,
			1, 6, 2,
			4, 0, 3,
			4, 3, 7
		};

		proto.vertexBuffers.push_back(std::move(vertexBuffer));
		proto.indexBuffer = std::move(indexBuffer);
		proto.subsets = { { static_cast<std::uint32_t>(proto.indexBuffer->GetElementsCount()),0,0 } };

		return proto;
	}

}
