#include <3rdPersonTemplatePlugin/3rdPersonTemplatePluginPCH.h>

// Plasma
#include <Core/WorldSerializer/WorldReader.h>
#include <Core/WorldSerializer/WorldWriter.h>
#include <GameEngine/Gameplay/InputComponent.h>

// Game
#include <3rdPersonTemplatePlugin/Player/TPPlayerCamera.h>
#include <3rdPersonTemplatePlugin/Player/TPPlayerController.h>
#include <3rdPersonTemplatePlugin/GameState/3rdPersonTemplateGameState.h>

plCVarBool cvar_InvertCamera("Game.Camera.Invert", false, plCVarFlags::Save, "Inverts up and down camera motion");

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(TPPlayerCameraManager, 1, plRTTINoAllocator)
PL_END_DYNAMIC_REFLECTED_TYPE;

TPPlayerCameraManager::TPPlayerCameraManager(plWorld* pWorld)
  : plComponentManager<TPPlayerCameraComponent, plBlockStorageType::Compact>(pWorld)
{
}

void TPPlayerCameraManager::Initialize()
{
  auto desc = plWorldModule::UpdateFunctionDesc(plWorldModule::UpdateFunction(&TPPlayerCameraManager::Update, this), "Camera Update");
  desc.m_Phase = plWorldModule::UpdateFunctionDesc::Phase::PostAsync;
  desc.m_bOnlyUpdateWhenSimulating = true;
  this->RegisterUpdateFunction(desc);
}

void TPPlayerCameraManager::Update(const plWorldModule::UpdateContext &context)
{
  for (auto it = this->m_ComponentStorage.GetIterator(context.m_uiFirstComponentIndex, context.m_uiComponentCount); it.IsValid(); ++it)
  {
    ComponentType* pComponent = it;
    if (pComponent->IsActiveAndInitialized())
    {
      pComponent->Update();
    }
  }
}

PL_BEGIN_COMPONENT_TYPE(TPPlayerCameraComponent, 1, plComponentMode::Dynamic)
{
  PL_BEGIN_PROPERTIES
  {
    PL_ACCESSOR_PROPERTY("CameraDragStrength", GetCameraDragStrength, SetCameraDragStrength)->AddAttributes(new plDefaultValueAttribute(5.0f)),
    PL_ACCESSOR_PROPERTY("MaxDistance", GetMaxCameraDistance, SetMaxCameraDistance)->AddAttributes(new plDefaultValueAttribute(5.0f)),
    PL_ACCESSOR_PROPERTY("CameraObject", GetCameraObject, SetCameraObjectHandle)->AddAttributes(new plGameObjectReferenceAttribute()),
    PL_MEMBER_PROPERTY("CollisionLayer", m_uiCollisionLayer)->AddAttributes(new plDynamicEnumAttribute("PhysicsCollisionLayer")),

  }
  PL_END_PROPERTIES;
  PL_BEGIN_ATTRIBUTES
  {
    new plCategoryAttribute("Game/Player"), // Component menu group
  }
  PL_END_ATTRIBUTES;
}
PL_END_COMPONENT_TYPE


TPPlayerCameraComponent::TPPlayerCameraComponent() = default;
TPPlayerCameraComponent::~TPPlayerCameraComponent() = default;

void TPPlayerCameraComponent::SetCameraObjectHandle(const char* szGuid)
{
  const auto resolver = GetWorld()->GetGameObjectReferenceResolver();

  if (!resolver.IsValid())
    return;

  m_hCamera = resolver(szGuid, GetHandle(), "CameraObject");
}

const char* TPPlayerCameraComponent::GetCameraObject() const
{
  return nullptr;
}

void TPPlayerCameraComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();


  ThirdPersonTemplateGameState::GetSingleton()->m_pMainCamera = this;

  // this component doesn't need to anything for initialization
}

void TPPlayerCameraComponent::Update()
{
  if (!IsActiveAndSimulating())
    return;

  if (m_pPhysicsModule == nullptr)
  {
    m_pPhysicsModule = GetWorld()->GetModule<plPhysicsWorldModuleInterface>();

    if (m_pPhysicsModule == nullptr)
      return;
  }

	if(m_pPlayerController == nullptr)
		m_pPlayerController = ThirdPersonTemplateGameState::GetSingleton()->m_pLocalPlayerController;

  plGameObject* pPlayerController = m_pPlayerController->GetOwner();
  if (pPlayerController)
  {
    plVec3 cameraPos = GetOwner()->GetGlobalPosition();
    plVec3 playerPos = pPlayerController->GetGlobalPosition();
    playerPos.z += 1;

    float distance = plMath::Abs(cameraPos.GetLength() - playerPos.GetLength());

    distance = plMath::Max(0.1f, distance - 0.2f);
    plVec3 newPos = playerPos;
    GetOwner()->SetGlobalPosition(newPos);
  }

  m_pPlayerController->Update(GetOwner()->GetGlobalRotation());

	plInputComponent* pInput = nullptr;
	if (!GetOwner()->TryGetComponentOfBaseType(pInput))
	{
		return;
	}

	float left  = pInput->GetCurrentInputState("RotateLeft", false);
	float right = pInput->GetCurrentInputState("RotateRight", false);
  float up    = pInput->GetCurrentInputState("LookUp", false);
  float down  = pInput->GetCurrentInputState("LookDown", false);

  m_NewHorizontalRotation += plAngle::MakeFromRadian(right - left);

  float verticalDiff = down - up;
  if(cvar_InvertCamera)
  {
    verticalDiff =  -verticalDiff;
  }

  m_NewVerticalRotation += plAngle::MakeFromRadian(verticalDiff);
  m_NewVerticalRotation = plMath::Clamp(m_NewVerticalRotation, -m_MaxVerticalRotation, m_MaxVerticalRotation);

  plQuat qHorizontal = plQuat::MakeFromAxisAndAngle(plVec3(0, 0, 1), m_NewHorizontalRotation);
  plQuat qVertical = plQuat::MakeFromAxisAndAngle(plVec3(0, 1, 0), m_NewVerticalRotation);
  plQuat qCombined = qHorizontal * qVertical;
  GetOwner()->SetLocalRotation(qCombined);

  // Push-pull the camera
  plGameObject* pCameraObject = GetOwner()->FindChildByName("Camera");

  if (pCameraObject)
  {
    // Define the starting position (player's position) and direction towards the camera's intended position
    plVec3 vOwnPos = m_pPlayerController->GetOwner()->GetGlobalPosition();
    plVec3 vCameraTargetPos = GetOwner()->GetGlobalPosition() - (GetOwner()->GetGlobalDirForwards() * m_fMaxCameraDistance);
    plVec3 vDirToCamera = vCameraTargetPos - vOwnPos;
    const float fRayLenToCamera = vDirToCamera.GetLength<float>();

    // Normalize the direction vector
    vDirToCamera.Normalize();

    // Slight offset to prevent detecting ground at feet
    plVec3 vStartPos = vOwnPos + plVec3(0, 0, 0.01f);
    plPhysicsCastResult hit;
    bool bHit = m_pPhysicsModule->Raycast(hit, vStartPos, vDirToCamera, fRayLenToCamera,
                       plPhysicsQueryParameters(m_uiCollisionLayer, plPhysicsShapeType::Static | plPhysicsShapeType::Dynamic));

    if (bHit && hit.m_hActorObject != GetOwner()->GetHandle())
    {
      // Collision detected, adjust the camera position
      plVec3 collisionPoint = hit.m_vPosition;
      plVec3 collisionNormal = hit.m_vNormal;

      // Move the camera to the collision point, slightly offset to avoid clipping
      float offsetDistance = 0.1f; // Adjust this value as needed
      plVec3 newCameraPos = collisionPoint + collisionNormal * offsetDistance;
      pCameraObject->SetGlobalPosition(newCameraPos);
    }
    else
    {
      // No collision detected, set camera to intended position
      pCameraObject->SetGlobalPosition(vCameraTargetPos);
    }
  }

	m_CurHorizontalRotation = m_NewHorizontalRotation;
  m_CurVerticalRotation = m_NewVerticalRotation;
}

void TPPlayerCameraComponent::SerializeComponent(plWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();
  stream.WriteGameObjectHandle(m_hCamera);
  s << m_uiCollisionLayer;
  s << m_fMaxCameraDistance;
}

void TPPlayerCameraComponent::DeserializeComponent(plWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const plUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();
  m_hCamera = stream.ReadGameObjectHandle();
  s >> m_uiCollisionLayer;
  s >> m_fMaxCameraDistance;
}
