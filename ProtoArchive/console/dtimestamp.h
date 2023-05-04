/*---------------------------------------------------------------------------

@file dtimestamp.h

@addtogroup debug tools

@brief debug macro: DEBUG_MAKE_TIMESTAMP, etc...

@details

      TDTimestamp - write to file processor tick state and position of
                      processed DEBUG_MAKE_TIMESTAMP in source code

                    Filename is GetModuleFileName + ".timestamp"


Output file format:
      ProcessorTick(musec)  TimeDelta(musec)    ThreadId  [File]  Func  Line


Using:
      DEBUG_MAKE_TIMESTAMP;

WARNINGS:
      None

HISTORY:
      24.08.2005   Roman S Samarev   Created

System-dependent parts: Win.
---------------------------------------------------------------------------*/

#ifndef _DTS_LOG_H_
#define _DTS_LOG_H_

#ifdef DEBUG_TIMESTAMP_ENABLE

#include <stdio.h>
#include <stdlib.h>
#ifdef __GNUC__
  #define DT_INT64 long long
#else
  #define DT_INT64 __int64
#endif

#if (defined(SPBACKUP_EXPORTS) || defined( _LIB))
    #define __export __declspec( dllexport )
#else
    #define __export
#endif

class __export TDTimestamp
{
  public:
    TDTimestamp();
    ~TDTimestamp();

    int Write(const char * format, ...);
		void Reset();

  protected:
    FILE      *m_file;
    char      *m_pszFileName;
    char      *m_pszCloseString;
    DT_INT64   m_PrevTime;

    void Init( char * a, bool bNewFile = false,
               char * szInitString  = 0,
               char * szCloseString = 0 );

};

#ifdef DTIMESTAMP_IMPLEMENTATION
   __export TDTimestamp _DTimeStamp;
#else   
   #if (defined(SPBACKUP_EXPORTS) || defined( _LIB) || defined (__GNUC__))
      extern TDTimestamp _DTimeStamp;
   #else
      __declspec( dllimport ) TDTimestamp _DTimeStamp;
   #endif
#endif

#if !defined( __BORLANDC__ ) && !defined( __GNUC__ )
  #define __FUNC__ __FUNCTION__
#endif

#ifdef  __GNUC__ 
  #define __FUNC__ __PRETTY_FUNCTION__
#endif

#ifdef DEBUG_TIMESTAMP_ENABLE_SRCFILE
  #define DEBUG_MAKE_TIMESTAMP \
   _DTimeStamp.Write( "%s\t%s\t%d", __FILE__, __FUNC__, __LINE__ );
#else
  #define DEBUG_MAKE_TIMESTAMP \
   _DTimeStamp.Write( "%s\t%d", __FUNC__, __LINE__ );
#endif

  #define DEBUG_TS_WRITE _DTimeStamp.Write
#else
  #define DEBUG_MAKE_TIMESTAMP

  inline int _DTimeStamp_Write_Stub(const char * format, ...){ return 0; };
  #define DEBUG_TS_WRITE _DTimeStamp_Write_Stub
#endif
#endif
