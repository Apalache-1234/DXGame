
#include "PCH/pch.h"
#include "ComponentList.h"
#include "Entity.h"

#include "CameraComponent.h"
#include "Collider.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Rigidbody.h"
#include "SkinnedRenderer.h"
#include "SkyBox.h"
#include "SpotLight.h"
#include "UIRenderer.h"
#include "CardScript.h"
#include "CardTextureScript.h"
#include "CPUScript.h"
#include "DeckScript.h"
#include "FloorScript.h"
#include "GameEndScript.h"
#include "GameManagerScript.h"
#include "GameStartingScript.h"
#include "GameStartUIScript.h"
#include "HandScript.h"
#include "PlaceCardUIScript.h"
#include "ReadyUIScript.h"
#include "SquareWipeScript.h"
#include "TableAndChairScript.h"
#include "TableDeckScript.h"
#include "TitleCardScript.h"
#include "TitleFrameScript.h"
#include "TitleNoiseScript.h"
#include "TitleSpotLightScript.h"


namespace ComponentList
{
    void SetComponent(std::shared_ptr<Entity> ent, UINT32 compHash)
    {

       if (compHash == 3575595261)
       {
           ent->AddComponent<CameraComponent>();
           return;
       }


        
       if (compHash == 754720349)
       {
           ent->AddComponent<Collider>();
           return;
       }


        
       if (compHash == 848175905)
       {
           ent->AddComponent<DirectionalLight>();
           return;
       }


        
       if (compHash == 782866619)
       {
           ent->AddComponent<PointLight>();
           return;
       }


        
       if (compHash == 2738293960)
       {
           ent->AddComponent<Rigidbody>();
           return;
       }


        
       if (compHash == 2198297612)
       {
           ent->AddComponent<SkinnedRenderer>();
           return;
       }


        
       if (compHash == 3913943491)
       {
           ent->AddComponent<SkyBox>();
           return;
       }


        
       if (compHash == 1077287143)
       {
           ent->AddComponent<SpotLight>();
           return;
       }


        
       if (compHash == 2214367556)
       {
           ent->AddComponent<UIRenderer>();
           return;
       }


        
       if (compHash == 2096259208)
       {
           ent->AddComponent<CardScript>();
           return;
       }


        
       if (compHash == 818243501)
       {
           ent->AddComponent<CardTextureScript>();
           return;
       }


        
       if (compHash == 1138193222)
       {
           ent->AddComponent<CPUScript>();
           return;
       }


        
       if (compHash == 884461499)
       {
           ent->AddComponent<DeckScript>();
           return;
       }


        
       if (compHash == 2184881586)
       {
           ent->AddComponent<FloorScript>();
           return;
       }


        
       if (compHash == 3362839791)
       {
           ent->AddComponent<GameEndScript>();
           return;
       }


        
       if (compHash == 3500165469)
       {
           ent->AddComponent<GameManagerScript>();
           return;
       }


        
       if (compHash == 1155301492)
       {
           ent->AddComponent<GameStartingScript>();
           return;
       }


        
       if (compHash == 298209524)
       {
           ent->AddComponent<GameStartUIScript>();
           return;
       }


        
       if (compHash == 2142116387)
       {
           ent->AddComponent<HandScript>();
           return;
       }


        
       if (compHash == 2540812287)
       {
           ent->AddComponent<PlaceCardUIScript>();
           return;
       }


        
       if (compHash == 3369718845)
       {
           ent->AddComponent<ReadyUIScript>();
           return;
       }


        
       if (compHash == 4238859442)
       {
           ent->AddComponent<SquareWipeScript>();
           return;
       }


        
       if (compHash == 1442683194)
       {
           ent->AddComponent<TableAndChairScript>();
           return;
       }


        
       if (compHash == 835409033)
       {
           ent->AddComponent<TableDeckScript>();
           return;
       }


        
       if (compHash == 3440784604)
       {
           ent->AddComponent<TitleCardScript>();
           return;
       }


        
       if (compHash == 4265638481)
       {
           ent->AddComponent<TitleFrameScript>();
           return;
       }


        
       if (compHash == 3981409466)
       {
           ent->AddComponent<TitleNoiseScript>();
           return;
       }


        
       if (compHash == 3318188956)
       {
           ent->AddComponent<TitleSpotLightScript>();
           return;
       }


        

        
       SystemManager::GetInstance().Debug("存在しない、または生成できないコンポーネントを追加しようとしています");
    }

    void RemoveComponent(std::shared_ptr<Entity> ent, UINT32 compHash)
    {

       if (compHash == 3575595261)
       {
           ent->RemoveComponent<CameraComponent>();
           return;
       }


        
       if (compHash == 754720349)
       {
           ent->RemoveComponent<Collider>();
           return;
       }


        
       if (compHash == 848175905)
       {
           ent->RemoveComponent<DirectionalLight>();
           return;
       }


        
       if (compHash == 782866619)
       {
           ent->RemoveComponent<PointLight>();
           return;
       }


        
       if (compHash == 2738293960)
       {
           ent->RemoveComponent<Rigidbody>();
           return;
       }


        
       if (compHash == 2198297612)
       {
           ent->RemoveComponent<SkinnedRenderer>();
           return;
       }


        
       if (compHash == 3913943491)
       {
           ent->RemoveComponent<SkyBox>();
           return;
       }


        
       if (compHash == 1077287143)
       {
           ent->RemoveComponent<SpotLight>();
           return;
       }


        
       if (compHash == 2214367556)
       {
           ent->RemoveComponent<UIRenderer>();
           return;
       }


        
       if (compHash == 2096259208)
       {
           ent->RemoveComponent<CardScript>();
           return;
       }


        
       if (compHash == 818243501)
       {
           ent->RemoveComponent<CardTextureScript>();
           return;
       }


        
       if (compHash == 1138193222)
       {
           ent->RemoveComponent<CPUScript>();
           return;
       }


        
       if (compHash == 884461499)
       {
           ent->RemoveComponent<DeckScript>();
           return;
       }


        
       if (compHash == 2184881586)
       {
           ent->RemoveComponent<FloorScript>();
           return;
       }


        
       if (compHash == 3362839791)
       {
           ent->RemoveComponent<GameEndScript>();
           return;
       }


        
       if (compHash == 3500165469)
       {
           ent->RemoveComponent<GameManagerScript>();
           return;
       }


        
       if (compHash == 1155301492)
       {
           ent->RemoveComponent<GameStartingScript>();
           return;
       }


        
       if (compHash == 298209524)
       {
           ent->RemoveComponent<GameStartUIScript>();
           return;
       }


        
       if (compHash == 2142116387)
       {
           ent->RemoveComponent<HandScript>();
           return;
       }


        
       if (compHash == 2540812287)
       {
           ent->RemoveComponent<PlaceCardUIScript>();
           return;
       }


        
       if (compHash == 3369718845)
       {
           ent->RemoveComponent<ReadyUIScript>();
           return;
       }


        
       if (compHash == 4238859442)
       {
           ent->RemoveComponent<SquareWipeScript>();
           return;
       }


        
       if (compHash == 1442683194)
       {
           ent->RemoveComponent<TableAndChairScript>();
           return;
       }


        
       if (compHash == 835409033)
       {
           ent->RemoveComponent<TableDeckScript>();
           return;
       }


        
       if (compHash == 3440784604)
       {
           ent->RemoveComponent<TitleCardScript>();
           return;
       }


        
       if (compHash == 4265638481)
       {
           ent->RemoveComponent<TitleFrameScript>();
           return;
       }


        
       if (compHash == 3981409466)
       {
           ent->RemoveComponent<TitleNoiseScript>();
           return;
       }


        
       if (compHash == 3318188956)
       {
           ent->RemoveComponent<TitleSpotLightScript>();
           return;
       }


        

        
        SystemManager::GetInstance().Debug("存在しないコンポーネントを削除しようとしています");
    }
}
    