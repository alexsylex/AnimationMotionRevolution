#pragma once

#include <charconv>
#include <limits>
#include <numbers>

namespace AMR
{
	class AnimMotionData
	{
	public:
		template <typename T>
		struct Motion
		{
			float time;
			T delta;
		};

		using Translation = Motion<RE::NiPoint3>;
		using Rotation = Motion<RE::NiQuaternion>;

		AnimMotionData() = default;

		AnimMotionData(const RE::hkaAnimation* a_animation, const Translation* a_translation) :
			animation{ a_animation }, translationList{ *a_translation }
		{}

		AnimMotionData(const RE::hkaAnimation* a_animation, const Rotation* a_rotation) :
			animation{ a_animation }, rotationList{ *a_rotation }
		{}

		void Add(const Translation* a_translation)
		{
			translationList.push_back(*a_translation);
		}

		void Add(const Rotation* a_rotation)
		{
			rotationList.push_back(*a_rotation);
		}

		void SortListsByTime()
		{
			if (!translationList.empty()) {
				std::sort(translationList.begin(), translationList.end(),
					[](const Translation& a_lhs, const Translation& a_rhs) -> bool {
						return a_lhs.time < a_rhs.time;
					});
			}

			if (!rotationList.empty()) {
				std::sort(rotationList.begin(), rotationList.end(),
					[](const Rotation& a_lhs, const Rotation& a_rhs) -> bool {
						return a_lhs.time < a_rhs.time;
					});
			}
		}

		const RE::hkaAnimation* animation = nullptr;
		std::vector<Translation> translationList;
		std::vector<Rotation> rotationList;
		std::int32_t activeCount = 1;
	};

	// Annotations we are looking for contain translation/rotation data
	static std::variant<std::monostate, AnimMotionData::Translation, AnimMotionData::Rotation>
		ParseAnnotation(const RE::hkaAnnotationTrack::Annotation& a_annotation)
	{
		constexpr std::string_view animmotionPrefix = "animmotion ";
		constexpr std::string_view animrotationPrefix = "animrotation ";

		std::string_view text{ a_annotation.text.c_str() };

		if (text.starts_with(animmotionPrefix)) {
			RE::NiPoint3 translation;

			std::string_view start = text.substr(animmotionPrefix.size());
			std::string_view end = start.substr(start.find(' ') + 1);

			std::from_chars(start.data(), end.data(), translation.x);

			start = end;
			end = start.substr(start.find(' ') + 1);

			std::from_chars(start.data(), end.data(), translation.y);

			start = end;
			end = start.substr(start.size());

			std::from_chars(start.data(), end.data(), translation.z);

			return AnimMotionData::Translation{ a_annotation.time, translation };
		} else if (text.starts_with(animrotationPrefix)) {
			RE::NiQuaternion rotation;

			std::string_view start = text.substr(animrotationPrefix.size());
			std::string_view end = start.substr(start.size());

			float yawDegrees;
			std::from_chars(start.data(), end.data(), yawDegrees);

			float roll = 0.0f;
			float pitch = 0.0f;
			float yaw = yawDegrees * std::numbers::pi_v<float> / 180.0f;

			rotation.w = std::cos(roll / 2) * std::cos(pitch / 2) * std::cos(yaw / 2) + std::sin(roll / 2) * std::sin(pitch / 2) * std::sin(yaw / 2);
			rotation.x = std::sin(roll / 2) * std::cos(pitch / 2) * std::cos(yaw / 2) - std::cos(roll / 2) * std::sin(pitch / 2) * std::sin(yaw / 2);
			rotation.y = std::cos(roll / 2) * std::sin(pitch / 2) * std::cos(yaw / 2) + std::sin(roll / 2) * std::cos(pitch / 2) * std::sin(yaw / 2);
			rotation.z = std::cos(roll / 2) * std::cos(pitch / 2) * std::sin(yaw / 2) - std::sin(roll / 2) * std::sin(pitch / 2) * std::cos(yaw / 2);

			return AnimMotionData::Rotation{ a_annotation.time, rotation };
		}

		return {};
	}
}