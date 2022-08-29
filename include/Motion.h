#pragma once

#include <charconv>
#include <limits>
#include <numbers>

template <typename T>
struct Motion
{
	float time;
	T delta;
};

using Translation = Motion<RE::NiPoint3>;
using Rotation = Motion<RE::NiQuaternion>;

// Annotations we are looking for contain translation/rotation data
static std::variant<std::monostate, Translation, Rotation>
	ParseAnnotation(const RE::hkaAnnotationTrack::Annotation& a_annotation)
{
	constexpr std::string_view animmotionPrefix = "animmotion ";
	constexpr std::string_view animrotationPrefix = "animrotation ";

	std::string_view text{ a_annotation.text.c_str() };

	if (text.starts_with(animmotionPrefix))
	{
		std::string_view start = text.substr(animmotionPrefix.size());
		std::string_view end = start.substr(start.find(' ') + 1);

		RE::NiPoint3 translation;

		std::from_chars(start.data(), end.data(), translation.x);

		start = end;
		end = start.substr(start.find(' ') + 1);

		std::from_chars(start.data(), end.data(), translation.y);

		start = end;
		end = start.substr(start.size());

		std::from_chars(start.data(), end.data(), translation.z);

		return Translation{ a_annotation.time, translation };
	}
	else if (text.starts_with(animrotationPrefix))
	{
		std::string_view start = text.substr(animrotationPrefix.size());
		std::string_view end = start.substr(start.size());

		float yawDegrees;
		std::from_chars(start.data(), end.data(), yawDegrees);

		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = yawDegrees * std::numbers::pi_v<float> / 180.0f;

		RE::NiQuaternion rotation
		{
			.w = std::cos(roll / 2) * std::cos(pitch / 2) * std::cos(yaw / 2) + std::sin(roll / 2) * std::sin(pitch / 2) * std::sin(yaw / 2),
			.x = std::sin(roll / 2) * std::cos(pitch / 2) * std::cos(yaw / 2) - std::cos(roll / 2) * std::sin(pitch / 2) * std::sin(yaw / 2),
			.y = std::cos(roll / 2) * std::sin(pitch / 2) * std::cos(yaw / 2) + std::sin(roll / 2) * std::cos(pitch / 2) * std::sin(yaw / 2),
			.z = std::cos(roll / 2) * std::cos(pitch / 2) * std::sin(yaw / 2) - std::sin(roll / 2) * std::sin(pitch / 2) * std::cos(yaw / 2)
		};

		return Rotation{ a_annotation.time, rotation };
	}

	return { };
}