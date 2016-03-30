#ifndef __CIM2GDI_DATATYPES_HPP__
#define __CIM2GDI_DATATYPES_HPP__

#if defined ( CIM_PLATFORM_WINDOWS )
#if defined ( CIM_ARCH_X86 )
#if defined ( CIM_BITSIZE_32 )
#include <DataTypes_Windows_x86_32.hpp>
#else
#error Unknown Windows platform
#endif // CIM_BITSIZE_32
#else
#error Unknown Windows architecture
#endif // CIM_ARCH_X86
#else
#error Unknown platform
#endif // CIM_PLATFORM_WINDOWS

#endif // __CIM2GDI_DATATYPES_HPP__
