#include <3rdPersonTemplatePlugin/3rdPersonTemplatePluginPCH.h>

// Plasma
#include <Core/World/World.h>

// Game
#include <3rdPersonTemplatePlugin/GameState/3rdPersonTemplateGameState.h>

PL_BEGIN_DYNAMIC_REFLECTED_TYPE(ThirdPersonTemplateGameState, 1, plRTTIDefaultAllocator<ThirdPersonTemplateGameState>)
PL_END_DYNAMIC_REFLECTED_TYPE;


ThirdPersonTemplateGameState* ThirdPersonTemplateGameState::s_pSingleton = nullptr;

ThirdPersonTemplateGameState::ThirdPersonTemplateGameState()
{
  s_pSingleton = this;
};
ThirdPersonTemplateGameState::~ThirdPersonTemplateGameState() = default;

void ThirdPersonTemplateGameState::OnActivation(plWorld* pWorld, const plTransform* pStartPosition)
{
  SUPER::OnActivation(pWorld, pStartPosition);
}

void ThirdPersonTemplateGameState::OnDeactivation()
{
  SUPER::OnDeactivation();
}

void ThirdPersonTemplateGameState::AfterWorldUpdate()
{
  SUPER::AfterWorldUpdate();
}

void ThirdPersonTemplateGameState::BeforeWorldUpdate()
{
  PL_LOCK(m_pMainWorld->GetWriteMarker());
}

plGameStatePriority ThirdPersonTemplateGameState::DeterminePriority(plWorld* pWorld) const
{
  return plGameStatePriority::Default;
}

void ThirdPersonTemplateGameState::ConfigureMainWindowInputDevices(plWindow* pWindow)
{
  SUPER::ConfigureMainWindowInputDevices(pWindow);

  // setup devices here
}

void ThirdPersonTemplateGameState::ConfigureInputActions()
{
  SUPER::ConfigureInputActions();
}

void ThirdPersonTemplateGameState::ProcessInput()
{
  SUPER::ProcessInput();

  plWorld* pWorld = m_pMainWorld;
}

void ThirdPersonTemplateGameState::ConfigureMainCamera()
{
  SUPER::ConfigureMainCamera();

  // do custom camera setup here
}
