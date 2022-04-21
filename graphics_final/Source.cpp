#include <GL/glew.h>
#include <GL/freeglut.h>
#include "cyCodeBase/cyTriMesh.h"
#include "cyCodeBase/cyMatrix.h"
#include <stdio.h>
#include <string>
#include<fstream>
#include<iostream>
#include "cyCodeBase/cyGL.h"
#include "lodepng/lodepng.h"
#include <stdlib.h>

//GLUT FUNCTIONS
void display_function();
void keyboard_function(unsigned char key, int x, int y);
void keyboard_up_function(unsigned char key, int x, int y);
void special_k_function(int key, int x, int y);
void special_k_up_function(int key, int x, int y);
void mouse_func(int x, int y);
void mouse_click_func(int button, int state, int x, int y);
void mouse_passive_func(int x, int y);
void idle_function();
void wind_reshape(int x, int y);
//HELPER FUNCTIONS
	//Initializations
void InitializeWindow();
void InitializeGlew();
void InitializePrograms();
void InitializeVao();
void InitializeGlutFuncs();
void InitializeNoise();
	//Object
void ComputeObjectMiddle();
std::vector<cy::Vec3f> build_object_buffer(cyTriMesh mesh);
std::vector<cy::Vec3f> build_norm_buffer(cyTriMesh mesh);
std::vector<cy::Vec2f> build_texCoord_buffer(cyTriMesh mesh);
std::vector<cy::Vec2f> build_rand_texCoord_buffer(cyTriMesh mesh);
std::vector<cy::Vec3f> build_noise_buffer(cyTriMesh mesh);
std::vector<cy::Vec3f> build_square_buff();

void render_g_buffer();
void render_edge_buffer();
void render_depth_buffer();
void render_shade_buffer();
void render_noise_buffer();
void render_drawing();

	//Matrices
cy::Matrix4f create_mvp(float mid, double rot_x, double rot_z, float distance);
cy::Matrix3f create_norm_mv(float mid, double rot_x, double rot_z, float distance);
	//program
void set_uniforms();
void set_vao();
//VARIABLES
	//Program
cy::GLSLProgram prog;
cy::GLSLProgram prog2;
cy::GLSLProgram prog3;
cy::GLSLProgram prog4;
cy::GLSLProgram output_prog;

	//Window
int window_width = 1920;
int window_height = 1080;
	//Vao
GLuint vao;
	//Object
cyTriMesh mesh = cyTriMesh();
int object_mid;
double rotx = -1.5;
double rotz = 0;
int obj_buff_size;
int norm_buff_size;
int square_buff;
	//Camera
float cam_dist = 50;
float fov = .7;
cy::Vec3f camera_pos = cy::Vec3f(0, 0, cam_dist);
	//Mouse
int mouse_x;
int mouse_y;
bool right_click = false;
	//Texture
cy::GLRenderTexture2D g_buff;
cy::GLRenderTexture2D edge_buff;
cy::GLRenderTexture2D shade_buff;
cy::GLRenderTexture2D noise_buff;
cy::GLRenderDepth2D depth_buff;
cyGLTexture2D noise_tex;
	//rand

float off = 1.5;
//main function
int main(int argc, char** argv)
{
	//Initialize
	glutInit(&argc, argv);
	InitializeWindow();
	InitializeGlew();
	InitializePrograms();
	InitializeVao();
	InitializeNoise();
	//Object from Command Line
	if (argc > 1)
	{
		mesh.LoadFromFileObj(argv[1]);
		ComputeObjectMiddle();

		set_vao();
	}
	set_uniforms();
	//Set Glut Functions
	InitializeGlutFuncs();
	glutMainLoop();
	return 0;

}
//helper functions
void InitializeWindow()
{
	glutInitContextVersion(4, 5);
	glutInitContextFlags(GLUT_DEBUG);
	glutInitWindowSize(window_width, window_height);
	glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Jack's GLUT");
}
void InitializeGlew()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
}
void InitializePrograms()
{
	prog.BuildFiles("gBuff.vert", "gBuff.frag");
	prog2.BuildFiles("edge.vert", "edge.frag");
	prog3.BuildFiles("shade.vert", "shade.frag");
	prog4.BuildFiles("noise.vert", "noise.frag");
	output_prog.BuildFiles("drawing.vert", "drawing.frag");
}
void InitializeVao()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
}
void InitializeGlutFuncs()
{
	glutDisplayFunc(display_function);
	glutKeyboardFunc(keyboard_function);
	glutKeyboardUpFunc(keyboard_up_function);
	glutIdleFunc(idle_function);
	glutMotionFunc(mouse_func);
	glutMouseFunc(mouse_click_func);
	glutPassiveMotionFunc(mouse_passive_func);
	glutSpecialFunc(special_k_function);
	glutReshapeFunc(wind_reshape);
	glutSpecialUpFunc(special_k_up_function);
	glClearColor(1,1,1, 1);
}
void InitializeNoise()
{
	noise_tex.Initialize();
	std::vector<unsigned char> image;
	unsigned tex_w, tex_h;
	lodepng::decode(image, tex_w, tex_h, "noiseTexture1.png");
	noise_tex.SetImage(&image.front(),4,tex_w,tex_h,0);
	noise_tex.BuildMipmaps();
}
void ComputeObjectMiddle()
{
	mesh.ComputeBoundingBox();
	while (!mesh.IsBoundBoxReady()) {}
	cy::Vec3f low_bound = mesh.GetBoundMin();
	cy::Vec3f high_bound = mesh.GetBoundMax();
	object_mid = (high_bound[2] - low_bound[2])/2;
}
void set_uniforms()
{
	//program 1
	prog["mvp"] = create_mvp(object_mid, rotx, rotz, cam_dist);
	prog["norm_mv"] = create_norm_mv(object_mid, rotx, rotz, cam_dist);
	//program 2
	prog2["mvp"] = create_mvp(object_mid,rotx,rotz,cam_dist);
	prog2["norm_mv"] = create_norm_mv(object_mid, rotx, rotz, cam_dist);
	prog2["texOff"] = cy::Vec2f(off,off);
	prog2["dim"] = cy::Vec2f(window_width,window_height);
	prog2["GBuffers"] = 0;
	//program 3
	prog3["mvp"] = create_mvp(object_mid, rotx, rotz, cam_dist);
	prog3["tex"] = 0;
	//program 4
	prog4["mvp"] = create_mvp(object_mid, rotx, rotz, cam_dist);
	prog4["noise"] = 0;
	//output
	output_prog["mvp"] = create_mvp(0, 0, 0, cam_dist/2);
	output_prog["noiseMap"] = 0;
	output_prog["shadeMap"] = 1;
	output_prog["edgeMap"] = 2;
	output_prog["depthMap"] = 3;
	output_prog["dim"] = cy::Vec2f(window_width, window_height);
	output_prog["dist"] = cam_dist;
	output_prog["edgeMat"] = cy::Matrix2f(1/(3*cam_dist),.003,.0001, 1 / (3 * cam_dist));
	output_prog["shadeMat"] = cy::Matrix2f(1/(3*cam_dist),.002,.01, 1 / (3 * cam_dist));

}
void set_vao()
{
	//Object Buffer 0 
	GLuint object_buff;
	glGenBuffers(1, &object_buff);
	std::vector<cy::Vec3f> obj_buff = build_object_buffer(mesh);
	glBindBuffer(GL_ARRAY_BUFFER, object_buff);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cy::Vec3f) * mesh.NF() * 3, &obj_buff.front(), GL_STATIC_DRAW);
	//norm buffer 1
	GLuint norm_buff;
	glGenBuffers(1, &norm_buff);
	std::vector<cy::Vec3f> n_buff = build_norm_buffer(mesh);
	glBindBuffer(GL_ARRAY_BUFFER, norm_buff);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(cy::Vec3f) * mesh.NF(), &n_buff.front(), GL_STATIC_DRAW);
	//texCoord buffer 2
	GLuint tex_buff;
	glGenBuffers(1, &tex_buff);
	std::vector<cy::Vec2f> t_buff = build_texCoord_buffer(mesh);
	glBindBuffer(GL_ARRAY_BUFFER, tex_buff);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(cy::Vec2f) * mesh.NF(), &t_buff.front(), GL_STATIC_DRAW);
	//noise buffer 3
	GLuint noise_buff;
	glGenBuffers(1, &noise_buff);
	std::vector<cy::Vec3f> nn_buff = build_noise_buffer(mesh);
	glBindBuffer(GL_ARRAY_BUFFER, noise_buff);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(cy::Vec3f) * mesh.NF(), &nn_buff.front(), GL_STATIC_DRAW);
	//square buffer
	GLuint plain_buff;
	glGenBuffers(1, &plain_buff);
	std::vector<cy::Vec3f> s_buff = build_square_buff();
	glBindBuffer(GL_ARRAY_BUFFER, plain_buff);
	glBufferData(GL_ARRAY_BUFFER, square_buff * sizeof(cy::Vec3f), &s_buff.front(), GL_STATIC_DRAW);

	glVertexArrayVertexBuffer(vao, 0, object_buff, 0, sizeof(cy::Vec3f));
	glVertexArrayAttribBinding(vao, 0, 0);
	glVertexArrayAttribFormat(vao, 0, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayBindingDivisor(vao, 0, 0);
	glEnableVertexArrayAttrib(vao, 0);

	glVertexArrayVertexBuffer(vao, 1, norm_buff, 0, sizeof(cy::Vec3f));
	glVertexArrayAttribBinding(vao, 1, 1);
	glVertexArrayAttribFormat(vao, 1, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayBindingDivisor(vao, 1, 0);
	glEnableVertexArrayAttrib(vao, 1);

	glVertexArrayVertexBuffer(vao, 2, tex_buff, 0, sizeof(cy::Vec2f));
	glVertexArrayAttribBinding(vao, 2, 2);
	glVertexArrayAttribFormat(vao, 2, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayBindingDivisor(vao, 2, 0);
	glEnableVertexArrayAttrib(vao, 2);

	glVertexArrayVertexBuffer(vao, 3, noise_buff, 0, sizeof(cy::Vec3f));
	glVertexArrayAttribBinding(vao, 3, 3);
	glVertexArrayAttribFormat(vao, 3, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayBindingDivisor(vao, 3, 0);
	glEnableVertexArrayAttrib(vao, 3);

	glVertexArrayVertexBuffer(vao, 4, plain_buff, 0, sizeof(cy::Vec3f));
	glVertexArrayAttribBinding(vao, 4, 4);
	glVertexArrayAttribFormat(vao, 4, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayBindingDivisor(vao, 4, 0);
	glEnableVertexArrayAttrib(vao, 4);

}
std::vector<cy::Vec3f> build_object_buffer(cyTriMesh mesh)
{
	int tri_points = mesh.NF();
	std::vector<cy::Vec3f> triangles;
	obj_buff_size = 3 * tri_points;
	for (int i = 0; i < tri_points; i++)
	{
		cy::TriMesh::TriFace face = mesh.F(i);
		triangles.push_back(mesh.V(face.v[0]));
		triangles.push_back(mesh.V(face.v[1]));
		triangles.push_back(mesh.V(face.v[2]));
	}
	return triangles;
}
std::vector<cy::Vec3f> build_norm_buffer(cyTriMesh mesh)
{
	int faces = mesh.NF();
	std::vector<cy::Vec3f> norms;
	norm_buff_size = 3 * faces;
	for (int i = 0; i < faces; i++)
	{
		cy::TriMesh::TriFace face = mesh.F(i);
		norms.push_back(mesh.VN(face.v[0]));
		norms.push_back(mesh.VN(face.v[1]));
		norms.push_back(mesh.VN(face.v[2]));
	}

	return norms;
}
std::vector<cy::Vec2f> build_texCoord_buffer(cyTriMesh mesh)
{
	int faces = mesh.NF();
	std::vector<cy::Vec2f> coords;
	norm_buff_size = 3 * faces;
	for (int i = 0; i < faces; i++)
	{
		cy::TriMesh::TriFace face = mesh.FT(i);
		cy::Vec3f texture1 = mesh.VT(face.v[0]);
		cy::Vec3f texture2 = mesh.VT(face.v[1]);
		cy::Vec3f texture3 = mesh.VT(face.v[2]);
		coords.push_back(texture1.XY());
		coords.push_back(texture2.XY());
		coords.push_back(texture3.XY());
	}

	return coords;
}
std::vector<cy::Vec2f> build_rand_texCoord_buffer(cyTriMesh mesh)
{
	int faces = mesh.NF();
	std::vector<cy::Vec2f> coords;
	norm_buff_size = 3 * faces;
	for (int i = 0; i < faces; i++)
	{
		double r = ((double)rand() / (RAND_MAX)) + 1;
		double r2 = ((double)rand() / (RAND_MAX)) + 1;
		coords.push_back(cy::Vec2f(r,r2));
		coords.push_back(cy::Vec2f(r, r2));
		coords.push_back(cy::Vec2f(r, r2));
	}

	return coords;
}
std::vector<cy::Vec3f> build_noise_buffer(cyTriMesh mesh)
{
	int faces = mesh.NF();
	std::vector<cy::Vec3f> norms;
	norm_buff_size = 3 * faces;
	srand((unsigned)time(NULL));
	for (int i = 0; i < faces; i++)
	{
		float x = (float)rand() / RAND_MAX;
		float y = (float)rand() / RAND_MAX;
		cy::Vec3f rand = cy::Vec3f(x/10,y/10,0);
		norms.push_back(rand);
		norms.push_back(rand);
		norms.push_back(rand);
	}

	return norms;
}
std::vector<cy::Vec3f> build_square_buff()
{
	std::vector<cy::Vec3f> triangles;
	double inc = 40;
	double i = -20;
	double j = -20;
	square_buff = (6);
	triangles.push_back(cy::Vec3f(i, j, 0));
	triangles.push_back(cy::Vec3f(i + inc, j, 0));
	triangles.push_back(cy::Vec3f(i, j + inc, 0));
	triangles.push_back(cy::Vec3f(i + inc, j, 0));
	triangles.push_back(cy::Vec3f(i, j + inc, 0));
	triangles.push_back(cy::Vec3f(i + inc, j + inc, 0));
	return triangles;
}
cy::Matrix4f create_mvp(float mid, double rot_x, double rot_z, float distance)
{
	cy::Vec3f cam_pos = { 0,0,distance };
	cy::Vec3f target = { 0,0,0 };
	cy::Vec3f cam_dir = (cam_pos - target).GetNormalized();
	cy::Vec3f up = { 0,1,0 };
	cy::Vec3f cam_r = up.Cross(cam_dir);
	cy::Vec3f cam_u = cam_dir.Cross(cam_r);
	cy::Matrix4f view = cy::Matrix4f::View(cam_pos, target, cam_u);
	cy::Matrix4f trans = cy::Matrix4f::Translation({ 0,0,-mid });
	cy::Matrix4f rtrans = cy::Matrix4f::Translation({ 0,0,mid });
	cy::Matrix3f xRot = cy::Matrix3f::RotationX(rot_x);
	cy::Matrix3f zRot = cy::Matrix3f::RotationZ(rot_z);
	cy::Matrix4f rot = rtrans * xRot * zRot * trans;
	cy::Matrix4f proj = cy::Matrix4f::Perspective(fov, float(window_width) / float(window_height), 10.0f, 1000.0f);

	return proj * view * rot;
}
cy::Matrix3f create_norm_mv(float mid, double rot_x, double rot_z, float distance)
{
	cy::Vec3f cam_pos = { 0,0,distance};
	cy::Vec3f target = { 0,0,0 };
	cy::Vec3f cam_dir = (cam_pos - target).GetNormalized();
	cy::Vec3f up = { 0,1,0 };
	cy::Vec3f cam_r = up.Cross(cam_dir);
	cy::Vec3f cam_u = cam_dir.Cross(cam_r);
	cy::Matrix4f trans = cy::Matrix4f::Translation({ 0,0,-mid });
	cy::Matrix4f rtrans = cy::Matrix4f::Translation({ 0,0,mid });
	cy::Matrix3f xRot = cy::Matrix3f::RotationX(rot_x);
	cy::Matrix3f zRot = cy::Matrix3f::RotationZ(rot_z);
	cy::Matrix4f view = cy::Matrix4f::View(cam_pos, target, cam_u);
	cy::Matrix4f rot = rtrans * xRot * zRot * trans;
	cy::Matrix4f mv = (view * rot).GetInverse().GetTranspose();
	return mv.GetSubMatrix3();
}
void render_edge_buffer()
{
	prog2.Bind();
	edge_buff.Initialize(true, 3, window_width, window_height);
	edge_buff.Bind();
	g_buff.BindTexture(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, obj_buff_size);
	edge_buff.Unbind();
	edge_buff.BuildTextureMipmaps();
	edge_buff.SetTextureAnisotropy(21);
	edge_buff.SetTextureFilteringMode(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
}
void render_depth_buffer()
{
	prog.Bind();
	depth_buff.Initialize(true, window_width, window_height);
	depth_buff.SetTextureFilteringMode(GL_LINEAR, GL_LINEAR);
	depth_buff.Bind();
	glClear(GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, obj_buff_size);
	depth_buff.Unbind();
}
void render_g_buffer()
{
	prog.Bind();
	g_buff.Initialize(true, 3, window_width, window_height);
	g_buff.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, obj_buff_size);
	g_buff.Unbind();
	g_buff.BuildTextureMipmaps();
	g_buff.SetTextureAnisotropy(21);
	g_buff.SetTextureFilteringMode(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
}
void render_shade_buffer()
{

	prog3.Bind();
	shade_buff.Initialize(true, 3, window_width, window_height);
	shade_buff.Bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, obj_buff_size);
	shade_buff.Unbind();
	shade_buff.BuildTextureMipmaps();
	shade_buff.SetTextureAnisotropy(21);
	shade_buff.SetTextureFilteringMode(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

}
void render_noise_buffer()
{
	prog4.Bind();

	noise_buff.Initialize(true, 3, window_width, window_height);
	noise_buff.Bind();
	noise_tex.Bind(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, obj_buff_size);
	noise_buff.Unbind();
	noise_buff.BuildTextureMipmaps();
	noise_buff.SetTextureAnisotropy(21);
	noise_buff.SetTextureFilteringMode(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);
}
void render_drawing()
{
	output_prog.Bind();
	noise_buff.BindTexture(0);
	shade_buff.BindTexture(1);
	edge_buff.BindTexture(2);
	depth_buff.BindTexture(3);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, obj_buff_size);

}
//glut functions
void display_function()
{
	glEnable(GL_DEPTH_TEST);
	render_depth_buffer();
	render_g_buffer();
	render_noise_buffer();
	render_shade_buffer();
	render_edge_buffer();
	render_drawing();
	glutSwapBuffers();
}
void keyboard_function(unsigned char key, int x, int y)
{
	switch (key) {
	case 27://esc
		glutLeaveMainLoop();
		break;
	}
	glutPostRedisplay();

}
void keyboard_up_function(unsigned char key, int x, int y)
{
}
void special_k_function(int key, int x, int y)
{

}
void special_k_up_function(int key, int x, int y)
{

}
void mouse_func(int x, int y)
{
	if (right_click)
	{
		cam_dist -= (x - mouse_x) / 2;
	}
	else
	{
		if (x - mouse_x != 0)
		{
			rotz += ((x - mouse_x)) * 3.14 / 180;
		}
		if (y - mouse_y != 0)
		{
			rotx += ((y - mouse_y) * 3.14) / 180;

		}
	}
	set_uniforms();
	mouse_y = y;
	mouse_x = x;
	glutPostRedisplay();
}
void mouse_click_func(int button, int state, int x, int y)
{
	if (button == 0)
	{
		mouse_x = x;
		mouse_y = y;
	}
	else
	{
		right_click = !right_click;
		mouse_x = x;
	}
	glutPostRedisplay();

}
void mouse_passive_func(int x, int y)
{
}
void idle_function()
{

}
void wind_reshape(int x, int y)
{
	window_width = x;
	window_height = y;
	set_uniforms();
	glViewport(0, 0, window_width, window_height);
	set_uniforms();
	glutPostRedisplay();
}
