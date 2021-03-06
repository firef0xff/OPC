﻿#pragma once
#ifndef win_ole_modeH
#define win_ole_modeH
#include "types.h"
#include "sync_thread.h"

#ifdef WINDOWS
#include "../opc.h"
#include <vector>
#include <memory>

namespace opc
{

class WinOleMode
{
public:
    WinOleMode( wchar_t const* ServerName );//конструктор
    ~WinOleMode();//деструктор

    GROUP_ID        AddGroup    ( wchar_t const* pGroupName, wchar_t const* Addresses[], size_t ItemsCount );
    OPCITEMSTATE*   Read        ( GROUP_ID id ); //вернуть минимально возможный набор который нужно будет распределить в массив
    HRESULT         WriteValue  ( GROUP_ID id, size_t pos, void *item, types type );
    HRESULT         WriteMass   ( GROUP_ID id, size_t pos, size_t mass_len, void *item, types type );
    void            OpcMassFree ( GROUP_ID id, OPCITEMSTATE* mass);
    bool            Connected   ();   
    void            GetArrayData (VARIANT& variant, void **values );
    void            FreeArrayData(VARIANT&);
    void            InitArrayData(VARIANT& variant, types type, size_t size );
    void            DataUnlock   (VARIANT& variant );
private:
    //типы
    class Group                 //структура с данными по шруппе ОПС сервера
    {
    public:
        Group():
            pItemMgt( 0 ),
            pSyncIO ( 0 ),
            pItemResult( 0 )
        {}
        ~Group()
        {
            if ( pItemResult )
            {
                CoTaskMemFree( pItemResult );
            }
        }

        IOPCItemMgt     *pItemMgt;      //указатель на Группу адресов с данными
        IOPCSyncIO      *pSyncIO;       //Указатель на ентерфейс для синхронизации данных
        std::vector<OPCITEMDEF> pItems; //массив с элементами группы
        OPCITEMRESULT   *pItemResult;   //указатель на элемент
        size_t          ItemsCount;     //количество элементов в группе
    };    
    typedef std::unique_ptr< Group > GroupPtr;
    typedef std::vector< GroupPtr > GroupsList;
    typedef GroupsList::const_iterator Item;

    //функции

    void Init           ( wchar_t const* ServerName );
    void DeInit         ();
    void AddGroupImpl   ( GROUP_ID& id, wchar_t const* pGroupName, wchar_t const* Addresses[], size_t ItemsCount );
    void OpcMassFreeImpl( GROUP_ID id, OPCITEMSTATE* mass);
    void ReadImpl       ( OPCITEMSTATE **res, GROUP_ID id );
    void WriteMassImpl  ( HRESULT &res, GROUP_ID id, size_t pos, size_t mass_len, void *item, types type );


    Item GetGroup ( GROUP_ID id );
    void LogErrStrong( HRESULT err );
    //данные
    IOPCServer  *pIOPCServer;	//указатель на OPC сервер

    GroupsList  Groups;//Данные по группам сервера
    OPCHANDLE   GrpSrvHandle;
    HRESULT     result;

    _GUID       clsid;		//идентификатор класса сервера
    long        TimeBias; 	//смещение времени
    float       PercentDeadband;
    DWORD       RevisedUpdateRate;

    //состояние сервера
    bool mConnected;

    SyncThread mThread;
};

}//namespace opc
#endif
#endif
