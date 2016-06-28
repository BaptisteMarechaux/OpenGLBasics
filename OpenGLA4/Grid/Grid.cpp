#include "common\EsgiShader.h"
EsgiShader basic;

#include <cstdio>

#ifdef _WIN32
	#pragma comment(lib, "opengl32.lib")
	#pragma comment(lib, "freeglut.lib")

#ifdef GLEW_STATIC
	#pragma comment(lib, "glew32s.lib")
#else
	#pragma comment(lib, "glew32.lib")
	#endif

	#include <Windows.h>

	#define FREEGLUT_LIB_PRAGMAS 0
#endif

#include "GL/glew.h"
#include "GL/freeglut.h"
#include <cmath>


struct vec3
{
	vec3() {}
	vec3(float f) : x(f), y(f), z(f) {}
	vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	vec3(const vec3& v) : x(v.x), y(v.y), z(v.z) {}

	float x, y, z;
};

struct mat4
{
	mat4() {}
	mat4(float f) { makeScale(f); }
	mat4(const mat4& _m) { memcpy(m, &_m, sizeof(float) * 16); }

	void identity() { makeScale(1.0f); }

	inline void makeScale(float f) {
		memset(m, 0, sizeof(float) * 16);
		m[0] = f; m[5] = f; m[10] = f; m[15] = 1.0f;
	};

	inline void makeRotateZ(float angle) {
		memset(m, 0, sizeof(float) * 16);
		m[0] = cos(angle); m[5] = 1.0f; m[10] = cos(angle);
		m[2] = sin(angle); m[8] = -sin(angle);
		m[15] = 1.0f;
	}

	inline void setPosition(const vec3& v) { m[12] = v.x; m[13] = v.y; m[14] = v.z; }

	float m[16];
};

struct Objet
{
	vec3 position;
	vec3 rotation;
	mat4 worldMatrix;
};

Objet g_Objet;

struct ViewProj
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec3 rotation;
} g_Camera;

mat4 perspectiveFov(float fov, float width, float height, float znear, float zfar)
{
	mat4 mat(0.0f);

	float aspect = width / height;

	float xymax = znear * tan(fov * 0.5f * 3.141592f / 180.0f);

	float ymin = -xymax;
	float xmin = -xymax;

	float near_width = xymax - xmin;
	float near_height = xymax - ymin;

	float depth = zfar - znear;
	float q = -(zfar + znear) / depth;
	float qn = -2 * (zfar * znear) / depth;

	float w = 2 * znear / near_width;
	w = w / aspect;
	float h = 2 * znear / near_height;

	mat.m[0] = w; mat.m[1] = 0.0f; mat.m[2] = 0.0f; mat.m[3] = 0.0f;
	mat.m[4] = 0.0f; mat.m[5] = h; mat.m[6] = 0.0f; mat.m[7] = 0.0f;
	mat.m[8] = 0.0f; mat.m[9] = 0.0f; mat.m[10] = q; mat.m[11] = -1.0f;
	mat.m[12] = 0.0f; mat.m[13] = 0.0f; mat.m[14] = qn; mat.m[15] = 0.0f;

	return mat;
}

void Initialize() 
{
	printf("Version OpenGL : %s\n", glGetString(GL_VERSION));
	printf("Fabricant : %s\n", glGetString(GL_VENDOR));
	printf("Pilote : %s\n", glGetString(GL_RENDERER));
	printf("Version GLSL : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	glewInit();

	int numExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	for (auto index = 0; index < numExtensions; ++index) 
	{
		printf("Extension[%d] : %s\n", index, glGetStringi(GL_EXTENSIONS, index));
	}

	basic.LoadVertexShader("base.vs");
	basic.LoadFragmentShader("base.fs");
	basic.LoadGeometryShader("base.gs");
	basic.Create();
}

void Terminate() 
{
	basic.Destroy();
}

void Update()
{
	glutPostRedisplay();
}

mat4 lookAt(const vec3& eye, const vec3& target, const vec3& up)
{
	mat4 mat(1.0f);

	mat.m[12] = -eye.x; mat.m[13] = -eye.y; mat.m[14] = -eye.z;

	return mat;
}

void Render() 
{
	auto width = glutGet(GLUT_WINDOW_WIDTH);
	auto height = glutGet(GLUT_WINDOW_HEIGHT);

	g_Camera.projectionMatrix = perspectiveFov(45.f, (float)width, (float)height, 0.1f, 1000.f);
	vec3 camera_position(0.0f, 0.0f, 0.0f);
	g_Camera.viewMatrix = lookAt(vec3(camera_position), vec3(camera_position.x + 5.0f, camera_position.y, camera_position.z + 0.71f), vec3(0.f, 1.f, 0.f));

	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	static const float points[] = {
		 0.0f,  0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f
	};

	auto program = basic.GetProgram();
	glUseProgram(program);

	GLint attrib_center = glGetAttribLocation(program, "center");
	GLint attrib_color = glGetAttribLocation(program, "color");
	GLint minAttrib = glGetAttribLocation(program, "min");
	GLint maxAttrib = glGetAttribLocation(program, "max");

	auto projectionLocation = glGetUniformLocation(program, "u_projectionMatrix");
	auto viewLocation = glGetUniformLocation(program, "u_viewMatrix");
	auto worldLocation = glGetUniformLocation(program, "u_worldMatrix");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, g_Camera.projectionMatrix.m);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, g_Camera.viewMatrix.m);
	g_Objet.worldMatrix.identity();
	vec3 objet_position(0.0f, 0.0f, -5.0f);
	g_Objet.worldMatrix.setPosition(objet_position);
	glUniformMatrix4fv(worldLocation, 1, GL_FALSE, g_Objet.worldMatrix.m);

	glEnableVertexAttribArray(attrib_center);
	glVertexAttribPointer(attrib_center, 2, GL_FLOAT, false, sizeof(float) * 7, points);

	glEnableVertexAttribArray(attrib_color);
	glVertexAttribPointer(attrib_color, 3, GL_FLOAT, false, sizeof(float) * 7, points + 2);

	glEnableVertexAttribArray(minAttrib);
	glVertexAttribPointer(minAttrib, 1, GL_FLOAT, false, sizeof(float) * 7, points + 5);

	glEnableVertexAttribArray(maxAttrib);
	glVertexAttribPointer(maxAttrib, 1, GL_FLOAT, false, sizeof(float) * 7, points + 6);

	glDrawArrays(GL_POINTS, 0, 1);

	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutCreateWindow("triangle");

	Initialize();

	glutIdleFunc(Update);

	glutDisplayFunc(Render);
	glutMainLoop();

	return 0;
}