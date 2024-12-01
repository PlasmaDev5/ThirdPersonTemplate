#include <3rdPersonTemplatePlugin/3rdPersonTemplatePluginPCH.h>

// Plasma
#include <JoltPlugin/Character/JoltDefaultCharacterComponent.h>
#include <GameEngine/Physics/CharacterControllerComponent.h>
#include <GameEngine/Gameplay/BlackboardComponent.h>
#include <GameEngine/Gameplay/InputComponent.h>

// Game
#include <3rdPersonTemplatePlugin/Player/TPPlayerController.h>
#include <3rdPersonTemplatePlugin/GameState/3rdPersonTemplateGameState.h>

PL_BEGIN_COMPONENT_TYPE(TPPlayerControllerComponent, 1, plComponentMode::Dynamic)
{
  PL_BEGIN_PROPERTIES
  {
    PL_MEMBER_PROPERTY("RotationSpeed", m_fRotationSpeed)->AddAttributes(new plDefaultValueAttribute(0.01f))
  }
  PL_END_PROPERTIES;
  PL_BEGIN_ATTRIBUTES
  {
    new plCategoryAttribute("Game/Player"),
  }
  PL_END_ATTRIBUTES;
}
PL_END_COMPONENT_TYPE

TPPlayerControllerComponent::TPPlayerControllerComponent() = default;
TPPlayerControllerComponent::~TPPlayerControllerComponent() = default;

#pragma optimize("", off)

void TPPlayerControllerComponent::OnSimulationStarted()
{
  SUPER::OnSimulationStarted();

  ThirdPersonTemplateGameState::GetSingleton()->m_pLocalPlayerController = this;
}

void TPPlayerControllerComponent::Update(plQuat cameraRotation)
{
  plInputComponent* pInput = nullptr;
  if(!GetOwner()->TryGetComponentOfBaseType(pInput))
  {
    return;
  }

  plJoltDefaultCharacterComponent* pCharacterController = nullptr;
  if (!GetOwner()->TryGetComponentOfBaseType( pCharacterController))
  {
    return;
  }

  if(GetOwner()->GetLinearVelocity().GetLength() > 0.2f)
  {
    plAngle out_x;
    plAngle out_y;
    plAngle out_z;

    cameraRotation.GetAsEulerAngles(out_x, out_y, out_z);

    const plQuat yawRotation = plQuat::MakeFromAxisAndAngle(plVec3(0, 0, 1), out_z);
    const plQuat newRotation = plQuat::MakeSlerp(GetOwner()->GetGlobalRotation(), yawRotation, m_fRotationSpeed);

    GetOwner()->SetGlobalRotation(newRotation);
  }

  // Move the character
  plMsgMoveCharacterController msg;
  {
    msg.m_bJump = pCharacterController->IsStandingOnGround() && pInput->GetCurrentInputState("Jump", true) > 0.5;
    msg.m_fMoveForwards = pInput->GetCurrentInputState("MoveForwards", false);
    msg.m_fMoveBackwards = pInput->GetCurrentInputState("MoveBackwards", false);
    msg.m_fStrafeLeft = pInput->GetCurrentInputState("StrafeLeft", false);
    msg.m_fStrafeRight = pInput->GetCurrentInputState("StrafeRight", false);
    msg.m_bCrouch = pInput->GetCurrentInputState("Crouch", false) > 0.5;

    pCharacterController->SendMessage(msg);
  }

  plBlackboardComponent *pBlackboardComponent = nullptr;
  if (GetOwner()->TryGetComponentOfBaseType(pBlackboardComponent))
  {
    float left = pInput->GetCurrentInputState("RotateLeft", false);
    float right = pInput->GetCurrentInputState("RotateRight", false);


    pBlackboardComponent->SetEntryValue("Rotate", right - left);
    pBlackboardComponent->SetEntryValue("MovY", msg.m_fMoveForwards - msg.m_fMoveBackwards);
    pBlackboardComponent->SetEntryValue("MovX", msg.m_fStrafeRight - msg.m_fStrafeLeft);
  }
}

void TPPlayerControllerComponent::SerializeComponent(plWorldWriter& stream) const
{
  SUPER::SerializeComponent(stream);

  auto& s = stream.GetStream();
  s << m_fRotationSpeed;
}

void TPPlayerControllerComponent::DeserializeComponent(plWorldReader& stream)
{
  SUPER::DeserializeComponent(stream);
  const plUInt32 uiVersion = stream.GetComponentTypeVersion(GetStaticRTTI());

  auto& s = stream.GetStream();
  s >> m_fRotationSpeed;
}
#pragma optimize("", on)
