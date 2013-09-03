#ifndef OPC_VALUE_H
#define OPC_VALUE_H
#include <miniOPC.h>
namespace opc
{
template <typename T,opc::types type>
class OPCValue
{
public:
    OPCValue():changed(true),value(T())
    {}

    opc::types Type(void)
    {
        return type;
    }
    const T & Value() const
    {
        return value;
    }
    void operator = (const T & r)
    {
        value = r;
        changed=true;
    }
private:
    bool changed;
    T value;
};

}


#endif // OPC_VALUE_H
