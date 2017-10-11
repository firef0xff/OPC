#include "sync_thread.h"

#ifndef EMBARCADERO
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#endif

//--------------------------------------------------------------------
SyncThread::SyncThread()
{}
SyncThread::~SyncThread()
{
}
void SyncThread::Start()
{
}
void SyncThread::Exec( Function func )
{
   if (func)
      func();
}
