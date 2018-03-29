#pragma once
#include <string>
#include <vector>
#include <functional>
#include "Type.h"
#include "..\shared\Games.h"
#include "Tabs.h"

using namespace std;

class Function {
public:
    enum CC {
        CC_CDECL,
        CC_STDCALL,
        CC_FASTCALL,
        CC_THISCALL,
        CC_UNKNOWN = 255
    };

    string mName; // only function name
    string mMangledName; // full name
    string mScope; // scope
    string mModuleName;
    CC mCC = CC_CDECL;
    Type mRetType;
    bool mIsConst = false;
    bool mIsEllipsis = false;
    string mComment;
    string mType;
    bool mIsOverloaded = false;

    struct Parameter {
        string mName;
        Type mType;
    };

    vector<Parameter> mParameters;

    struct ExeVersionInfo {
        unsigned int mAddress = 0;
        string mRefsStr;
    };

    ExeVersionInfo mVersionInfo[Games::GetMaxGameVersions()];

    string GetFullName() const; // combine name + scope

    void ForAllParameters(std::function<void(Parameter &p, bool first, bool last)> callback);
    void ForAllParameters(std::function<void(Parameter &p)> callback);
    void ForAllParameters(std::function<void(Parameter &p, unsigned int index)> callback);

    void WriteDefinition(ofstream &stream, tabs t, Games::IDs game);
    void WriteDeclaration(ofstream &stream, tabs t, Games::IDs game);
    void WriteMeta(ofstream &stream, tabs t, Games::IDs game);

    string GetPtrTypeStr() const;
};
