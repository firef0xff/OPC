#pragma once
#ifndef sync_threadH
#define sync_threadH

#include <memory>
#include <functional>

class SyncThread
{
public:
   class Impl;

   typedef std::function<void(void)> Function;

   SyncThread();
   ~SyncThread();

   void Start();
   void Exec( Function func );

};



#endif
