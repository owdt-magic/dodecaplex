// Hue shift utility function using RGB rotation matrix
vec3 hueShift(vec3 color, float shift) {
    float angle = radians(shift);
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    
    // RGB rotation matrix for hue shift
    mat3 hue_matrix = mat3(
        cos_a + (1.0 - cos_a) / 3.0, (1.0 - cos_a) / 3.0 - sin_a / sqrt(3.0), (1.0 - cos_a) / 3.0 + sin_a / sqrt(3.0),
        (1.0 - cos_a) / 3.0 + sin_a / sqrt(3.0), cos_a + (1.0 - cos_a) / 3.0, (1.0 - cos_a) / 3.0 - sin_a / sqrt(3.0),
        (1.0 - cos_a) / 3.0 - sin_a / sqrt(3.0), (1.0 - cos_a) / 3.0 + sin_a / sqrt(3.0), cos_a + (1.0 - cos_a) / 3.0
    );
    
    return hue_matrix * color;
}
