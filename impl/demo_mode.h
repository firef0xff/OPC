﻿#ifndef demo_modeH
#define demo_modeH

#include "types.h"
#include <memory>
#include <vector>
#include <string>


#ifndef WINDOWS
typedef int HRESULT;
enum VARIANT_TYPE
{
   VT_BOOL,
   VT_I4,
   VT_R4,

};
struct VARIANT
{
   VARIANT_TYPE vt;
   bool boolVal = false;
   float fltVal = 0.0;
   long lVal    = 0;
   std::shared_ptr<uint8_t> parray;
};
enum RESULTS : int
{
    S_OK,
    S_FALSE,
    E_FAIL
};
struct OPCITEMSTATE
{
//    OPCHANDLE hClient;
//    FILETIME ftTimeStamp;
//    WORD wQuality;
//    WORD wReserved;
VARIANT vDataValue;
};

#else
#include <windows.h>
#include "../opc.h"
#endif

namespace opc
{

class DemoMode
{
public:
    DemoMode( wchar_t const* );//конструктор
    ~DemoMode();//деструктор

    GROUP_ID        AddGroup    ( wchar_t const* pGroupName, wchar_t const* Addresses[], size_t ItemsCount );
    OPCITEMSTATE*   Read        ( GROUP_ID id ); //вернуть минимально возможный набор который нужно будет распределить в массив
    HRESULT         WriteValue  ( GROUP_ID id, size_t pos, void *item, types type );
    HRESULT         WriteMass   ( GROUP_ID id, size_t pos, size_t mass_len, void *item, types type );
    void            OpcMassFree ( GROUP_ID id, OPCITEMSTATE* mass );
    void            GetArrayData( VARIANT& variant, void **values );
    void            FreeArrayData(VARIANT& variant);
    void            InitArrayData(VARIANT& variant, types type, size_t size );
    void            DataLock     ( VARIANT& variant );
    void            DataUnlock     ( VARIANT& variant );
    bool            Connected   ();

private:
    //типы
    struct Group                 //структура с данными по шруппе ОПС сервера
    {
        std::vector< std::wstring > Addresses; //адреса элементов в группе
        std::wstring Name;
    };
    typedef std::unique_ptr< Group > GroupPtr;
    typedef std::vector< GroupPtr > GroupsList;
    typedef GroupsList::const_iterator Item;

    //функции
    Item GetGroup ( GROUP_ID id );
    //данные
    GroupsList Groups;//Данные по группам сервера
};

}//namespace opc
#endif
