#include "demo_mode.h"
#include "cache/memory.h"

namespace opc
{
static memory::Memory& MEM()
{
    static memory::Memory m;
    return m;
}

DemoMode::DemoMode( wchar_t const* )
{}
DemoMode::~DemoMode()
{
}

GROUP_ID        DemoMode::AddGroup    ( wchar_t const* pGroupName, wchar_t const* Addresses[], size_t ItemsCount )
{
    GroupPtr ptr( new Group );
    Group& lnk = *ptr;
    lnk.Name = pGroupName;
    for ( size_t i = 0; i < ItemsCount; ++i )
        lnk.Addresses.push_back( Addresses[i] );
    Groups.push_back( std::move( ptr ) );

    return Groups.size();
}
DemoMode::Item  DemoMode::GetGroup    ( GROUP_ID id )
{
    if ( id > 0)
    {
        --id;
        if ( id < Groups.size() )
        {
            return Groups.begin() + id;
        }
    }
    return Groups.end();
}

OPCITEMSTATE*   DemoMode::Read        ( GROUP_ID id )
{
    Item item = GetGroup( id );
    if ( item == Groups.end() )
        return nullptr;

    Group* ptr = item->get();
    OPCITEMSTATE* res = new OPCITEMSTATE[ ptr->Addresses.size() ]();

    for ( size_t i = 0; i < ptr->Addresses.size(); ++i )
    {
        res[i].vDataValue = MEM().Page< VARIANT >().Read( ptr->Addresses[i] );        
    }
    return res;
}

HRESULT         DemoMode::WriteValue  ( GROUP_ID id, size_t pos, void *item, types type )
{
    return WriteMass( id, pos, 1, item, type );
}
HRESULT         DemoMode::WriteMass   ( GROUP_ID id, size_t pos, size_t mass_len, void *item, types type )
{
    Item group = GetGroup( id );
    if ( group == Groups.end() )
        return E_FAIL;

    Group* ptr = group->get();
    if ( ptr->Addresses.size() <= pos )
        return E_FAIL;

    for ( size_t i = 0; i < mass_len; i++ )
    {
        VARIANT value;
        switch ( type )
        {
            case tBOOL:
            {
                value.vt        = VT_BOOL;
                value.boolVal   = *( static_cast< bool* >( item ) + i );
                break;
            }
            case tINT:
            {
                value.vt    = VT_I4;
                value.lVal  = *( static_cast< int* >( item ) + i );
                break;
            }
            case tFLOAT:
            {
                value.vt      = VT_R4;
                value.fltVal 	= *( static_cast< float* >( item ) + i );
                break;
            }
            case tVARIANT:
            {                
                value = *(static_cast< VARIANT* >( item ) + i);
                break;
            }
            default:
            {
                return E_FAIL;
            }
        }
        auto &page = MEM().Page<VARIANT>();
        VARIANT old = page.Read( ptr->Addresses.at( pos + i) );
        FreeArrayData( old );
        DataLock( value );
        page.Write( ptr->Addresses.at( pos + i), value );
    }

    return S_OK;
}
void            DemoMode::OpcMassFree ( GROUP_ID /*id*/, OPCITEMSTATE* mass )
{
    delete[] mass;
}

void            DemoMode::GetArrayData( VARIANT& variant, void **values )
{
#ifndef WINDOWS
    *values = variant.parray.get();
#else
    auto res = SafeArrayAccessData( variant.parray, values );
    if ( res ==  E_INVALIDARG )
       return;
    if ( res ==  E_UNEXPECTED )
       return;
#endif
}
void            DemoMode::FreeArrayData( VARIANT& variant )
{
#ifndef WINDOWS
   variant.parray.reset();
#else
   if ( variant.vt == VT_SAFEARRAY )
   {
      SafeArrayUnaccessData( variant.parray );
      SafeArrayDestroy( variant.parray );
      variant.vt = VT_EMPTY;
      variant.parray = nullptr;
   }
#endif
}
void            DemoMode::InitArrayData(VARIANT& variant, types type, size_t size )
{
#ifndef WINDOWS
   sigset_t sz = 0;
   switch ( type )
   {
   case tBOOL:
      sz = size * sizeof(bool);
      break;
   case tFLOAT:
      sz = size * sizeof(float);
      break;
   case tINT:
      sz = size * sizeof(int);
      break;
   default:
      break;
   }
   if ( sz )
      variant.parray.reset( new uint8_t[sz] );
#else
   VARENUM t = VT_EMPTY;
   switch ( type )
   {
   case tBOOL:
      t = VT_BOOL;
      break;
   case tFLOAT:
      t = VT_R4;
      break;
   case tINT:
      t = VT_I4;
      break;
   default:
      break;
   }
   if ( size && t != VT_EMPTY  )
   {
      SAFEARRAY* ptr = SafeArrayCreateVector( t, 0, size );
      variant.parray = ptr;
      variant.vt = VT_SAFEARRAY;
   }

#endif
}
void            DemoMode::DataLock     ( VARIANT& variant )
{
#ifdef WINDOWS
   if ( variant.vt == VT_SAFEARRAY )
   {
      SafeArrayLock( variant.parray );
   }
#endif
}
void            DemoMode::DataUnlock     ( VARIANT& variant )
{
#ifdef WINDOWS
   if ( variant.vt == VT_SAFEARRAY )
   {
      SafeArrayUnlock( variant.parray );
   }
#endif
}
bool            DemoMode::Connected   ()
{
    return true;
}

}//namespace opc

