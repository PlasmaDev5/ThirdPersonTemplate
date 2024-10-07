#pragma once

// Plasma
#include <Core/World/Declarations.h>
#include <GameEngine/GameState/FallbackGameState.h>
#include <GameEngine/GameState/GameState.h>

// Game
#include <3rdPersonTemplatePlugin/3rdPersonTemplatePluginDLL.h>

class TPPlayerControllerComponent;
class TPPlayerCameraComponent;

class ThirdPersonTemplateGameState : public plFallbackGameState
{
  PL_ADD_DYNAMIC_REFLECTION(ThirdPersonTemplateGameState, plFallbackGameState);

  static ThirdPersonTemplateGameState* s_pSingleton;

public:
  ThirdPersonTemplateGameState();
  ~ThirdPersonTemplateGameState();

  virtual plGameStatePriority DeterminePriority(plWorld* pWorld) const override;

  virtual void ProcessInput() override;


  static ThirdPersonTemplateGameState* GetSingleton() { return s_pSingleton; }


  TPPlayerControllerComponent* m_pLocalPlayerController = nullptr;
  TPPlayerCameraComponent* m_pMainCamera = nullptr;
protected:
  virtual void ConfigureMainWindowInputDevices(plWindow* pWindow) override;
  virtual void ConfigureInputActions() override;
  virtual void ConfigureMainCamera() override;

private:
  virtual void OnActivation(plWorld* pWorld, const plTransform* pStartPosition) override;
  virtual void OnDeactivation() override;
  virtual void BeforeWorldUpdate() override;
  virtual void AfterWorldUpdate() override;

  plDeque<plGameObjectHandle> m_SpawnedObjects;
};
