#pragma once
#ifndef miniOPCH
#define miniOPCH
#include "impl/types.h"
#include <memory>
#include <mutex>
#include <map>
#include <thread>

#ifdef DEMO
#include "impl/demo_mode.h"
#else
#ifdef WINDOWS
#include "impl/win_ole_mode.h"
#endif
#endif

namespace opc
{
template < class Impl, class NameRegistrator >
class Server : public Impl
{
public:
   static Server& Instance()
   {
      static std::mutex m;
      std::lock_guard< std::mutex > lock( m );
      auto id =std::this_thread::get_id();
      typedef std::unique_ptr< Server > Ptr;
      static std::map< std::thread::id, std::unique_ptr< Server > >  ptrs;

      Ptr& ptr = ptrs[id];

      if( !ptr || !ptr->Connected() )
      {
         ptr.reset( new Server() );
      }
      return *ptr;
   }

private:
    Server():Impl( NameRegistrator::Name() ){}
    Server( Server const& );
    void operator = ( Server const& );
};

struct SimaticNET
{
    static wchar_t const* Name()
    {
        return L"OPC.SimaticNET";
    }
};
struct VipaOpc7
{
    static wchar_t const* Name()
    {
        return L"VIPA.OPCServer";
    }
};

#ifndef USE_OPC_SERVER
#define USE_OPC_SERVER VipaOpc7
#endif

#ifdef DEMO
typedef Server< DemoMode, USE_OPC_SERVER > miniOPC;
#else
#ifdef WINDOWS
typedef Server< WinOleMode, USE_OPC_SERVER > miniOPC;
#endif
#endif

template < class T >
void    ReadToArray (  VARIANT& variant, T* array, uint32_t size )
{
    T* values = nullptr;
    miniOPC::Instance().GetArrayData( variant, reinterpret_cast<void**>(&values) );
    if ( values )
    {
        for (size_t i = 0; i < size; i++)
        {
            array[ i ] = values[i];
        }
    }
    miniOPC::Instance().FreeArrayData( variant );
}

namespace
{

template <class T>
types Type();

template <>types Type<float>()
{
   return tFLOAT;
}
template <>types Type<int>()
{
   return tINT;
}
template <>types Type<bool>()
{
   return tBOOL;
}
}

template < class T >
void    LoadToVariant (  VARIANT& variant, T* array, uint32_t size )
{
    T* values = nullptr;
    opc::miniOPC::Instance().InitArrayData( variant, Type<T>(), size );
    miniOPC::Instance().GetArrayData( variant, reinterpret_cast<void**>(&values) );
    if ( values )
    {
        for (size_t i = 0; i < size; i++)
        {
            values[i] = array[ i ];
        }
    }
    opc::miniOPC::Instance().DataUnlock( variant );
}

}//namespace opc
#endif
