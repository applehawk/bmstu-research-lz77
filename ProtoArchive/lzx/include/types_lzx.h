#ifndef __TYPES_LZX_H__
#define __TYPES_LZX_H__

typedef unsigned char uint8;
typedef unsigned long uint32;
typedef unsigned short uint16;
typedef signed char int8;
typedef signed long int32;
typedef signed short int16;
//ТИПЫ ДЛЯ MSVC компилятора(для других нужно прописывать)
#ifdef _MSC_VER
typedef unsigned __int64 uint64;
typedef signed __int64 int64;
#endif

#ifdef __GNUC__
typedef unsigned long long uint64;
typedef signed long long int64;
#endif

#endif
