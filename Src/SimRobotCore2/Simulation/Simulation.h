/**
 * @file Simulation/Simulation.h
 * Declaration of class Simulation
 * @author Colin Graf
 */

#pragma once

#include "Graphics/GraphicsContext.h"
#include "Simulation/Appearances/ComplexAppearance.h"
#include <string>
#include <list>
#include <unordered_map>
#include <ode/common.h>
#ifdef MULTI_THREADING
#include <ode/threading.h>
#endif

class Scene;
class ElementCore2;

/**
 * @class Simulation
 * A class for managing the simulation
 */
class Simulation
{
public:
  static Simulation* simulation;

  Scene* scene = nullptr; /**< The root of the scene graph */
  std::list<ElementCore2*> elements; /**< All scene graph elements */

  dWorldID physicalWorld = nullptr; /**< The physical world */
  dSpaceID rootSpace = nullptr; /**< The root collision space */
  dSpaceID staticSpace = nullptr; /**< The collision space for static objects */
  dSpaceID movableSpace = nullptr; /**< The collision space for movable objects */
#ifdef MULTI_THREADING
  dThreadingImplementationID threading; /**< Needed for multithreaded physics. */
  dThreadingThreadPoolID pool; /**< The thread pool for physics. */
#endif

  GraphicsContext graphicsContext; /**< The object that does graphics. */
  GraphicsContext::Mesh* xAxisMesh = nullptr; /**< The mesh for the x axis in object renderers. */
  GraphicsContext::Mesh* yAxisMesh = nullptr; /**< The mesh for the y axis in object renderers. */
  GraphicsContext::Mesh* zAxisMesh = nullptr; /**< The mesh for the z axis in object renderers. */
  GraphicsContext::Mesh* dragPlaneMesh = nullptr; /**< The mesh for the drag plane in object renderers. */
  GraphicsContext::Mesh* bodyComSphereMesh = nullptr; /**< The mesh for the physical CoM drawing of bodies. */
  GraphicsContext::Surface* xAxisSurface = nullptr; /**< The surface for the x axis in object renderers. */
  GraphicsContext::Surface* yAxisSurface = nullptr; /**< The surface for the y axis in object renderers. */
  GraphicsContext::Surface* zAxisSurface = nullptr; /**< The surface for the z axis in object renderers. */
  GraphicsContext::Surface* dragPlaneSurface = nullptr; /**< The surface for the drag plane in object renderers. */
  GraphicsContext::Surface* bodyComSphereSurface = nullptr; /**< The surface for the physical CoM drawing of bodies. */
  GraphicsContext::ModelMatrix* originModelMatrix = nullptr; /**< The model matrix for the origin in object renderers. */
  GraphicsContext::ModelMatrix* dragPlaneModelMatrix = nullptr; /**< The model matrix for the drag plane in object renderers. */
  Pose3f originPose; /**< Pose of the origin (assuming that renderers are sequential. */
  Pose3f dragPlanePose; /**< Pose of the drag plane (assuming it is not possible to drag simultaneously in multiple renderers). */
  std::vector<GraphicsContext::Surface*> bodySurfaces; /**< The special surfaces for each body, used by \c ObjectSegmentedImageSensor. */
  std::unordered_map<ComplexAppearance::Descriptor, GraphicsContext::Mesh*, ComplexAppearance::Hasher> complexAppearanceMeshCache; /**< The cache for meshes generated by complex appearances. */

  unsigned int currentFrameRate = 0; /**< The current frame rate of the simulation */

  /** Default Constructor. */
  Simulation();

  /** Destructor. */
  virtual ~Simulation();

  /**
   * Loads a file and initializes the simulation
   * @param filename The name of the file
   * @param errors The errors that occured during parsing.
   */
  bool loadFile(const std::string& filename, std::list<std::string>& errors);

  /** Executes one simulation step */
  void doSimulationStep();
  unsigned int simulationStep = 0;
  double simulatedTime = 0;
  unsigned int collisions = 0;
  unsigned int contactPoints = 0;

  /** Registers all objects of the simulation (including children, actuators and sensors) at SimRobot's GUI */
  void registerObjects();

private:
  dJointGroupID contactGroup = nullptr; /**< The joint group for temporary contact joints used for collision handling */

  /** Computes the frame rate of simulation */
  void updateFrameRate();
  unsigned int lastFrameRateComputationTime = 0;
  unsigned int lastFrameRateComputationStep = 0;

  /**
   * Static callback method for handling the collision of two geometries
   * @param simulation The simulation
   * @param geom1 The first geometry object for collision testing
   * @param geom2 The second geometry object for collision testing
   */
  static void staticCollisionCallback(Simulation *simulation, dGeomID geom1, dGeomID geom2);

  /**
   * Static callback method for handling the collision of a static geometry with a movable space
   * @param simulation The simulation
   * @param geom1 The first geometry object for collision testing
   * @param geom2 The second geometry object for collision testing
   */
  static void staticCollisionWithSpaceCallback(Simulation *simulation, dGeomID geom1, dGeomID geom2);

  /**
   * Static callback method for handling the collision of two movable spaces
   * @param simulation The simulation
   * @param geom1 The first geometry object for collision testing
   * @param geom2 The second geometry object for collision testing
   */
  static void staticCollisionSpaceWithSpaceCallback(Simulation *simulation, dGeomID geom1, dGeomID geom2);
};
