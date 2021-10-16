//Memory.cpp
#include "memory.h"
#include <map>

static std::multimap<uint64_t, uintptr_t> g_hints;

void Memory::executable_meta::EnsureInit() {

	if ( m_begin ) {
		return;
	}

	HMODULE gModule = GetModuleHandle( NULL );

	m_begin = reinterpret_cast<uintptr_t>(gModule);
	const IMAGE_DOS_HEADER * dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(gModule);
	const IMAGE_NT_HEADERS * ntHeader = reinterpret_cast<const IMAGE_NT_HEADERS64*>( reinterpret_cast<const uint8_t*>(dosHeader)+dosHeader->e_lfanew );
	m_end = m_begin + ntHeader->OptionalHeader.SizeOfCode;
	m_size = ntHeader->OptionalHeader.SizeOfImage;
}

void Memory::TransformPattern( const std::string & pattern, std::string & data, std::string & mask ) {

	std::stringstream dataStr;
	std::stringstream maskStr;

	uint8_t tempDigit = 0;
	bool tempFlag = false;

	for ( auto & ch : pattern ) {

		if ( ch == ' ' ) {
			continue;
		} else if ( ch == '?' ) {

			dataStr << '\x00';
			maskStr << '?';
		} else if ( ( ch >= '0' && ch <= '9' ) || ( ch >= 'A' && ch <= 'F' ) || ( ch >= 'a' && ch <= 'f' ) ) {

			char str[] = { ch, 0 };
			int thisDigit = strtol( str, nullptr, 16 );

			if ( !tempFlag ) {

				tempDigit = ( thisDigit << 4 );
				tempFlag = true;
			} else {

				tempDigit |= thisDigit;
				tempFlag = false;

				dataStr << tempDigit;
				maskStr << 'x';
			}
		}
	}

	data = dataStr.str();
	mask = maskStr.str();
}

void Memory::pattern::Initialize( const char* pattern, size_t length ) {

	// get the hash for the base pattern
	std::string baseString( pattern, length );
	m_hash = fnv_1()( baseString );

	m_matched = false;

	// transform the base pattern from IDA format to canonical format
	TransformPattern( baseString, m_bytes, m_mask );

	m_size = m_mask.size();

	// if there's hints, try those first
	auto range = g_hints.equal_range( m_hash );

	if ( range.first != range.second ) {

		std::for_each( range.first, range.second, [&]( const std::pair<uint64_t, uintptr_t> & hint ) {
			ConsiderMatch( hint.second );
		} );

		// if the hints succeeded, we don't need to do anything more
		if ( m_matches.size() > 0 ) {
			m_matched = true;
			return;
		}
	}
}

uintptr_t Memory::get_base()
{
	executable_meta executable;
	executable.EnsureInit();
	return executable.begin();
}

DWORD Memory::get_size()
{
	executable_meta executable;
	executable.EnsureInit();
	return executable.size();
}

uintptr_t Memory::get_base_offsetted(DWORD offset)
{
	return get_base() + offset;
}

uintptr_t Memory::get_multilayer_pointer(uintptr_t base_address, std::vector<DWORD> offsets)
{
	uintptr_t ptr = *(uintptr_t*)(base_address);
	if (!ptr) {

		return NULL;
	}
	auto level = offsets.size();

	for (auto i = 0; i < level; i++)
	{
		if (i == level - 1)
		{
			ptr += offsets[i];
			if (!ptr) return NULL;
			return ptr;
		}
		else
		{
			ptr = *(uint64_t*)(ptr + offsets[i]);
			if (!ptr) return NULL;
		}
	}

	return ptr;
}

static bool compare(const uint8_t* pData, const uint8_t* bMask, const char* sMask)
{
	for (; *sMask; ++sMask, ++pData, ++bMask)
		if (*sMask == 'x' && *pData != *bMask)
			return false;

	return *sMask == NULL;
}

std::vector<DWORD64> Memory::get_string_addresses(std::string str)
{
	std::string currentMask;
	const char* to_scan = str.c_str();
	for (uint8_t i = 0; i < strlen(to_scan); i++) currentMask += "x";
	const char *mask = currentMask.c_str();
	std::vector<DWORD64> foundAddrs;
	for (uint32_t i = 0; i < get_size(); ++i)
	{
		auto address = get_base() + i;
		if (compare((BYTE *)(address), (BYTE *)to_scan, mask))
		{
			foundAddrs.push_back((address));
		}
	}

	return foundAddrs;

}

bool Memory::pattern::ConsiderMatch( uintptr_t offset ) {

	const char * pattern = m_bytes.c_str();
	const char * mask = m_mask.c_str();

	char * ptr = reinterpret_cast<char*>( offset );

	for ( size_t i = 0; i < m_size; i++ ) {

		if ( mask[i] == '?' ) {
			continue;
		}

		if ( pattern[i] != ptr[i] ) {
			return false;
		}
	}

	m_matches.push_back( pattern_match( ptr ) );

	return true;
}
/***
*   intrin.h - declarations/definitions for platform specific intrinsic functions.
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*Purpose:
*   This include file contains the declarations for platform specific intrinsic
*   functions, or will include other files that have declaration of intrinsic
*   functions. Also there will be some platform specific macros to be used with
*   intrinsic functions.
*
****/

#pragma once
#define __INTRIN_H_
#ifndef RC_INVOKED
#ifndef __midl

#include <intrin0.h>
#include <vcruntime.h>
#include <setjmp.h>

#ifndef _M_CEE_PURE

#if (defined (_M_IX86) || defined (_M_X64)) && !defined(_CHPE_ONLY_)
#include <immintrin.h>
#include <ammintrin.h>
#endif

#if defined (_M_IX86) && !defined(_CHPE_ONLY_)
#include <mm3dnow.h>
#endif

#if defined (_M_ARM)
#include <armintr.h>
#include <arm_neon.h>
#endif

#if defined (_M_ARM64) || defined(_M_HYBRID_X86_ARM64)
#include <arm64intr.h>
#include <arm64_neon.h>
#endif
#endif

#if defined (__cplusplus)
extern "C" {
#endif  /* defined (__cplusplus) */

	__MACHINEARM(int _AddSatInt(int, int))
		__MACHINE(void * _AddressOfReturnAddress(void))
		__MACHINE(unsigned char _BitScanForward(unsigned long * _Index, unsigned long _Mask))
		__MACHINEX64(unsigned char _BitScanForward64(unsigned long * _Index, unsigned __int64 _Mask))
		__MACHINEARM64(unsigned char _BitScanForward64(unsigned long * _Index, unsigned __int64 _Mask))
		__MACHINE(unsigned char _BitScanReverse(unsigned long * _Index, unsigned long _Mask))
		__MACHINEX64(unsigned char _BitScanReverse64(unsigned long * _Index, unsigned __int64 _Mask))
		__MACHINEARM64(unsigned char _BitScanReverse64(unsigned long * _Index, unsigned __int64 _Mask))
		__MACHINEARM_ARM64(double _CopyDoubleFromInt64(__int64))
		__MACHINEARM_ARM64(float _CopyFloatFromInt32(__int32))
		__MACHINEARM_ARM64(__int32 _CopyInt32FromFloat(float))
		__MACHINEARM_ARM64(__int64 _CopyInt64FromDouble(double))
		__MACHINEARM_ARM64(unsigned int _CountLeadingOnes(unsigned long))
		__MACHINEARM_ARM64(unsigned int _CountLeadingOnes64(unsigned __int64))
		__MACHINEARM_ARM64(unsigned int _CountLeadingSigns(long))
		__MACHINEARM_ARM64(unsigned int _CountLeadingSigns64(__int64))
		__MACHINEARM_ARM64(unsigned int _CountLeadingZeros(unsigned long))
		__MACHINEARM_ARM64(unsigned int _CountLeadingZeros64(unsigned __int64))
		__MACHINEARM(unsigned int _CountOneBits(unsigned long))
		__MACHINEARM(unsigned int _CountOneBits64(unsigned __int64))
		__MACHINEARM(int _DAddSatInt(int, int))
		__MACHINEARM(int _DSubSatInt(int, int))
		__MACHINECHPEX86ARM64(void _HybridGenerateThunks(void *, unsigned))
		__MACHINEARM_ARM64(long _InterlockedAdd(long volatile * _Addend, long _Value))
		__MACHINEARM_ARM64(__int64 _InterlockedAdd64(__int64 volatile * _Addend, __int64 _Value))
		__MACHINEARM_ARM64(__int64 _InterlockedAdd64_acq(__int64 volatile * _Addend, __int64 _Value))
		__MACHINEARM_ARM64(__int64 _InterlockedAdd64_nf(__int64 volatile * _Addend, __int64 _Value))
		__MACHINEARM_ARM64(__int64 _InterlockedAdd64_rel(__int64 volatile * _Addend, __int64 _Value))
		__MACHINEX86(long _InterlockedAddLargeStatistic(__int64 volatile * _Addend, long _Value))
		__MACHINEARM_ARM64(long _InterlockedAdd_acq(long volatile * _Addend, long _Value))
		__MACHINEARM_ARM64(long _InterlockedAdd_nf(long volatile * _Addend, long _Value))
		__MACHINEARM_ARM64(long _InterlockedAdd_rel(long volatile * _Addend, long _Value))
		__MACHINEX64(short _InterlockedAnd16_np(short volatile * _Value, short _Mask))
		__MACHINEX64(__int64 _InterlockedAnd64_np(__int64 volatile * _Value, __int64 _Mask))
		__MACHINEX64(char _InterlockedAnd8_np(char volatile * _Value, char _Mask))
		__MACHINEX64(long _InterlockedAnd_np(long volatile * _Value, long _Mask))
		__MACHINEARM64_X64(unsigned char _InterlockedCompareExchange128(__int64 volatile * _Destination, __int64 _ExchangeHigh, __int64 _ExchangeLow, __int64 * _ComparandResult))
		__MACHINEARM64(unsigned char _InterlockedCompareExchange128_acq(__int64 volatile * _Destination, __int64 _ExchangeHigh, __int64 _ExchangeLow, __int64 * _ComparandResult))
		__MACHINEARM64(unsigned char _InterlockedCompareExchange128_nf(__int64 volatile * _Destination, __int64 _ExchangeHigh, __int64 _ExchangeLow, __int64 * _ComparandResult))
		__MACHINEX64(unsigned char _InterlockedCompareExchange128_np(__int64 volatile * _Destination, __int64 _ExchangeHigh, __int64 _ExchangeLow, __int64 * _ComparandResult))
		__MACHINEARM64(unsigned char _InterlockedCompareExchange128_rel(__int64 volatile * _Destination, __int64 _ExchangeHigh, __int64 _ExchangeLow, __int64 * _ComparandResult))
		__MACHINEX64(short _InterlockedCompareExchange16_np(short volatile * _Destination, short _Exchange, short _Comparand))
		__MACHINEX64(__int64 _InterlockedCompareExchange64_np(__int64 volatile * _Destination, __int64 _Exchange, __int64 _Comparand))
		__MACHINE(void * _InterlockedCompareExchangePointer(void * volatile * _Destination, void * _Exchange, void * _Comparand))
		__MACHINEARM_ARM64(void * _InterlockedCompareExchangePointer_acq(void * volatile * _Destination, void * _Exchange, void * _Comparand))
		__MACHINEARM_ARM64(void * _InterlockedCompareExchangePointer_nf(void * volatile * _Destination, void * _Exchange, void * _Comparand))
		__MACHINEX64(void * _InterlockedCompareExchangePointer_np(void * volatile * _Destination, void * _Exchange, void * _Comparand))
		__MACHINEARM_ARM64(void * _InterlockedCompareExchangePointer_rel(void * volatile * _Destination, void * _Exchange, void * _Comparand))
		__MACHINEX64(long _InterlockedCompareExchange_np(long volatile * _Destination, long _Exchange, long _Comparand))
		__MACHINE(short _InterlockedDecrement16(short volatile * _Addend))
		__MACHINEARM_ARM64(short _InterlockedDecrement16_acq(short volatile * _Addend))
		__MACHINEARM_ARM64(short _InterlockedDecrement16_nf(short volatile * _Addend))
		__MACHINEARM_ARM64(short _InterlockedDecrement16_rel(short volatile * _Addend))
		__MACHINEARM_ARM64_X64(__int64 _InterlockedDecrement64(__int64 volatile * _Addend))
		__MACHINEARM_ARM64(__int64 _InterlockedDecrement64_acq(__int64 volatile * _Addend))
		__MACHINEARM_ARM64(__int64 _InterlockedDecrement64_nf(__int64 volatile * _Addend))
		__MACHINEARM_ARM64(__int64 _InterlockedDecrement64_rel(__int64 volatile * _Addend))
		__MACHINEARM_ARM64(long _InterlockedDecrement_acq(long volatile * _Addend))
		__MACHINEARM_ARM64(long _InterlockedDecrement_nf(long volatile * _Addend))
		__MACHINEARM_ARM64(long _InterlockedDecrement_rel(long volatile * _Addend))
		__MACHINE(void * _InterlockedExchangePointer(void * volatile * _Target, void * _Value))
		__MACHINEARM_ARM64(void * _InterlockedExchangePointer_acq(void * volatile * _Target, void * _Value))
		__MACHINEARM_ARM64(void * _InterlockedExchangePointer_nf(void * volatile * _Target, void * _Value))
		__MACHINEARM_ARM64(void * _InterlockedExchangePointer_rel(void * volatile * _Target, void * _Value))
		__MACHINE(short _InterlockedIncrement16(short volatile * _Addend))
		__MACHINEARM_ARM64(short _InterlockedIncrement16_acq(short volatile * _Addend))
		__MACHINEARM_ARM64(short _InterlockedIncrement16_nf(short volatile * _Addend))
		__MACHINEARM_ARM64(short _InterlockedIncrement16_rel(short volatile * _Addend))
		__MACHINEARM_ARM64_X64(__int64 _InterlockedIncrement64(__int64 volatile * _Addend))
		__MACHINEARM_ARM64(__int64 _InterlockedIncrement64_acq(__int64 volatile * _Addend))
		__MACHINEARM_ARM64(__int64 _InterlockedIncrement64_nf(__int64 volatile * _Addend))
		__MACHINEARM_ARM64(__int64 _InterlockedIncrement64_rel(__int64 volatile * _Addend))
		__MACHINEARM_ARM64(long _InterlockedIncrement_acq(long volatile * _Addend))
		__MACHINEARM_ARM64(long _InterlockedIncrement_nf(long volatile * _Addend))
		__MACHINEARM_ARM64(long _InterlockedIncrement_rel(long volatile * _Addend))
		__MACHINEX64(short _InterlockedOr16_np(short volatile * _Value, short _Mask))
		__MACHINEX64(__int64 _InterlockedOr64_np(__int64 volatile * _Value, __int64 _Mask))
		__MACHINEX64(char _InterlockedOr8_np(char volatile * _Value, char _Mask))
		__MACHINEX64(long _InterlockedOr_np(long volatile * _Value, long _Mask))
		__MACHINEX64(short _InterlockedXor16_np(short volatile * _Value, short _Mask))
		__MACHINEX64(__int64 _InterlockedXor64_np(__int64 volatile * _Value, __int64 _Mask))
		__MACHINEX64(char _InterlockedXor8_np(char volatile * _Value, char _Mask))
		__MACHINEX64(long _InterlockedXor_np(long volatile * _Value, long _Mask))
		__MACHINEARM(unsigned int _MoveFromCoprocessor(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))
		__MACHINEARM(unsigned int _MoveFromCoprocessor2(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))
		__MACHINEARM(unsigned __int64 _MoveFromCoprocessor64(unsigned int, unsigned int, unsigned int))
		__MACHINEARM(void _MoveToCoprocessor(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))
		__MACHINEARM(void _MoveToCoprocessor2(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))
		__MACHINEARM(void _MoveToCoprocessor64(unsigned __int64, unsigned int, unsigned int, unsigned int))
		__MACHINEARM(long _MulHigh(long, long))
		__MACHINEARM(unsigned long _MulUnsignedHigh(unsigned long, unsigned long))
		__MACHINE(void _ReadBarrier(void))
		__MACHINEARM(int _ReadStatusReg(int))
		__MACHINEARM64(__int64 _ReadStatusReg(int))
		__MACHINEARM64(unsigned __int64 __getReg(int))
		__MACHINEARM64(unsigned __int64 __getCallerReg(int))
		__MACHINEARM64(double __getRegFp(int))
		__MACHINEARM64(double __getCallerRegFp(int))
		__MACHINE(void * _ReturnAddress(void))
		__MACHINEARM(int _SubSatInt(int, int))
		__MACHINE(void _WriteBarrier(void))
		__MACHINEARM(void _WriteStatusReg(int, int, int))
		__MACHINEARM64(void _WriteStatusReg(int, __int64))
		__MACHINEARM64(void __setReg(int, unsigned __int64))
		__MACHINEARM64(void __setCallerReg(int, unsigned __int64))
		__MACHINEARM64(void __setRegFp(int, double))
		__MACHINEARM64(void __setCallerRegFp(int, double))
		__MACHINEX86(void __addfsbyte(unsigned long, unsigned char))
		__MACHINEX86(void __addfsdword(unsigned long, unsigned long))
		__MACHINEX86(void __addfsword(unsigned long, unsigned short))
		__MACHINEX64(void __addgsbyte(unsigned long, unsigned char))
		__MACHINEX64(void __addgsdword(unsigned long, unsigned long))
		__MACHINEX64(void __addgsqword(unsigned long, unsigned __int64))
		__MACHINEX64(void __addgsword(unsigned long, unsigned short))
		__MACHINEARM64(void __addx18byte(unsigned long, unsigned char))
		__MACHINEARM64(void __addx18dword(unsigned long, unsigned long))
		__MACHINEARM64(void __addx18qword(unsigned long, unsigned __int64))
		__MACHINEARM64(void __addx18word(unsigned long, unsigned short))
		__MACHINEX86_X64(void __clts(void))
		__MACHINE(void __code_seg(const char *))
		__MACHINEX86_X64(void __cpuid(int[4], int))
		__MACHINEX86_X64(void __cpuidex(int[4], int, int))
		__MACHINE(void __cdecl __debugbreak(void))
		__MACHINEARM_ARM64(void __emit(unsigned __int32))
		__MACHINEX86_X64(__int64 __emul(int, int))
		__MACHINEX86_X64(unsigned __int64 __emulu(unsigned int, unsigned int))
		__MACHINE(__declspec(noreturn) void __fastfail(unsigned int))
		__MACHINEX64(void __faststorefence(void))
		__MACHINEX86_X64(unsigned int __getcallerseflags(void))
		__MACHINEX86_X64(void __halt(void))
		__MACHINEARM_ARM64(unsigned int __hvc(unsigned int, ...))
		__MACHINEARM64(void __break(int))
		__MACHINEX86_X64(unsigned char __inbyte(unsigned short))
		__MACHINEX86_X64(void __inbytestring(unsigned short, unsigned char *, unsigned long))
		__MACHINEX86(void __incfsbyte(unsigned long))
		__MACHINEX86(void __incfsdword(unsigned long))
		__MACHINEX86(void __incfsword(unsigned long))
		__MACHINEX64(void __incgsbyte(unsigned long))
		__MACHINEX64(void __incgsdword(unsigned long))
		__MACHINEX64(void __incgsqword(unsigned long))
		__MACHINEX64(void __incgsword(unsigned long))
		__MACHINEARM64(void __incx18byte(unsigned long))
		__MACHINEARM64(void __incx18dword(unsigned long))
		__MACHINEARM64(void __incx18qword(unsigned long))
		__MACHINEARM64(void __incx18word(unsigned long))
		__MACHINEX86_X64(unsigned long __indword(unsigned short))
		__MACHINEX86_X64(void __indwordstring(unsigned short, unsigned long *, unsigned long))
		__MACHINEX86_X64(void __int2c(void))
		__MACHINEX86_X64(void __invlpg(void *))
		__MACHINEX86_X64(unsigned short __inword(unsigned short))
		__MACHINEX86_X64(void __inwordstring(unsigned short, unsigned short *, unsigned long))
		__MACHINEX86_X64(void __lidt(void *))
		__MACHINEX86_X64(unsigned __int64 __ll_lshift(unsigned __int64, int))
		__MACHINEX86_X64(__int64 __ll_rshift(__int64, int))
		__MACHINEX86_X64(unsigned int __lzcnt(unsigned int))
		__MACHINEX86_X64(unsigned short __lzcnt16(unsigned short))
		__MACHINEX64(unsigned __int64 __lzcnt64(unsigned __int64))
		__MACHINEX86_X64(void __movsb(unsigned char *, unsigned char const *, size_t))
		__MACHINEX86_X64(void __movsd(unsigned long *, unsigned long const *, size_t))
		__MACHINEX64(void __movsq(unsigned long long *, unsigned long long const *, size_t))
		__MACHINEX86_X64(void __movsw(unsigned short *, unsigned short const *, size_t))
		__MACHINEARM64_X64(__int64 __mulh(__int64, __int64))
		__MACHINE(void __nop(void))
		__MACHINEX86_X64(void __nvreg_restore_fence(void))
		__MACHINEX86_X64(void __nvreg_save_fence(void))
		__MACHINEX86_X64(void __outbyte(unsigned short, unsigned char))
		__MACHINEX86_X64(void __outbytestring(unsigned short, unsigned char *, unsigned long))
		__MACHINEX86_X64(void __outdword(unsigned short, unsigned long))
		__MACHINEX86_X64(void __outdwordstring(unsigned short, unsigned long *, unsigned long))
		__MACHINEX86_X64(void __outword(unsigned short, unsigned short))
		__MACHINEX86_X64(void __outwordstring(unsigned short, unsigned short *, unsigned long))
		__MACHINEX86_X64(unsigned int __popcnt(unsigned int))
		__MACHINEX86_X64(unsigned short __popcnt16(unsigned short))
		__MACHINEX64(unsigned __int64 __popcnt64(unsigned __int64))
		__MACHINEARM_ARM64(void __cdecl __prefetch(const void *))
		__MACHINEARM64(void __cdecl __prefetch2(const void *, uint8_t prfop))
		__MACHINEARM(void __cdecl __prefetchw(const void *))
		__MACHINEARM(unsigned __int64 __rdpmccntr64(void))
		__MACHINEX86_X64(unsigned __int64 __rdtsc(void))
		__MACHINEX86_X64(unsigned __int64 __rdtscp(unsigned int *))
		__MACHINEX64(unsigned __int64 __readcr0(void))
		__MACHINEX86(unsigned long __readcr0(void))
		__MACHINEX64(unsigned __int64 __readcr2(void))
		__MACHINEX86(unsigned long __readcr2(void))
		__MACHINEX64(unsigned __int64 __readcr3(void))
		__MACHINEX86(unsigned long __readcr3(void))
		__MACHINEX64(unsigned __int64 __readcr4(void))
		__MACHINEX86(unsigned long __readcr4(void))
		__MACHINEX64(unsigned __int64 __readcr8(void))
		__MACHINEX86(unsigned long __readcr8(void))
		__MACHINEX64(unsigned __int64 __readdr(unsigned int))
		__MACHINEX86(unsigned int __readdr(unsigned int))
		__MACHINEX64(unsigned __int64 __readeflags(void))
		__MACHINEX86(unsigned int __readeflags(void))
		__MACHINEX86(unsigned char __readfsbyte(unsigned long))
		__MACHINEX86(unsigned long __readfsdword(unsigned long))
		__MACHINEX86(unsigned __int64 __readfsqword(unsigned long))
		__MACHINEX86(unsigned short __readfsword(unsigned long))
		__MACHINEX64(unsigned char __readgsbyte(unsigned long))
		__MACHINEX64(unsigned long __readgsdword(unsigned long))
		__MACHINEX64(unsigned __int64 __readgsqword(unsigned long))
		__MACHINEX64(unsigned short __readgsword(unsigned long))
		__MACHINEX86_X64(unsigned __int64 __readmsr(unsigned long))
		__MACHINEX86_X64(unsigned __int64 __readpmc(unsigned long))
		__MACHINEARM64(unsigned char __readx18byte(unsigned long))
		__MACHINEARM64(unsigned long __readx18dword(unsigned long))
		__MACHINEARM64(unsigned __int64 __readx18qword(unsigned long))
		__MACHINEARM64(unsigned short __readx18word(unsigned long))
		__MACHINEX86_X64(unsigned long __segmentlimit(unsigned long))
		__MACHINEARM_ARM64(void __sev(void))
		__MACHINEX64(unsigned __int64 __shiftleft128(unsigned __int64 _LowPart, unsigned __int64 _HighPart, unsigned char _Shift))
		__MACHINEX64(unsigned __int64 __shiftright128(unsigned __int64 _LowPart, unsigned __int64 _HighPart, unsigned char _Shift))
		__MACHINEX86_X64(void __sidt(void *))
		__MACHINEARM_ARM64(void __static_assert(int, const char *))
		__MACHINEX86_X64(void __stosb(unsigned char *, unsigned char, size_t))
		__MACHINEX86_X64(void __stosd(unsigned long *, unsigned long, size_t))
		__MACHINEX64(void __stosq(unsigned __int64 *, unsigned __int64, size_t))
		__MACHINEX86_X64(void __stosw(unsigned short *, unsigned short, size_t))
		__MACHINEX86_X64(void __svm_clgi(void))
		__MACHINEX86_X64(void __svm_invlpga(void *, int))
		__MACHINEX86_X64(void __svm_skinit(int))
		__MACHINEX86_X64(void __svm_stgi(void))
		__MACHINEX86_X64(void __svm_vmload(size_t))
		__MACHINEX86_X64(void __svm_vmrun(size_t))
		__MACHINEX86_X64(void __svm_vmsave(size_t))
		__MACHINEARM(unsigned int __swi(unsigned int, ...))
		__MACHINEARM_ARM64(unsigned int __svc(unsigned int, ...))
		__MACHINEARM64(unsigned int __hlt(unsigned int, ...))
		__MACHINEARM64(unsigned int __sys(int, __int64))
		__MACHINEARM(int __trap(int, ...))
		__MACHINEX86_X64(void __ud2(void))
		__MACHINEX86_X64(unsigned __int64 __ull_rshift(unsigned __int64, int))
		__MACHINEARM64_X64(unsigned __int64 __umulh(unsigned __int64, unsigned __int64))
		__MACHINEX86_X64(void __vmx_off(void))
		__MACHINEX64(unsigned char __vmx_on(unsigned __int64 *))
		__MACHINEX64(unsigned char __vmx_vmclear(unsigned __int64 *))
		__MACHINEX64(unsigned char __vmx_vmlaunch(void))
		__MACHINEX64(unsigned char __vmx_vmptrld(unsigned __int64 *))
		__MACHINEX86_X64(void __vmx_vmptrst(unsigned __int64 *))
		__MACHINEX64(unsigned char __vmx_vmread(size_t, size_t *))
		__MACHINEX64(unsigned char __vmx_vmresume(void))
		__MACHINEX64(unsigned char __vmx_vmwrite(size_t, size_t))
		__MACHINEX86_X64(void __wbinvd(void))
		__MACHINEARM_ARM64(void __wfe(void))
		__MACHINEARM_ARM64(void __wfi(void))
		__MACHINEX64(void __writecr0(unsigned __int64))
		__MACHINEX86(void __writecr0(unsigned int))
		__MACHINEX64(void __writecr2(unsigned __int64))
		__MACHINEX86(void __writecr2(unsigned int))
		__MACHINEX64(void __writecr3(unsigned __int64))
		__MACHINEX86(void __writecr3(unsigned int))
		__MACHINEX64(void __writecr4(unsigned __int64))
		__MACHINEX86(void __writecr4(unsigned int))
		__MACHINEX64(void __writecr8(unsigned __int64))
		__MACHINEX86(void __writecr8(unsigned int))
		__MACHINEX64(void __writedr(unsigned int, unsigned __int64))
		__MACHINEX86(void __writedr(unsigned int, unsigned int))
		__MACHINEX64(void __writeeflags(unsigned __int64))
		__MACHINEX86(void __writeeflags(unsigned int))
		__MACHINEX86(void __writefsbyte(unsigned long, unsigned char))
		__MACHINEX86(void __writefsdword(unsigned long, unsigned long))
		__MACHINEX86(void __writefsqword(unsigned long, unsigned __int64))
		__MACHINEX86(void __writefsword(unsigned long, unsigned short))
		__MACHINEX64(void __writegsbyte(unsigned long, unsigned char))
		__MACHINEX64(void __writegsdword(unsigned long, unsigned long))
		__MACHINEX64(void __writegsqword(unsigned long, unsigned __int64))
		__MACHINEX64(void __writegsword(unsigned long, unsigned short))
		__MACHINEX86_X64(void __writemsr(unsigned long, unsigned __int64))
		__MACHINEARM64(void __writex18byte(unsigned long, unsigned char))
		__MACHINEARM64(void __writex18dword(unsigned long, unsigned long))
		__MACHINEARM64(void __writex18qword(unsigned long, unsigned __int64))
		__MACHINEARM64(void __writex18word(unsigned long, unsigned short))
		__MACHINEARM64_X64(unsigned char _bittest64(__int64 const *, __int64))
		__MACHINE(unsigned char _bittestandcomplement(long *, long))
		__MACHINEARM64_X64(unsigned char _bittestandcomplement64(__int64 *, __int64))
		__MACHINE(unsigned char _bittestandreset(long *, long))
		__MACHINEARM64_X64(unsigned char _bittestandreset64(__int64 *, __int64))
		__MACHINE(unsigned char _bittestandset(long *, long))
		__MACHINEARM64_X64(unsigned char _bittestandset64(__int64 *, __int64))
		__MACHINE(_Check_return_ unsigned __int64 __cdecl _byteswap_uint64(_In_ unsigned __int64))
		__MACHINE(_Check_return_ unsigned long __cdecl _byteswap_ulong(_In_ unsigned long))
		__MACHINE(_Check_return_ unsigned short __cdecl _byteswap_ushort(_In_ unsigned short))
		__MACHINE(void __cdecl _disable(void))
		__MACHINE(void __cdecl _enable(void))
		__MACHINE(unsigned char _interlockedbittestandreset(long volatile *, long))
		__MACHINEARM64_X64(unsigned char _interlockedbittestandreset64(__int64 volatile *, __int64))
		__MACHINEARM_ARM64(unsigned char _interlockedbittestandreset_acq(long volatile *, long))
		__MACHINEARM_ARM64(unsigned char _interlockedbittestandreset_nf(long volatile *, long))
		__MACHINEARM_ARM64(unsigned char _interlockedbittestandreset_rel(long volatile *, long))
		__MACHINEARM64(unsigned char _interlockedbittestandreset64_acq(__int64 volatile *, __int64))
		__MACHINEARM64(unsigned char _interlockedbittestandreset64_rel(__int64 volatile *, __int64))
		__MACHINEARM64(unsigned char _interlockedbittestandreset64_nf(__int64 volatile *, __int64))
		__MACHINEARM64_X64(unsigned char _interlockedbittestandset64(__int64 volatile *, __int64))
		__MACHINEARM64(unsigned char _interlockedbittestandset64_acq(__int64 volatile *, __int64))
		__MACHINEARM64(unsigned char _interlockedbittestandset64_rel(__int64 volatile *, __int64))
		__MACHINEARM64(unsigned char _interlockedbittestandset64_nf(__int64 volatile *, __int64))
		__MACHINEARM64(unsigned __int32 __crc32b(unsigned __int32, unsigned __int32))
		__MACHINEARM64(unsigned __int32 __crc32h(unsigned __int32, unsigned __int32))
		__MACHINEARM64(unsigned __int32 __crc32w(unsigned __int32, unsigned __int32))
		__MACHINEARM64(unsigned __int32 __crc32d(unsigned __int32, unsigned __int64))
		__MACHINEARM64(unsigned __int32 __crc32cb(unsigned __int32, unsigned __int32))
		__MACHINEARM64(unsigned __int32 __crc32ch(unsigned __int32, unsigned __int32))
		__MACHINEARM64(unsigned __int32 __crc32cw(unsigned __int32, unsigned __int32))
		__MACHINEARM64(unsigned __int32 __crc32cd(unsigned __int32, unsigned __int64))
		__MACHINEARM(int _isunordered(double, double))
		__MACHINEARM(int _isunorderedf(float, float))
		__MACHINE(_Check_return_ unsigned long __cdecl _lrotl(_In_ unsigned long, _In_ int))
		__MACHINE(_Check_return_ unsigned long __cdecl _lrotr(_In_ unsigned long, _In_ int))
		__MACHINEX86(void _m_empty(void))
		__MACHINEX86(void _m_femms(void))
		__MACHINEX86(__m64 _m_from_float(float))
		__MACHINEX86(__m64 _m_from_int(int))
		__MACHINEX86(void _m_maskmovq(__m64, __m64, char *))
		__MACHINEX86(__m64 _m_packssdw(__m64, __m64))
		__MACHINEX86(__m64 _m_packsswb(__m64, __m64))
		__MACHINEX86(__m64 _m_packuswb(__m64, __m64))
		__MACHINEX86(__m64 _m_paddb(__m64, __m64))
		__MACHINEX86(__m64 _m_paddd(__m64, __m64))
		__MACHINEX86(__m64 _m_paddsb(__m64, __m64))
		__MACHINEX86(__m64 _m_paddsw(__m64, __m64))
		__MACHINEX86(__m64 _m_paddusb(__m64, __m64))
		__MACHINEX86(__m64 _m_paddusw(__m64, __m64))
		__MACHINEX86(__m64 _m_paddw(__m64, __m64))
		__MACHINEX86(__m64 _m_pand(__m64, __m64))
		__MACHINEX86(__m64 _m_pandn(__m64, __m64))
		__MACHINEX86(__m64 _m_pavgb(__m64, __m64))
		__MACHINEX86(__m64 _m_pavgusb(__m64, __m64))
		__MACHINEX86(__m64 _m_pavgw(__m64, __m64))
		__MACHINEX86(__m64 _m_pcmpeqb(__m64, __m64))
		__MACHINEX86(__m64 _m_pcmpeqd(__m64, __m64))
		__MACHINEX86(__m64 _m_pcmpeqw(__m64, __m64))
		__MACHINEX86(__m64 _m_pcmpgtb(__m64, __m64))
		__MACHINEX86(__m64 _m_pcmpgtd(__m64, __m64))
		__MACHINEX86(__m64 _m_pcmpgtw(__m64, __m64))
		__MACHINEX86(int _m_pextrw(__m64, int))
		__MACHINEX86(__m64 _m_pf2id(__m64))
		__MACHINEX86(__m64 _m_pf2iw(__m64))
		__MACHINEX86(__m64 _m_pfacc(__m64, __m64))
		__MACHINEX86(__m64 _m_pfadd(__m64, __m64))
		__MACHINEX86(__m64 _m_pfcmpeq(__m64, __m64))
		__MACHINEX86(__m64 _m_pfcmpge(__m64, __m64))
		__MACHINEX86(__m64 _m_pfcmpgt(__m64, __m64))
		__MACHINEX86(__m64 _m_pfmax(__m64, __m64))
		__MACHINEX86(__m64 _m_pfmin(__m64, __m64))
		__MACHINEX86(__m64 _m_pfmul(__m64, __m64))
		__MACHINEX86(__m64 _m_pfnacc(__m64, __m64))
		__MACHINEX86(__m64 _m_pfpnacc(__m64, __m64))
		__MACHINEX86(__m64 _m_pfrcp(__m64))
		__MACHINEX86(__m64 _m_pfrcpit1(__m64, __m64))
		__MACHINEX86(__m64 _m_pfrcpit2(__m64, __m64))
		__MACHINEX86(__m64 _m_pfrsqit1(__m64, __m64))
		__MACHINEX86(__m64 _m_pfrsqrt(__m64))
		__MACHINEX86(__m64 _m_pfsub(__m64, __m64))
		__MACHINEX86(__m64 _m_pfsubr(__m64, __m64))
		__MACHINEX86(__m64 _m_pi2fd(__m64))
		__MACHINEX86(__m64 _m_pi2fw(__m64))
		__MACHINEX86(__m64 _m_pinsrw(__m64, int, int))
		__MACHINEX86(__m64 _m_pmaddwd(__m64, __m64))
		__MACHINEX86(__m64 _m_pmaxsw(__m64, __m64))
		__MACHINEX86(__m64 _m_pmaxub(__m64, __m64))
		__MACHINEX86(__m64 _m_pminsw(__m64, __m64))
		__MACHINEX86(__m64 _m_pminub(__m64, __m64))
		__MACHINEX86(int _m_pmovmskb(__m64))
		__MACHINEX86(__m64 _m_pmulhrw(__m64, __m64))
		__MACHINEX86(__m64 _m_pmulhuw(__m64, __m64))
		__MACHINEX86(__m64 _m_pmulhw(__m64, __m64))
		__MACHINEX86(__m64 _m_pmullw(__m64, __m64))
		__MACHINEX86(__m64 _m_por(__m64, __m64))
		__MACHINEX86_X64(void _m_prefetch(void *))
		__MACHINEX86_X64(void _m_prefetchw(volatile const void *))
		__MACHINEX86(__m64 _m_psadbw(__m64, __m64))
		__MACHINEX86(__m64 _m_pshufw(__m64, int))
		__MACHINEX86(__m64 _m_pslld(__m64, __m64))
		__MACHINEX86(__m64 _m_pslldi(__m64, int))
		__MACHINEX86(__m64 _m_psllq(__m64, __m64))
		__MACHINEX86(__m64 _m_psllqi(__m64, int))
		__MACHINEX86(__m64 _m_psllw(__m64, __m64))
		__MACHINEX86(__m64 _m_psllwi(__m64, int))
		__MACHINEX86(__m64 _m_psrad(__m64, __m64))
		__MACHINEX86(__m64 _m_psradi(__m64, int))
		__MACHINEX86(__m64 _m_psraw(__m64, __m64))
		__MACHINEX86(__m64 _m_psrawi(__m64, int))
		__MACHINEX86(__m64 _m_psrld(__m64, __m64))
		__MACHINEX86(__m64 _m_psrldi(__m64, int))
		__MACHINEX86(__m64 _m_psrlq(__m64, __m64))
		__MACHINEX86(__m64 _m_psrlqi(__m64, int))
		__MACHINEX86(__m64 _m_psrlw(__m64, __m64))
		__MACHINEX86(__m64 _m_psrlwi(__m64, int))
		__MACHINEX86(__m64 _m_psubb(__m64, __m64))
		__MACHINEX86(__m64 _m_psubd(__m64, __m64))
		__MACHINEX86(__m64 _m_psubsb(__m64, __m64))
		__MACHINEX86(__m64 _m_psubsw(__m64, __m64))
		__MACHINEX86(__m64 _m_psubusb(__m64, __m64))
		__MACHINEX86(__m64 _m_psubusw(__m64, __m64))
		__MACHINEX86(__m64 _m_psubw(__m64, __m64))
		__MACHINEX86(__m64 _m_pswapd(__m64))
		__MACHINEX86(__m64 _m_punpckhbw(__m64, __m64))
		__MACHINEX86(__m64 _m_punpckhdq(__m64, __m64))
		__MACHINEX86(__m64 _m_punpckhwd(__m64, __m64))
		__MACHINEX86(__m64 _m_punpcklbw(__m64, __m64))
		__MACHINEX86(__m64 _m_punpckldq(__m64, __m64))
		__MACHINEX86(__m64 _m_punpcklwd(__m64, __m64))
		__MACHINEX86(__m64 _m_pxor(__m64, __m64))
		__MACHINEX86(float _m_to_float(__m64))
		__MACHINEX86(int _m_to_int(__m64))
		__MACHINEX86_X64(__m128i _mm_abs_epi16(__m128i))
		__MACHINEX86_X64(__m128i _mm_abs_epi32(__m128i))
		__MACHINEX86_X64(__m128i _mm_abs_epi8(__m128i))
		__MACHINEX86(__m64 _mm_abs_pi16(__m64))
		__MACHINEX86(__m64 _mm_abs_pi32(__m64))
		__MACHINEX86(__m64 _mm_abs_pi8(__m64))
		__MACHINEX86_X64(__m128i _mm_add_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_add_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_add_epi64(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_add_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_add_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_add_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_add_sd(__m128d, __m128d))
		__MACHINEX86(__m64 _mm_add_si64(__m64, __m64))
		__MACHINEX86_X64(__m128 _mm_add_ss(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_adds_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_adds_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_adds_epu16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_adds_epu8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_addsub_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_addsub_ps(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_alignr_epi8(__m128i, __m128i, int))
		__MACHINEX86(__m64 _mm_alignr_pi8(__m64, __m64, int))
		__MACHINEX86_X64(__m128d _mm_and_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_and_ps(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_and_si128(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_andnot_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_andnot_ps(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_andnot_si128(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_avg_epu16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_avg_epu8(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_blend_epi16(__m128i, __m128i, int))
		__MACHINEX86_X64(__m128d _mm_blend_pd(__m128d, __m128d, int))
		__MACHINEX86_X64(__m128 _mm_blend_ps(__m128, __m128, int))
		__MACHINEX86_X64(__m128i _mm_blendv_epi8(__m128i, __m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_blendv_pd(__m128d, __m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_blendv_ps(__m128, __m128, __m128))
		__MACHINEX86_X64(void _mm_clflush(void const *))
		__MACHINEX86_X64(void _mm_clflushopt(void const *))
		__MACHINEX86_X64(void _mm_clwb(void const *))
		__MACHINEX86_X64(void _mm_clzero(void const *))
		__MACHINEX86_X64(__m128i _mm_cmpeq_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_cmpeq_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_cmpeq_epi64(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_cmpeq_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_cmpeq_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpeq_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpeq_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpeq_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_cmpestra(__m128i, int, __m128i, int, int))
		__MACHINEX86_X64(int _mm_cmpestrc(__m128i, int, __m128i, int, int))
		__MACHINEX86_X64(int _mm_cmpestri(__m128i, int, __m128i, int, int))
		__MACHINEX86_X64(__m128i _mm_cmpestrm(__m128i, int, __m128i, int, int))
		__MACHINEX86_X64(int _mm_cmpestro(__m128i, int, __m128i, int, int))
		__MACHINEX86_X64(int _mm_cmpestrs(__m128i, int, __m128i, int, int))
		__MACHINEX86_X64(int _mm_cmpestrz(__m128i, int, __m128i, int, int))
		__MACHINEX86_X64(__m128d _mm_cmpge_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpge_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpge_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpge_ss(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_cmpgt_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_cmpgt_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_cmpgt_epi64(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_cmpgt_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_cmpgt_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpgt_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpgt_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpgt_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_cmpistra(__m128i, __m128i, int))
		__MACHINEX86_X64(int _mm_cmpistrc(__m128i, __m128i, int))
		__MACHINEX86_X64(int _mm_cmpistri(__m128i, __m128i, int))
		__MACHINEX86_X64(__m128i _mm_cmpistrm(__m128i, __m128i, int))
		__MACHINEX86_X64(int _mm_cmpistro(__m128i, __m128i, int))
		__MACHINEX86_X64(int _mm_cmpistrs(__m128i, __m128i, int))
		__MACHINEX86_X64(int _mm_cmpistrz(__m128i, __m128i, int))
		__MACHINEX86_X64(__m128d _mm_cmple_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmple_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmple_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmple_ss(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_cmplt_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_cmplt_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_cmplt_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_cmplt_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmplt_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmplt_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmplt_ss(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpneq_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpneq_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpneq_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpneq_ss(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpnge_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpnge_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpnge_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpnge_ss(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpngt_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpngt_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpngt_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpngt_ss(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpnle_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpnle_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpnle_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpnle_ss(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpnlt_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpnlt_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpnlt_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpnlt_ss(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpord_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpord_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpord_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpord_ss(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpunord_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpunord_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_cmpunord_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_cmpunord_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_comieq_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_comieq_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_comige_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_comige_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_comigt_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_comigt_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_comile_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_comile_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_comilt_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_comilt_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_comineq_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_comineq_ss(__m128, __m128))
		__MACHINEX86_X64(unsigned int _mm_crc32_u16(unsigned int, unsigned short))
		__MACHINEX86_X64(unsigned int _mm_crc32_u32(unsigned int, unsigned int))
		__MACHINEX64(unsigned __int64 _mm_crc32_u64(unsigned __int64, unsigned __int64))
		__MACHINEX86_X64(unsigned int _mm_crc32_u8(unsigned int, unsigned char))
		__MACHINEX86(__m128 _mm_cvt_pi2ps(__m128, __m64))
		__MACHINEX86(__m64 _mm_cvt_ps2pi(__m128))
		__MACHINEX86_X64(__m128 _mm_cvt_si2ss(__m128, int))
		__MACHINEX86_X64(int _mm_cvt_ss2si(__m128))
		__MACHINEX86_X64(__m128i _mm_cvtepi16_epi32(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepi16_epi64(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepi32_epi64(__m128i))
		__MACHINEX86_X64(__m128d _mm_cvtepi32_pd(__m128i))
		__MACHINEX86_X64(__m128 _mm_cvtepi32_ps(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepi8_epi16(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepi8_epi32(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepi8_epi64(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepu16_epi32(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepu16_epi64(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepu32_epi64(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepu8_epi16(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepu8_epi32(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtepu8_epi64(__m128i))
		__MACHINEX86_X64(__m128i _mm_cvtpd_epi32(__m128d))
		__MACHINEX86(__m64 _mm_cvtpd_pi32(__m128d))
		__MACHINEX86_X64(__m128 _mm_cvtpd_ps(__m128d))
		__MACHINEX86(__m128d _mm_cvtpi32_pd(__m64))
		__MACHINEX86_X64(__m128i _mm_cvtps_epi32(__m128))
		__MACHINEX86_X64(__m128d _mm_cvtps_pd(__m128))
		__MACHINEX86_X64(int _mm_cvtsd_si32(__m128d))
		__MACHINEX64(__int64 _mm_cvtsd_si64(__m128d))
		__MACHINEX64(__int64 _mm_cvtsd_si64x(__m128d))
		__MACHINEX86_X64(__m128 _mm_cvtsd_ss(__m128, __m128d))
		__MACHINEX86_X64(int _mm_cvtsi128_si32(__m128i))
		__MACHINEX64(__int64 _mm_cvtsi128_si64(__m128i))
		__MACHINEX64(__int64 _mm_cvtsi128_si64x(__m128i))
		__MACHINEX86_X64(__m128d _mm_cvtsi32_sd(__m128d, int))
		__MACHINEX86_X64(__m128i _mm_cvtsi32_si128(int))
		__MACHINEX64(__m128d _mm_cvtsi64_sd(__m128d, __int64))
		__MACHINEX64(__m128i _mm_cvtsi64_si128(__int64))
		__MACHINEX64(__m128 _mm_cvtsi64_ss(__m128, __int64))
		__MACHINEX64(__m128d _mm_cvtsi64x_sd(__m128d, __int64))
		__MACHINEX64(__m128i _mm_cvtsi64x_si128(__int64))
		__MACHINEX64(__m128 _mm_cvtsi64x_ss(__m128, __int64))
		__MACHINEX86_X64(__m128d _mm_cvtss_sd(__m128d, __m128))
		__MACHINEX64(__int64 _mm_cvtss_si64(__m128))
		__MACHINEX64(__int64 _mm_cvtss_si64x(__m128))
		__MACHINEX86(__m64 _mm_cvtt_ps2pi(__m128))
		__MACHINEX86_X64(int _mm_cvtt_ss2si(__m128))
		__MACHINEX86_X64(__m128i _mm_cvttpd_epi32(__m128d))
		__MACHINEX86(__m64 _mm_cvttpd_pi32(__m128d))
		__MACHINEX86_X64(__m128i _mm_cvttps_epi32(__m128))
		__MACHINEX86_X64(int _mm_cvttsd_si32(__m128d))
		__MACHINEX64(__int64 _mm_cvttsd_si64(__m128d))
		__MACHINEX64(__int64 _mm_cvttsd_si64x(__m128d))
		__MACHINEX64(__int64 _mm_cvttss_si64(__m128))
		__MACHINEX64(__int64 _mm_cvttss_si64x(__m128))
		__MACHINEX86_X64(__m128d _mm_div_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_div_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_div_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_div_ss(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_dp_pd(__m128d, __m128d, int))
		__MACHINEX86_X64(__m128 _mm_dp_ps(__m128, __m128, int))
		__MACHINEX86_X64(int _mm_extract_epi16(__m128i, int))
		__MACHINEX86_X64(int _mm_extract_epi32(__m128i, int))
		__MACHINEX64(__int64 _mm_extract_epi64(__m128i, int))
		__MACHINEX86_X64(int _mm_extract_epi8(__m128i, int))
		__MACHINEX86_X64(int _mm_extract_ps(__m128, int))
		__MACHINEX86_X64(__m128i _mm_extract_si64(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_extracti_si64(__m128i, int, int))
		__MACHINEX86_X64(unsigned int _mm_getcsr(void))
		__MACHINEX86_X64(__m128i _mm_hadd_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_hadd_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_hadd_pd(__m128d, __m128d))
		__MACHINEX86(__m64 _mm_hadd_pi16(__m64, __m64))
		__MACHINEX86(__m64 _mm_hadd_pi32(__m64, __m64))
		__MACHINEX86_X64(__m128 _mm_hadd_ps(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_hadds_epi16(__m128i, __m128i))
		__MACHINEX86(__m64 _mm_hadds_pi16(__m64, __m64))
		__MACHINEX86_X64(__m128i _mm_hsub_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_hsub_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_hsub_pd(__m128d, __m128d))
		__MACHINEX86(__m64 _mm_hsub_pi16(__m64, __m64))
		__MACHINEX86(__m64 _mm_hsub_pi32(__m64, __m64))
		__MACHINEX86_X64(__m128 _mm_hsub_ps(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_hsubs_epi16(__m128i, __m128i))
		__MACHINEX86(__m64 _mm_hsubs_pi16(__m64, __m64))
		__MACHINEX86_X64(__m128i _mm_insert_epi16(__m128i, int, int))
		__MACHINEX86_X64(__m128i _mm_insert_epi32(__m128i, int, int))
		__MACHINEX64(__m128i _mm_insert_epi64(__m128i, __int64, int))
		__MACHINEX86_X64(__m128i _mm_insert_epi8(__m128i, int, int))
		__MACHINEX86_X64(__m128 _mm_insert_ps(__m128, __m128, int))
		__MACHINEX86_X64(__m128i _mm_insert_si64(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_inserti_si64(__m128i, __m128i, int, int))
		__MACHINEX86_X64(__m128i _mm_lddqu_si128(__m128i const *))
		__MACHINEX86_X64(void _mm_lfence(void))
		__MACHINEX86_X64(__m128d _mm_load1_pd(double const *))
		__MACHINEX86_X64(__m128d _mm_load_pd(double const *))
		__MACHINEX86_X64(__m128 _mm_load_ps(float const *))
		__MACHINEX86_X64(__m128 _mm_load_ps1(float const *))
		__MACHINEX86_X64(__m128d _mm_load_sd(double const *))
		__MACHINEX86_X64(__m128i _mm_load_si128(__m128i const *))
		__MACHINEX86_X64(__m128 _mm_load_ss(float const *))
		__MACHINEX86_X64(__m128d _mm_loaddup_pd(double const *))
		__MACHINEX86_X64(__m128d _mm_loadh_pd(__m128d, double const *))
		__MACHINEX86_X64(__m128 _mm_loadh_pi(__m128, __m64 const *))
		__MACHINEX86_X64(__m128i _mm_loadl_epi64(__m128i const *))
		__MACHINEX86_X64(__m128d _mm_loadl_pd(__m128d, double const *))
		__MACHINEX86_X64(__m128 _mm_loadl_pi(__m128, __m64 const *))
		__MACHINEX86_X64(__m128d _mm_loadr_pd(double const *))
		__MACHINEX86_X64(__m128 _mm_loadr_ps(float const *))
		__MACHINEX86_X64(__m128d _mm_loadu_pd(double const *))
		__MACHINEX86_X64(__m128 _mm_loadu_ps(float const *))
		__MACHINEX86_X64(__m128i _mm_loadu_si128(__m128i const *))
		__MACHINEX86_X64(__m128i _mm_madd_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_maddubs_epi16(__m128i, __m128i))
		__MACHINEX86(__m64 _mm_maddubs_pi16(__m64, __m64))
		__MACHINEX86_X64(void _mm_maskmoveu_si128(__m128i, __m128i, char *))
		__MACHINEX86_X64(__m128i _mm_max_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_max_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_max_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_max_epu16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_max_epu32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_max_epu8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_max_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_max_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_max_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_max_ss(__m128, __m128))
		__MACHINEX86_X64(void _mm_mfence(void))
		__MACHINEX86_X64(__m128i _mm_min_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_min_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_min_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_min_epu16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_min_epu32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_min_epu8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_min_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_min_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_min_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_min_ss(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_minpos_epu16(__m128i))
		__MACHINEX86_X64(void _mm_monitor(void const *, unsigned int, unsigned int))
		__MACHINEX86_X64(__m128i _mm_move_epi64(__m128i))
		__MACHINEX86_X64(__m128d _mm_move_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_move_ss(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_movedup_pd(__m128d))
		__MACHINEX86_X64(__m128 _mm_movehdup_ps(__m128))
		__MACHINEX86_X64(__m128 _mm_movehl_ps(__m128, __m128))
		__MACHINEX86_X64(__m128 _mm_moveldup_ps(__m128))
		__MACHINEX86_X64(__m128 _mm_movelh_ps(__m128, __m128))
		__MACHINEX86_X64(int _mm_movemask_epi8(__m128i))
		__MACHINEX86_X64(int _mm_movemask_pd(__m128d))
		__MACHINEX86_X64(int _mm_movemask_ps(__m128))
		__MACHINEX86(__m64 _mm_movepi64_pi64(__m128i))
		__MACHINEX86(__m128i _mm_movpi64_epi64(__m64))
		__MACHINEX86_X64(__m128i _mm_mpsadbw_epu8(__m128i, __m128i, int))
		__MACHINEX86_X64(__m128i _mm_mul_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_mul_epu32(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_mul_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_mul_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_mul_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_mul_ss(__m128, __m128))
		__MACHINEX86(__m64 _mm_mul_su32(__m64, __m64))
		__MACHINEX86_X64(__m128i _mm_mulhi_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_mulhi_epu16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_mulhrs_epi16(__m128i, __m128i))
		__MACHINEX86(__m64 _mm_mulhrs_pi16(__m64, __m64))
		__MACHINEX86_X64(__m128i _mm_mullo_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_mullo_epi32(__m128i, __m128i))
		__MACHINEX86_X64(void _mm_mwait(unsigned int, unsigned int))
		__MACHINEX86_X64(__m128d _mm_or_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_or_ps(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_or_si128(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_packs_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_packs_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_packus_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_packus_epi32(__m128i, __m128i))
		__MACHINEX86_X64(void _mm_pause(void))
		__MACHINEX86_X64(int _mm_popcnt_u32(unsigned int))
		__MACHINEX64(__int64 _mm_popcnt_u64(unsigned __int64))
		__MACHINEX86_X64(void _mm_prefetch(char const *, int))
		__MACHINEX86_X64(__m128 _mm_rcp_ps(__m128))
		__MACHINEX86_X64(__m128 _mm_rcp_ss(__m128))
		__MACHINEX86_X64(__m128d _mm_round_pd(__m128d, int))
		__MACHINEX86_X64(__m128 _mm_round_ps(__m128, int))
		__MACHINEX86_X64(__m128d _mm_round_sd(__m128d, __m128d, int))
		__MACHINEX86_X64(__m128 _mm_round_ss(__m128, __m128, int))
		__MACHINEX86_X64(__m128 _mm_rsqrt_ps(__m128))
		__MACHINEX86_X64(__m128 _mm_rsqrt_ss(__m128))
		__MACHINEX86_X64(__m128i _mm_sad_epu8(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_set1_epi16(short))
		__MACHINEX86_X64(__m128i _mm_set1_epi32(int))
		__MACHINEX86(__m128i _mm_set1_epi64(__m64))
		__MACHINEX86_X64(__m128i _mm_set1_epi64x(__int64))
		__MACHINEX86_X64(__m128i _mm_set1_epi8(char))
		__MACHINEX86_X64(__m128d _mm_set1_pd(double))
		__MACHINEX86(__m64 _mm_set1_pi16(short))
		__MACHINEX86(__m64 _mm_set1_pi32(int))
		__MACHINEX86(__m64 _mm_set1_pi8(char))
		__MACHINEX86_X64(__m128i _mm_set_epi16(short, short, short, short, short, short, short, short))
		__MACHINEX86_X64(__m128i _mm_set_epi32(int, int, int, int))
		__MACHINEX86(__m128i _mm_set_epi64(__m64, __m64))
		__MACHINEX86_X64(__m128i _mm_set_epi64x(__int64, __int64))
		__MACHINEX86_X64(__m128i _mm_set_epi8(char, char, char, char, char, char, char, char, char, char, char, char, char, char, char, char))
		__MACHINEX86_X64(__m128d _mm_set_pd(double, double))
		__MACHINEX86(__m64 _mm_set_pi16(short, short, short, short))
		__MACHINEX86(__m64 _mm_set_pi32(int, int))
		__MACHINEX86(__m64 _mm_set_pi8(char, char, char, char, char, char, char, char))
		__MACHINEX86_X64(__m128 _mm_set_ps(float, float, float, float))
		__MACHINEX86_X64(__m128 _mm_set_ps1(float))
		__MACHINEX86_X64(__m128d _mm_set_sd(double))
		__MACHINEX86_X64(__m128 _mm_set_ss(float))
		__MACHINEX86_X64(void _mm_setcsr(unsigned int))
		__MACHINEX86_X64(__m128i _mm_setl_epi64(__m128i))
		__MACHINEX86_X64(__m128i _mm_setr_epi16(short, short, short, short, short, short, short, short))
		__MACHINEX86_X64(__m128i _mm_setr_epi32(int, int, int, int))
		__MACHINEX86(__m128i _mm_setr_epi64(__m64, __m64))
		__MACHINEX86_X64(__m128i _mm_setr_epi64x(__int64, __int64))
		__MACHINEX86_X64(__m128i _mm_setr_epi8(char, char, char, char, char, char, char, char, char, char, char, char, char, char, char, char))
		__MACHINEX86_X64(__m128d _mm_setr_pd(double, double))
		__MACHINEX86(__m64 _mm_setr_pi16(short, short, short, short))
		__MACHINEX86(__m64 _mm_setr_pi32(int, int))
		__MACHINEX86(__m64 _mm_setr_pi8(char, char, char, char, char, char, char, char))
		__MACHINEX86_X64(__m128 _mm_setr_ps(float, float, float, float))
		__MACHINEX86_X64(__m128d _mm_setzero_pd(void))
		__MACHINEX86_X64(__m128 _mm_setzero_ps(void))
		__MACHINEX86_X64(__m128i _mm_setzero_si128(void))
		__MACHINEX86(__m64 _mm_setzero_si64(void))
		__MACHINEX86_X64(void _mm_sfence(void))
		__MACHINEX86_X64(__m128i _mm_shuffle_epi32(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_shuffle_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_shuffle_pd(__m128d, __m128d, int))
		__MACHINEX86(__m64 _mm_shuffle_pi8(__m64, __m64))
		__MACHINEX86_X64(__m128 _mm_shuffle_ps(__m128, __m128, unsigned int))
		__MACHINEX86_X64(__m128i _mm_shufflehi_epi16(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_shufflelo_epi16(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_sign_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_sign_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_sign_epi8(__m128i, __m128i))
		__MACHINEX86(__m64 _mm_sign_pi16(__m64, __m64))
		__MACHINEX86(__m64 _mm_sign_pi32(__m64, __m64))
		__MACHINEX86(__m64 _mm_sign_pi8(__m64, __m64))
		__MACHINEX86_X64(__m128i _mm_sll_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_sll_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_sll_epi64(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_slli_epi16(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_slli_epi32(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_slli_epi64(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_slli_si128(__m128i, int))
		__MACHINEX86_X64(__m128d _mm_sqrt_pd(__m128d))
		__MACHINEX86_X64(__m128 _mm_sqrt_ps(__m128))
		__MACHINEX86_X64(__m128d _mm_sqrt_sd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_sqrt_ss(__m128))
		__MACHINEX86_X64(__m128i _mm_sra_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_sra_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_srai_epi16(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_srai_epi32(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_srl_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_srl_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_srl_epi64(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_srli_epi16(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_srli_epi32(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_srli_epi64(__m128i, int))
		__MACHINEX86_X64(__m128i _mm_srli_si128(__m128i, int))
		__MACHINEX86_X64(void _mm_store1_pd(double *, __m128d))
		__MACHINEX86_X64(void _mm_store_pd(double *, __m128d))
		__MACHINEX86_X64(void _mm_store_ps(float *, __m128))
		__MACHINEX86_X64(void _mm_store_ps1(float *, __m128))
		__MACHINEX86_X64(void _mm_store_sd(double *, __m128d))
		__MACHINEX86_X64(void _mm_store_si128(__m128i *, __m128i))
		__MACHINEX86_X64(void _mm_store_ss(float *, __m128))
		__MACHINEX86_X64(void _mm_storeh_pd(double *, __m128d))
		__MACHINEX86_X64(void _mm_storeh_pi(__m64 *, __m128))
		__MACHINEX86_X64(void _mm_storel_epi64(__m128i *, __m128i))
		__MACHINEX86_X64(void _mm_storel_pd(double *, __m128d))
		__MACHINEX86_X64(void _mm_storel_pi(__m64 *, __m128))
		__MACHINEX86_X64(void _mm_storer_pd(double *, __m128d))
		__MACHINEX86_X64(void _mm_storer_ps(float *, __m128))
		__MACHINEX86_X64(void _mm_storeu_pd(double *, __m128d))
		__MACHINEX86_X64(void _mm_storeu_ps(float *, __m128))
		__MACHINEX86_X64(void _mm_storeu_si128(__m128i *, __m128i))
#if defined(_CRT_WINDOWS) || defined(BUILD_WINDOWS)
		__MACHINEX86_X64(__m128i _mm_stream_load_si128(__m128i *))
#else
		__MACHINEX86_X64(__m128i _mm_stream_load_si128(const __m128i *))
#endif
		__MACHINEX86_X64(void _mm_stream_pd(double *, __m128d))
		__MACHINEX86(void _mm_stream_pi(__m64 *, __m64))
		__MACHINEX86_X64(void _mm_stream_ps(float *, __m128))
		__MACHINEX86_X64(void _mm_stream_sd(double *, __m128d))
		__MACHINEX86_X64(void _mm_stream_si128(__m128i *, __m128i))
		__MACHINEX86_X64(void _mm_stream_si32(int *, int))
		__MACHINEX64(void _mm_stream_si64x(__int64 *, __int64))
		__MACHINEX86_X64(void _mm_stream_ss(float *, __m128))
		__MACHINEX86_X64(__m128i _mm_sub_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_sub_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_sub_epi64(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_sub_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_sub_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_sub_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_sub_sd(__m128d, __m128d))
		__MACHINEX86(__m64 _mm_sub_si64(__m64, __m64))
		__MACHINEX86_X64(__m128 _mm_sub_ss(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_subs_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_subs_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_subs_epu16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_subs_epu8(__m128i, __m128i))
		__MACHINEX86_X64(int _mm_testc_si128(__m128i, __m128i))
		__MACHINEX86_X64(int _mm_testnzc_si128(__m128i, __m128i))
		__MACHINEX86_X64(int _mm_testz_si128(__m128i, __m128i))
		__MACHINEX86_X64(int _mm_ucomieq_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_ucomieq_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_ucomige_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_ucomige_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_ucomigt_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_ucomigt_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_ucomile_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_ucomile_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_ucomilt_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_ucomilt_ss(__m128, __m128))
		__MACHINEX86_X64(int _mm_ucomineq_sd(__m128d, __m128d))
		__MACHINEX86_X64(int _mm_ucomineq_ss(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_unpackhi_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_unpackhi_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_unpackhi_epi64(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_unpackhi_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_unpackhi_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_unpackhi_ps(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_unpacklo_epi16(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_unpacklo_epi32(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_unpacklo_epi64(__m128i, __m128i))
		__MACHINEX86_X64(__m128i _mm_unpacklo_epi8(__m128i, __m128i))
		__MACHINEX86_X64(__m128d _mm_unpacklo_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_unpacklo_ps(__m128, __m128))
		__MACHINEX86_X64(__m128d _mm_xor_pd(__m128d, __m128d))
		__MACHINEX86_X64(__m128 _mm_xor_ps(__m128, __m128))
		__MACHINEX86_X64(__m128i _mm_xor_si128(__m128i, __m128i))
		__MACHINEX64(__int64 _mul128(__int64 _Multiplier, __int64 _Multiplicand, __int64 * _HighProduct))
		__MACHINE(unsigned int __cdecl _rotl(_In_ unsigned int _Value, _In_ int _Shift))
		__MACHINE(unsigned short __cdecl _rotl16(unsigned short _Value, unsigned char _Shift))
		__MACHINE(unsigned __int64 __cdecl _rotl64(_In_ unsigned __int64 _Value, _In_ int _Shift))
		__MACHINE(unsigned char __cdecl _rotl8(unsigned char _Value, unsigned char _Shift))
		__MACHINE(unsigned int __cdecl _rotr(_In_ unsigned int _Value, _In_ int _Shift))
		__MACHINE(unsigned short __cdecl _rotr16(unsigned short _Value, unsigned char _Shift))
		__MACHINE(unsigned __int64 __cdecl _rotr64(_In_ unsigned __int64 _Value, _In_ int _Shift))
		__MACHINE(unsigned char __cdecl _rotr8(unsigned char _Value, unsigned char _Shift))
		__MACHINE(int __cdecl _setjmp(jmp_buf))
		__MACHINEARM_ARM64_X64(int __cdecl _setjmpex(jmp_buf))
		__MACHINEX64(unsigned __int64 _umul128(unsigned __int64 _Multiplier, unsigned __int64 _Multiplicand, unsigned __int64 * _HighProduct))
		__MACHINEX86_X64(void _rsm(void))
		__MACHINEX86_X64(void _lgdt(void *))
		__MACHINEX86_X64(void _sgdt(void *))
		__MACHINEX86_X64(void _clac(void))
		__MACHINEX86_X64(void _stac(void))
		__MACHINEX86_X64(unsigned char __cdecl _addcarry_u8(unsigned char, unsigned char, unsigned char, unsigned char *))
		__MACHINEX86_X64(unsigned char __cdecl _subborrow_u8(unsigned char, unsigned char, unsigned char, unsigned char *))
		__MACHINEX86_X64(unsigned char __cdecl _addcarry_u16(unsigned char, unsigned short, unsigned short, unsigned short *))
		__MACHINEX86_X64(unsigned char __cdecl _subborrow_u16(unsigned char, unsigned short, unsigned short, unsigned short *))
		__MACHINEX86_X64(unsigned char __cdecl _addcarry_u32(unsigned char, unsigned int, unsigned int, unsigned int *))
		__MACHINEX86_X64(unsigned char __cdecl _subborrow_u32(unsigned char, unsigned int, unsigned int, unsigned int *))
		__MACHINEX64(unsigned char __cdecl _addcarry_u64(unsigned char, unsigned __int64, unsigned __int64, unsigned __int64 *))
		__MACHINEX64(unsigned char __cdecl _subborrow_u64(unsigned char, unsigned __int64, unsigned __int64, unsigned __int64 *))
		__MACHINEX86_X64(void _mm_monitorx(void const *, unsigned int, unsigned int))
		__MACHINEX86_X64(void _mm_mwaitx(unsigned int, unsigned int, unsigned int))

#if defined (__cplusplus)
}
#endif  /* defined (__cplusplus) */
#endif  /* __midl */
#endif  /* RC_INVOKED */



void Memory::pattern::EnsureMatches( int maxCount ) {

	if ( m_matched ) {
		return;
	}

	// Scan the executable for code
	static executable_meta executable;

	executable.EnsureInit();

	// Check if SSE 4.2 is supported
	int cpuid[4];
	__cpuid( cpuid, 0 );

	bool sse42 = false;

	if ( m_mask.size() <= 16 ) {

		if ( cpuid[0] >= 1 ) {

			__cpuidex( cpuid, 1, 0 );

			sse42 = ( cpuid[2] & ( 1 << 20 ) ) == 1;
		}
	}

	auto matchSuccess = [&]( uintptr_t address ) {

		g_hints.insert( std::make_pair( m_hash, address ) );

		return ( m_matches.size() == maxCount );
	};

	LARGE_INTEGER ticks;
	QueryPerformanceCounter( &ticks );

	static std::string d, m;
	Memory::TransformPattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC 50 48 8B EA 4C 8B FA 48 8B D9 4D 85 C9", d, m);


	uint64_t startTicksOld = ticks.QuadPart;

	if ( !sse42 ) {

		for ( uintptr_t i = executable.begin(); i <= executable.end(); i++ ) {

			if ( ConsiderMatch( i ) ) {

				if ( matchSuccess( i ) ) {
					break;
				}
			}
		}
	} else {

		__declspec( align( 16 ) ) char desiredMask[16] = { 0 };

		for ( int i = 0; i < m_mask.size(); i++ ) {
			desiredMask[i / 8] |= ( ( m_mask[i] == '?' ) ? 0 : 1 ) << ( i % 8 );
		}

		__m128i mask = _mm_load_si128( reinterpret_cast<const __m128i*>( desiredMask ) );
		__m128i comparand = _mm_loadu_si128( reinterpret_cast<const __m128i*>( m_bytes.c_str() ) );

		for ( uintptr_t i = executable.begin(); i <= executable.end(); i++ ) {

			__m128i value = _mm_loadu_si128( reinterpret_cast<const __m128i*>( i ) );
			__m128i result = _mm_cmpestrm( value, 16, comparand, (int)m_bytes.size(), _SIDD_CMP_EQUAL_EACH );

			// As the result can match more bits than the mask contains
			__m128i matches = _mm_and_si128( mask, result );
			__m128i equivalence = _mm_xor_si128( mask, matches );

			if ( _mm_test_all_zeros( equivalence, equivalence ) ) {

				m_matches.push_back( pattern_match( (void*)i ) );

				if ( matchSuccess( i ) ) {
					break;
				}
			}
		}
	}

	m_matched = true;
}

void Memory::pattern::hint( uint64_t hash, uintptr_t address ) {

	auto range = g_hints.equal_range( hash );

	for ( auto it = range.first; it != range.second; it++ ) {

		if ( it->second == address ) {
			return;
		}
	}

	g_hints.insert( std::make_pair( hash, address ) );
}
