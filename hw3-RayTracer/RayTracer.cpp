#include "RayTracer.h"
#include "stdio.h"

int debugCount = 0;

void debug(Color color, char* msg = "") {
    printf("%s (%.2f %.2f %.2f)\n",msg, color.r, color.g, color.b);
}
void debug(vec3 vec, char* msg = "") {
    printf("%s (%.2f %.2f %.2f)\n",msg, vec.x, vec.y, vec.z);
}

Ray RayTracer::GenerateRay(const Camera& camera, int i, int j, int height, int width) {
    vec3 w = glm::normalize(camera.eye - camera.center);
	vec3 u = glm::normalize(glm::cross(camera.up, w));
	vec3 v = glm::cross(w, u);
	
	float fovy = camera.fovy * PI / 180.0; // degree to radian
	float x_range = tan(fovy / 2.0) * width / height;
	float b = tan(fovy / 2.0) * (height/2.0 - i) / (height / 2.0);
	float a =  x_range * (j - width/2.0) / (width / 2.0);
    return Ray(camera.eye, -w + u*a + v*b);
}
Ray RayTracer::TransformRay(const Ray& ray, const Object* object) {
    //return ray;
    vec4 o_extend(ray.o, 1.0);
    vec4 dir_extend(ray.direction, 0.0);
    o_extend = (o_extend * object->InversedTransform);
    dir_extend = (dir_extend * object->InversedTransform);
    vec3 o = vec3(o_extend.x / o_extend.w, o_extend.y / o_extend.w, o_extend.z / o_extend.w);
    vec3 dir = vec3(dir_extend.x, dir_extend.y, dir_extend.z);
    return Ray(o, dir);
    //o_extend * 
}
bool RayTracer::GetIntersection(const Ray& ray, const Scene& scene, 
                                const Object* &hit_object, vec3* hit_point) {
    float nearest_dist = INF;
    hit_object = NULL;
    for (int i = 0; i < (int)scene.objects.size(); ++i) {
        // Ray in object space
        Ray transformed_ray = TransformRay(ray, scene.objects[i]);
        float dist;
        if (scene.objects[i]->Intersect(transformed_ray, &dist)) {
            // Get back the hit point.
            vec3 hit_trans = transformed_ray.o + transformed_ray.direction * dist;
            vec4 hit_extend(hit_trans, 1.0);
            hit_extend = hit_extend * scene.objects[i]->transform;
            vec3 hit = vec3(hit_extend.x / hit_extend.w, hit_extend.y / hit_extend.w, hit_extend.z / hit_extend.w);
            
            dist = glm::length(hit - ray.o);
            if (dist < nearest_dist) {
                nearest_dist = dist;
                hit_object = scene.objects[i];
                *hit_point = hit;
            }
        }
    }
    if (hit_object == NULL)
        return false;
    else {
        return true;
    }
}
int debugPixH, debugPixW;
Color RayTracer::Trace(const Ray& ray, const Scene& scene, int depth,
                       int pixH, int pixW) {
    debugPixH = pixH;
    debugPixW = pixW;
    if (depth > RAYTRACE_DEPTH_LIMIT) {
        return BLACK;
    }
    const Object* hit_object;
    vec3 hit_point;
    if (!GetIntersection(ray, scene, hit_object, &hit_point))
        return BLACK;
    
    Color color(hit_object->materials.ambient);
    for (int i = 0; i < (int)scene.lights.size(); ++i) {
       
        if (scene.lights[i].type == Light::point) {
            Ray light_ray(scene.lights[i].position(),
                          hit_point - scene.lights[i].position());
            
            const Object* tmp_obj;
            vec3 light_hit;
            
            bool ok = GetIntersection(light_ray, scene, tmp_obj, &light_hit);
            /*
            if (!ok) {
                printf("not ok at : %d %d\n",pixH, pixW);
                assert(0);
            }
            */
            if (ok) {
                if (IsSameVector(hit_point, light_hit)) {
                    color = color + CalcLight(scene.lights[i], hit_object, ray, hit_point);
                }
            }
        } else {
            color = color + CalcLight(scene.lights[i], hit_object, ray, hit_point);
        }
    }
    return color;
/*
    if (materials.specular > 0) {
        Ray reflect_ray = ray.CreateReflectRay(hit_point, normal);
        // Make a recursive call to trace the reflected ray
        Color temp_color = Trace(reflect_ray, scene, depth+1);
        color += materials.specular * temp_color;
    }
*/
}
Color RayTracer::CalcLight(const Light& light, const Object* hit_object, 
                           const Ray& ray, const vec3& hit_point) {

    vec3 light_direction;
    if (light.type == Light::point) {
        light_direction = glm::normalize(light.positionOrDirection - hit_point);
    } else 
        light_direction = glm::normalize(light.positionOrDirection);
    
    vec3 normal = glm::normalize(hit_object->InterpolatePointNormal(hit_point));
    
    const Materials& materials = hit_object->materials;
	float nDotL = max(glm::dot(normal, light_direction), 0.0f);
	Color diffuse = materials.diffuse * light.color * nDotL;
    
    vec3 halfvec = glm::normalize(light_direction + glm::normalize(-ray.direction));
	float nDotH = max(glm::dot(normal, halfvec), 0.0f);
	Color specular = materials.specular * light.color * pow(nDotH, materials.shininess);
	
	//debug(specular, "specular = ");
	//debug(materials.specular, "materials.specular = ");
	//printf("nDotH = %f\n",nDotH);
	//debug(light.color,)
	if (hit_object->index == 100) {
	    if (++debugCount < 10) {
	        //Sphere* sphere = dynamic_cast<Sphere*>(hit_object);
	        //debug(sphere->o, "sphere = ");
	        printf("pix = (%d %d)\n",debugPixH, debugPixW);
	        debug(hit_point, "hit is ");
	        debug(normal, "normal is ");
	        debug(light_direction, "light_direction is ");
            printf("nDotL = %f\n",glm::dot(normal, light_direction));
            puts("");
	    } else 
            assert(0);
	}
	return diffuse + specular;
}

