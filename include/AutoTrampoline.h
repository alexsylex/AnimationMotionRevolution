#pragma once

#if defined(SKSE_SUPPORT_XBYAK)

#include <xbyak/xbyak.h>

#include "SKSE/Trampoline.h"

namespace SKSE
{
	template<std::size_t N>
	static constexpr std::uintptr_t AutoTrampoline(std::uintptr_t a_location, const Xbyak::CodeGenerator& a_hookCode)
	{
		static_assert(N == 5 || N == 6);

		auto& hookCode = const_cast<Xbyak::CodeGenerator&>(a_hookCode);
		hookCode.ready();

		// Reference: write_5branch() and write_6branch() of Trampoline.h
		if constexpr (N == 5) 
		{
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

			AllocTrampoline(hookCode.getSize() + sizeof(TrampolineAssembly));
		} 
		else if constexpr (N == 6)
		{
			AllocTrampoline(hookCode.getSize() + sizeof(std::uintptr_t));
		}

		return GetTrampoline().write_call<N>(a_location, GetTrampoline().allocate(hookCode));
	}
}
#else
#error "Xbyak support needed for AutoTrampoline"
#endif