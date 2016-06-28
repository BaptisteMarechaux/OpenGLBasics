//
// ESGI
// ObjLoaderAvecSkyBox.cpp 
//

#include <cstdio>
#include <cmath>

#include <vector>
#include <string>

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

#include <cstdio>
#include <cmath>
#include <cstdint>

#include "GL/glew.h"
#include "GL/freeglut.h"

#include "EsgiShader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "dds.h"

#include "tinyobjloader/tiny_obj_loader.h"

// ---

GLuint textureID;

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
		memset(m, 0, sizeof(float)*16);
		m[0] = f; m[5] = f; m[10] = f; m[15] = 1.0f;
	};

	inline void makeRotateY(float angle) {
		memset(m, 0, sizeof(float)*16);
		m[0] = cos(angle); m[5] = 1.0f; m[10] = cos(angle);
		m[2] = sin(angle); m[8] = -sin(angle);
		m[15] = 1.0f;
	}

	inline void setPosition(const vec3& v) { m[12] = v.x; m[13] = v.y; m[14] = v.z; }

	float m[16];
};

#ifndef M_PI
#define M_PI 3.141592f
#endif


static inline float radians(float deg) {
	return deg * M_PI / 180.0f;
}

static inline float degrees(float rad) {
	return rad * 180.0f / M_PI;
}

mat4 perspectiveFov(float fov, float width, float height, float znear, float zfar)
{
	mat4 mat(0.0f);

	float aspect = width / height;

	float xymax = znear * tan(fov * 0.5f * M_PI/180.0f);

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
 
	// ne pas oublier il s'agit de colonnes !
	mat.m[0]  = w; mat.m[1]  = 0.0f; mat.m[2]  = 0.0f; mat.m[3]  = 0.0f;
	mat.m[4]  = 0.0f; mat.m[5]  = h; mat.m[6]  = 0.0f; mat.m[7]  = 0.0f;
	mat.m[8]  = 0.0f; mat.m[9]  = 0.0f; mat.m[10] = q; mat.m[11] = -1.0f;
	mat.m[12] = 0.0f; mat.m[13] = 0.0f; mat.m[14] = qn; mat.m[15] = 0.0f;

	return mat;
}

mat4 lookAt(const vec3& eye, const vec3& target, const vec3& up)
{
	mat4 mat(1.0f);

	// TODO: une vraie lookAt matrix (cf TD au format doc)

	mat.m[12] = -eye.x; mat.m[13] = -eye.y; mat.m[14] = -eye.z;

	return mat;
}

// ---

bool LoadAndCreateTexture(const char* nom, GLuint& textureObj)
{
	int w, h, comp;
	int req_comp = 4;
	auto* image = stbi_load(nom, &w, &h, &comp, req_comp);
	if (image)
	{
		glGenTextures(1, &textureObj);
		glBindTexture(GL_TEXTURE_2D, textureObj);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}
	return false;
}

bool LoadAndCreateCubeMapTexture( const char * (&nom)[6], GLuint & textureObj)
{
	int w, h, comp;
	int req_comp = STBI_rgb_alpha;

	glGenTextures(1, &textureObj);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureObj);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	for (auto index = 0; index < 6; index++)
	{
		auto* image = stbi_load(nom[index], &w, &h, &comp, req_comp);
		if (image)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
			stbi_image_free(image);			
		}
		else
			return false;
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return true;
}

inline uint32_t CalcMipmapSize(uint32_t w, uint32_t h) {
	return ((w + 3) / 4)*((h + 3) / 4)*sizeof(uint64_t);
}

GLuint CreateTexture(const char* nom)
{
	uint32_t w, h;
	uint8_t * image  = new uint8_t[0];
	uint32_t sizeImage = LoadImageDDS(&image, w, h, nom);
	if (sizeImage)
	{
		GLuint textureObj;
		glGenTextures(1, &textureObj);
		glBindTexture(GL_TEXTURE_2D, textureObj);
		/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, w, h, 0, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, image);
		//LoadImageDDS
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		// Par defaut il utilise T qui attend une mipmap donc l'image sera noir
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		for (auto index = 0; index < 6; index++)
		{
			auto* image = stbi_load(nom[index], &w, &h, &comp, req_comp);
			if (image)
			{
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
				stbi_image_free(image);
			}
			else
				return false;
		}
		*/

		uint32_t nHeight = h;
		uint32_t nWidth = w;

		// Nombre de mips
		int nNumMipMaps = 9;
		//int nBlockSize = 8;

		// A défnir avant
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int nSize;
		// Décallage dans la chaine de bits
		int nOffset = 0;

		for (int i = 0; i < nNumMipMaps; ++i)
		{
			if (nWidth == 0) nWidth = 1;
			if (nHeight == 0) nHeight = 1;

			nSize = CalcMipmapSize(nWidth, nHeight);
			//nSize = ((nWidth + 3) / 4)*((nHeight + 3) / 4)*nBlockSize;

			glCompressedTexImage2D(GL_TEXTURE_2D, i, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, nWidth, nHeight, 0 , nSize, image + nOffset);

			nOffset += nSize;

			nWidth = (nWidth / 2);
			nHeight = (nHeight / 2);
		}

		FreeImageDDS(&image);

		//Rendre actif sur l'unité de texture 0
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0); //Dessiner sans texture

		return textureObj;
	}
	printf("Erreur DDS");
	return 0;
}

void DestroyTexture(GLuint textureObj)
{
	glDeleteTextures(1, &textureObj);
}

// ---

EsgiShader g_BasicShader;
EsgiShader g_SkyBoxShader;

int previousTime = 0;

struct ViewProj
{
	mat4 viewMatrix;
	mat4 projectionMatrix;
	vec3 rotation;	
} g_Camera;

struct Objet
{
	// transform
	vec3 position;
	vec3 rotation;
	mat4 worldMatrix;	
	// mesh
	GLuint VBO;
	GLuint IBO;
	GLuint ElementCount;
	GLenum PrimitiveType;
	GLuint VAO;
	// material
	GLuint textureObj;
};

Objet g_Objet;
Objet g_CubeMap;

// ---

void DestroyCubeMap()
{
	g_SkyBoxShader.Destroy();
	glDeleteTextures(1, &g_CubeMap.textureObj);
	glDeleteVertexArrays(1, &g_CubeMap.VAO);
	glDeleteBuffers(1, &g_CubeMap.VBO);
}

void CreateCubeMap()
{
	#include "skycube.h"
#include "ObjLoaderAvecSkyBox.h"

	glGenVertexArrays(1, &g_CubeMap.VAO);
	glBindVertexArray(g_CubeMap.VAO);
	glGenBuffers(1, &g_CubeMap.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_CubeMap.VBO);
	// 6 faces composees de 2 triangles chacun compose de 3 vertices au format x,y,z
	glBufferData(GL_ARRAY_BUFFER, 6*(2*3)*(3*sizeof(float)), skyboxVertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), nullptr);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	const char* skybox[] = {
		"skybox/right.jpg"
		, "skybox/left.jpg"
		, "skybox/top.jpg"
		, "skybox/bottom.jpg"
		, "skybox/back.jpg"
		, "skybox/front.jpg"
	};

	LoadAndCreateCubeMapTexture(skybox, g_CubeMap.textureObj);

	g_SkyBoxShader.LoadVertexShader("skybox.vs");
	g_SkyBoxShader.LoadFragmentShader("skybox.fs");
	g_SkyBoxShader.Create();
}

void LoadOBJ(const std::string &inputFile)
{	
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err = tinyobj::LoadObj(shapes, materials, inputFile.c_str());
	const std::vector<unsigned int>& indices = shapes[0].mesh.indices;
	const std::vector<float>& positions = shapes[0].mesh.positions;
	const std::vector<float>& normals = shapes[0].mesh.normals;
	const std::vector<float>& texcoords = shapes[0].mesh.texcoords;

	g_Objet.ElementCount = indices.size();
	
	uint32_t stride = 0;

	if (positions.size()) {
		stride += 3 * sizeof(float);
	}
	if (normals.size()) {
		stride += 3 * sizeof(float);
	}
	if (texcoords.size()) {
		stride += 2 * sizeof(float);
	}

	const auto count = positions.size() / 3;
	const auto totalSize = count * stride;
	
	glGenBuffers(1, &g_Objet.IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_Objet.IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), &indices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &g_Objet.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_Objet.VBO);
	glBufferData(GL_ARRAY_BUFFER, totalSize, nullptr, GL_STATIC_DRAW);

	// glMapBuffer retourne un pointeur sur la zone memoire allouee par glBufferData 
	// du Buffer Object qui est actuellement actif - via glBindBuffer(<cible>, <id>)
	// il est imperatif d'appeler glUnmapBuffer() une fois que l'on a termine car le
	// driver peut tres bien etre amener a modifier l'emplacement memoire du BO.
	float* vertices = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);		
	for (auto index = 0; index < count; ++index)
	{
		if (positions.size()) {
			memcpy(vertices, &positions[index * 3], 3 * sizeof(float));
			vertices += 3;
		}
		if (normals.size()) {
			memcpy(vertices, &normals[index * 3], 3 * sizeof(float));
			vertices += 3;
		}
		if (texcoords.size()) {
			memcpy(vertices, &texcoords[index * 2], 2 * sizeof(float));
			vertices += 2;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	glGenVertexArrays(1, &g_Objet.VAO);
	glBindVertexArray(g_Objet.VAO);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_Objet.IBO);

	glBindBuffer(GL_ARRAY_BUFFER, g_Objet.VBO);
	uint32_t offset = 3 * sizeof(float);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, nullptr);
	glEnableVertexAttribArray(0);
	if (normals.size()) {		
		glVertexAttribPointer(1, 3, GL_FLOAT, false, stride, (GLvoid *)offset);
		glEnableVertexAttribArray(1);
		offset += 3 * sizeof(float);
	}
	if (texcoords.size()) {
		glVertexAttribPointer(2, 2, GL_FLOAT, false, stride, (GLvoid *)offset);
		glEnableVertexAttribArray(2);
		offset += 2 * sizeof(float);
	}

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	

	LoadAndCreateTexture(materials[0].diffuse_texname.c_str(), g_Objet.textureObj);
}

void CleanObjet(Objet& objet)
{
	if (objet.textureObj)
		glDeleteTextures(1, &objet.textureObj);
	if (objet.VAO)
		glDeleteVertexArrays(1, &objet.VAO);
	if (objet.VBO)
		glDeleteBuffers(1, &objet.VBO);
	if (objet.IBO)
		glDeleteBuffers(1, &objet.IBO);
}

// Initialisation et terminaison ---

void Initialize()
{
	printf("Version Pilote OpenGL : %s\n", glGetString(GL_VERSION));
	printf("Type de GPU : %s\n", glGetString(GL_RENDERER));
	printf("Fabricant : %s\n", glGetString(GL_VENDOR));
	printf("Version GLSL : %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	int numExtensions;
	glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
	
	GLenum error = glewInit();
	if (error != GL_NO_ERROR) {
		// TODO
	}

#if LIST_EXTENSIONS
	for (int index = 0; index < numExtensions; ++index)
	{
		printf("Extension[%d] : %s\n", index, glGetStringi(GL_EXTENSIONS, index));
	}
#endif
	
#ifdef _WIN32
	// on coupe la synchro vertical pour voir l'effet du delta time
	//wglSwapIntervalEXT(0);
#endif

	// render states par defaut
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);	

	// Objets OpenGL

	g_BasicShader.LoadVertexShader("basic.vs");
	g_BasicShader.LoadFragmentShader("basic.fs");
	g_BasicShader.LoadGeometryShader("basic.gs");
	g_BasicShader.Create();


	auto program = g_BasicShader.GetProgram();

	// Setup

	previousTime = glutGet(GLUT_ELAPSED_TIME);

	const std::string inputFile = "rock.obj";
	LoadOBJ(inputFile);

	CreateCubeMap();

	textureID = CreateTexture("mt.dds"); 
	//textureID = CreateTexture("Rock-Texture-Surface.jpg");
}

void Terminate()
{	
	DestroyCubeMap();

	CleanObjet(g_Objet);

	g_BasicShader.Destroy();
}

// boucle principale ---

void Resize(GLint width, GLint height) 
{
	glViewport(0, 0, width, height);
	
	g_Camera.projectionMatrix = perspectiveFov(45.f, (float)width, (float)height, 0.1f, 1000.f);
}

static float time = 0.0f;

void Update() 
{
	auto currentTime = glutGet(GLUT_ELAPSED_TIME);
	auto delta = currentTime - previousTime;
	previousTime = currentTime;
	auto elapsedTime = delta / 1000.0f;
	time += elapsedTime;
	//g_Objet.rotation += vec3(36.0f * elapsedTime);
	//g_Camera.rotation.y += 10.f * elapsedTime;

	glutPostRedisplay();
}

void Render()
{
	auto width = glutGet(GLUT_WINDOW_WIDTH);
	auto height = glutGet(GLUT_WINDOW_HEIGHT);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// variables uniformes (constantes) 

	g_Camera.projectionMatrix = perspectiveFov(45.f, (float)width, (float)height, 0.1f, 1000.f);
	
	//vec3 camera_position(0.0f, 0.0f, 0.0f);
	//g_Camera.viewMatrix = lookAt(vec3(camera_position), vec3(camera_position.x + 0.71f, camera_position.y, camera_position.z + 0.71f), vec3(0.f, 1.f, 0.f));
	g_Camera.viewMatrix.makeRotateY(time);

	g_Objet.worldMatrix.identity();
	vec3 objet_position(0.0f, 0.0f, -5.0f);
	g_Objet.worldMatrix.setPosition(objet_position);

	// rendu	

	auto program = g_BasicShader.GetProgram();
	glUseProgram(program);
	
	GLint timeLocation = glGetUniformLocation(program, "u_time");
	glUniform1f(timeLocation, time);

	auto projectionLocation = glGetUniformLocation(program, "u_projectionMatrix");
	auto viewLocation = glGetUniformLocation(program, "u_viewMatrix");
	auto worldLocation = glGetUniformLocation(program, "u_worldMatrix");
	//auto a = glGetUniformLocation(program, "a");
	//glUniform1i(a, 0);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, g_Camera.projectionMatrix.m);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, g_Camera.viewMatrix.m);
	glUniformMatrix4fv(worldLocation, 1, GL_FALSE, g_Objet.worldMatrix.m);

	glBindTexture(GL_TEXTURE_2D, textureID);
	//glBindTexture(GL_TEXTURE_2D, g_Objet.textureObj);

	glBindVertexArray(g_Objet.VAO);

	//glBindTexture(GL_TEXTURE_CUBE_MAP, g_CubeMap.textureObj);

	glDepthFunc(GL_LESS);
	glDrawElements(GL_TRIANGLES, g_Objet.ElementCount, GL_UNSIGNED_INT, 0);

	/*objet_position = vec3(4.0f, 0.0f, -3.0f);
	g_Objet.worldMatrix.setPosition(objet_position);
	glUniformMatrix4fv(worldLocation, 1, GL_FALSE, g_Objet.worldMatrix.m);
	//glUniform1d(a, 1);
	glDrawElements(GL_TRIANGLES, g_Objet.ElementCount, GL_UNSIGNED_INT, 0);*/

	glBindVertexArray(0);

	// Skybox 

	program = g_SkyBoxShader.GetProgram();
	glUseProgram(program);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_CubeMap.textureObj);
	auto skyboxSampler = glGetUniformLocation(program, "u_cubemap");
	glUniform1i(skyboxSampler, 1);

	projectionLocation = glGetUniformLocation(program, "u_projectionMatrix");
	viewLocation = glGetUniformLocation(program, "u_viewMatrix");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, g_Camera.projectionMatrix.m);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, g_Camera.viewMatrix.m);

	glBindVertexArray(g_CubeMap.VAO);		
	glDepthFunc(GL_LEQUAL);
	glDrawArrays(GL_TRIANGLES, 0, 6*2*3); // 36
	glActiveTexture(GL_TEXTURE0);


	program = g_BasicShader.GetProgram();

	// Create VBO with point coordinates
	GLuint vbo;
	glGenBuffers(1, &vbo);

	GLfloat points[] = {
		//  Coordinates     Color             Sides
		-0.45f,  0.45f, 1.0f, 0.0f, 0.0f,  4.0f,
		0.45f,  0.45f, 0.0f, 1.0f, 0.0f,  8.0f,
		0.45f, -0.45f, 0.0f, 0.0f, 1.0f, 16.0f,
		-0.45f, -0.45f, 1.0f, 1.0f, 0.0f, 32.0f
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

	// Create VAO
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Specify the layout of the vertex data
	GLint posAttrib = glGetAttribLocation(program, "pos");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

	GLint colAttrib = glGetAttribLocation(program, "color");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

	GLint sidesAttrib = glGetAttribLocation(program, "sides");
	glEnableVertexAttribArray(sidesAttrib);
	glVertexAttribPointer(sidesAttrib, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));

	glDrawArrays(GL_POINTS, 0, 4);

	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("OBJ Loader avec SkyBox");

#ifdef FREEGLUT
	// Note: glutSetOption n'est disponible qu'avec freeGLUT
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,
				  GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif

	Initialize();

	glutReshapeFunc(Resize);
	glutIdleFunc(Update);
	glutDisplayFunc(Render);

	glutMainLoop();

	Terminate();

	return 0;
}

