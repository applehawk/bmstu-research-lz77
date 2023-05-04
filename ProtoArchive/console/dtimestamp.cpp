/**--------------------------------------------------------------------------

@file dtimestamp.cpp

@addtogroup debug tools

@brief debug macro: DEBUG_MAKE_TIMESTAMP, etc...

@details

      TDTimestamp - write to file processor tick state and position of
                      processed DEBUG_MAKE_TIMESTAMP in source code


                    Filename is GetModuleFileName + ".timestamp"

Output file format:
      ProcessorTick(musec)  TimeDelta(musec)    ThreadId   File:Func:Line

Using:
      DEBUG_MAKE_TIMESTAMP;

HISTORY:
      24.08.2005   Roman Samarev   Created

System-dependent parts: Win.

---------------------------------------------------------------------------*/

#if defined( WIN32 ) || defined( __WIN32__ )
  #include <windows.h>
  #define WINDOWS
#else
  #define _XOPEN_SOURCE 500
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <pthread.h>
  #include <stdarg.h>
#endif

#ifdef DEBUG_TIMESTAMP_ENABLE

#define DTIMESTAMP_IMPLEMENTATION
#include "dtimestamp.h"

// TDTimestamp _DTimeStamp;

DT_INT64 DTSGetTickCount64();
DT_INT64 GetProcTime64();

DT_INT64 CPUTicksPerMicroSecond = 1;
void GetCPUTicsPerUSec(DT_INT64 *pTickPerUSec);


class TDTSCs {
  private:
#ifdef WINDOWS  	
    CRITICAL_SECTION m_Cs;
#else    
    pthread_mutex_t m_mutex;
#endif
  public:
    TDTSCs( void );
    ~TDTSCs( void );

    void Lock( void );
    void Unlock( void );
};
#ifndef WINDOWS
TDTSCs :: TDTSCs( void ) {
  pthread_mutexattr_t attr;
  int iResult;

//  m_mutex = 0; 

  pthread_mutexattr_init( &attr );

  pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_DEFAULT );

  if ( ( iResult = pthread_mutex_init( &m_mutex, &attr ) ) != 0 ) {
    printf( "Mutex init failed, code = %d", iResult );
  }
  pthread_mutexattr_destroy( &attr );
}
TDTSCs :: ~TDTSCs( void ) {  
  pthread_mutex_destroy( &m_mutex );
}
void TDTSCs :: Lock( void ) {
  pthread_mutex_lock( &m_mutex );
}
void TDTSCs :: Unlock( void ) {
  pthread_mutex_unlock( &m_mutex );
}
#else
TDTSCs :: TDTSCs( void ) {
  InitializeCriticalSection( &m_Cs );
}
TDTSCs :: ~TDTSCs( void ) {  
  DeleteCriticalSection( &m_Cs );
}
void TDTSCs :: Lock( void ) {
  EnterCriticalSection( &m_Cs );
}
void TDTSCs :: Unlock( void ) {
  LeaveCriticalSection( &m_Cs );
}
#endif

TDTSCs * _pDTSCs;

TDTimestamp::TDTimestamp()
{
   m_pszCloseString = NULL;
   m_pszFileName = NULL;

   _pDTSCs = new TDTSCs();

   GetCPUTicsPerUSec( &CPUTicksPerMicroSecond );
   m_PrevTime = GetProcTime64();

   char * buf;//[0xFFF];
//   GetModuleFileName ( NULL, buf, 0xEFF );

   char * szCmdLine;

#ifdef WINDOWS
   szCmdLine = GetCommandLineA();
   buf = new char [strlen( szCmdLine )+20];
   strcpy( buf, szCmdLine );
#else
   FILE * cmdF = fopen( "/proc/self/cmdline", "r" );
   if( cmdF )
   {
       int Size=0xFF;
       buf = new char [Size];
       fgets( buf, Size, cmdF );       
       fclose(cmdF);       
   }
   
#endif

   char * temp = NULL;

   bool bStop = false;
   for( int i = (int)strlen(buf)-1; i>=0 && !bStop ; i-- )
   {
       switch( buf[i] )
       {
       case '\\':
       case '/':
          {
            if ( temp != NULL ) *temp=0;
            memmove( buf, buf+i+1, strlen(buf)-i);
            bStop = true;
            break;
           }
       case '.':
           temp = buf+i;
           break;
       }
   };

//   strcpy( buf, file );
   strcat( buf, ".timestamp");

   Init( buf, true,
         "*****************start*****************",
         "*****************stop*****************" );

   delete[] buf;   
};



//============================================================================
// TDTimestamp::TDTimestamp
//----------------------------------------------------------------------------
// Parameter:
// char * a - file name
// bool bNewFile - file will be new if true
// char *szInitString - initialize string for writing to file (may be NULL)
// char *szCloseString - close string for writing to file (may be NULL)
// Desription:
//     constructor
// Modify:
// Errors:
//============================================================================
void TDTimestamp::Init( char * a, bool bNewFile,
                        char * szInitString, char * szCloseString )
{
   m_pszFileName = new char [strlen(a)+1];
   strcpy( m_pszFileName , a );

   if( bNewFile ) remove(a);

   if( szCloseString )
   {
     m_pszCloseString = new char [ strlen(szCloseString)+1 ];
     strcpy( m_pszCloseString , szCloseString );
   }
   m_file = fopen( m_pszFileName, "a" );
   if( szInitString ) Write( "%s", szInitString );
};

TDTimestamp::~TDTimestamp()
{
   if( m_pszCloseString ) Write( "%s", m_pszCloseString );
   fclose(m_file);

   delete[] m_pszCloseString;
   delete[] m_pszFileName;
   delete _pDTSCs;
};

//===========================================================================
//
//---------------------------------------------------------------------------
//
//===========================================================================
void TDTimestamp::Reset()
{
	DT_INT64 tCurr = GetProcTime64();
	DT_INT64 tPrev;

	tPrev = m_PrevTime;
	m_PrevTime = tCurr;
}
//============================================================================
// TDTimestamp::Write
//----------------------------------------------------------------------------
// Parameter:
// const char * format, ... - see sprintf
// Desription:
//     Write data to file. date and time will be added
// Modify:
// Errors:
//============================================================================
int TDTimestamp::Write(const char * format, ...)
{
   int result = 0;
   char buf[ 1024 ];
   char format1[ 1024 ];

   va_list va;
   va_start( va, format );
   vsprintf( format1, format, va );
   va_end( va );

   char szTime[0xFF];

   DT_INT64 tCurr = GetProcTime64();
   DT_INT64 tPrev;

   tPrev = m_PrevTime;
   m_PrevTime = tCurr;

#ifndef __GNUC__
   _i64toa( tCurr, szTime, 10 );
#else
#ifdef WIN32
   sprintf( szTime, "%I64d",tCurr);
#else
   sprintf( szTime, "%lld", tCurr );
#endif
#endif

   sprintf( buf, "%s\t%d\t%d\t%s\n",
            szTime,
            (unsigned long)(tCurr - tPrev),
#ifdef WINDOWS	    
            GetCurrentThreadId(),
#else
            pthread_self(),
#endif	    
            format1 );

   if( m_file )
   {
     _pDTSCs->Lock();
     if( !fwrite( buf, 1, strlen(buf), m_file ) )
       result = -1;
     //fflush( m_file );
     _pDTSCs->Unlock();
   }
   return result;
};

//******************************************************************************
void GetCPUTicsPerUSec(DT_INT64 *pTickPerUSec)
{
    DT_INT64 CPUTicksPerMicroSecond = 1;
    DT_INT64 t1=0, t2=0, diff, elapsedTime;
#ifdef WINDOWS
    DWORD timeStart, timeStop;
#else
    time_t timeStart, timeStop;
#endif
    
#ifdef WINDOWS
#ifndef __GNUG__
    __try
    {
      _asm
      {
        RDTSC
      }
    }
    __except(1)
    {
       return;
    }
#endif

    HANDLE hProcess = GetCurrentProcess();
    HANDLE hThread = GetCurrentThread();
    int iPriority = GetThreadPriority(hThread);

    DWORD_PTR prevMask = SetThreadAffinityMask( hThread, 1 ); Sleep(0);

    if ( iPriority != THREAD_PRIORITY_ERROR_RETURN )
    {
      SetPriorityClass( hProcess, REALTIME_PRIORITY_CLASS );
      SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
      Sleep(0);
    }

    timeStart = GetTickCount();    
#else
    timeStart = clock()/(CLOCKS_PER_SEC/1000);
#endif

    for ( ;; )
    {
#ifdef WINDOWS    
      timeStop = GetTickCount();
#else
      timeStop = clock()/(CLOCKS_PER_SEC/1000);
#endif      
      if ( (int)(timeStop - timeStart) > 0 )
      {
        t1 = DTSGetTickCount64();
        break;
      }
    }

    timeStart = timeStop;

    for ( ;; )
    {
#ifdef WINDOWS    
      timeStop = GetTickCount();
#else
      timeStop = clock()/(CLOCKS_PER_SEC/1000);      
#endif      
      if ( (int)(timeStop - timeStart) > 100 )
      {
        t2 = DTSGetTickCount64();
        break;
      }
    }
#ifdef WINDOWS
    if ( iPriority != THREAD_PRIORITY_ERROR_RETURN )
    {
      SetThreadPriority(hThread, iPriority);
      SetPriorityClass( hProcess, NORMAL_PRIORITY_CLASS );
    }
    SetThreadAffinityMask( hThread, prevMask );
#endif
    diff = t2 - t1;
    elapsedTime = (DT_INT64)(timeStop - timeStart);

    CPUTicksPerMicroSecond = (diff) / (elapsedTime * 1000 );

    if( CPUTicksPerMicroSecond == 0 )
    {
       // possible only in case of using Codeguard or BoundsChecker
       //  DTSGetTickCount64();
        CPUTicksPerMicroSecond = 2000;
    };

    *pTickPerUSec = CPUTicksPerMicroSecond;
//    *pTickPerUSec = 1715;
};

DT_INT64 GetProcTime64()
{

#ifdef WINDOWS	   
   DT_INT64 result;
   HANDLE hThread = GetCurrentThread();
   DWORD dwCurPrior = GetThreadPriority(hThread);

   SetThreadPriority( hThread, dwCurPrior+1 );
   DWORD_PTR prevMask = SetThreadAffinityMask( hThread, 1 ); Sleep(0);
   result = DTSGetTickCount64() / CPUTicksPerMicroSecond;
   SetThreadAffinityMask( hThread, prevMask );               Sleep(0);
   SetThreadPriority( hThread, dwCurPrior );
   return result;
#else
   DT_INT64 result = DTSGetTickCount64() / CPUTicksPerMicroSecond;
   return result;

   printf("%lld, %lld, %lld\n", clock(), CLOCKS_PER_SEC, (CLOCKS_PER_SEC / 1000000ll));
   return (clock() / (CLOCKS_PER_SEC / 1000ll));
#endif
};

#ifdef __BORLANDC__
#pragma warn -8070
#endif
DT_INT64 DTSGetTickCount64(){
#ifndef __GNUC__
    _asm
    {
        rdtsc
    };
#else
  __asm__("rdtsc");
#endif   
};
#ifdef __BORLANDC__
#pragma warn +8070
#endif

//******************************************************************************
#endif
