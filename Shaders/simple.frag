#version 330

in vec4 v_color;
//in vec3 v_position;

layout (location = 0) out vec4 final_color;

void main(void) {
    //vec3 normal = normalize(cross(dFdx(v_position), dFdy(v_position)));
    final_color = v_color;
    //final_color.rgb = normal;
}
