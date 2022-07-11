#include "util.h"


void util_bary_coefficients(vec3 points[3], vec3 p, vec3 out)
{
    vec3 ba, ca;
    glm_vec3_sub(points[1], points[0], ba);
    glm_vec3_sub(points[2], points[0], ca);
    /* Vec3f ba = vec_sub(points[1], points[0]); */
    /* Vec3f ca = vec_sub(points[2], points[0]); */

    vec3 pa, pb, pc;
    glm_vec3_sub(p, points[0], pa);
    glm_vec3_sub(p, points[1], pb);
    glm_vec3_sub(p, points[2], pc);
    /* Vec3f pa = vec_sub(p, points[0]); */
    /* Vec3f pb = vec_sub(p, points[1]); */
    /* Vec3f pc = vec_sub(p, points[2]); */

    vec3 n;
    glm_vec3_cross(ba, ca, n);
    /* Vec3f n = vec_cross(ba, ca); */
    float s_abc = glm_vec3_distance(n, (vec3){ 0.f, 0.f, 0.f });
    /* float s_abc = vec_len(n); */
    /* n = vec_normalize(n); */
    glm_normalize(n);

    vec3 pbc, apc, abp;

    glm_vec3_cross(pb, pc, pbc);
    float s_pbc = glm_vec3_dot(n, pbc);
    /* Vec3f pbc = vec_cross(pb, pc); */
    /* float s_pbc = vec_dot(n, pbc); */

    glm_vec3_cross(pa, ca, apc);
    float s_apc = glm_vec3_dot(n, apc);
    /* Vec3f apc = vec_cross(pa, ca); */
    /* float s_apc = vec_dot(n, apc); */

    glm_vec3_cross(ba, pa, abp);
    float s_abp = glm_vec3_dot(n, abp);
    /* Vec3f abp = vec_cross(ba, pa); */
    /* float s_abp = vec_dot(n, abp); */

    glm_vec3_copy((vec3){
        s_pbc / s_abc,
        s_apc / s_abc,
        s_abp / s_abc
    }, out);
}

