/**
 * @file Simulation/Sensor.h
 * Declaration of class Sensor
 * @author Colin Graf
 */

#pragma once

#include "Simulation/SimObject.h"
#include "Simulation/PhysicalObject.h"
#include <QStringList>

/**
 * @class Sensor
 * An abstract class for sensors
 */
class Sensor : public PhysicalObject, public SimRobotCore2::Sensor
{
public:
  class Port : public SimRobotCore2::SensorPort
  {
  public:
    QString fullName; /**< The path name to the object in the scene graph */
    SensorType sensorType; /**< The data type of the sensor readings */
    Data data; /**< The sensor reading */
    QList<int> dimensions; /**< The dimensions of the sensor readings */
    QStringList descriptions; /**< A description for each sensor reading dimension */
    QString unit; /**< The unit of the sensor readings */
    unsigned int lastSimulationStep = 0xffffffff; /**< The last time this sensor was computed. */

    /** Update the sensor value. Is called when required. */
    virtual void updateValue() = 0;

  private:
    // API
    const QString& getFullName() const override {return fullName;}
    const QIcon* getIcon() const override;
    SimRobot::Widget* createWidget() override;
    const QList<int>& getDimensions() const override {return dimensions;}
    const QStringList& getDescriptions() const override {return descriptions;}
    const QString& getUnit() const override {return unit;}
    SensorType getSensorType() const override {return sensorType;}
    Data getValue() override;
    bool renderCameraImages(SimRobotCore2::SensorPort**, unsigned int) override {return false;}
  };

protected:
  /**
   * Creates the physical objects used by the OpenDynamicsEngine (ODE).
   * These are a geometry object for collision detection and/or a body,
   * if the simulation object is movable.
   * @param graphicsContext The graphics context to create resources in
   */
  void createPhysics(GraphicsContext& graphicsContext) override;

private:
  // API
  const QString& getFullName() const override {return SimObject::getFullName();}
  SimRobot::Widget* createWidget() override {return SimObject::createWidget();}
  const QIcon* getIcon() const override {return SimObject::getIcon();}
  SimRobotCore2::Renderer* createRenderer() override {return SimObject::createRenderer();}
  bool registerDrawing(SimRobotCore2::Controller3DDrawing& drawing) override {return ::PhysicalObject::registerDrawing(drawing);}
  bool unregisterDrawing(SimRobotCore2::Controller3DDrawing& drawing) override {return ::PhysicalObject::unregisterDrawing(drawing);}
  SimRobotCore2::Body* getParentBody() override {return ::PhysicalObject::getParentBody();}
};
