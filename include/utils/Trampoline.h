#pragma once

#if defined(SKSE_SUPPORT_XBYAK)

#include <xbyak/xbyak.h>

#include "SKSE/Trampoline.h"

namespace utils
{
	template <std::size_t N>
	static constexpr std::size_t GetTrampolineBaseSize()
	{
		static_assert(N == 5 || N == 6);

		// Reference: write_5branch() and write_6branch() of Trampoline.h
		if constexpr (N == 5) {
#pragma pack(push, 1)
			// FF /4
			// JMP r/m64
			struct TrampolineAssembly
			{
				// jmp [rip]
				std::uint8_t jmp;	 // 0 - 0xFF
				std::uint8_t modrm;	 // 1 - 0x25
				std::int32_t disp;	 // 2 - 0x00000000
				std::uint64_t addr;	 // 6 - [rip]
			};
#pragma pack(pop)

			return sizeof(TrampolineAssembly);
		}
		
		return sizeof(std::uintptr_t);
	}

	template <std::size_t N>
	static constexpr void AllocExactSizeTrampoline(Xbyak::CodeGenerator& a_hookCode)
	{
		a_hookCode.ready();

		SKSE::AllocTrampoline(GetTrampolineBaseSize<N>() + a_hookCode.getSize());
	}

	template <std::size_t N>
	static constexpr void AllocExactSizeTrampoline()
	{
		SKSE::AllocTrampoline(GetTrampolineBaseSize<N>());
	}

	template <std::size_t N>
	static constexpr std::uintptr_t WriteBranchTrampoline(std::uintptr_t a_location, const Xbyak::CodeGenerator& a_hookCode)
	{
		auto& hookCode = const_cast<Xbyak::CodeGenerator&>(a_hookCode);
		
		AllocExactSizeTrampoline<N>(hookCode);

		return SKSE::GetTrampoline().write_branch<N>(a_location, SKSE::GetTrampoline().allocate(hookCode));
	}

	template <std::size_t N>
	static constexpr std::uintptr_t WriteCallTrampoline(std::uintptr_t a_location, const Xbyak::CodeGenerator& a_hookCode)
	{
		auto& hookCode = const_cast<Xbyak::CodeGenerator&>(a_hookCode);

		AllocExactSizeTrampoline<N>(hookCode);

		return SKSE::GetTrampoline().write_call<N>(a_location, SKSE::GetTrampoline().allocate(hookCode));
	}
}

#else
#error "Xbyak support needed for Trampoline"
#endif