
#ifndef COMPONENTLIST_H
#define COMPONENTLIST_H
#define IM_ARRAYSIZE(_ARR)          ((int)(sizeof(_ARR) / sizeof(*(_ARR)))) 

#include <iostream>
#include <vector>
#include <Windows.h>

class CameraComponent;
class Collider;
class DirectionalLight;
class PointLight;
class Rigidbody;
class SkinnedRenderer;
class SkyBox;
class SpotLight;
class UIRenderer;
class Component;
class DebugRenderer;
class Light;
class PostEffect;
class Renderer;
class ScriptComponent;
class DebugCollider;
class DebugDirectionalLight;
class DebugGrids;
class DebugOrigin;
class DebugPointLight;
class DebugRayCast;
class DebugSpotLight;
class DebugUIRenderer;
class PEEdgeFilter;
class PEHorizonGaussianBlur;
class PEMosaicFilter;
class PENoiseFilter;
class PERadicalBlur;
class PEVerticalGaussianBlur;
class CardScript;
class CardTextureScript;
class CPUScript;
class DeckScript;
class FloorScript;
class GameEndScript;
class GameManagerScript;
class GameStartingScript;
class GameStartUIScript;
class HandScript;
class PlaceCardUIScript;
class ReadyUIScript;
class SquareWipeScript;
class TableAndChairScript;
class TableDeckScript;
class TitleCardScript;
class TitleFrameScript;
class TitleNoiseScript;
class TitleSpotLightScript;

class Entity;

namespace ComponentList
{
    template<typename T>
    inline constexpr UINT32 COMPONENT_HASH =  0;
    template<> inline constexpr UINT32 COMPONENT_HASH<CameraComponent> = 3575595261;
    template<> inline constexpr UINT32 COMPONENT_HASH<Collider> = 754720349;
    template<> inline constexpr UINT32 COMPONENT_HASH<DirectionalLight> = 848175905;
    template<> inline constexpr UINT32 COMPONENT_HASH<PointLight> = 782866619;
    template<> inline constexpr UINT32 COMPONENT_HASH<Rigidbody> = 2738293960;
    template<> inline constexpr UINT32 COMPONENT_HASH<SkinnedRenderer> = 2198297612;
    template<> inline constexpr UINT32 COMPONENT_HASH<SkyBox> = 3913943491;
    template<> inline constexpr UINT32 COMPONENT_HASH<SpotLight> = 1077287143;
    template<> inline constexpr UINT32 COMPONENT_HASH<UIRenderer> = 2214367556;
    template<> inline constexpr UINT32 COMPONENT_HASH<Component> = 718440320;
    template<> inline constexpr UINT32 COMPONENT_HASH<DebugRenderer> = 1763775919;
    template<> inline constexpr UINT32 COMPONENT_HASH<Light> = 2329082063;
    template<> inline constexpr UINT32 COMPONENT_HASH<PostEffect> = 1429954018;
    template<> inline constexpr UINT32 COMPONENT_HASH<Renderer> = 1498036510;
    template<> inline constexpr UINT32 COMPONENT_HASH<ScriptComponent> = 1193747209;
    template<> inline constexpr UINT32 COMPONENT_HASH<DebugCollider> = 731954124;
    template<> inline constexpr UINT32 COMPONENT_HASH<DebugDirectionalLight> = 3177182272;
    template<> inline constexpr UINT32 COMPONENT_HASH<DebugGrids> = 2894991665;
    template<> inline constexpr UINT32 COMPONENT_HASH<DebugOrigin> = 3465866426;
    template<> inline constexpr UINT32 COMPONENT_HASH<DebugPointLight> = 1931597330;
    template<> inline constexpr UINT32 COMPONENT_HASH<DebugRayCast> = 218704851;
    template<> inline constexpr UINT32 COMPONENT_HASH<DebugSpotLight> = 2864534580;
    template<> inline constexpr UINT32 COMPONENT_HASH<DebugUIRenderer> = 1079839001;
    template<> inline constexpr UINT32 COMPONENT_HASH<PEEdgeFilter> = 1114763505;
    template<> inline constexpr UINT32 COMPONENT_HASH<PEHorizonGaussianBlur> = 3160968123;
    template<> inline constexpr UINT32 COMPONENT_HASH<PEMosaicFilter> = 3136265188;
    template<> inline constexpr UINT32 COMPONENT_HASH<PENoiseFilter> = 1879205770;
    template<> inline constexpr UINT32 COMPONENT_HASH<PERadicalBlur> = 1507482013;
    template<> inline constexpr UINT32 COMPONENT_HASH<PEVerticalGaussianBlur> = 1395031206;
    template<> inline constexpr UINT32 COMPONENT_HASH<CardScript> = 2096259208;
    template<> inline constexpr UINT32 COMPONENT_HASH<CardTextureScript> = 818243501;
    template<> inline constexpr UINT32 COMPONENT_HASH<CPUScript> = 1138193222;
    template<> inline constexpr UINT32 COMPONENT_HASH<DeckScript> = 884461499;
    template<> inline constexpr UINT32 COMPONENT_HASH<FloorScript> = 2184881586;
    template<> inline constexpr UINT32 COMPONENT_HASH<GameEndScript> = 3362839791;
    template<> inline constexpr UINT32 COMPONENT_HASH<GameManagerScript> = 3500165469;
    template<> inline constexpr UINT32 COMPONENT_HASH<GameStartingScript> = 1155301492;
    template<> inline constexpr UINT32 COMPONENT_HASH<GameStartUIScript> = 298209524;
    template<> inline constexpr UINT32 COMPONENT_HASH<HandScript> = 2142116387;
    template<> inline constexpr UINT32 COMPONENT_HASH<PlaceCardUIScript> = 2540812287;
    template<> inline constexpr UINT32 COMPONENT_HASH<ReadyUIScript> = 3369718845;
    template<> inline constexpr UINT32 COMPONENT_HASH<SquareWipeScript> = 4238859442;
    template<> inline constexpr UINT32 COMPONENT_HASH<TableAndChairScript> = 1442683194;
    template<> inline constexpr UINT32 COMPONENT_HASH<TableDeckScript> = 835409033;
    template<> inline constexpr UINT32 COMPONENT_HASH<TitleCardScript> = 3440784604;
    template<> inline constexpr UINT32 COMPONENT_HASH<TitleFrameScript> = 4265638481;
    template<> inline constexpr UINT32 COMPONENT_HASH<TitleNoiseScript> = 3981409466;
    template<> inline constexpr UINT32 COMPONENT_HASH<TitleSpotLightScript> = 3318188956;


    template<typename T>
    inline constexpr std::string_view COMPONENT_NAME = "";
    template<> inline constexpr std::string_view COMPONENT_NAME<CameraComponent> = "CameraComponent";
    template<> inline constexpr std::string_view COMPONENT_NAME<Collider> = "Collider";
    template<> inline constexpr std::string_view COMPONENT_NAME<DirectionalLight> = "DirectionalLight";
    template<> inline constexpr std::string_view COMPONENT_NAME<PointLight> = "PointLight";
    template<> inline constexpr std::string_view COMPONENT_NAME<Rigidbody> = "Rigidbody";
    template<> inline constexpr std::string_view COMPONENT_NAME<SkinnedRenderer> = "SkinnedRenderer";
    template<> inline constexpr std::string_view COMPONENT_NAME<SkyBox> = "SkyBox";
    template<> inline constexpr std::string_view COMPONENT_NAME<SpotLight> = "SpotLight";
    template<> inline constexpr std::string_view COMPONENT_NAME<UIRenderer> = "UIRenderer";
    template<> inline constexpr std::string_view COMPONENT_NAME<Component> = "Component";
    template<> inline constexpr std::string_view COMPONENT_NAME<DebugRenderer> = "DebugRenderer";
    template<> inline constexpr std::string_view COMPONENT_NAME<Light> = "Light";
    template<> inline constexpr std::string_view COMPONENT_NAME<PostEffect> = "PostEffect";
    template<> inline constexpr std::string_view COMPONENT_NAME<Renderer> = "Renderer";
    template<> inline constexpr std::string_view COMPONENT_NAME<ScriptComponent> = "ScriptComponent";
    template<> inline constexpr std::string_view COMPONENT_NAME<DebugCollider> = "DebugCollider";
    template<> inline constexpr std::string_view COMPONENT_NAME<DebugDirectionalLight> = "DebugDirectionalLight";
    template<> inline constexpr std::string_view COMPONENT_NAME<DebugGrids> = "DebugGrids";
    template<> inline constexpr std::string_view COMPONENT_NAME<DebugOrigin> = "DebugOrigin";
    template<> inline constexpr std::string_view COMPONENT_NAME<DebugPointLight> = "DebugPointLight";
    template<> inline constexpr std::string_view COMPONENT_NAME<DebugRayCast> = "DebugRayCast";
    template<> inline constexpr std::string_view COMPONENT_NAME<DebugSpotLight> = "DebugSpotLight";
    template<> inline constexpr std::string_view COMPONENT_NAME<DebugUIRenderer> = "DebugUIRenderer";
    template<> inline constexpr std::string_view COMPONENT_NAME<PEEdgeFilter> = "PEEdgeFilter";
    template<> inline constexpr std::string_view COMPONENT_NAME<PEHorizonGaussianBlur> = "PEHorizonGaussianBlur";
    template<> inline constexpr std::string_view COMPONENT_NAME<PEMosaicFilter> = "PEMosaicFilter";
    template<> inline constexpr std::string_view COMPONENT_NAME<PENoiseFilter> = "PENoiseFilter";
    template<> inline constexpr std::string_view COMPONENT_NAME<PERadicalBlur> = "PERadicalBlur";
    template<> inline constexpr std::string_view COMPONENT_NAME<PEVerticalGaussianBlur> = "PEVerticalGaussianBlur";
    template<> inline constexpr std::string_view COMPONENT_NAME<CardScript> = "CardScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<CardTextureScript> = "CardTextureScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<CPUScript> = "CPUScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<DeckScript> = "DeckScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<FloorScript> = "FloorScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<GameEndScript> = "GameEndScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<GameManagerScript> = "GameManagerScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<GameStartingScript> = "GameStartingScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<GameStartUIScript> = "GameStartUIScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<HandScript> = "HandScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<PlaceCardUIScript> = "PlaceCardUIScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<ReadyUIScript> = "ReadyUIScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<SquareWipeScript> = "SquareWipeScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<TableAndChairScript> = "TableAndChairScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<TableDeckScript> = "TableDeckScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<TitleCardScript> = "TitleCardScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<TitleFrameScript> = "TitleFrameScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<TitleNoiseScript> = "TitleNoiseScript";
    template<> inline constexpr std::string_view COMPONENT_NAME<TitleSpotLightScript> = "TitleSpotLightScript";



    struct LeafData
    {
        UINT32 hash;
        INT32 parentIndex = -1;
    };


    inline constexpr LeafData leaveDepth0[] =
    {
        {718440320 , -1} 
    };


    
    inline constexpr LeafData leaveDepth1[] =
    {
        {3575595261 , 0} ,
        {754720349 , 0} ,
        {2329082063 , 0} ,
        {1498036510 , 0} ,
        {2738293960 , 0} ,
        {1193747209 , 0} 
    };


    
    inline constexpr LeafData leaveDepth2[] =
    {
        {848175905 , 2} ,
        {782866619 , 2} ,
        {1077287143 , 2} ,
        {1763775919 , 3} ,
        {1429954018 , 3} ,
        {2198297612 , 3} ,
        {3913943491 , 3} ,
        {2214367556 , 3} ,
        {2096259208 , 5} ,
        {818243501 , 5} ,
        {1138193222 , 5} ,
        {884461499 , 5} ,
        {2184881586 , 5} ,
        {3362839791 , 5} ,
        {3500165469 , 5} ,
        {1155301492 , 5} ,
        {298209524 , 5} ,
        {2142116387 , 5} ,
        {2540812287 , 5} ,
        {3369718845 , 5} ,
        {4238859442 , 5} ,
        {1442683194 , 5} ,
        {835409033 , 5} ,
        {3440784604 , 5} ,
        {4265638481 , 5} ,
        {3981409466 , 5} ,
        {3318188956 , 5} 
    };


    
    inline constexpr LeafData leaveDepth3[] =
    {
        {731954124 , 3} ,
        {3177182272 , 3} ,
        {2894991665 , 3} ,
        {3465866426 , 3} ,
        {1931597330 , 3} ,
        {218704851 , 3} ,
        {2864534580 , 3} ,
        {1079839001 , 3} ,
        {1114763505 , 4} ,
        {3160968123 , 4} ,
        {3136265188 , 4} ,
        {1879205770 , 4} ,
        {1507482013 , 4} ,
        {1395031206 , 4} 
    };


    

inline constexpr std::span<const LeafData> tree[] = {
        leaveDepth0,
        leaveDepth1,
        leaveDepth2,
        leaveDepth3
    };


    struct TypeData
    {
        UINT32 hash;
        std::string_view name;
    };

    inline constexpr TypeData typeDatas[] =
    {
        {3575595261 , "CameraComponent"},
        {754720349 , "Collider"},
        {848175905 , "DirectionalLight"},
        {782866619 , "PointLight"},
        {2738293960 , "Rigidbody"},
        {2198297612 , "SkinnedRenderer"},
        {3913943491 , "SkyBox"},
        {1077287143 , "SpotLight"},
        {2214367556 , "UIRenderer"},
        {718440320 , "Component"},
        {1763775919 , "DebugRenderer"},
        {2329082063 , "Light"},
        {1429954018 , "PostEffect"},
        {1498036510 , "Renderer"},
        {1193747209 , "ScriptComponent"},
        {731954124 , "DebugCollider"},
        {3177182272 , "DebugDirectionalLight"},
        {2894991665 , "DebugGrids"},
        {3465866426 , "DebugOrigin"},
        {1931597330 , "DebugPointLight"},
        {218704851 , "DebugRayCast"},
        {2864534580 , "DebugSpotLight"},
        {1079839001 , "DebugUIRenderer"},
        {1114763505 , "PEEdgeFilter"},
        {3160968123 , "PEHorizonGaussianBlur"},
        {3136265188 , "PEMosaicFilter"},
        {1879205770 , "PENoiseFilter"},
        {1507482013 , "PERadicalBlur"},
        {1395031206 , "PEVerticalGaussianBlur"},
        {2096259208 , "CardScript"},
        {818243501 , "CardTextureScript"},
        {1138193222 , "CPUScript"},
        {884461499 , "DeckScript"},
        {2184881586 , "FloorScript"},
        {3362839791 , "GameEndScript"},
        {3500165469 , "GameManagerScript"},
        {1155301492 , "GameStartingScript"},
        {298209524 , "GameStartUIScript"},
        {2142116387 , "HandScript"},
        {2540812287 , "PlaceCardUIScript"},
        {3369718845 , "ReadyUIScript"},
        {4238859442 , "SquareWipeScript"},
        {1442683194 , "TableAndChairScript"},
        {835409033 , "TableDeckScript"},
        {3440784604 , "TitleCardScript"},
        {4265638481 , "TitleFrameScript"},
        {3981409466 , "TitleNoiseScript"},
        {3318188956 , "TitleSpotLightScript"},
    };

    void SetComponent(std::shared_ptr<Entity> ent, UINT32 compHash);

    void RemoveComponent(std::shared_ptr<Entity> ent, UINT32 compHash);

    template<typename T>
    bool IsParentComponent(UINT32 treeIndex , UINT32 leafIndex)
    {
        const UINT32& parentHash = COMPONENT_HASH<T>;
        LeafData childLeaf = tree[treeIndex][leafIndex];
        int depth = treeIndex;

        int index = childLeaf.parentIndex;

        while (index != -1 || depth != 0)
        {
            depth--;
            if (tree[depth][index].hash == parentHash)
                return true;
            index = tree[depth][index].parentIndex;
        }
       
        return false;
    }
}
#endif
    