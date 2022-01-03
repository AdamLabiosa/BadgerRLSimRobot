/**
 * @file Simulation/Appearances/Appearance.cpp
 * Implementation of class Appearance
 * @author Colin Graf
 */

#include "Appearance.h"
#include "CoreModule.h"
#include "Platform/Assert.h"
#include "Simulation/Scene.h"
#include "Tools/OpenGLTools.h"

Appearance::Surface::Surface()
{
  diffuseColor[0] = 0.8f;
  diffuseColor[1] = 0.8f;
  diffuseColor[2] = 0.8f;
  diffuseColor[3] = 1.0f;

  ambientColor[0] = 0.2f;
  ambientColor[1] = 0.2f;
  ambientColor[2] = 0.2f;
  ambientColor[3] = 1.0f;

  specularColor[0] = 0.0f;
  specularColor[1] = 0.0f;
  specularColor[2] = 0.0f;
  specularColor[3] = 1.0f;

  emissionColor[0] = 0.0f;
  emissionColor[1] = 0.0f;
  emissionColor[2] = 0.0f;
  emissionColor[3] = 1.0f;
}

void Appearance::Surface::createGraphics(GraphicsContext& graphicsContext)
{
  if(surface)
    return;

  if(!diffuseTexture.empty())
  {
    ASSERT(!texture);
    texture = graphicsContext.requestTexture(diffuseTexture);
  }

  static_assert(sizeof(diffuseColor) == sizeof(ambientColor), "diffuseColor and ambientColor must have the same size");
  if(!hasAmbientColor)
    std::memcpy(ambientColor, diffuseColor, sizeof(diffuseColor));

  surface = graphicsContext.requestSurface(diffuseColor, ambientColor, specularColor, emissionColor, shininess, texture);
}

void Appearance::createGraphics(GraphicsContext& graphicsContext)
{
  if(!created)
  {
    OpenGLTools::convertTransformation(rotation, translation, transformation);
    if(surface)
      surface->createGraphics(graphicsContext);
    created = true;
  }

  graphicsContext.pushModelMatrix(transformation);
  if(surface)
    modelMatrices.push_back(graphicsContext.requestModelMatrix());
  GraphicalObject::createGraphics(graphicsContext);
  graphicsContext.popModelMatrix();

  if(!mesh)
    mesh = createMesh(graphicsContext);
}

const QIcon* Appearance::getIcon() const
{
  return &CoreModule::module->appearanceIcon;
}

void Appearance::addParent(Element& element)
{
  SimObject::addParent(element);
  GraphicalObject::addParent(element);
}

void Appearance::Surface::addParent(Element& element)
{
  Appearance* appearance = dynamic_cast<Appearance*>(&element);
  ASSERT(appearance);
  ASSERT(!appearance->surface);
  appearance->surface = this;
}

void Appearance::drawAppearances(GraphicsContext& graphicsContext, bool drawControllerDrawings) const
{
  if(!drawControllerDrawings && mesh)
    graphicsContext.draw(mesh, modelMatrices[modelMatrixIndex], surface->surface);

  // TODO: This breaks completely when creating SimObjectRenderers for sub-parts of the scene.
  if(!modelMatrices.empty())
  {
    ++modelMatrixIndex;
    modelMatrixIndex %= modelMatrices.size();
  }

  GraphicalObject::drawAppearances(graphicsContext, drawControllerDrawings);
}
