import xml.etree.ElementTree as XET
import glob
import json
import os

classParent = {}
TreeData : list[list[tuple]] = []
depth = 0
leaveDepth = "leaveDepth"

def ParseXML():
    global classParent

    xmls = glob.glob("xml/class_*.xml")

    print(f"{len(xmls)}")

    for xml in xmls:
        tree = XET.parse(xml)
        root = tree.getroot()

        for compound in root.findall(".//compounddef"):
            if compound.get("kind") in ["class", "struct"]:
                className = compound.findtext("compoundname")

                parents = []

                if className != "Component":
                    for parent in compound.findall("basecompoundref"):
                        parents.append(parent.text)

                classParent[className] = parents

def MakeTree():
    global TreeData
    global depth

    ParseXML()  

    for child , parent in classParent.items():
        if not parent:
            TreeData.append([(child , depth , "" , -1)])

    isNoChild = False
    while not isNoChild:
        isNoChild = True
        widthData = TreeData[depth]
        childTree = []
        i = 0
        for data in widthData:
            for child , parent in classParent.items():
                for base in parent:
                    if base == data[0]:
                        childTree.append((child , depth + 1 , base , i))
                        isNoChild = False
            i = i + 1
        
        depth = depth + 1
        TreeData.append(childTree)

def MakeLeaf(depthCur , jsonData : dict[str,int]) -> str:
    global TreeData
    global depth

    text = ""
    widthData = TreeData[depthCur]
    i = 0
    for child in widthData:
        text += f"        {{{jsonData[child[0]]} , {child[3]}}} ,\n"
        i = i + 1

    text = text[:-2]
    return text

def MakeLeave(jsonData : dict[str,int]) -> str:
    global depth
    global leaveDepth

    depthCur = 0
    text = ""

    while not depthCur == depth:
        text += f"""
    inline constexpr LeafData {leaveDepth}{depthCur}[] =
    {{
{MakeLeaf(depthCur , jsonData)}
    }};\n\n
    """
        depthCur += 1
    return text


def MakeVar() -> str:
    global leaveDepth
    text = ""
    for i in range(len(TreeData) - 1):
        text += f"        {leaveDepth}{i},\n"
    text = text[:-2]
    return text

def Start(jsonData : dict[str,int]) -> str:
    MakeTree()

    code = f"""
    struct LeafData
    {{
        UINT32 hash;
        INT32 parentIndex = -1;
    }};

{MakeLeave(jsonData)}

inline constexpr std::span<const LeafData> tree[] = {{
{MakeVar()}
    }};
"""
    return code

def Test():
    jsonPath = "JsonForHash.json"
    jsonData : dict[str,int] = {}
    if os.path.exists(jsonPath):
        with open(jsonPath , "r" , encoding="utf-8") as f:
            jsonData = json.load(f)
    else:
        print("ファイルがない")
    Start(jsonData)