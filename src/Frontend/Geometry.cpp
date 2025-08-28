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

		auto positionBuffer = std::make_unique<Core::VertexBufferPrototypeFloat3>(Core::VertexBufferPrototype::Settings {
			{ Core::VertexBufferPrototype::Semantics::Position },
			{}
		});
		auto indexBuffer = std::make_unique<Core::IndexBufferPrototype16>(Core::IndexBufferPrototype::Settings {});

		positionBuffer->data = {
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

		proto.vertexBuffers.push_back(std::move(positionBuffer));
		proto.indexBuffer = std::move(indexBuffer);
		proto.subsets = { { static_cast<std::uint32_t>(proto.indexBuffer->GetElementsCount()),0,0 } };

		return proto;
	}

	template<>
	Core::GeometryPrototype Geometry::Generate<Geometry::PrimitiveType::Quad>()
	{
		Core::GeometryPrototype proto;

		auto positionBuffer = std::make_unique<Core::VertexBufferPrototypeFloat3>(Core::VertexBufferPrototype::Settings{
			{ Core::VertexBufferPrototype::Semantics::Position },
			{}
			});
		auto uvBuffer = std::make_unique<Core::VertexBufferPrototypeFloat2>(Core::VertexBufferPrototype::Settings{
			{ Core::VertexBufferPrototype::Semantics::UV },
			{}
			});
		auto indexBuffer = std::make_unique<Core::IndexBufferPrototype16>(Core::IndexBufferPrototype::Settings{});

		positionBuffer->data = {
			Core::Vector3Float{-0.5f, -0.5f, 0.0f},
			Core::Vector3Float{-0.5f, +0.5f, 0.0f},
			Core::Vector3Float{+0.5f, -0.5f, 0.0f},
			Core::Vector3Float{+0.5f, +0.5f, 0.0f}
		};

		uvBuffer->data = {
			Core::Vector2Float{0.0f, 1.0f},
			Core::Vector2Float{0.0f, 0.0f},
			Core::Vector2Float{1.0f, 1.0f},
			Core::Vector2Float{1.0f, 0.0f}
		};

		indexBuffer->data = {
			0, 1, 2,
			2, 1, 3
		};

		proto.vertexBuffers.push_back(std::move(positionBuffer));
		proto.vertexBuffers.push_back(std::move(uvBuffer));

		proto.indexBuffer = std::move(indexBuffer);
		proto.subsets = { { static_cast<std::uint32_t>(proto.indexBuffer->GetElementsCount()),0,0 } };

		return proto;
	}

}
