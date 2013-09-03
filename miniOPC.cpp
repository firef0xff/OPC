#pragma hdrstop
#include <miniOPC.h>
#pragma package(smart_init)

#define LOCALE_ID    0x409	// Code 0x409 = ENGLISH
#define REQUESTED_UPDATE_RATE 500

namespace opc
{

myOPC::myOPC(const wchar_t *ServerName):TimeBias(0),PercentDeadband(0.0),connected(false)
{
	//подключение к серверу
	#ifdef _mDEBUG
    log<<L"Конструктор: \n";
    log<<L"Подключение к OPC серверу "<<ServerName<<" \n";
    log<<L"Инициализация COM библиотек\n";
	#endif

	result=CoInitialize(NULL); //подготовка СОМ библиотек к работе

	#ifdef _mDEBUG
	if (result==S_OK||result==S_FALSE)
	{
        log<<L"OK\n";
	}else
	{
        log<<L"FAIL\n";
	}
    log<<L"Получение идентификатора класса сервера по его имени (clsid)\n";
	#endif

	// получение идентификатора класса по имени сервера
	result=CLSIDFromProgID(ServerName,&clsid);
	if (result!=S_OK)
	{
	#ifdef _mDEBUG
        log<<L"FAIL\n";
	#endif
		return;
	}
	#ifdef _mDEBUG
	else
	{
        log<<L"OK\n";
	}
    log<<L"Получение адреса сервера (pIOPCServer)\n";
	#endif

	// получение адреса сервера
	result=CoCreateInstance(clsid,NULL,CLSCTX_LOCAL_SERVER,IID_IOPCServer,(void**)&pIOPCServer);
	if (result!=S_OK)
	{
	#ifdef _mDEBUG
        log<<L"FAIL\n";
	#endif
		return;
	}
	#ifdef _mDEBUG
	else
	{
        log<<L"OK\n";
	}
    log<<"LСоединение с сервером установлено\n\n";
	#endif
    connected=true;
	Groups.clear();
}

myOPC::~myOPC(void)
{
	#ifdef _mDEBUG
    log<<L"Деструктор \n";
    log<<L"Чистка списка подключенных групп \n";
	#endif
    for (GroupItem i=Groups.begin(); i!=Groups.end(); i++)
	{
    	delete (*i);
	}
	Groups.clear();
	#ifdef _mDEBUG
    log<<L"Удаление подключения к OPC\n";
	#endif
	result=pIOPCServer->Release();
	#ifdef _mDEBUG
	if (result==S_OK||result==S_FALSE)
	{
        log<<L"OK \n";
	}else
	{
        log<<L"FAIL \n";
	}
	#endif
}

GROUP_ID  myOPC::AddGroup(wchar_t *pGroupName,const AddressesList &Addresses/*массив второго уровня*/,
							size_t ItemsCount/*массив считается элементом*/)
{
	#ifdef _mDEBUG
    log<<L"Добавление группы "<<pGroupName<<"\n";
	#endif
    if (!connected)
	{
	#ifdef _mDEBUG
        log<<L"Возврат ибо нет соединения с сервером\n";
	#endif
		return 0;
	}

	GroupPTRs *tmp=new GroupPTRs();
	tmp->ItemsCount=ItemsCount;
	result=pIOPCServer->AddGroup(pGroupName,true,REQUESTED_UPDATE_RATE,Groups.size()+1,&TimeBias,&PercentDeadband,
								LOCALE_ID,&GrpSrvHandle,&RevisedUpdateRate,
								IID_IOPCItemMgt,(LPUNKNOWN*)&tmp->pItemMgt);
	if(result!=S_OK)
	{
	#ifdef _mDEBUG
        log<<L"FAIL\n";
	#endif
		delete tmp;
		return 0;
	}
	#ifdef _mDEBUG
    log<<L"OK\n";
    log<<L"Сопоставление элементов сервера с адресами переменных и нициализация первоначальными значениями\n";
	#endif

	tmp->pItems=new OPCITEMDEF[tmp->ItemsCount];
    for(int i=0;i<tmp->ItemsCount;i++)
	{
		(*(tmp->pItems+i)).szAccessPath        	=  L"";
        (*(tmp->pItems+i)).szItemID		     	=  Addresses[i].c_str();
		(*(tmp->pItems+i)).bActive             	=  TRUE;
		(*(tmp->pItems+i)).hClient             	=  1;
		(*(tmp->pItems+i)).dwBlobSize          	=  0;
		(*(tmp->pItems+i)).pBlob			    =  NULL;
		(*(tmp->pItems+i)).vtRequestedDataType 	=  0;
	}
	#ifdef _mDEBUG
    log<<L"OK\n";
    log<<L"Регистрация группы на сервере\n";
	#endif

	result=tmp->pItemMgt->AddItems(tmp->ItemsCount,tmp->pItems,&tmp->pItemResult,&pErrors);
	if(result!=S_OK&&result!=S_FALSE)
	{
	#ifdef _mDEBUG
		pIOPCServer->GetErrorString(pErrors[0],LOCALE_ID,&ErrorStr);
        log<<L"FAIL ("<<ErrorStr<<")\n";
		CoTaskMemFree(ErrorStr);
	#endif
		delete tmp;
		return 0;
	}
	#ifdef _mDEBUG
	else
	{
        log<<L"OK\n";
	}
    log<<L"Получение указателя на интерфейс синхронизации данных\n";
	#endif

	result=tmp->pItemMgt->QueryInterface(IID_IOPCSyncIO, (void**)&tmp->pSyncIO);
	if(result<0)
	{
	#ifdef _mDEBUG
        log<<L"FAIL\n";
	#endif
		delete tmp;
        return 0;
	}
	#ifdef _mDEBUG
	else
	{
        log<<L"OK\n";
	}
    log<<L"Сохранение в списке, генерация дескриптора\n";
	#endif
	Groups.push_back(tmp);
	#ifdef _mDEBUG
    log<<L"OK\n";
    log<<L"Возврат дескриптора \n\n";
	#endif
	return Groups.size();
}

GroupItem myOPC::GetGroup(GROUP_ID _id)
{
	#ifdef _mDEBUG
    log<<L"Поиск дескриптора в списке\n";
	#endif
	int k=0,pos=_id-1;//счетчик и реальная позиция в списке
    for (GroupItem i =Groups.begin(); i!=Groups.end(); i++,k++)
	{
		if (k==pos)
		{
	#ifdef _mDEBUG
            log<<L"OK\n\n";
	#endif
			return i;
		}
	}
	#ifdef _mDEBUG
    log<<L"FAIL\n\n";
	#endif
	return Groups.end();
}

void 	myOPC::OpcMassFree(GROUP_ID _id,OPCITEMSTATE* mass)
{
    if (!connected)
	{
	#ifdef _mDEBUG
        log<<L"Возврат ибо нет соединения с сервером\n";
	#endif
		return /*nullptr*/;
	}
    GroupItem _item=GetGroup(_id);
	#ifdef _mDEBUG
    log<<L"Проверка полученного итератора\n";
	#endif
	if (_item==Groups.end())
	{
	#ifdef _mDEBUG
        log<<L"FAIL\n";
	#endif
		return /*nullptr*/;
	}
	#ifdef _mDEBUG
    log<<L"OK\n";
    log<<L"Очистка массива\n";
	#endif
	GroupPTRs* __item=*_item;
	for(int i=0;i<__item->ItemsCount;i++)
	{
		VariantClear(&mass[i].vDataValue);
	}
	#ifdef _mDEBUG
    log<<L"OK\n\n";
	#endif
}

OPCITEMSTATE*	myOPC::Read 	(GROUP_ID _id)
{
    if (!connected)
	{
	#ifdef _mDEBUG
        log<<L"Возврат ибо нет соединения с сервером\n";
	#endif
		return nullptr;
	}
    GroupItem _item=GetGroup(_id);
	#ifdef _mDEBUG
    log<<L"Проверка полученного итератора\n";
	#endif
	if (_item==Groups.end())
	{
	#ifdef _mDEBUG
        log<<L"FAIL\n";
	#endif
		return nullptr;
	}
	#ifdef _mDEBUG
    log<<L"OK\n";
    log<<L"Чтение данных с сервера\n";
	#endif

	GroupPTRs* __item=*_item;
    OPCHANDLE		*phServer=new OPCHANDLE[__item->ItemsCount];//массив указателией на OPC
	OPCITEMSTATE	*pItemsValues=nullptr; //указатель на состояния итемов в опс
	LPWSTR		 	ErrorStr=L"";    //текст ошибки

	#ifdef _mDEBUG
    log<<L"Получение массива указателей на серверы для чтения из групп (phServer)\n";
	#endif
	for(int i=0;i<__item->ItemsCount;i++)
	{
		phServer[i]=__item->pItemResult[i].hServer;
	}
	#ifdef _mDEBUG
    log<<L"OK\n";
    log<<L"Чтение данных по группам\n";
	#endif
	result=__item->pSyncIO->Read(OPC_DS_CACHE,__item->ItemsCount,phServer,&pItemsValues,&pRErrors);
    delete [] phServer;  //удаление массива с указателями на источники для элементов групп
	if(result==S_OK)
	{
	#ifdef _mDEBUG
        log<<L"OK\n";
        log<<L"Возврат OPCITEMSTATE	*pItemsValues для наполнения массивов данных программы\n\n";
	#endif
		return pItemsValues;
	}
	else
	{
	#ifdef _mDEBUG
		pIOPCServer->GetErrorString(pRErrors[0],LOCALE_ID,&ErrorStr);
        log<<L"FAIL - Ошибка получения данных в блоке "<<_id<<"\n\n";
		CoTaskMemFree(ErrorStr);
	#endif
		return nullptr;
	}
}

HRESULT	myOPC::WriteValue   (GROUP_ID _id,size_t pos,void *item,types type)
{
	return  WriteMass(_id,pos,1,item,type);
}
HRESULT	myOPC::WriteMass    (GROUP_ID _id,size_t pos,size_t mass_len,void *item,types type)
{
    if (!connected)
	{
	#ifdef _mDEBUG
        log<<L"Возврат ибо нет соединения с сервером\n";
	#endif
		return E_FAIL;
	}
    GroupItem _item=GetGroup(_id);
	#ifdef _mDEBUG
    log<<L"Проверка полученного итератора\n";
	#endif
	if (_item==Groups.end())
	{
	#ifdef _mDEBUG
        log<<L"FAIL\n";
	#endif
		return E_FAIL;
	}
	#ifdef _mDEBUG
    log<<L"OK\n";
    log<<L"Запись данных на сервер\n";
	#endif

	GroupPTRs* __item=*_item;
	if (__item->ItemsCount<=pos)
	{
	#ifdef _mDEBUG
    log<<L"FAIL выход за пределы группы\n";
	#endif
		return E_FAIL;
	}

	OPCHANDLE	*phServer=new OPCHANDLE[mass_len];
	VARIANT     *values=new VARIANT[mass_len];
	HRESULT		*pWErrors;
	LPWSTR		 ErrorStr;

	for (size_t i=0; i<mass_len; i++)
	{
		*(phServer+i)  		= __item->pItemResult[pos+i].hServer;
		#ifdef _mDEBUG
        log<<L"Определение типа данных\n";
		#endif
		switch (type)
		{
			case tBOOL:
				{
		#ifdef _mDEBUG
                    log<<L"OK - BOOL\n";
		#endif
					(*(values+i)).vt		= VT_BOOL;
					(*(values+i)).boolVal 	= *(((bool *)item)+i);
					break;
				}
			case tINT:
				{
		#ifdef _mDEBUG
                    log<<L"OK - INT\n";
		#endif
					(*(values+i)).vt		= VT_I4;
					(*(values+i)).lVal 	= *(((int *)item)+i);
					break;
				}
			case tFLOAT:
				{
		#ifdef _mDEBUG
                    log<<L"OK - FLOAT\n";
		#endif
					(*(values+i)).vt		= VT_R4;
					(*(values+i)).fltVal 	= *(((float *)item)+i);
					break;
				}
		default:
			{
		#ifdef _mDEBUG
                log<<L"FAIL - UNCNOWN\n";
		#endif
				return E_FAIL;
			}
        }
    }
	#ifdef _mDEBUG
    log<<L"Запись данных в контроллер\n";
	#endif
	result=__item->pSyncIO->Write(mass_len,phServer,values,&pWErrors);
	delete [] phServer;
	delete [] values;
	if(result == S_OK||result==S_FALSE)
	{
	#ifdef _mDEBUG
        log<<L"OK\n";
	#endif
		return *pWErrors;
	}
	else
	{
	#ifdef _mDEBUG
		pIOPCServer->GetErrorString(pWErrors[0],LOCALE_ID,&ErrorStr);
        log<<L"FAIL - Ощибка записи данных группы "<<_id<<" "<<ErrorStr<<"\n";
		CoTaskMemFree(pWErrors);
	#endif
	return *pWErrors;
	}
}

}
