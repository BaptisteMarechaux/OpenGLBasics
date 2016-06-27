//#version 150
#version 330 core

    in vec3 fColor;

    out vec4 outColor;


uniform sampler2D u_sampler;

uniform samplerCube skybox;

in VertexData {
    vec4 v_color;
    vec3 v_normal;
    vec2 v_texcoords;
} IN;

// Vecteur VERS la source lumineuse
const vec3 L = normalize(vec3(1.0, 1.0, 0.0));
// Vecteur incident
const vec3 T = normalize(vec3(0.0, 0.0, -1.0));

out vec4 Fragment;

const vec3 Position = vec3(0.0f, 0.0f, 0.0f);
const vec3 cameraPos= vec3(3.0f, 0.0f, 0.0f);

void main(void)
{
    vec3 N = normalize(IN.v_normal);

    vec4 texColor = texture(u_sampler, IN.v_texcoords);
    //vec4 Ambient = IN.v_color;

	// Equation de Lambert - calcul de l'intensite de la reflexion diffuse
	// I = cosA avec A = angle(L,N)
	float NdotL = max(dot(L, N), 0.0);
	vec4 Diffuse = vec4(vec3(NdotL), 1.0);

	vec3 I = T;
    vec3 R = reflect(I, normalize(IN.v_normal));

    Fragment = texture(skybox, R);//Diffuse * texColor /** vec4(fColor, 1.0)*/;


}