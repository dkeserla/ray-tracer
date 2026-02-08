#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI *traceUI;

#include "../fileio/images.h"
#include <glm/gtx/io.hpp>
#include <iostream>

using namespace std;
extern bool debugMode;

Material::~Material() {}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene *scene, const ray &r, const isect &i) const {
  // at this point we know that our ray has insersected with some object
  // we have the point it insersected with and the material + light for this ray
  //

  // YOUR CODE HERE

  // For now, this method just returns the diffuse color of the object.
  // This gives a single matte color for every distinct surface in the
  // scene, and that's it.  Simple, but enough to get you started.
  // (It's also inconsistent with the phong model...)

  // Your mission is to fill in this method with the rest of the phong
  // shading model, including the contributions of all the light sources.
  // You will need to call both distanceAttenuation() and
  // shadowAttenuation()
  // somewhere in your code in order to compute shadows and light falloff.
  //	if( debugMode )
  //		std::cout << "Debugging Phong code..." << std::endl;


  // attenuation
  glm::dvec3 color = ka(i) * scene->ambient();
  // simple just calculate the light rate from this ray to the light via intersection

  // When you're iterating through the lights,
  // you'll want to use code that looks something
  // like this:
  //  factor in time and distance in the object, affects the attentuation, something about add up time in object recursively


  glm::dvec3 p = r.at(i);          // hit point
  glm::dvec3 n = glm::normalize(i.getN());

  for ( const auto& pLight : scene->getAllLights() )
  {
               // pLight has type Light* .
    // for point light the ray to the light is just straightforward to it


    auto L = pLight->getDirection(p); // to light
    auto distAtten = pLight->distanceAttenuation(p);      // 1 for directional, falloff for point
    ray toLight(p, L, r.getAtten(), ray::SHADOW);
    auto shadowAtten = pLight->shadowAttenuation(toLight, p); // shadow rays
    auto I = pLight->getColor() * distAtten * shadowAtten;

    double NdotL = std::max(0.0, glm::dot(n, L));

    // Diffuse
    color += kd(i) * I * NdotL;

glm::dvec3 v_vec = glm::normalize(-r.getDirection());
glm::dvec3 r_vec = glm::normalize(2 * glm::dot(n, L) * n - L);
    color += ks(i) * I * std::pow(std::max(0.0, glm::dot(v_vec, r_vec)), i.getMaterial().shininess(i));
  }
  // return kd(i);
  return color;
}

TextureMap::TextureMap(string filename) {
  data = readImage(filename.c_str(), width, height);
  if (data.empty()) {
    width = 0;
    height = 0;
    string error("Unable to load texture map '");
    error.append(filename);
    error.append("'.");
    throw TextureMapException(error);
  }
}

glm::dvec3 TextureMap::getMappedValue(const glm::dvec2 &coord) const {
  // YOUR CODE HERE
  //
  // In order to add texture mapping support to the
  // raytracer, you need to implement this function.
  // What this function should do is convert from
  // parametric space which is the unit square
  // [0, 1] x [0, 1] in 2-space to bitmap coordinates,
  // and use these to perform bilinear interpolation
  // of the values.

  return glm::dvec3(1, 1, 1);
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const {
  // YOUR CODE HERE
  //
  // In order to add texture mapping support to the
  // raytracer, you need to implement this function.

  return glm::dvec3(1, 1, 1);
}

glm::dvec3 MaterialParameter::value(const isect &is) const {
  if (0 != _textureMap)
    return _textureMap->getMappedValue(is.getUVCoordinates());
  else
    return _value;
}

double MaterialParameter::intensityValue(const isect &is) const {
  if (0 != _textureMap) {
    glm::dvec3 value(_textureMap->getMappedValue(is.getUVCoordinates()));
    return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
  } else
    return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}
