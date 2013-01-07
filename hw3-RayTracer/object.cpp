#include "object.h"
#include <algorithm>
#include <iostream>

const float eps = 1e-6;
int sgn(float x) { return x > eps? 1 : x < -eps? -1 : 0; }

Ray::Ray(const vec3& _o, const vec3& _direction) :
    o(_o), direction(glm::normalize(_direction)) {
}
bool Object::Intersect(const Ray& ray, float* dis_to_ray) {
    std::cerr << "Ray should not intersect with abstract object" << std::endl;
    throw 2; // This should never happen
}

Sphere::Sphere(const vec3& _o, const float& _r) :
	o(_o), r(_r) {
}
bool Sphere::Intersect(const Ray& ray, float* dis_to_ray) {
    //dir^2 * t^2 + 2*dir*(p-o)*t + (p-o)^2 == r*r;
    const vec3& dir = ray.direction;
    const vec3& p = ray.o;
    float c2 = glm::dot(dir, dir);
    float c1 = 2 * glm::dot(dir, p-o);
    float c0 = glm::dot(p-o, p-o) - r*r;
    float delta = c1*c1 - 4*c2*c0;
    if (sgn(delta) < 0) { 
        return false;
    }
    delta = abs(delta);
    // closest intersection point
    float x = std::min((-c1 - sqrt(delta)) / (2*c2), 
                   (-c1 + sqrt(delta)) / (2*c2));
    if (x < eps) {
        return false;
    } else {
        *dis_to_ray = x;
        return true;
    }
}


bool Color::operator == (const Color &otherColor) {
    return r == otherColor.r && g == otherColor.g && b == otherColor.b;
}
Triangle::Triangle(
    const vec3& _a, 
    const vec3& _b, 
    const vec3& _c,
    vec3 _na, 
    vec3 _nb, 
    vec3 _nc) : 
    a(vertexes[0]),
    b(vertexes[1]),
    c(vertexes[2]),
    na(vertexNormals[0]),
    nb(vertexNormals[1]),
    nc(vertexNormals[2]) {
        a = _a;
        b = _b;
        c = _c;
        na = _na;
        nb = _nb;
        nc = _nc;
        type = triangle;
    };
bool Triangle::Intersect(const Ray& ray, float* dis_to_ray) {
    vec3 n = glm::normalize(glm::cross(b-a, c-a));
    const vec3& p = ray.o;
    const vec3& dir = ray.direction;
    
    // Ray parallel with plane
    if (sgn(glm::dot(dir, n)) == 0) {
        return false;
    }
    
    //plane ray intersection -- (p+dir*t)*n = a*n;
    float t = (glm::dot(a,n) - glm::dot(p,n)) / glm::dot(dir,n);
    vec3 p0 = p + (dir * t);
    
    // check if p0 in triangle, because triangle is convex.
    // if p0 in triangle, then absolute area equals to area_vec length, otherwise not.
    vec3 area_vec(0,0,0);
    float abs_area = 0.0;
    for (int i = 0; i < 3; ++i) {
        abs_area += glm::length(glm::cross(p0-vertexes[i], p0-vertexes[(i+1)%3]));
        area_vec += glm::cross(p0-vertexes[i], p0-vertexes[(i+1)%3]);
    }
    if (sgn(abs_area - glm::length(area_vec)) == 0) {
        *dis_to_ray = t;
        return true;
    } else
        return false;
}
Object::~Object() {
}
Triangle::~Triangle() {
}
Sphere::~Sphere() {
}