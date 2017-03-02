#include "win32.h"



//int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
int main()
{
	AppInterface *app = new App();

	app->Init();

	MatrixStack::SetProjectionMatrix(MatrixStack::projection);
	MatrixStack::SetModelviewMatrix(MatrixStack::modelview);

	ShaderInterface::SetMatrices(MatrixStack::projection,MatrixStack::modelview);

	OpenGLShader::Create("unlit",unlit_vert,unlit_frag);
	OpenGLShader::Create("unlit_color",unlit_color_vert,unlit_color_frag);
	OpenGLShader::Create("unlit_texture",unlit_texture_vs,unlit_texture_fs);
	OpenGLShader::Create("font",font_pixsh,font_frgsh);
	OpenGLShader::Create("shaded_texture",texture_vertex_shaded_vert,texture_vertex_shaded_frag);

	app->AppLoop();


	return 0;
}