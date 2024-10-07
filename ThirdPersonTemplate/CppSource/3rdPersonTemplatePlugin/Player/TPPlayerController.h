#pragma once

// Plasma
#include <Core/World/Component.h>
#include <Core/World/ComponentManager.h>

// Game
#include <3rdPersonTemplatePlugin/3rdPersonTemplatePluginDLL.h>

class plJoltCharacterControllerComponent;

using TPPlayerControllerManager = plComponentManager<class TPPlayerControllerComponent, plBlockStorageType::Compact>;

class TPPlayerControllerComponent : public plComponent
{
  PL_DECLARE_COMPONENT_TYPE(TPPlayerControllerComponent, plComponent, TPPlayerControllerManager);

public:
  TPPlayerControllerComponent();
  ~TPPlayerControllerComponent();

  virtual void SerializeComponent(plWorldWriter& inout_stream) const override;
  virtual void DeserializeComponent(plWorldReader& inout_stream) override;

  void Update(plQuat cameraRotation);
protected:
  virtual void OnSimulationStarted() override;

private:
  plQuat m_targetRotation;
  plVec3 m_LastPos;
  float m_fRotationSpeed;
};
