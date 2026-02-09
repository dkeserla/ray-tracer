#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/extended_min_max.hpp>
#include <glm/gtx/io.hpp>

using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3 &) const {
  // distance to light is infinite, so f(di) goes to 0.  Return 1.
  return 1.0;
}

glm::dvec3 DirectionalLight::shadowAttenuation(const ray &r,
                                               const glm::dvec3 &p) const {
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.
  isect i;
  ray r_new(r);
  if (!scene->intersect(r_new, i)) {
    return {1, 1, 1};
  }

  const Material &m = i.getMaterial();

  if (!m.Trans()) {
    return glm::dvec3(0);
  }

  // basically we have shot a ray from our initial object towards the light, we hit transmissive, lets go through?
  auto kt = m.kt(i);
  auto enter = r_new.at(i);
  // ray through_object(r_new.at(i), r_new.getDirection(), r_new.getAtten());
  r_new.setPosition(enter + 1e-4 * r_new.getDirection());
  // assume it will always have end of object
  if (!scene->intersect(r_new, i)) {
    return glm::dvec3(1);
  }

  // t is now correctly the time spent in the object, assuming no layering of obhjects (matroshka doll)
  auto atten = glm::pow(kt, glm::dvec3(i.getT()));
  if (atten.x < 1e-6 && atten.y < 1e-6 && atten.z < 1e-6) {
    return glm::dvec3(0.0);
  }


  ray rest(r_new.at(i)+ 1e-4 * r_new.getDirection(), r_new.getDirection(), r_new.getAtten());
  return atten * shadowAttenuation(rest, rest.getPosition());
}

glm::dvec3 DirectionalLight::getColor() const { return color; }

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3 &) const {
  return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3 &P) const {
  // YOUR CODE HERE

  // You'll need to modify this method to attenuate the intensity
  // of the light based on the distance between the source and the
  // point P.  For now, we assume no attenuation and just return 1.0
  const auto dist = glm::distance(position, P);
  const auto denom = constantTerm + linearTerm * dist + quadraticTerm * dist * dist;
  return glm::min(1.0, 1/denom);
}

glm::dvec3 PointLight::getColor() const { return color; }

glm::dvec3 PointLight::getDirection(const glm::dvec3 &P) const {
  return glm::normalize(position - P);
}

glm::dvec3 PointLight::shadowAttenuation(const ray &r,
                                         const glm::dvec3 &p) const {
  // YOUR CODE HERE:
  // You should implement shadow-handling code here.

  // if an insersection came, and the object is semi transparent
  // recrusively calculate shadowA(ray, r.at(i))
  // until we have no intersection or we hit light?
  // if sufficiently low attenutation stop almost 0 stop recursing
  // basically just return product_i of kt_i^d


  // TODO add some preturbation to make sure that we don't hit objects accidently

  isect i;
  ray r_new(r);
  if (!scene->intersect(r_new, i)) {
    return glm::dvec3(1, 1, 1);
  }

  // object is behind light
  double lightDist = glm::distance(p, position);
  if (i.getT() >= lightDist) {
    return glm::dvec3(1);
  }

  const Material &m = i.getMaterial();

  if (!m.Trans()) {
    return glm::dvec3(0);
  }

  // basically we have shot a ray from our initial object towards the light, we hit transmissive, lets go through?
  auto kt = m.kt(i);
  auto enter = r_new.at(i);
  // ray through_object(r_new.at(i), r_new.getDirection(), r_new.getAtten());
  r_new.setPosition(enter + 1e-4 * r_new.getDirection());
  // assume it will always have end of object
  if (!scene->intersect(r_new, i)) {
    return glm::dvec3(1);
  }

  // t is now correctly the time spent in the object, assuming no layering of obhjects (matroshka doll)
  auto atten = glm::pow(kt, glm::dvec3(i.getT()));
  if (atten.x < 1e-6 && atten.y < 1e-6 && atten.z < 1e-6) {
    return glm::dvec3(0.0);
  }


  ray rest(r_new.at(i)+ 1e-4 * r_new.getDirection(), r_new.getDirection(), r_new.getAtten());
  return atten * shadowAttenuation(rest, rest.getPosition());
}

#define VERBOSE 0

