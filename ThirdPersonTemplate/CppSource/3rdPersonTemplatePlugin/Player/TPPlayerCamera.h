#pragma once

// Plasma
#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>

// Game
#include <3rdPersonTemplatePlugin/3rdPersonTemplatePluginDLL.h>

class TPPlayerControllerComponent;

class TPPlayerCameraManager : public plComponentManager<class TPPlayerCameraComponent, plBlockStorageType::Compact>
{
  PL_ADD_DYNAMIC_REFLECTION(TPPlayerCameraManager, plComponentManager);

public:
  TPPlayerCameraManager(plWorld* pWorld);

  virtual void Initialize() override;

  void Update(const plWorldModule::UpdateContext& context);
};

class TPPlayerCameraComponent : public plComponent
{
  PL_DECLARE_COMPONENT_TYPE(TPPlayerCameraComponent, plComponent, TPPlayerCameraManager);

public:
  TPPlayerCameraComponent();
  ~TPPlayerCameraComponent();

  virtual void SerializeComponent(plWorldWriter& stream) const override;
  virtual void DeserializeComponent(plWorldReader& stream) override;

  float GetCameraDragStrength() const { return m_fCameraDragStrength; }
  void SetCameraDragStrength(float value) { m_fCameraDragStrength = value; }

  float GetMaxCameraDistance() const { return m_fMaxCameraDistance; }
  void SetMaxCameraDistance(float value) { m_fMaxCameraDistance = value; }

  const char* GetCameraObject() const;
  void SetCameraObjectHandle(const char* szGuid);

protected:
  virtual void OnSimulationStarted() override;

private:
  void Update();

  plAngle m_NewHorizontalRotation;                       // [ property ]
  plAngle m_CurHorizontalRotation;

  plAngle m_MaxVerticalRotation = plAngle::MakeFromDegree(80); // [ property ]
  plAngle m_NewVerticalRotation;                       // [ property ]
  plAngle m_CurVerticalRotation;

  float m_fCameraDragStrength = 5;

  float m_fMaxCameraDistance = 5;
  float m_fMinCameraDistance = 0.5f;
  plUInt8 m_uiCollisionLayer = 0; // [ property ]
  plGameObjectHandle m_hCamera; // [ property ]

  TPPlayerControllerComponent* m_pPlayerController = nullptr;
  plPhysicsWorldModuleInterface* m_pPhysicsModule = nullptr;
};
