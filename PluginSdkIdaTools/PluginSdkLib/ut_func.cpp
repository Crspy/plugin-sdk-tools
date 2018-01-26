#include "ut_func.h"
#include "ut_string.h"

unsigned int Function::Find(qstring const &name, qvector<Function> const &entries) {
    for (auto const &i : entries) {
        if (i.m_name == name)
            return i.m_address;
    }
    return 0;
}

qvector<Function> Function::FromCSV(char const *filepath) {
    qvector<Function> entries;
    auto inFile = qfopen(filepath, "rt");
    if (inFile) {
        static char line[1024];
        if (qfgets(line, 1024, inFile)) {
            while (qfgets(line, 1024, inFile)) {
                Function entry;
                qstring addr, isConstStr, paramsStr;
                readcsv(line, addr, entry.m_module, entry.m_name, entry.m_demangledName, entry.m_type,
                    entry.m_cc, entry.m_retType, paramsStr, isConstStr, entry.m_comment);
                entry.m_address = toNumber(addr);
                entry.m_isConst = isConstStr != "0";
                // raw CPool<CPed> *:pool int:value
                // [raw] Type : Name
                size_t currPos = 0;
                while (1) {
                    auto colonPos = paramsStr.find(':', currPos);
                    if (colonPos == qstring::npos)
                        break;
                    qstring paramType = paramsStr.substr(currPos, colonPos);
                    paramType.trim2(' ');
                    Param param;
                    if (startsWith(paramType, "raw ")) {
                        param.m_rawType = true;
                        param.m_type = paramType.substr(4);
                    }
                    else
                        param.m_type = paramType;
                    auto spacePos = paramsStr.find(' ', colonPos + 1);
                    if (spacePos = qstring::npos) {
                        param.m_name = paramsStr.substr(colonPos + 1);
                        entry.m_params.push_back(param);
                        break;
                    }
                    param.m_name = paramsStr.substr(colonPos + 1, spacePos);
                    currPos = spacePos + 1;
                    entry.m_params.push_back(param);
                }
                entries.push_back(entry);
            }
        }
        else
            warning("File '%s' is empty", filepath);
        qfclose(inFile);
    }
    else
        warning("Unable to open '%s' file for reading", filepath);
    return entries;
}

qvector<Function> Function::FromReferenceCSV(char const *filepath, qvector<Function> const &baseFuncs) {
    qvector<Function> entries;
    size_t baseCount = baseFuncs.size();
    bool wrongAddresses = false;
    if (baseCount > 0) {
        auto inFile = qfopen(filepath, "rt");
        if (inFile) {
            static char line[1024];
            if (qfgets(line, 1024, inFile)) {
                unsigned int counter = 0;
                while (qfgets(line, 1024, inFile)) {
                    counter++;
                    if (baseCount < counter) {
                        entries.clear();
                        warning("Address count in reference file doesn't match with base file ('%s')", filepath);
                        break;
                    }
                    qstring addrBase, addrRef, refName;
                    readcsv(line, addrBase, addrRef, refName);
                    int intAddrBase = toNumber(addrBase);
                    int intAddrRef = toNumber(addrRef);
                    if (intAddrBase != baseFuncs[counter - 1].m_address) {
                        warning("Address '0x%X' in reference file doesn't match with address ('0x%X') in base file ('%s')",
                            intAddrBase, baseFuncs[counter - 1].m_address, filepath);
                        wrongAddresses = true;
                        break;
                    }
                    Function entry = baseFuncs[counter - 1];
                    entry.m_address = intAddrRef;
                    entries.push_back(entry);
                }
                if (!wrongAddresses && baseCount != counter) {
                    entries.clear();
                    warning("Address count in reference file doesn't match with base file ('%s')", filepath);
                }
            }
            else
                warning("File '%s' is empty", filepath);
            qfclose(inFile);
        }
        else
            warning("Unable to open '%s' file for reading", filepath);
    }
    return entries;
}

bool Function::ToCSV(qvector<Function> const &entries, char const *filepath, char const *version) {
    auto outFile = qfopen(filepath, "wt");
    if (outFile) {
        // header
        qfprintf(outFile, "%s,Module,Name,DemangledName,Type,CC,RetType,Parameters,IsConst,Comment\n", version);
        // entries
        for (auto const &i : entries) {
            qstring retType;
            if (i.m_rawRetType)
                retType = "raw ";
            retType += i.m_rawRetType;
            qfprintf(outFile, "0x%X,%s,%s,%s,%s,%s,%s,", i.m_address,
                csvvalue(i.m_module).c_str(),
                csvvalue(i.m_name).c_str(),
                csvvalue(i.m_demangledName).c_str(),
                csvvalue(i.m_type).c_str(),
                csvvalue(i.m_cc).c_str(),
                csvvalue(retType).c_str());
            qstring parameters;
            for (size_t p = 0; p < i.m_params.size(); p++) {
                if (i.m_params[p].m_rawType)
                    parameters += "raw ";
                parameters += i.m_params[p].m_type;
                parameters += ':';
                parameters += i.m_params[p].m_name;
                if (p != (i.m_params.size() - 1))
                    parameters += ' ';
            }
            qfprintf(outFile, "%s,%d,%s\n", csvvalue(parameters).c_str(), i.m_isConst, csvvalue(i.m_comment).c_str());
        }
        qfclose(outFile);
        return true;
    }
    warning("Unable to open '%s' file for writing", filepath);
    return false;
}

bool Function::ToReferenceCSV(qvector<Function> const &baseEntries, char const *baseVersion,
    qvector<unsigned int> const &addresses, char const *version, char const *filepath)
{
    auto outFile = qfopen(filepath, "wt");
    if (outFile) {
        qfprintf(outFile, "%s,%s,NameComment\n", baseVersion, version);
        for (size_t i = 0; i < baseEntries.size(); i++)
            qfprintf(outFile, "0x%X,0x%X,%s\n", baseEntries[i].m_address, addresses[i], csvvalue(baseEntries[i].m_demangledName).c_str());
        qfclose(outFile);
        return true;
    }
    warning("Unable to open '%s' for writing", filepath);
    return false;
}

bool Function::ToReferenceCSV(qvector<Function> const &baseEntries, char const *baseVersion,
    qvector<Function> const &entries, char const *version, char const *filepath)
{
    qvector<unsigned int> addresses;
    for (size_t i = 0; i < baseEntries.size(); i++) {
        if (!baseEntries[i].m_name.empty())
            addresses.push_back(Function::Find(baseEntries[i].m_name, entries));
        else
            addresses.push_back(0);
        
    }
    return ToReferenceCSV(baseEntries, baseVersion, addresses, version, filepath);
}
