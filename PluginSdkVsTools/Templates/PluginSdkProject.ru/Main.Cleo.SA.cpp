#include "plugin.h"
#include "CLEO.h"

using namespace plugin;

class $classname$ {
public:
    static OpcodeResult WINAPI OpcodeFunc(CScriptThread* thread) {
        // �������� ����� ���� ������
        
        return OR_CONTINUE;
    }

    $classname$() {
        CLEO_RegisterOpcode(/*�������� ����� ID ������*/, OpcodeFunc);
    }
} $classinstancename$;
