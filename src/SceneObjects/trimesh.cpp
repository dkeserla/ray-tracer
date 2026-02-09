#include "trimesh.h"
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <float.h>
#include <string.h>
#include "../ui/TraceUI.h"
extern TraceUI *traceUI;
extern TraceUI *traceUI;

using namespace std;

Trimesh::~Trimesh() {
  for (auto f : faces)
    delete f;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex(const glm::dvec3 &v) { vertices.emplace_back(v); }

void Trimesh::addNormal(const glm::dvec3 &n) { normals.emplace_back(n); }

void Trimesh::addColor(const glm::dvec3 &c) { vertColors.emplace_back(c); }

void Trimesh::addUV(const glm::dvec2 &uv) { uvCoords.emplace_back(uv); }

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace(int a, int b, int c) {
  int vcnt = vertices.size();

  if (a >= vcnt || b >= vcnt || c >= vcnt)
    return false;

  TrimeshFace *newFace = new TrimeshFace(this, a, b, c);
  if (!newFace->degen)
    faces.push_back(newFace);
  else
    delete newFace;

  // Don't add faces to the scene's object list so we can cull by bounding
  // box
  return true;
}

// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
const char *Trimesh::doubleCheck() {
  if (!vertColors.empty() && vertColors.size() != vertices.size())
    return "Bad Trimesh: Wrong number of vertex colors.";
  if (!uvCoords.empty() && uvCoords.size() != vertices.size())
    return "Bad Trimesh: Wrong number of UV coordinates.";
  if (!normals.empty() && normals.size() != vertices.size())
    return "Bad Trimesh: Wrong number of normals.";

  return 0;
}

bool Trimesh::intersectLocal(ray &r, isect &i) const {
  bool have_one = false;
  for (auto face : faces) {
    isect cur;
    if (face->intersectLocal(r, cur)) {
      if (!have_one || (cur.getT() < i.getT())) {
        i = cur;
        have_one = true;
      }
    }
  }
  if (!have_one)
    i.setT(1000.0);
  return have_one;
}

bool TrimeshFace::intersect(ray &r, isect &i) const {
  return intersectLocal(r, i);
}


// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray &r, isect &i) const {
  if (degen) {
    return false;
  }

  // YOUR CODE HERE
  //
  // FIXME: Add ray-trimesh intersection
  glm::dvec3 p = r.getPosition();
  glm::dvec3 d = r.getDirection();

  // gives us the points in 3d space of abc
  glm::dvec3 a_coords = parent->vertices[ids[0]];
  glm::dvec3 b_coords = parent->vertices[ids[1]];
  glm::dvec3 c_coords = parent->vertices[ids[2]];

  // first find if ray has intersection with plane
  auto denominator =  glm::dot(d, normal);
  if (denominator == 0.0) return false;
  double t = glm::dot((a_coords - p), normal) / denominator;
  if (t < 0) return false;

  // now we have a t such that p + dt is on the plane, check if in bounds
  // vectors from subtraction of points
  auto plane_p = p + d * t;

  glm::dvec3 ba = b_coords - a_coords;
  glm::dvec3 cb = c_coords - b_coords;
  glm::dvec3 ac = a_coords - c_coords;

  auto pa = plane_p - a_coords;
  auto pb = plane_p - b_coords;
  auto pc = plane_p - c_coords;

  if (!validPoint(ba, pa) || !validPoint(cb, pb) || !validPoint(ac, pc)) {
    return false;
  }

  // at this point it will intersect

  auto p2p1 = ba;
  auto p3p1 = -ac;
  auto cp1 = plane_p - a_coords;

  auto a_r = glm::dot(p2p1, p2p1);
  auto b_r = glm::dot(p3p1, p2p1);
  auto c_r = glm::dot(p2p1, p3p1);
  auto d_r = glm::dot(p3p1, p3p1);
  glm::dmat2x2 mass_mat(a_r, b_r, c_r, d_r);
  auto res = glm::inverse(mass_mat) * glm::dvec2(glm::dot(cp1, p2p1), glm::dot(cp1, p3p1));

  auto beta = res[0];
  auto gamma = res[1];
  auto alpha = 1 - beta - gamma; // there should be no problems since we already determined point as valid
  if (alpha < 0 || beta < 0 || gamma < 0) { // can also check if they are larger than 1 but thats chill I hope
    return false;
  }
  i.setBary(alpha, beta, gamma);

  auto n = normal;
  if (parent->vertNorms && !parent->normals.empty()) {
    n = alpha * parent->normals[ids[0]] +
        beta * parent->normals[ids[1]] +
        gamma * parent->normals[ids[2]];
    n = glm::normalize(n);
  }

  i.setT(t);
  i.setN(n);
  i.setObject(parent);

  if (!parent->uvCoords.empty()) {
    i.setUVCoordinates(
alpha * parent->uvCoords[ids[0]] +
      beta  * parent->uvCoords[ids[1]] +
      gamma * parent->uvCoords[ids[2]]
    );
    i.setMaterial(parent->material);
  }
  else if (!parent->vertColors.empty()) {
    Material m = parent->material;
    m.setDiffuse(
   alpha * parent->vertColors[ids[0]] +
      beta  * parent->vertColors[ids[1]] +
      gamma * parent->vertColors[ids[2]]
    );
    i.setMaterial(m);
  }
  else {
    i.setMaterial(parent->material);
  }

  /* To determine the color of an intersection, use the following rules:
     - If the parent mesh has non-empty `uvCoords`, barycentrically interpolate
       the UV coordinates of the three vertices of the face, then assign it to
       the intersection using i.setUVCoordinates().
     - Otherwise, if the parent mesh has non-empty `vertexColors`,
       barycentrically interpolate the colors from the three vertices of the
       face. Create a new material by copying the parent's material, set the
       diffuse color of this material to the interpolated color, and then 
       assign this material to the intersection.
     - If neither is true, assign the parent's material to the intersection.
  */

  return true;
}


bool TrimeshFace::validPoint(glm::dvec3 &side_vec, glm::dvec3 &point_vec) const {
  return glm::dot(glm::cross(side_vec, point_vec), normal) >= 0;
}

// Once all the verts and faces are loaded, per vertex normals can be
// generated by averaging the normals of the neighboring faces.
void Trimesh::generateNormals() {
  int cnt = vertices.size();
  normals.resize(cnt);
  std::vector<int> numFaces(cnt, 0);

  for (auto face : faces) {
    glm::dvec3 faceNormal = face->getNormal();

    for (int i = 0; i < 3; ++i) {
      normals[(*face)[i]] += faceNormal;
      ++numFaces[(*face)[i]];
    }
  }

  for (int i = 0; i < cnt; ++i) {
    if (numFaces[i])
      normals[i] /= numFaces[i];
  }

  vertNorms = true;
}

