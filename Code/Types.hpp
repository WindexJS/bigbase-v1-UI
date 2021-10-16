#pragma once

namespace Big
{
	using Void = std::uint32_t;
	using Any = std::uint32_t;
	using Hash = std::uint32_t;
	using Entity = std::int32_t;
	using Player = std::int32_t;
	using FireId = std::int32_t;
	using Ped = Entity;
	using Vehicle = Entity;
	using Cam = std::int32_t;
	using CarGenerator = std::int32_t;
	using Group = std::int32_t;
	using Train = std::int32_t;
	using Object = Entity;
	using Pickup = Object;
	using Weapon = std::int32_t;
	using Interior = std::int32_t;
	using Blip = std::int32_t;
	using Texture = std::int32_t;
	using TextureDict = std::int32_t;
	using CoverPoint = std::int32_t;
	using Camera = std::int32_t;
	using TaskSequence = std::int32_t;
	using ColourIndex = std::int32_t;
	using Sphere = std::int32_t;
	using ScrHandle = std::int32_t;

#pragma pack(push, 1)
	struct Vector2
	{
		float x{};
		float y{};
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct NativeVector3
	{
	public:
		NativeVector3() = default;
		NativeVector3(float x, float y, float z):
			x(x), y(y), z(z)
		{}
	public:
		float x{};
	private:
		char paddingX[4];
	public:
		float y{};
	private:
		char paddingY[4];
	public:
		float z{};
	private:
		char paddingZ[4];
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct Vector3
	{
		float x{};
		float y{};
		float z{};
	};
#pragma pack(pop)

#pragma pack(push, 1)
	struct Color
	{
		std::uint8_t r{ 255 };
		std::uint8_t g{ 255 };
		std::uint8_t b{ 255 };
		std::uint8_t a{ 255 };
	};
#pragma pack(pop)
}
