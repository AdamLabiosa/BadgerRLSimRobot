/**
 * @file Simulation/Actuators/Actuator.cpp
 * Implementation of class Actuator
 * @author Colin Graf
 */

#include "Actuator.h"
#include "ActuatorsWidget.h"
#include "CoreModule.h"
#include "Graphics/GraphicsContext.h"
#include "Tools/OpenGLTools.h"

void Actuator::createPhysics(GraphicsContext &graphicsContext)
{
  OpenGLTools::convertTransformation(rotation, translation, poseInParent);

  graphicsContext.pushModelMatrix(poseInParent);
  ASSERT(!modelMatrix);
  modelMatrix = graphicsContext.requestModelMatrix(GraphicsContext::ModelMatrix::physicalDrawing);
  ::PhysicalObject::createPhysics(graphicsContext);
  graphicsContext.popModelMatrix();
}

void Actuator::addParent(Element& element)
{
  ::PhysicalObject::addParent(element);
}

const QIcon* Actuator::Port::getIcon() const
{
  return &CoreModule::module->actuatorIcon;
}

SimRobot::Widget* Actuator::Port::createWidget()
{
  CoreModule::module->application->openObject(CoreModule::module->actuatorsObject);
  if(ActuatorsWidget::actuatorsWidget)
    ActuatorsWidget::actuatorsWidget->openActuator(fullName);
  return nullptr;
}
