#include "sync_thread.h"

#ifndef EMBARCADERO
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#endif

//--------------------------------------------------------------------

class SyncThread::Impl
{
public:
   Impl():
      interrupt(false)
   {}
   ~Impl()
   {
      interrupt = true;
      mRunWaiter.notify_all();
      if ( mThread.joinable() )
          mThread.join();
   }

   void Start()
   {
      std::unique_lock< std::mutex > lock( Mutex );
      mThread = std::thread( &Impl::Run, this );
      mRunWaiter.wait(lock);
   }
   void Exec( SyncThread::Function func )
   {
      std::unique_lock< std::mutex > lock( Mutex );
      mFunc = std::move( func );
      mRunWaiter.notify_one();
      mRunWaiter.wait(lock);
   }
private:
   void Run()
   {
      std::unique_lock< std::mutex > lock( Mutex );
      for(;;)
      {
         try
         {
            mRunWaiter.notify_one();
            mRunWaiter.wait(lock);
            if ( interrupt )
               return;
            if (mFunc)
               mFunc();
         }
         catch(...){}
      }
   }
   bool interrupt;
   std::thread mThread;
   SyncThread::Function mFunc;
   std::mutex Mutex;
   std::condition_variable mRunWaiter;
};

SyncThread::SyncThread():
    mImpl( new SyncThread::Impl() )
{}
SyncThread::~SyncThread()
{
}
void SyncThread::Start()
{
   mImpl->Start();
}
void SyncThread::Exec( Function func )
{
   mImpl->Exec( std::move( func ) );
}
