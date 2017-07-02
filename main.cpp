#include <iostream>
#include "miniOPC.h"
#include "impl/sync_thread.h"
#include <thread>

namespace t1
{
class Test //тест чтения/записи одиночных элементов
{
public:
    Test()
    {
        std::cout << "connecting to opc" << std::endl;
        opc::miniOPC& server = opc::miniOPC::Instance();
        std::cout << "connecting done" << std::endl;

        std::cout << "add group" << std::endl;
        mGroupID = server.AddGroup( L"DB31", mAdresses, BOOL_COUNT + FLOAT_COUNT );
        std::cout << "add group done" << std::endl;
    }

    void Run()
    {        
        //чтение содержимого ОПЦ сервера
        Read();
        Write1();
        Read();
        //запись нескольких элементов
        int i = 10;
        while ( i > 0 )
        {
            std::chrono::time_point< std::chrono::system_clock > start, end;
            start = std::chrono::system_clock::now();
            Write2();
            end = std::chrono::system_clock::now();
            int mcs = std::chrono::duration_cast< std::chrono::microseconds > ( end - start ).count();
            std::cout << "write time msec: " << mcs << std::endl;

            start = std::chrono::system_clock::now();
            Read( false );
            end = std::chrono::system_clock::now();
            mcs = std::chrono::duration_cast< std::chrono::microseconds > ( end - start ).count();
            std::cout << "read time msec: " << mcs << std::endl;
            Write3();
            Read();
            //--i;
//            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }

    void Read( bool p = true )
    {
        //чтение данных
        opc::miniOPC& server = opc::miniOPC::Instance();
        auto res = server.Read( mGroupID );
        if ( res )
        {
            for (size_t i = 0; i < BOOL_COUNT + FLOAT_COUNT; i++)
            {
                if ( i < BOOL_COUNT )
                    mBoolData[ i ] = res[i].vDataValue.boolVal;
                else
                    mFloatData[ i - BOOL_COUNT ] = res[i].vDataValue.fltVal;
            }
            server.OpcMassFree( mGroupID, res );
        }

        for ( size_t i = 0; i < BOOL_COUNT && p; ++i )
        {
            std::cout << i << " = " << mBoolData[i] << std::endl;
        }
        for ( size_t i = 0; i < FLOAT_COUNT  && p; ++i )
        {
            std::cout << BOOL_COUNT + i << " = " << mFloatData[i] << std::endl;
        }

    }
    void Write1()
    {
        //запись одного элемента
        opc::miniOPC& server = opc::miniOPC::Instance();
        //запись одного элемента
        server.WriteValue( mGroupID, 0, static_cast<void*>( &t ), opc::tBOOL );
    }
    void Write2()
    {
        //массовая запись элементов
        opc::miniOPC& server = opc::miniOPC::Instance();
        server.WriteMass( mGroupID, 0, BOOL_COUNT, static_cast<void*>( mWriteBoolData ), opc::tBOOL );
        server.WriteMass( mGroupID, BOOL_COUNT, FLOAT_COUNT, static_cast<void*>( mWriteFloatData ), opc::tFLOAT );
    }
    void Write3()
    {
        //массовая запись элементов
        opc::miniOPC& server = opc::miniOPC::Instance();
        server.WriteMass( mGroupID, 0, BOOL_COUNT, static_cast<void*>( mWriteBoolDataZero ), opc::tBOOL );
        server.WriteMass( mGroupID, BOOL_COUNT, FLOAT_COUNT, static_cast<void*>( mWriteFloatDataZero ), opc::tFLOAT );
    }
    enum
    {
        BOOL_COUNT = 3,
        FLOAT_COUNT = 1
    };

    wchar_t const* mAdresses[ BOOL_COUNT + FLOAT_COUNT ] = {
        //bool
        L"CPU/DB31.Start_Oper",
        L"CPU/DB31.Stop_Oper",
        L"CPU/DB31.Nasos_M2",

        L"CPU/DB31.N_Operation"
    };

    uint64_t mGroupID = 0;


    bool mWriteBoolData[ BOOL_COUNT ] = {false, true, true};
    float mWriteFloatData[ FLOAT_COUNT ] = { 100.0 };

    bool mWriteBoolDataZero[ BOOL_COUNT ] = {false, false, false};
    float mWriteFloatDataZero[ FLOAT_COUNT ] = { 0.0 };

    bool t = true;

    bool mBoolData[ BOOL_COUNT ] = {false};
    float mFloatData[ FLOAT_COUNT ] = { 0.0 };
};

}

namespace t2
{
class Test
{
public:
    Test()
    {
        std::cout << "connecting to opc" << std::endl;
        opc::miniOPC& server = opc::miniOPC::Instance();
        std::cout << "connecting done" << std::endl;

        std::cout << "add group" << std::endl;
        mGroupID = server.AddGroup( L"TestArray", mAdresses, ArrayCount );
        std::cout << "add group done" << std::endl;
    }

    void Run()
    {
        Read();
        while ( true )
        {
           Write1();
           Read();
           Write2();
           Read();
           Read();
        }
    }

    void Read()
    {
        OPCITEMSTATE* rez = opc::miniOPC::Instance().Read( mGroupID );
        if (!rez)
            return;

        memset(mFloatData1, 0, sizeof(mFloatData1));
        memset(mFloatData2, 0, sizeof(mFloatData2));
        opc::ReadToArray( rez[0].vDataValue, mFloatData1, Array1Size );
        opc::ReadToArray( rez[1].vDataValue, mFloatData2, Array2Size );

        opc::miniOPC::Instance().OpcMassFree( mGroupID, rez );

        for ( size_t i = 0; i < Array1Size; ++i )
        {
            std::cout << mFloatData1[i] << " ";
        }
        std::cout <<std::endl;
        for ( size_t i = 0; i < Array2Size; ++i )
        {
            std::cout << mFloatData2[i] << " ";
        }
        std::cout <<std::endl;
    }

    void Write1()
    {
        VARIANT var[ArrayCount];
        opc::LoadToVariant( var[0], mFloatData5, Array1Size );
        opc::LoadToVariant( var[1], mFloatData6, Array2Size );

        HRESULT res = E_FAIL;
        while ( res == E_FAIL )
           res = opc::miniOPC::Instance().WriteMass( mGroupID, 0, ArrayCount, static_cast<void*>( &var ), opc::tVARIANT );
    }
    void Write2()
    {
        VARIANT var[ArrayCount];
        opc::LoadToVariant( var[0], mFloatData3, Array1Size );
        opc::LoadToVariant( var[1], mFloatData4, Array2Size );

        HRESULT res = E_FAIL;
        while ( res == E_FAIL )
           res = opc::miniOPC::Instance().WriteMass( mGroupID, 0, ArrayCount, static_cast<void*>( &var ), opc::tVARIANT );
    }

    enum
    {
        ArrayCount = 2,
        Array1Size = 3,
        Array2Size = 5
    };

    wchar_t const* mAdresses[ ArrayCount ] = {
        //bool
        L"CPU/test_array1",
        L"CPU/test_array2"
    };

    uint64_t mGroupID = 0;

    float mFloatData1[ Array1Size ] = { 1.0 };
    float mFloatData2[ Array2Size ] = { 1.0 };


    float mFloatData3[ Array1Size ] = { 1.0, 2.3, 3.4 };
    float mFloatData4[ Array2Size ] = { 5.6, 7.8, 9.1, 10.0, 2.5 };

    float mFloatData5[ Array1Size ] = { 1.0 };
    float mFloatData6[ Array2Size ] = { 1.0 };
};
}

namespace t3
{
class Test //тест синхронизирующего потока
{
public:
    Test()
    {
        thread.Start();
    }

    void Run()
    {
        int i =0;
        while ( true )
        {
            thread.Exec( [ &i ]()
            {
            std::cout << i << std::endl;
            } );
            ++i;
        }
    }

    SyncThread thread;
};
}

namespace t4
{
class Test //тест выделения памяти под варианты
{
public:
   void Run()
   {
      SAFEARRAY* ptr = 0;
      size_t size = 10;
      int i = 0;
      while (true)
      {
         ++i;
         //create
         {
            ptr = SafeArrayCreateVector( VT_R4, 0, size);
         }
         //fill
         {
            float* values = 0;
            SafeArrayAccessData( ptr, reinterpret_cast<void**>(&values) );
            if ( values )
            {
                for (size_t i = 0; i < size; i++)
                {
                    values[i] = i;
                }
            }
         }

         //clear
         {
            SafeArrayUnaccessData( ptr );
            SafeArrayDestroy(ptr);
         }
      }
   }
};
}
int main()
{
    using namespace t2;
    Test t;
    new std::thread( &Test::Run, &t );

    std::this_thread::sleep_for(std::chrono::seconds(3600));

    return 0;
}

