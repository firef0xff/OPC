//---------------------------------------------------------------------------

#ifndef miniOPCH
#define miniOPCH
#include <lib/opc.h>
#include <def.cpp>
#include <list>
#include <vector>
#include <string>

namespace opc
{

//типы
class GroupPTRs                 //структура с данными по группе ОПС сервера
{
public:
    GroupPTRs():pItemMgt(nullptr),pSyncIO(nullptr),pItems(nullptr),pItemResult(nullptr){}
    ~GroupPTRs()
    {
        if (pItemResult)
        {
            CoTaskMemFree(pItemResult);
        }
        if (pItems)
        {
            delete []pItems;
        }
    }
IOPCItemMgt		*pItemMgt;		//указатель на Группу адресов с данными
IOPCSyncIO  	*pSyncIO;		//Указатель на ентерфейс для синхронизации данных
OPCITEMDEF		*pItems;		//массив с элементами группы
OPCITEMRESULT   *pItemResult;	//указатель на элемент
size_t			ItemsCount;		//количество элементов в группе
};

typedef std::list   <GroupPTRs*> GroupList;
typedef std::list   <GroupPTRs*>::const_iterator GroupItem;
typedef std::vector <std::wstring> AddressesList;
typedef unsigned long long GROUP_ID;
enum types{tBOOL,tINT,tFLOAT};

class myOPC
{
public:
    myOPC(const wchar_t *ServerName=L"OPC.SimaticNET");//конструктор
    ~myOPC(void);//деструктор

    GROUP_ID  AddGroup(wchar_t *pGroupName, const AddressesList &Addresses, size_t ItemsCount);
    OPCITEMSTATE*	Read 	(GROUP_ID _id); //вернуть минимально возможный набор который нужно будет распределить в массив
    HRESULT	WriteValue      (GROUP_ID _id,size_t pos,void *item,types type);
    HRESULT	WriteMass       (GROUP_ID _id,size_t pos,size_t mass_len,void *item,types type);
    void OpcMassFree(GROUP_ID _id,OPCITEMSTATE* mass);
private:
    //функции
    GroupItem GetGroup(GROUP_ID _id);
    //данные
    IOPCServer 		*pIOPCServer;	//указатель на OPC сервер

    GroupList       Groups;//Данные по группам сервера
    OPCHANDLE		GrpSrvHandle;
    HRESULT		    *pErrors,*pRErrors,result;
    UINT			qnr;

    _GUID           clsid;		//идентификатор класса сервера
    long            TimeBias; 	//смещение времени
    float           PercentDeadband;
    DWORD           RevisedUpdateRate;


    //состояние сервера
    bool connected;
    #ifdef _mDEBUG
public:
    static std::wstringstream log;
private:
    LPWSTR ErrorStr;
    #endif
};
#ifdef _mDEBUG
std::wstring myOPC::log("");
#endif
}

#endif
