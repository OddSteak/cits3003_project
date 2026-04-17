// Calculate a normal matrix so that non-uniform scale transformations properly transform normals
// See: https://github.com/graphitemaster/normals_revisited
// and: https://gist.github.com/shakesoda/8485880f71010b79bc8fed0f166dabac
mat3 cofactor(mat4 mat) {
    return mat3(
        cross(vec3(mat[1]), vec3(mat[2])),
        cross(vec3(mat[2]), vec3(mat[0])),
        cross(vec3(mat[0]), vec3(mat[1]))
    );
}