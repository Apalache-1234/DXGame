import os
import json
import glob
import re
import sys
import ParentTreeGenerater

FNV32_OFFSET = 0x811C9DC5   # 2166136261
FNV32_PRIME = 0x01000193    # 16777619
FNV64_OFFSET = 0xCBF29CE484222325
FNV64_PRIME = 0x100000001B3

jsonPath = "JsonForHash.json"
jsonData : dict[str,int] = {}
hashList : list[int] = []

def fnv1a_32(data: bytes) -> int:
    h = FNV32_OFFSET
    for b in data:
        h ^= b
        h = (h * FNV32_PRIME) & 0xFFFFFFFF
    return h

def GetNameHash(name : str) -> int:
    hashValue = fnv1a_32(name.encode("utf-8"))
    while hashValue in hashList:
        hashValue =+ 1
    hashList.append(hashValue)
    return hashValue

def AddInclude(list : list[str]) -> str:
    text = ""

    for name in list:
        text += f"""#include "{name}.h"\n"""

    return text

def AddSetFunctionText(list : list[str]) -> str:
    text = ""

    for name in list:
        text += f"""
       if (compHash == {jsonData[name]})
       {{
           ent->AddComponent<{name}>();
           return;
       }}\n\n
        """

    return text

def AddRemoveFunctionText(list : list[str]) -> str:
    text = ""

    for name in list:
        text += f"""
       if (compHash == {jsonData[name]})
       {{
           ent->RemoveComponent<{name}>();
           return;
       }}\n\n
        """

    return text

def AddGetNameFunctionText(list : list[str]) -> str:
    text = ""

    for name in list:
        text += f"""
    {{
        const auto& compCast = std::dynamic_pointer_cast<{name}>(comp);

        if (compCast)
        {{
            return "{name}";
        }}
    }}\n\n"""

    return text

def SearchClassName(path , name):
        with open(path , "r" , encoding="cp932") as file:
            codes = file.read()
            pattern = rf'\s+{name}[\s+;]'

            text = re.findall(pattern , codes)

            if not text:
                print("ヘッダーのファイル名とクラス名が一致しません")
                sys.exit(1)

def MakeJsonForHash(list : list[str]):
    global jsonData
    data : dict[str , int] = {}
    if os.path.exists(jsonPath):
        with open(jsonPath , "r" , encoding="utf-8") as f:
            data = json.load(f)
        for name in list:
            if not name in data:
                data[name] = GetNameHash(name)
    else:
        for name in list:
            data[name] = GetNameHash(name)

    jsonData = data.copy()

    with open(jsonPath , "w" , encoding="utf-8") as f:
         json.dump(data , f , indent=4)
    return

def MakeStructArray(list : list[str]) -> str:
    text = ""
    for name in list:
        text += f"""        {{{jsonData[name]} , "{name}"}},\n"""
    text = text[:-1]
    return text

def MakeIntHash(list : list[str]) -> str:
    text = ""
    
    for name in list:
        text += f"""    template<> inline constexpr UINT32 COMPONENT_HASH<{name}> = {jsonData[name]};\n"""

    return text

def MakeNames(list : list[str]) -> str:
    text = ""
    
    for name in list:
        text += f"""    template<> inline constexpr std::string_view COMPONENT_NAME<{name}> = "{name}";\n"""

    return text

def MakeForwardClass(list : list[str]) -> str:
    text = ""
    
    for name in list:
        text += f"""class {name};\n"""

    return text

def ReplaceExtentToNone(fileNames):

    fileNamesWithoutEx = []

    for name in fileNames:
        nameWOPath = os.path.basename(name)
        nameWOPath = nameWOPath.replace(".h" , "") 
        SearchClassName(name , nameWOPath)
        fileNamesWithoutEx.append(nameWOPath)

    return fileNamesWithoutEx

def MakeFile():
    fileNames = glob.glob("include/**/*.h" , recursive = True)
    fileNames.extend(glob.glob("Script/include/*.h"))
    fileNamesWithoutEx : list[str]

    fileNamesWithoutEx = ReplaceExtentToNone(fileNames);

    MakeJsonForHash(fileNamesWithoutEx)

    headerCode = f"""
#ifndef COMPONENTLIST_H
#define COMPONENTLIST_H
#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR) / sizeof(*(_ARR)))) 

#include <iostream>
#include <vector>
#include <Windows.h>

{MakeForwardClass(fileNamesWithoutEx)}
class Entity;

namespace ComponentList
{{
    template<typename T>
    inline constexpr UINT32 COMPONENT_HASH =  0;
{MakeIntHash(fileNamesWithoutEx)}

    template<typename T>
    inline constexpr std::string_view COMPONENT_NAME = "";
{MakeNames(fileNamesWithoutEx)}

{ParentTreeGenerater.Start(jsonData)}

    struct TypeData
    {{
        UINT32 hash;
        std::string_view name;
    }};

    inline constexpr TypeData typeDatas[] =
    {{
{MakeStructArray(fileNamesWithoutEx)}
    }};

    void SetComponent(std::shared_ptr<Entity> ent, UINT32 compHash);

    void RemoveComponent(std::shared_ptr<Entity> ent, UINT32 compHash);

    template<typename T>
    bool IsParentComponent(UINT32 treeIndex , UINT32 leafIndex)
    {{
        const UINT32& parentHash = COMPONENT_HASH<T>;
        LeafData childLeaf = tree[treeIndex][leafIndex];
        int depth = treeIndex;

        int index = childLeaf.parentIndex;

        while (index != -1 || depth != 0)
        {{
            depth--;
            if (tree[depth][index].hash == parentHash)
                return true;
            index = tree[depth][index].parentIndex;
        }}
       
        return false;
    }}
}}
#endif
    """

    outputFile = "ComponentList/include/ComponentList.h"

    with open(outputFile , "w" , encoding="utf-8") as file:
        file.write(headerCode)

    fileNames = glob.glob("include/*.h" , recursive = True)
    fileNames.extend(glob.glob("Script/include/*.h"))
    fileNamesWithoutEx : list[str]

    fileNamesWithoutEx = ReplaceExtentToNone(fileNames);

    cppCode = f"""
#include "PCH/pch.h"
#include "ComponentList.h"
#include "Entity.h"

{AddInclude(fileNamesWithoutEx)}

namespace ComponentList
{{
    void SetComponent(std::shared_ptr<Entity> ent, UINT32 compHash)
    {{
{AddSetFunctionText(fileNamesWithoutEx)}

        
       SystemManager::GetInstance().Debug("存在しない、または生成できないコンポーネントを追加しようとしています");
    }}

    void RemoveComponent(std::shared_ptr<Entity> ent, UINT32 compHash)
    {{
{AddRemoveFunctionText(fileNamesWithoutEx)}

        
        SystemManager::GetInstance().Debug("存在しないコンポーネントを削除しようとしています");
    }}
}}
    """

    outputFile = "ComponentList/src/ComponentList.cpp"

    with open(outputFile , "w" , encoding="utf-8-sig") as file:
        file.write(cppCode)
    
    return

MakeFile()