#include "win32.h"

#define USEFBX 0

#if USEFBX
	#include "fbxutil.h"
#endif


void ___saferelease(IUnknown* iPtr)
{
	if(0!=iPtr)
	{
		iPtr->Release();
		iPtr=0;
	}
}


bool KeyboardInput::IsPressed(unsigned int iCharCode)
{
	return ((::GetKeyState(iCharCode) >> 8) & 0xff)!=0;
}


PFNWGLCHOOSEPIXELFORMATEXTPROC wglChoosePixelFormatARB = 0;
PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = 0;

#ifndef GL_GLEXT_PROTOTYPES
extern PFNGLATTACHSHADERPROC glAttachShader=0;
extern PFNGLBINDBUFFERPROC glBindBuffer=0;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray=0;
extern PFNGLBUFFERDATAPROC glBufferData=0;
extern PFNGLCOMPILESHADERPROC glCompileShader=0;
extern PFNGLCREATEPROGRAMPROC glCreateProgram=0;
extern PFNGLCREATESHADERPROC glCreateShader=0;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers=0;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram=0;
extern PFNGLDELETESHADERPROC glDeleteShader=0;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays=0;
extern PFNGLDETACHSHADERPROC glDetachShader=0;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray=0;
extern PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib=0;
extern PFNGLGENBUFFERSPROC glGenBuffers=0;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays=0;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation=0;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog=0;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv=0;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog=0;
extern PFNGLGETSHADERIVPROC glGetShaderiv=0;
extern PFNGLLINKPROGRAMPROC glLinkProgram=0;
extern PFNGLSHADERSOURCEPROC glShaderSource=0;
extern PFNGLUSEPROGRAMPROC glUseProgram=0;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer=0;
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation=0;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation=0;
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv=0;
extern PFNGLACTIVETEXTUREPROC glActiveTexture=0;
extern PFNGLUNIFORM1IPROC glUniform1i=0;
extern PFNGLUNIFORM1FPROC glUniform1f=0;
extern PFNGLGENERATEMIPMAPPROC glGenerateMipmap=0;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray=0;
extern PFNGLUNIFORM3FVPROC glUniform3fv=0;
extern PFNGLUNIFORM4FVPROC glUniform4fv=0;
extern PFNGLTEXBUFFERPROC glTexBuffer=0;
extern PFNGLTEXTUREBUFFERPROC glTextureBuffer=0;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData=0;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers=0;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers=0;
extern PFNGLREADNPIXELSPROC glReadnPixels=0;
extern PFNGLUNIFORM2FPROC glUniform2f=0;
extern PFNGLUNIFORM2FVPROC glUniform2fv=0;
extern PFNGLUNIFORM3FPROC glUniform3f=0;
extern PFNGLUNIFORM4FPROC glUniform4f=0;
extern PFNWGLGETPIXELFORMATATTRIBIVARBPROC wglGetPixelFormatAttribivARB=0;
#endif


bool InitSplitter();

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////ThreadWin32//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

DWORD WINAPI threadFunc(LPVOID data)

{
	Thread* pThread=(Thread*)data;

	while(true)
	{
		Timer::instance->update();

		for(std::list<Task*>::iterator taskIter=pThread->tasks.begin();taskIter!=pThread->tasks.end();)
		{
			Task* pTask=(*taskIter);

			if(!pThread->pause && !pTask->pause && pTask->owner==pThread)
			{
				pThread->executing=pTask;

				if(pTask->func)
				{
					pTask->executing=true;
					pTask->func();
					pTask->executing=false;
				}
				if(pTask->remove)
				{
					pTask->func=nullptr;
					taskIter=pThread->tasks.erase(taskIter);
				}
				else
					taskIter++;
				
				pThread->executing=0;
			}
			else
				taskIter++;
		}

		::Sleep(pThread->sleep);
	}


	ExitThread(0);
}


ThreadWin32::ThreadWin32()
{
	handle=CreateThread(0,0,threadFunc,this,/*CREATE_SUSPENDED*/0,(DWORD*)(int*)&id);
	pause=false;
	executing=0;
	sleep=1;
}

ThreadWin32::~ThreadWin32()
{
	TerminateThread((HANDLE)handle,0);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////Direct2DBase//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

ID2D1Factory*			Direct2D::factory=0;
IWICImagingFactory*		Direct2D::imager=0;
IDWriteFactory*			Direct2D::writer=0;
IDWriteTextFormat*		Direct2D::texter=0;


char	Direct2D::charsWidth[255]={0};

const int Direct2D::fontLogicSize=10;
const wchar_t* Direct2D::fontFaceName=L"Verdana";

void Direct2D::Release()
{
	SAFERELEASE(factory);
	SAFERELEASE(imager);
	SAFERELEASE(writer);
	SAFERELEASE(texter);
};

void Direct2D::Init()
{
	if(!factory)
	{
		printf("Initializing Direct2D\n");

		HRESULT res=S_OK;

		res=CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,(LPVOID*)&Direct2D::imager);
		if(S_OK!=res)
			DEBUG_BREAK();

		res=D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &Direct2D::factory);
		if(S_OK!=res || !Direct2D::factory)
			DEBUG_BREAK();

		res=DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(writer),reinterpret_cast<IUnknown **>(&Direct2D::writer));
		if(S_OK!=res || !Direct2D::writer)
			DEBUG_BREAK();

		res=Direct2D::writer->CreateTextFormat(Direct2D::fontFaceName,NULL,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,Direct2D::fontLogicSize,L"",&Direct2D::texter);
		if(S_OK!=res || !Direct2D::texter)
			DEBUG_BREAK();
		
		res=Direct2D::texter->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
		if(S_OK!=res)
			DEBUG_BREAK();

		res=Direct2D::texter->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM,13.333334f,Direct2D::fontLogicSize);

		if(S_OK!=res)
			DEBUG_BREAK();

		{
 			const void*	tTableData=0;
			UINT32		tTableSize=0;
			void*		tTableContext=0;

			IDWriteFontFace* tFontFace = NULL;
			IDWriteFontFile* tFontFile = NULL;

			res = Direct2D::writer->CreateFontFileReference(L"c:\\windows\\fonts\\verdana.ttf",NULL,&tFontFile);

			if(S_OK!=res || !tFontFile)
				DEBUG_BREAK();

			res = Direct2D::writer->CreateFontFace(DWRITE_FONT_FACE_TYPE_TRUETYPE,1,&tFontFile,0,DWRITE_FONT_SIMULATIONS_NONE,&tFontFace);

			if(S_OK!=res || !tFontFace)
				DEBUG_BREAK();

			BOOL exists=false;	
		
			res=tFontFace->TryGetFontTable(DWRITE_MAKE_OPENTYPE_TAG('h','d','m','x'),&tTableData,&tTableSize,&tTableContext,&exists);

			if(S_OK!=res || !exists)
				DEBUG_BREAK();

			UINT16 numRecords=0;
			UINT32 recordSize=0;

			UINT8 *tablePointer=(UINT8*)tTableData;

			//getting record infos
			{
				UINT8 __word[4];

				__word[0]=*(tablePointer + 3);
				__word[1]=*(tablePointer + 2);

				numRecords=*((UINT16*)__word);

				UINT8 __long[4];

				__long[0]=*(tablePointer + 7);
				__long[1]=*(tablePointer + 6);
				__long[2]=*(tablePointer + 5);
				__long[3]=*(tablePointer + 4);

				recordSize=*((UINT32*)__long);
			}

			//records begin here
			tablePointer+=8;

			//matching infos
			for(int iRecord=0;iRecord<numRecords;iRecord++)
			{
				UINT8 ppem=*tablePointer;

				if(ppem!=Direct2D::GetFontSize())
					tablePointer+=recordSize*(iRecord+1);
				else
					break;
			}

			for(int i=0;i<255;i++)
				Direct2D::charsWidth[i]=*(tablePointer+i);

			SAFERELEASE(tFontFace);
			SAFERELEASE(tFontFile);

			if(0)
			{
				printf("----------\n");


				char *prova=" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~";
				char* t=prova;


				while(*t)
				{
					int width=charsWidth[(*t)-32];
					printf("%c: %d\n",*t,width);
					t++;
				}

				printf("\n");
			}
		}

		// Center the text horizontally and vertically.
		//textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		/*res=texter->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		if(S_OK!=res)
			__debugbreak();*/
	}
}

void dump(unsigned char* t)
{
	printf("\n");
	for(int i=0;i<1600;i++)
	{
		printf("0x%x,",t[i]);
	}
	printf("\n");
	system("pause");
}

void Direct2D::CreateRawBitmap(const wchar_t* fname,unsigned char*& buffer,float &width,float &height)
{
	wprintf(L"loading %s from disk...\n",fname);

	ID2D1Bitmap *bitmap=0;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRESULT hr;

	hr = imager->CreateDecoderFromFilename(fname,NULL,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&pDecoder);

	if (!SUCCEEDED(hr))
		DEBUG_BREAK();

	unsigned int frameCount;
	pDecoder->GetFrameCount(&frameCount);

	hr = pDecoder->GetFrame(0, &pSource);

	if (!SUCCEEDED(hr))
		DEBUG_BREAK();

	hr = imager->CreateFormatConverter(&pConverter);

	if (!SUCCEEDED(hr))
		DEBUG_BREAK();

	hr = pConverter->Initialize(pSource,GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,NULL,0.f,WICBitmapPaletteTypeMedianCut);

	if(buffer)
	delete [] buffer;

	UINT _width,_height;
	pSource->GetSize(&_width,&_height);

	buffer=new unsigned char[_width*_height*4];

	hr=pSource->CopyPixels(0,_width*4,_width*_height*4,buffer);

	width=(float)_width;
	height=(float)_height;

	if (!SUCCEEDED(hr))
		DEBUG_BREAK();

	dump(buffer);

	if (SUCCEEDED(hr))
	{
		SAFERELEASE(pDecoder);
		SAFERELEASE(pSource);
		SAFERELEASE(pStream);
		SAFERELEASE(pConverter);
		SAFERELEASE(pScaler);
	}
}

vec2 Direct2D::MeasureText(ID2D1RenderTarget*renderer,const char* iText,int iSlen)
{
	float fontHeight=Direct2D::GetFontHeight((ID2D1HwndRenderTarget*)renderer);

	float width=0;
	float height=0;

	char* t=(char*)iText;

	while(*t)
	{
		if(*t=='\n' || *t=='\r')
			height+=fontHeight;
		else
			width+=Direct2D::charsWidth[*t];

		t++;
	}

	return vec2(width,height);
}

vec2 Direct2D::MeasureText(ID2D1RenderTarget*renderer,const wchar_t* iText,int iSlen)
{
	float fontHeight=Direct2D::GetFontHeight((ID2D1HwndRenderTarget*)(renderer));

	float width=0;
	float height=0;

	wchar_t* t=(wchar_t*)iText;

	while(*t)
	{
		if(*t=='\n' || *t=='\r')
			height+=fontHeight;
		else
			width+=Direct2D::charsWidth[*t];

		t++;
	}

	return vec2(width,height);
}

float Direct2D::GetFontHeight(ID2D1HwndRenderTarget* renderer)
{
	const float tTypographicPoint=1.0f/72.0f;
	const float tDeviceIndependentPixelFontHeightValue=Direct2D::texter->GetFontSize();

	float tDotPerInchX,tDotPerInchY;

	renderer->GetDpi(&tDotPerInchX,&tDotPerInchY);

	float tDeviceIndependentPixelScalar=tDotPerInchY * tTypographicPoint;
	float tPixelValue=tDeviceIndependentPixelScalar * tDeviceIndependentPixelFontHeightValue;

	return tPixelValue;
}

float Direct2D::GetFontSize()
{
	return Direct2D::texter->GetFontSize();
}

float Direct2D::GetCharWidth(char iCharacter)
{
	return iCharacter!='\t' ? Direct2D::charsWidth[iCharacter-27] : Direct2D::charsWidth[' '-27];
}


void Direct2D::DrawText(ID2D1RenderTarget*iRenderer,ID2D1Brush* iBrush,const wchar_t* iText,float x1,float y1, float x2,float y2,float iAlignPosX,float iAlignPosY,bool iClip)
{
	if(!iText)
		return;

	unsigned int tFinalTextLength=wcslen(iText);

	if(iClip)
		iRenderer->PushAxisAlignedClip(D2D1::RectF(x1,y1,x2,y2),D2D1_ANTIALIAS_MODE_ALIASED);

	if(iAlignPosX<0 && iAlignPosY<0)
	{
		//Direct2D::DrawRectangle(iRenderer,iBrush,x1,y1,x2,y2,false);
		iRenderer->DrawText(iText,tFinalTextLength,texter,D2D1::RectF(x1,y1,x2+500,y2+500),iBrush,D2D1_DRAW_TEXT_OPTIONS_NONE,DWRITE_MEASURING_MODE_GDI_CLASSIC);
	}
	else
	{
		vec2 tSize=MeasureText(iRenderer,iText);

		float width=x2-x1;
		float height=y2-y1;

		vec4 rect(-tSize.x/2.0f,-tSize.y/2.0f,(float)tSize.x,(float)tSize.y);

		if(tSize.x>width || tSize.y>height)
		{

		}
		
		//
		iAlignPosX>=0 ? rect.x+=x1+iAlignPosX*width : rect.x=x1;
		iAlignPosY>=0 ? rect.y+=y1+iAlignPosY*height : rect.y=y1;

		/*rect.x = x > rect.x ? x : (x+w < rect.x+rect.z ? rect.x - (rect.x+rect.z - (x+w)) - tSize.cx/2.0f: rect.x);
		rect.y = y > rect.y ? y : (y+h < rect.y+rect.w ? rect.y - (rect.y+rect.w - (y+h)) - tSize.c/2.0f: rect.y);*/

		//Direct2D::DrawRectangle(iRenderer,iBrush,rect.x,rect.y,rect.x+rect.z,rect.y+rect.w,false);
		iRenderer->DrawText(iText,tFinalTextLength,texter,D2D1::RectF(rect.x,rect.y,rect.x+rect.z,rect.y+rect.w),iBrush,D2D1_DRAW_TEXT_OPTIONS_NONE,DWRITE_MEASURING_MODE_GDI_CLASSIC);
		
	}

	if(iClip)
		iRenderer->PopAxisAlignedClip();
}

void Direct2D::DrawText(ID2D1RenderTarget*iRenderer,ID2D1Brush* iBrush,const char* iText,float x1,float y1, float x2,float y2,float iAlignPosX,float iAlignPosY,bool iClip)
{
	if(!iText)
		return;

	unsigned int tFinalTextLength=0;
	wchar_t *tWcharTextOutput=0;

	{
		unsigned int tTextLength=strlen(iText);
		unsigned int tCharSize=sizeof(const char);
		tFinalTextLength=tCharSize*tTextLength;
		tWcharTextOutput=new wchar_t[tFinalTextLength+1];
		int tCharsWrited=mbstowcs(tWcharTextOutput,iText,tFinalTextLength);

		if(tCharsWrited!=tFinalTextLength)
			DEBUG_BREAK();

		tWcharTextOutput[tFinalTextLength]='\0';//needed, see mbstowcs reference
	}

	Direct2D::DrawText(iRenderer,iBrush,tWcharTextOutput,x1,y1,x2,y2,iAlignPosX,iAlignPosY,iClip);

	SAFEDELETEARRAY(tWcharTextOutput);
}

void Direct2D::DrawRectangle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y, float w,float h,bool fill)
{
	fill ? renderer->FillRectangle(D2D1::RectF(x,y,w,h),brush) : renderer->DrawRectangle(D2D1::RectF(x,y,w,h),brush);
}

void Direct2D::DrawBitmap(ID2D1RenderTarget*renderer,ID2D1Bitmap* bitmap,float x,float y, float w,float h)
{
	renderer->DrawBitmap(bitmap,D2D1::RectF(x,y,w,h));
}

void Direct2D::PushScissor(ID2D1RenderTarget*renderer,float x,float y,float w,float h)
{
	renderer->PushAxisAlignedClip(D2D1::RectF(x,y,w,h),D2D1_ANTIALIAS_MODE_ALIASED);
}
void Direct2D::PopScissor(ID2D1RenderTarget*renderer)
{
	renderer->PopAxisAlignedClip();
}

void Direct2D::Translate(ID2D1RenderTarget* renderer,float x,float y)
{
	renderer->SetTransform(D2D1::Matrix3x2F::Translation(x,y));
}

void Direct2D::Identity(ID2D1RenderTarget* renderer)
{
	renderer->SetTransform(D2D1::Matrix3x2F::Identity());
}


///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////Renderer2DWin32///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


Renderer2DWin32::CaretWin32::CaretWin32(Renderer2D* iRendererWin32):
	Caret(iRendererWin32),
	background(0)
{}

Renderer2DWin32::CaretWin32::~CaretWin32()
{
	SAFERELEASE(this->background);
}

void Renderer2DWin32::CaretWin32::set(GuiRect* iGuiRect,vec2 iPosition,vec2 iRect)
{
	if(iPosition.x<0 || iPosition.y<0 || iRect.x<0 || iRect.y<0)
		DEBUG_BREAK();

	this->guiRect=iGuiRect;
	this->newPosition=iPosition;
	this->newRect=iRect;

	if(this->renderer2D->caretGlobal!=this)
		this->renderer2D->caretGlobal=this;
}

void Renderer2DWin32::CaretWin32::enable(bool iEnable)
{
	this->enabled=iEnable;

	if(this->renderer2D->caretGlobal!=this)
		this->renderer2D->caretGlobal=this;
}
	
void Renderer2DWin32::CaretWin32::draw()
{
	if(this->enabled && this->guiRect==this->renderer2D->tabContainer->GetFocus())
	{
		if(!this->background || this->position!=this->newPosition || this->rect!=this->newRect)
		{
			Renderer2DWin32* tRenderer2DWin32=(Renderer2DWin32*)this->renderer2D;

			this->lastBlinkTime=Timer::instance->GetTime()+this->blinkingRate;
			this->blinking=true;
		
			SAFERELEASE(this->background);

			D2D1_BITMAP_PROPERTIES tBitmapProperties=D2D1::BitmapProperties();

			tBitmapProperties.pixelFormat=tRenderer2DWin32->renderer->GetPixelFormat();

			HRESULT tResult=tRenderer2DWin32->renderer->CreateBitmap(D2D1::SizeU(this->newRect.x,this->newRect.y),tBitmapProperties,&this->background);

			if(S_OK!=tResult || !this->background)
				DEBUG_BREAK();

			tResult=this->background->CopyFromRenderTarget(&D2D1::Point2U(0,0),tRenderer2DWin32->renderer,&D2D1::RectU(this->newPosition.x,this->newPosition.y,this->newPosition.x+this->newRect.x,this->newPosition.y+this->newRect.y));

			if(S_OK!=tResult || !this->background)
				DEBUG_BREAK();

			this->position=this->newPosition;
			this->rect=this->newRect;
		}

		if(this->guiRect && this->background && Timer::instance->GetTime()-this->lastBlinkTime > this->blinkingRate)
		{
			Renderer2DWin32* renderer2DWin32=(Renderer2DWin32*)this->renderer2D;

			if(renderer2DWin32->tabContainer->BeginDraw())
			{
				bool tSelfClip=this->guiRect->BeginSelfClip(renderer2DWin32->tabContainer);

				if(this->blinking)
					renderer2DWin32->renderer->DrawLine(D2D1::Point2F(this->position.x+1,this->position.y),D2D1::Point2F(this->position.x+1,this->position.y+renderer2DWin32->GetFontHeight()),renderer2DWin32->SetColorWin32(0x00000000),2.0f);
				else
					renderer2DWin32->renderer->DrawBitmap(this->background,D2D1::Rect(this->position.x,this->position.y,this->position.x+this->rect.x,this->position.y+this->rect.y));

				this->guiRect->EndSelfClip(renderer2DWin32->tabContainer,tSelfClip);

				renderer2DWin32->tabContainer->EndDraw();
			}

			this->lastBlinkTime=Timer::instance->GetTime();
			this->blinking=!this->blinking;
		}
	}
}


Renderer2DWin32::Renderer2DWin32(Tab* iTabContainer,HWND handle):Renderer2D(iTabContainer),brush(0),renderer(0),caretWin32((CaretWin32*&)caret)
{
	Direct2D::Init();

	this->SetTabSpaces(this->tabSpaces);
	this->caret=new CaretWin32(this);
}
Renderer2DWin32::~Renderer2DWin32()
{
	SAFEDELETE(this->caret);
}

bool Renderer2DWin32::RecreateTarget(HWND iHandle)
{
	SAFERELEASE(this->renderer);
	SAFERELEASE(this->brush);
	//SAFEDELETE(this->caret);
	

	HRESULT result=S_OK;

	{
		RECT rc;
		GetClientRect(iHandle, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top);

		D2D1_RENDER_TARGET_PROPERTIES rRenderTargetProperties=D2D1::RenderTargetProperties();

		rRenderTargetProperties.type=D2D1_RENDER_TARGET_TYPE_HARDWARE;
		rRenderTargetProperties.usage=D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;//we wants resource sharing

		result=Direct2D::factory->CreateHwndRenderTarget(rRenderTargetProperties,D2D1::HwndRenderTargetProperties(iHandle,size,D2D1_PRESENT_OPTIONS_IMMEDIATELY),&renderer);
	}
	
	if(S_OK!=result || !this->renderer)
		DEBUG_BREAK();

	result=this->renderer->CreateSolidColorBrush(D2D1::ColorF(Renderer2D::COLOR_TAB_BACKGROUND),&brush);

	if(S_OK!=result || !this->brush)
		DEBUG_BREAK();

	//this->caret=new CaretWin32(this);
	
	return true;
}
void Renderer2DWin32::DrawText(const char* iText,float iX,float iY, float iW,float iH,unsigned int iColor,float iAlignPosX,float iAlignPosY,bool iClip)
{
	Direct2D::DrawText(this->renderer,this->SetColorWin32(iColor),iText,iX,iY,iW,iH,iAlignPosX,iAlignPosY,iClip);
}
void Renderer2DWin32::DrawText(const wchar_t* iText,float iX,float iY, float iW,float iH,unsigned int iColor,float iAlignPosX,float iAlignPosY,bool iClip)
{
	Direct2D::DrawText(this->renderer,this->SetColorWin32(iColor),iText,iX,iY,iW,iH,iAlignPosX,iAlignPosY,iClip);
}
void Renderer2DWin32::DrawRectangle(float iX,float iY, float iW,float iH,unsigned int iColor,bool iFill)
{
	Direct2D::DrawRectangle(this->renderer,this->SetColorWin32(iColor),iX,iY,iW,iH,iFill);
}
void Renderer2DWin32::DrawRectangle(vec4& iXYWH,unsigned int iColor,bool iFill)
{
	Direct2D::DrawRectangle(this->renderer,this->SetColorWin32(iColor),iXYWH.x,iXYWH.y,iXYWH.x+iXYWH.z,iXYWH.y+iXYWH.w,iFill);
}
void Renderer2DWin32::DrawBitmap(GuiImage* iBitmap,float iX,float iY, float iW,float iH)
{
	Direct2D::DrawBitmap(this->renderer,((GuiImageWin32*)iBitmap)->handle,iX,iY,iW,iH);
}

void Renderer2DWin32::PushScissor(float iX,float iY, float iW,float iH)
{
	Direct2D::PushScissor(this->renderer,iX,iY,iW,iH);
}
void Renderer2DWin32::PopScissor()
{
	Direct2D::PopScissor(this->renderer);
}

void Renderer2DWin32::Translate(float iX,float iY)
{
	Direct2D::Translate(this->renderer,iX,iY);
}
void Renderer2DWin32::Identity()
{
	Direct2D::Identity(this->renderer);
}

vec2 Renderer2DWin32::MeasureText(const char* iText,int iSlen)
{
	return Direct2D::MeasureText(this->renderer,iText,iSlen);
}

float Renderer2DWin32::GetFontSize()
{
	return Direct2D::GetFontSize();
}

float Renderer2DWin32::GetFontHeight()
{
	return Direct2D::GetFontHeight(this->renderer);
}

float Renderer2DWin32::GetCharWidth(char iCharacter)
{
	float tCharWidth=Direct2D::GetCharWidth(iCharacter);

	return iCharacter!='\t' ? tCharWidth : tCharWidth*this->tabSpaces;
}

ID2D1Brush* Renderer2DWin32::SetColorWin32(unsigned int color)
{
	this->brush->SetColor(D2D1::ColorF(color));
	return this->brush;
}

void Renderer2DWin32::SetTabSpaces(unsigned int iNumOfSpaces)
{
	this->tabSpaces=iNumOfSpaces;
	unsigned int tSpaceCharWidth=Direct2D::GetCharWidth(' ');
	Direct2D::texter->SetIncrementalTabStop(tSpaceCharWidth*this->tabSpaces);
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////ContainerWin32////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



ContainerWin32::ContainerWin32():
	windowDataWin32((WindowDataWin32*&)window),
	splitterContainerWin32((SplitterWin32*&)splitter)
	
{
	window=new WindowDataWin32;
	splitter=new SplitterWin32;
}



TabWin32* ContainerWin32::CreateTabContainer(float x,float y,float w,float h)
{
	TabWin32* result=new TabWin32(x,y,w,h,this->windowDataWin32->hwnd);
	this->tabContainers.push_back(result);
	return result;
}

void ContainerWin32::OnSizing()
{
	LPRECT sz=(LPRECT)this->windowDataWin32->lparam;
	
	RECT trc;
	GetWindowRect(this->windowDataWin32->hwnd,&trc);

	this->resizeDiffWidth=(sz->right-sz->left)-(trc.right-trc.left);
	this->resizeDiffHeight=(sz->bottom-sz->top)-(trc.bottom-trc.top);

	switch(this->windowDataWin32->wparam)
	{
	case WMSZ_LEFT:this->resizeEnumType=0;break;
	case WMSZ_TOP:this->resizeEnumType=1;break;
	case WMSZ_RIGHT:this->resizeEnumType=2;break;
	case WMSZ_BOTTOM:this->resizeEnumType=3;break;
	default:
		DEBUG_BREAK();
	}
}

void ContainerWin32::OnSize()
{
	this->OnSize();
	//SplitterContainer::OnSize(this->hwnd,this->wparam,this->lparam);

	if(this->resizeEnumType<0)
		return;

	this->resizeCheckWidth=0;
	this->resizeCheckHeight=0;

	for(std::vector<Tab*>::iterator i=this->tabContainers.begin();i!=this->tabContainers.end();i++)
	{
		TabWin32* tab=static_cast<TabWin32*>(*(i));

		if(tab && tab->windowData->siblings[this->resizeEnumType].empty())
			tab->OnResizeContainer();
	}
}


///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////MainContainerWin32////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void MainContainerWin32::Init()
{
	ContainerWin32* container=new ContainerWin32;

	this->containers.push_back(container);

	menuMain=CreateMenu();
	menuEntities=CreateMenu();

	InsertMenu(menuMain,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)menuEntities,"Entities");
	{
		InsertMenu(menuEntities,0,MF_BYPOSITION|MF_STRING,MAINMENU_ENTITIES_IMPORTENTITY,"Import...");
	}

	container->windowDataWin32->hwnd=CreateWindow(WC_MAINAPPWINDOW,WC_MAINAPPWINDOW,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,0,menuMain,0,container);

	RECT rc;
	GetClientRect(container->windowDataWin32->hwnd,&rc);

	TabWin32* tabContainer1=container->CreateTabContainer(0.0f,0.0f,(float)300,(float)200);
	TabWin32* tabContainer2=container->CreateTabContainer(0.0f,204.0f,(float)300,(float)200);
	TabWin32* tabContainer3=container->CreateTabContainer(0.0f,408.0f,(float)300,(float)rc.bottom-(rc.top+408));
	TabWin32* tabContainer4=container->CreateTabContainer(304.0f,0.0f,(float)rc.right-(rc.left+304),(float)rc.bottom-rc.top);

	GuiSceneViewer* scene=tabContainer1->tabs.SceneViewer();
	GuiViewport* viewport=tabContainer4->tabs.Viewport();

	viewport->rootEntity=scene->entityRoot;

	tabContainer3->tabs.ProjectViewer();
	tabContainer2->tabs.EntityViewer();

	Tab::BroadcastToPoolSelecteds(&GuiRect::OnSize);
	Tab::BroadcastToPoolSelecteds(&GuiRect::OnActivate);

	ShowWindow(container->windowDataWin32->hwnd,true);
}



ContainerWin32* MainContainerWin32::CreateContainer()
{
	ContainerWin32* rootContainer=(ContainerWin32*)this->containers[0];
	ContainerWin32* container=new ContainerWin32;

	container->windowDataWin32->hwnd=CreateWindow(WC_MAINAPPWINDOW,WC_MAINAPPWINDOW,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,rootContainer->windowDataWin32->hwnd,0,0,container);

	this->containers.push_back(container);

	return container;
}


LRESULT CALLBACK MainContainerWin32::MainWindowProcedure(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	ContainerWin32* mainw=(ContainerWin32*)GetWindowLongPtr(hwnd,GWLP_USERDATA);

	LRESULT result=0;

	if(mainw)
		mainw->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);

	switch(msg)
	{
		/*case WM_PAINT:
			//result=DefWindowProc(hwnd,msg,wparam,lparam);
			printf("main painting\n");
		break;*/
		case WM_CREATE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)((LPCREATESTRUCT)lparam)->lpCreateParams);
		}
		break;
		case WM_CLOSE:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			PostQuitMessage(1);
		break;
		case WM_SIZING:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			mainw->OnSizing();
		break;
		case WM_SIZE:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			mainw->OnSize();
		break;
		/*case WM_ERASEBKGND:
			return (LRESULT)1;*/
		case WM_LBUTTONDOWN:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			mainw->splitterContainerWin32->OnLButtonDown(hwnd,lparam);
		break;
		case WM_LBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			mainw->splitterContainerWin32->OnLButtonUp(hwnd);
		break;
		case WM_MOUSEMOVE:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			mainw->splitterContainerWin32->OnMouseMove(hwnd,lparam);
		break;
		case WM_KEYDOWN:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			mainw->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			break;
		case WM_COMMAND:
			{
				if(!HIWORD(wparam))//menu notification
				{
					SetFocus(hwnd);

					switch(LOWORD(wparam))
					{
						case MAINMENU_ENTITIES_IMPORTENTITY:
						{
							char charpretval[5000]={0};

							OPENFILENAME openfilename={0};
							openfilename.lStructSize=sizeof(OPENFILENAME);
							openfilename.hwndOwner=hwnd;
							openfilename.lpstrFilter="Fbx File Format (*.fbx)\0*.fbx\0\0";
							openfilename.nFilterIndex=1;
							openfilename.lpstrFile=charpretval;
							openfilename.nMaxFile=5000;
							 
							if(GetOpenFileName(&openfilename) && openfilename.lpstrFile!=0)
							{
#if USEFBX
								EditorEntity* importedEntities=ImportFbxScene(openfilename.lpstrFile);
								Tab::BroadcastToPool(&Tab::OnEntitiesChange,importedEntities);
#endif							
							}
						}
						break;
					}
				}
			}
		break;
		default:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
	}

	return result;
}


LRESULT CALLBACK TabWin32::TabContainerWindowClassProcedure(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	TabWin32* tc=(TabWin32*)GetWindowLongPtr(hwnd,GWLP_USERDATA);

	LPARAM result=0;

	switch(msg)
	{
		case WM_CREATE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			LPCREATESTRUCT lpcs=(LPCREATESTRUCT)lparam;

			if(lpcs)
			{
				TabWin32* tabContainer=(TabWin32*)lpcs->lpCreateParams;
				SetWindowLongPtr(hwnd,GWLP_USERDATA,(LONG_PTR)tabContainer);
				tabContainer->windowDataWin32->hwnd=hwnd;
			}
		}
		break;
		case WM_ERASEBKGND:
			/*if(tc->splitterContainer->floatingTabRef)
				tc->OnPaint();*/
			return 1;
		case WM_SIZE:
			tc->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnGuiSize();
		break;
		case WM_DESTROY:
			printf("window %p of TabContainer %p destroyed\n",hwnd,tc);
		break;
		case WM_WINDOWPOSCHANGED:
			tc->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnWindowPosChanging();
		break;
		case WM_LBUTTONDOWN:
			{
				tc->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);

				InputManager::mouseInput.left=true;

				unsigned int tOldTime=InputManager::mouseInput.lastLeft;
				InputManager::mouseInput.lastLeft=Timer::instance->GetTime();

				if(tc->windowDataWin32->hwnd!=::GetFocus())
					::SetFocus(tc->windowDataWin32->hwnd);

				if(InputManager::mouseInput.lastLeft-tOldTime<1000/6.0f)
					tc->OnGuiDLMouseDown();
				else
					tc->OnGuiLMouseDown();
			}
		break;
		case WM_MOUSEWHEEL:
			{
				tc->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
				float wheelValue=GET_WHEEL_DELTA_WPARAM(wparam)>0 ? 1.0f : (GET_WHEEL_DELTA_WPARAM(wparam)<0 ? -1.0f : 0);
				tc->OnGuiMouseWheel(&wheelValue);
			}
		break;
		case WM_MOUSEMOVE:
			tc->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnGuiMouseMove();
		break;
		case WM_NCHITTEST:
			if(tc->splitterContainer->floatingTabRef)
				return HTTRANSPARENT;
			else 
				result=DefWindowProc(hwnd,msg,wparam,lparam);
			break;
		case WM_LBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			tc->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			InputManager::mouseInput.left=false;
			tc->OnGuiLMouseUp();
		break;
		case WM_RBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			tc->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			tc->OnGuiRMouseUp();
		break;
		case WM_KEYDOWN:
			{
				result=DefWindowProc(hwnd,msg,wparam,lparam);
				tc->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
				MSG _msg;
				PeekMessage(&_msg, NULL, 0, 0, PM_NOREMOVE);

				if(_msg.message==WM_CHAR)
				{
					TranslateMessage(&_msg);
					DefWindowProc(_msg.hwnd,_msg.message,_msg.wParam,_msg.lParam);
					tc->OnGuiKeyDown(&_msg.wParam);
				}
				else
					tc->OnGuiKeyDown();
			}
			break;
		case WM_PAINT:
		{
			tc->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			//tc->OnGuiPaint();
			EndPaint(hwnd,&ps);
			tc->SetDraw();
			result=0;
		}
		break;
		default:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			
	}

	if(tc)
	{
		tc->windowDataWin32->msg=0;
		tc->windowDataWin32->wparam;
		tc->windowDataWin32->lparam=0;
	}

	return result;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////TimerWin32///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

TimerWin32::TimerWin32(){}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////WindowDataWin32///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


//#pragma message(LOCATION " LNK1123: Failure during conversion to COFF: file invalid or corrupt\" was resolved renaming cvtres.exe to cvtres1.exe.")

void WindowDataWin32::CopyProcedureData(HWND  h,UINT m,WPARAM w,LPARAM l){hwnd=h,msg=m,wparam=w,lparam=l;}


void WindowDataWin32::OnSize()
{
	width=LOWORD(lparam);
	height=HIWORD(lparam);
}
void WindowDataWin32::OnWindowPosChanging()
{
	width=(float)((LPWINDOWPOS)lparam)->cx;
	height=(float)((LPWINDOWPOS)lparam)->cy;
}

void WindowDataWin32::LinkSibling(WindowData* t,int pos)
{
	if(!t)
		return;

	int reciprocal = pos<2 ? pos+2 : pos-2;

	this->siblings[pos].push_back(t);
	t->siblings[reciprocal].push_back(this);
}

void WindowDataWin32::UnlinkSibling(WindowData* t)
{
	if(t)
	{
		for(int i=0;i<4;i++)
			t->siblings[i].remove(this);
	}


	for(int i=0;i<4;i++)
	{
		if(t)
		{
			this->siblings[i].remove(t);
		}
		else
		{
			for(std::list<WindowData*>::iterator it=this->siblings[i].begin();it!=this->siblings[i].end();it++)
			{
				WindowData* tabToUnlinkFromThis=(*it);

				for(int i=0;i<4;i++)
					tabToUnlinkFromThis->siblings[i].remove(this);
			}

			this->siblings[i].clear();
		}
	}
}

WindowDataWin32* WindowDataWin32::FindSiblingOfSameSize()
{
	for(int i=0;i<4;i++)
	{
		for(std::list<WindowData*>::iterator it=this->siblings[i].begin();it!=this->siblings[i].end();it++)
		{
			WindowDataWin32* window=static_cast<WindowDataWin32*>(*it);

			if(window->width==this->width || window->height==this->height)
				return window;
		}
	}

	return 0;
}

int WindowDataWin32::FindSiblingPosition(WindowData* t)
{
	for(int i=0;i<4;i++)
	{
		if(std::find(this->siblings[i].begin(),this->siblings[i].end(),t)!=this->siblings[i].end())
			return i;
	}

	return -1;
}

bool WindowDataWin32::FindAndGrowSibling()
{
	WindowDataWin32* growingTab=this->FindSiblingOfSameSize();

	if(growingTab)
	{
		int tabReflinkPosition=this->FindSiblingPosition(growingTab);

		RECT growingTabRc,thisRc;
		GetClientRect(growingTab->hwnd,&growingTabRc);
		GetClientRect(this->hwnd,&thisRc);
		MapWindowRect(growingTab->hwnd,GetParent(growingTab->hwnd),&growingTabRc);
		MapWindowRect(this->hwnd,GetParent(growingTab->hwnd),&thisRc);

		switch(tabReflinkPosition)
		{
		case 0:
			SetWindowPos(growingTab->hwnd,0,growingTabRc.left,thisRc.top,thisRc.right-growingTabRc.left,thisRc.bottom-thisRc.top,SWP_SHOWWINDOW);
			break;
		case 1:
			SetWindowPos(growingTab->hwnd,0,thisRc.left,growingTabRc.top,thisRc.right-thisRc.left,thisRc.bottom-growingTabRc.top,SWP_SHOWWINDOW);
			break;
		case 2:
			SetWindowPos(growingTab->hwnd,0,thisRc.left,thisRc.top,growingTabRc.right-thisRc.left,thisRc.bottom-thisRc.top,SWP_SHOWWINDOW);
			break;
		case 3:
			SetWindowPos(growingTab->hwnd,0,thisRc.left,thisRc.top,thisRc.right-thisRc.left,growingTabRc.bottom-thisRc.top,SWP_SHOWWINDOW);
			break;
		default:
			DEBUG_BREAK();
			break;
		}

		this->UnlinkSibling();

		return true;
	}

	return false;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////AppWin32////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

EngineIDEWin32::EngineIDEWin32():
processThreadHandle(0),
processHandle(0)
{
	this->timerMain=new TimerWin32;
	this->mainAppWindow=new MainContainerWin32;
}

int EngineIDEWin32::Initialize()
{
	HRESULT result;

	{
		this->processHandle=::GetCurrentProcess();
		this->processId=(unsigned int)::GetProcessId(this->processHandle);
		this->processThreadHandle=::GetCurrentThread();
		this->processThreadId=(unsigned int)::GetCurrentThreadId();

		printf("Engine: process id is %d\n",this->processId);
		printf("Engine: process main thread id is %d\n",this->processThreadId);
	}

	result=CoInitialize(0);

	if(S_OK!=result)
		DEBUG_BREAK();
	
	{//projectFolder
		char _pszDisplayName[MAX_PATH]="";

		BROWSEINFO bi={0};
		bi.pszDisplayName=_pszDisplayName;
		bi.lpszTitle="Select Project Directory";

		PIDLIST_ABSOLUTE tmpProjectFolder=SHBrowseForFolder(&bi);

		DWORD err=GetLastError();

		if(tmpProjectFolder)
		{
			char path[MAX_PATH];

			if(SHGetPathFromIDList(tmpProjectFolder,path))
			{
				this->projectFolder=path;
				printf("User project folder: %s\n",this->projectFolder.Buffer());
			}
		}
	}

	{//exeFolder
		char ch[5000];
		if(!GetModuleFileName(0,ch,5000))
			DEBUG_BREAK();

		this->ideExecutable=ch;

		if(!SetDllDirectory(this->ideExecutable.Path().Buffer()))
			printf("failed to add dll search path\n");

		

		printf("Application folder: %s\n",this->ideExecutable.Path().Buffer());
	}

	this->subsystem=new SubsystemWin32;
	this->compiler=new CompilerWin32;

	this->compiler->ideSourcesPath=this->ideExecutable.PathUp(5) + "\\src";
	this->compiler->ideLibraryPath=this->ideExecutable.Path();

	this->debugger=new DebuggerWin32;

	{//applicationDataFolder
		char ch[5000];
		if(S_OK!=SHGetFolderPath(0,CSIDL_APPDATA,0,SHGFP_TYPE_CURRENT,ch))
			DEBUG_BREAK();

		this->applicationDataFolder=String(ch) + "\\EngineAppData";

		if(!PathFileExists(this->applicationDataFolder.Buffer()))
		{
			SECURITY_ATTRIBUTES sa;
			CreateDirectory(this->applicationDataFolder.Buffer(),&sa);
		}
		else
		{
			this->subsystem->Execute(this->applicationDataFolder.Buffer(),"del /F /Q *.*");
			this->subsystem->Execute(this->applicationDataFolder.Buffer(),"FOR /D %p IN (*.*) DO rmdir \"%p\" /s /q");
		}

		printf("Application data folder: %s\n",this->applicationDataFolder.Buffer());
	}

	Direct2D::Init();

	int error=ERROR_SUCCESS;

	InitSplitter();

	MainContainerWin32* mainAppWindowWin32=new MainContainerWin32;

	this->mainAppWindow=mainAppWindowWin32;

	if(!this->mainAppWindow)
		printf("failed to create main window!\n");
		
	mainAppWindowWin32->Init();

	//threadUpdate=new ThreadWin32;

	return error;
}

void EngineIDEWin32::Deinitialize()
{
	SetDllDirectory(0);
	Direct2D::Release();
	CoUninitialize();
}

void EngineIDEWin32::CreateNodes(String dir,ResourceNodeDir* iParent)
{
	HANDLE handle;
	WIN32_FIND_DATA data;

	String scanDir=dir;
	scanDir+="\\*";

	handle=FindFirstFile(scanDir.Buffer(),&data); //. dir

	//printf("GuiProjectViewer::CreateNodes(%s)\n",scanDir);

	if(!handle || INVALID_HANDLE_VALUE == handle)
	{
		DEBUG_BREAK();
		return;
	}
	else
		FindNextFile(handle,&data);

	int tEngineExtensionCharSize=strlen(EngineIDE::instance->GetEntityExtension());

	while(FindNextFile(handle,&data))
	{
		if(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			continue;

		if(strstr(data.cFileName,EngineIDE::instance->GetEntityExtension()))
		{
			String tEngineFile=dir + "\\" + String(data.cFileName);

			bool selectedLeft;
			bool selectedRight;
			bool isDir;
			bool expanded;

			{
				FILE* f=fopen(tEngineFile.Buffer(),"r");

				if(f)
				{
					fread(&selectedLeft,1,1,f);
					fread(&selectedRight,1,1,f);
					fread(&isDir,1,1,f);
					fread(&expanded,1,1,f);

					fclose(f);
				}
			}

			std::string tOriginalFileName(data.cFileName,&data.cFileName[strlen(data.cFileName)-tEngineExtensionCharSize]);

			if(isDir)
			{
				ResourceNodeDir* dirNode=new ResourceNodeDir;

				iParent->dirs.push_back(dirNode);
				dirNode->parent=iParent;
				dirNode->fileName=tOriginalFileName.c_str();
				dirNode->level = iParent ? iParent->level+1 : 0;
				dirNode->selectedLeft=selectedLeft;
				dirNode->selectedRight=selectedRight;
				dirNode->expanded=expanded;
				dirNode->isDir=isDir;

				this->CreateNodes(dir + "\\" + dirNode->fileName,dirNode);
			}
			else
			{
				ResourceNode* fileNode=new ResourceNode;

				iParent->files.push_back(fileNode);
				fileNode->parent=iParent;
				fileNode->fileName=tOriginalFileName.c_str();
				fileNode->level = iParent ? iParent->level+1 : 0;
				fileNode->selectedLeft=selectedLeft;
				fileNode->selectedRight=selectedRight;
				fileNode->isDir=isDir;
			}
		}
	}

	FindClose(handle);
	handle=0;
}

void EngineIDEWin32::ScanDir(String dir)
{
	HANDLE handle;
	WIN32_FIND_DATA data;

	String scanDir=dir;
	scanDir+="\\*";

	handle=FindFirstFile(scanDir.Buffer(),&data); //. dir

	//printf("GuiProjectViewer::ScanDir(%s)\n",scanDir);

	if(!handle || INVALID_HANDLE_VALUE == handle)
	{
		DEBUG_BREAK();
		return;
	}
	else
		FindNextFile(handle,&data);

	while(FindNextFile(handle,&data))
	{
		if(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			continue;

		if(strstr(data.cFileName,EngineIDE::instance->GetEntityExtension()))
		{
			String s = dir + "\\" + String(data.cFileName);

			if(!PathFileExists(s.Buffer()))
			{
				//delete .engine file
				continue;
			}
		}
		else
		{
			String engineFile = dir + "\\" + String(data.cFileName) + EngineIDE::instance->GetEntityExtension();

			if(!PathFileExists(engineFile.Buffer()))
			{
				FILE* f=fopen(engineFile.Buffer(),"w");

				if(f)
				{
					unsigned char _true=1;
					unsigned char _false=0;

					fwrite(&_false,1,1,f);//selectedLeft
					fwrite(&_false,1,1,f);//selectedRight
					fwrite((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)? &_true : &_false,1,1,f);//isDir
					fwrite(&_false,1,1,f);//expanded
					fclose(f);

					DWORD fileAttribute=GetFileAttributes(engineFile.Buffer());
					SetFileAttributes(engineFile.Buffer(),FILE_ATTRIBUTE_HIDDEN|fileAttribute);


					if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						this->ScanDir(dir +"\\"+data.cFileName);

				}
			}
		}
	}

	FindClose(handle);
	handle=0;
}


void EngineIDEWin32::Run()
{
//#pragma message (LOCATION " PeekMessage has 0 as hwnd to let other windows work")

	MSG msg;

	while(true)
	{
		if(GetMessage(&msg,0,0,0))
		{
			if(msg.message==WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	this->Deinitialize();
}

void EngineIDEWin32::Sleep(int iMilliseconds)
{
	::Sleep(iMilliseconds);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////TimerWin32//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

unsigned int TimerWin32::GetTime()
{
	return (unsigned int)timeGetTime();
}

void TimerWin32::update()
{
	lastFrameTime=currentFrameTime;
	currentFrameTime=GetTime();
	currentFrameDeltaTime=currentFrameTime-lastFrameTime;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////DirectXRenderer/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////




DirectXRenderer::DirectXRenderer(Tab* iTabContainer):
	Renderer3D(iTabContainer)
{
	driverType = D3D_DRIVER_TYPE_NULL;
	featureLevel = D3D_FEATURE_LEVEL_11_0;
	pd3dDevice = nullptr;
	pImmediateContext = nullptr;
	pSwapChain = nullptr;
	pRenderTargetView = nullptr;
}

DirectXRenderer::~DirectXRenderer()
{
	if( pImmediateContext ) pImmediateContext->ClearState();

	if( pRenderTargetView ) pRenderTargetView->Release();
	if( pSwapChain ) pSwapChain->Release();
	if( pImmediateContext ) pImmediateContext->Release();
	if(pd3dDevice ) pd3dDevice->Release();
}

char* DirectXRenderer::Name()
{
	return 0;
}

void DirectXRenderer::Create(HWND container)
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect( container, &rc );
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	DXGI_SWAP_CHAIN_DESC scd = {0};
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;    // how the swap chain should be used
	scd.BufferCount = 2;                                  // a front buffer and a back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 
	scd.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;    // the recommended flip mode
	scd.SampleDesc.Count = 1;
	scd.OutputWindow = container;
	scd.Windowed=true;


	//hr = D3D11CreateDevice( nullptr, D3D_DRIVER_TYPE_HARDWARE ,nullptr, 0, nullptr, 0,D3D11_SDK_VERSION, &pd3dDevice, &featureLevel, &pImmediateContext );
	hr = D3D11CreateDeviceAndSwapChain( nullptr, D3D_DRIVER_TYPE_HARDWARE ,nullptr, 0, nullptr, 0,D3D11_SDK_VERSION,&scd,&pSwapChain, &pd3dDevice, &featureLevel, &pImmediateContext );


	if(S_OK!=hr)
		return;
}


void DirectXRenderer::Render()
{
	pSwapChain->Present( 1, 0 );
}

///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////OpenGLUtils//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



//#pragma message (LOCATION " this should go to common opengl part of the project cause there is no os-related call within")




void glCheckError()
{
	GLenum err=glGetError();
	if(err!=GL_NO_ERROR)
	{
		HGLRC currentContext=wglGetCurrentContext();
		HDC currentContextDC=wglGetCurrentDC();

		printf("OPENGL ERROR %d, HGLRC: %p, HDC: %p\n",err,currentContext,currentContextDC);

		DEBUG_BREAK();
	}
}



int simple_shader(const char* name,int shader_type, const char* shader_src)
{
	GLint compile_success = 0;
	GLchar message[1024];
	int len=0;
	int shader_id;

	shader_id = glCreateShader(shader_type);glCheckError();

	if(!shader_id)
	{
		printf("glCreateShader error for %s,%s\n",shader_type,shader_src);glCheckError();
		DEBUG_BREAK();
		return 0;
	}

	glShaderSource(shader_id, 1, &shader_src,NULL);glCheckError();
	glCompileShader(shader_id);glCheckError();
	glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);glCheckError();

	if (GL_FALSE==compile_success)
	{
		sprintf(message,"glCompileShader[%s] error:\n",name);
		glGetShaderInfoLog(shader_id, sizeof(message), &len, &message[strlen(message)]);
		MessageBox(0,message,"Engine",MB_OK|MB_ICONEXCLAMATION);
		DEBUG_BREAK();
	}

	return shader_id;
}


int create_program(const char* name,const char* vertexsh,const char* fragmentsh)
{
	GLint link_success=0;
	GLint program=0;
	GLint vertex_shader=0;
	GLint fragment_shader=0;
	GLchar message[1024]={0};
	GLint len=0;

	program = glCreateProgram();glCheckError();
	
	if(!program)
	{
		printf("glCreateProgram error for %s,%s\n",vertexsh,fragmentsh);
		DEBUG_BREAK();
		return 0;
	}

	vertex_shader=simple_shader(name,GL_VERTEX_SHADER, vertexsh);
	fragment_shader=simple_shader(name,GL_FRAGMENT_SHADER, fragmentsh);

	glAttachShader(program, vertex_shader);glCheckError();
	glAttachShader(program, fragment_shader);glCheckError();
	glLinkProgram(program);glCheckError();
	glGetProgramiv(program, GL_LINK_STATUS, &link_success);glCheckError();

	if (GL_FALSE==link_success)
	{
		printf("glLinkProgram error for %s\n",message);
		DEBUG_BREAK();
	}

	glGetProgramInfoLog(program,sizeof(message),&len,message);glCheckError();


	//print infos
	/*if(len && len<sizeof(message))
		printf("%s",message);*/
	

	return program;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////ShaderOpenGL//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


Shader* ShaderOpenGL::Create(const char* name,const char* pix,const char* frag)
{
	int program=create_program(name,pix,frag);

	if(program)
	{
		Shader* shader=new ShaderOpenGL;

		shader->SetName(name);
		shader->SetProgram(program);
		shader->Use();
		shader->init();
		
		return shader;
	}
	else

		printf("error creating shader %s\n",name);

	return 0;
}



unsigned int& ShaderOpenGL::GetBufferObject()
{
	return vbo;
}

int ShaderOpenGL::GetProgram(){return program;}
void ShaderOpenGL::SetProgram(int p){program=p;}

int ShaderOpenGL::GetUniform(int slot,char* var)
{
	return glGetUniformLocation(slot,var);glCheckError();
}
int ShaderOpenGL::GetAttrib(int slot,char* var)
{
	return glGetAttribLocation(slot,var);glCheckError();
}

void ShaderOpenGL::SetProjectionMatrix(float* pm)
{
	this->SetMatrix4f(this->GetProjectionSlot(),pm);
}
void ShaderOpenGL::SetModelviewMatrix(float* mm)
{
	this->SetMatrix4f(this->GetModelviewSlot(),mm);
}

void ShaderOpenGL::SetMatrices(float* view,float* mdl)
{
	if(view)
		this->SetProjectionMatrix(view);

	if(mdl)
		this->SetModelviewMatrix(mdl);
}


void ShaderOpenGL::Use()
{
	glUseProgram(program);glCheckError();
}

const char* ShaderOpenGL::GetPixelShader(){return 0;}
const char* ShaderOpenGL::GetFragmentShader(){return 0;}

int ShaderOpenGL::init()
{
	mat4 m;

	int proj=GetProjectionSlot();
	int mdlv=GetModelviewSlot();

	bool bOk = this->SetMatrix4f(proj,m) && this->SetMatrix4f(mdlv,m);

	return bOk;
}

int ShaderOpenGL::GetAttribute(const char* attrib)
{
	int location=glGetAttribLocation(program,attrib);glCheckError();
	return location;
}

int ShaderOpenGL::GetUniform(const char* uniform)
{
	int location=glGetUniformLocation(program,uniform);glCheckError();
	return location;
}

int ShaderOpenGL::GetPositionSlot()
{
	return GetAttribute("position");
}
int ShaderOpenGL::GetColorSlot()
{
	return GetAttribute("color");
}
int ShaderOpenGL::GetProjectionSlot()
{
	return GetUniform("projection");
}
int ShaderOpenGL::GetModelviewSlot()
{
	return GetUniform("modelview");
}
int ShaderOpenGL::GetTexcoordSlot()
{
	return GetAttribute("texcoord");
}
int ShaderOpenGL::GetTextureSlot()
{
	return GetUniform("texture");
}
int ShaderOpenGL::GetMouseSlot()
{
	return GetUniform("mpos");
}
int ShaderOpenGL::GetLightposSlot()
{
	return GetUniform("lightpos");
}
int ShaderOpenGL::GetLightdiffSlot()
{
	return GetUniform("lightdiff");
}
int ShaderOpenGL::GetLightambSlot()
{
	return GetUniform("lightamb");
}
int ShaderOpenGL::GetNormalSlot()
{
	return GetAttribute("normal");
}
int ShaderOpenGL::GetHoveringSlot()
{
	return GetAttribute("hovered");
}

void ShaderOpenGL::SetSelectionColor(bool pick,void* ptr,vec2 mposNorm)
{
	int _mousepos=this->GetMouseSlot();
	int _ptrclr=this->GetUniform("ptrclr");

	if(_ptrclr>=0)
	{
		if(pick)
		{
			unsigned char* clr1=(unsigned char*)&ptr;

			float fcx=clr1[3]/255.0f;
			float fcy=clr1[2]/255.0f;
			float fcz=clr1[1]/255.0f;
			float fcw=clr1[0]/255.0f;

			glUniform4f(_ptrclr,fcx,fcy,fcz,fcw);
		}
		else glUniform4f(_ptrclr,0,0,0,0);

		if(_mousepos>=0)
			glUniform2f(_mousepos,mposNorm.x,mposNorm.y);
	}
}

bool ShaderOpenGL::SetMatrix4f(int slot,float* mtx)
{
	if(slot<0)
		return false;

	glUniformMatrix4fv(slot,1,0,mtx);glCheckError();
	return true;
}

void ShaderOpenGL::SetName(const char* n)
{
	name=n;
}
const char* ShaderOpenGL::GetName()
{
	return name.Buffer();
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

//#pragma message (LOCATION " processNode should go to common part (maybe entities.h) of the project cause there is no os-related call within")
//#pragma message (LOCATION " multiple opengl context needs glew32mx.lib")
//#pragma message (LOCATION " TODO: move OpenGL bitmap render in the RendererViewportInterface cause is for the gui only")




///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////////Shaders///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

#include "shaders.cpp"

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////Renderer3DOpenGL//////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

/*
GLuint OpenGLRenderer::vertexArrayObject=0;
GLuint OpenGLRenderer::vertexBufferObject=0;
GLuint OpenGLRenderer::textureBufferObject=0;
GLuint OpenGLRenderer::indicesBufferObject=0;
GLuint OpenGLRenderer::frameBuffer=0;
GLuint OpenGLRenderer::textureColorbuffer=0;
GLuint OpenGLRenderer::textureRenderbuffer=0;
GLuint OpenGLRenderer::pixelBuffer=0;
GLuint OpenGLRenderer::renderBufferColor=0;
GLuint OpenGLRenderer::renderBufferDepth=0;*/



bool GLEW_INITED=false;

#define USE_COMMON_PIXELFORMAT_SYSTEM 1


vec3 rayStart;
vec3 rayEnd;


Renderer3DOpenGL::Renderer3DOpenGL(TabWin32* iTabContainer):
	Renderer3D(iTabContainer),
	hglrc(0),
	tabContainerWin32((TabWin32*&)iTabContainer)
{
}

Renderer3DOpenGL::~Renderer3DOpenGL()
{
}

char* Renderer3DOpenGL::Name()
{
	return 0;
}

void Renderer3DOpenGL::Create(HWND hwnd)
{
	hdc=GetDC(hwnd);

	RECT r;
	GetClientRect(hwnd,&r);

	int width=(int)(r.right-r.left);
	int height=(int)(r.bottom-r.top);

	DWORD error=0;


	int pixelFormat;

	for(int i=0;i<1;i++)
	{
		if(!hdc)
			MessageBox(0,"Getting Device Context","GetDC",MB_OK|MB_ICONEXCLAMATION);

		PIXELFORMATDESCRIPTOR pfd={0};
		pfd.nVersion=1;
		pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
		pfd.dwFlags=PFD_DOUBLEBUFFER|PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL;
		pfd.iPixelType=PFD_TYPE_RGBA;
		pfd.cColorBits=32;
		pfd.cDepthBits=32;
		pfd.cStencilBits=32;

		pixelFormat = ChoosePixelFormat(hdc,&pfd);

		error=GetLastError();

		if(error!=NO_ERROR && error!=ERROR_OLD_WIN_VERSION)
			DEBUG_BREAK();

		if(pixelFormat==0)
			DEBUG_BREAK();

		if(!SetPixelFormat(hdc,pixelFormat,&pfd))
			DEBUG_BREAK();

		if(!(hglrc = wglCreateContext(hdc)))
			DEBUG_BREAK();

		if(!wglMakeCurrent(hdc,hglrc))
			DEBUG_BREAK();

		if(!wglChoosePixelFormatARB)wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATEXTPROC) wglGetProcAddress("wglChoosePixelFormatARB");
		if(!wglCreateContextAttribsARB)wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");

		#ifndef GL_GLEXT_PROTOTYPES

		if(!glAttachShader)glAttachShader = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
		if(!glBindBuffer)glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
		if(!glBindVertexArray) glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) wglGetProcAddress("glBindVertexArray");
		if(!glBufferData) glBufferData = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
		if(!glCompileShader) glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
		if(!glCreateProgram) glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
		if(!glCreateShader) glCreateShader = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
		if(!glDeleteBuffers) glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) wglGetProcAddress("glDeleteBuffers");
		if(!glDeleteProgram) glDeleteProgram = (PFNGLDELETEPROGRAMPROC) wglGetProcAddress("glDeleteProgram");
		if(!glDeleteShader) glDeleteShader = (PFNGLDELETESHADERPROC) wglGetProcAddress("glDeleteShader");
		if(!glDeleteVertexArrays) glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) wglGetProcAddress("glDeleteVertexArrays");
		if(!glDetachShader) glDetachShader = (PFNGLDETACHSHADERPROC) wglGetProcAddress("glDetachShader");
		if(!glEnableVertexAttribArray) glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glEnableVertexAttribArray");
		if(!glEnableVertexArrayAttrib) glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC) wglGetProcAddress("glEnableVertexArrayAttrib");
		if(!glGenBuffers) glGenBuffers = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
		if(!glGenVertexArrays) glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) wglGetProcAddress("glGenVertexArrays");
		if(!glGetAttribLocation) glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) wglGetProcAddress("glGetAttribLocation");
		if(!glGetProgramInfoLog) glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
		if(!glGetProgramiv) glGetProgramiv = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");
		if(!glGetShaderInfoLog) glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
		if(!glGetShaderiv) glGetShaderiv = (PFNGLGETSHADERIVPROC) wglGetProcAddress("glGetShaderiv");
		if(!glLinkProgram) glLinkProgram = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
		if(!glShaderSource) glShaderSource = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
		if(!glUseProgram) glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
		if(!glVertexAttribPointer) glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) wglGetProcAddress("glVertexAttribPointer");
		if(!glBindAttribLocation) glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) wglGetProcAddress("glBindAttribLocation");
		if(!glGetUniformLocation) glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation");
		if(!glUniformMatrix4fv) glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) wglGetProcAddress("glUniformMatrix4fv");
		if(!glActiveTexture) glActiveTexture = (PFNGLACTIVETEXTUREPROC) wglGetProcAddress("glActiveTexture");
		if(!glUniform1i) glUniform1i = (PFNGLUNIFORM1IPROC) wglGetProcAddress("glUniform1i");
		if(!glUniform1f) glUniform1f = (PFNGLUNIFORM1FPROC) wglGetProcAddress("glUniform1f");
		if(!glUniform3f) glUniform3f = (PFNGLUNIFORM3FPROC) wglGetProcAddress("glUniform3f");
		if(!glGenerateMipmap) glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmap");
		if(!glDisableVertexAttribArray) glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glDisableVertexAttribArray");
		if(!glUniform3fv) glUniform3fv = (PFNGLUNIFORM3FVPROC) wglGetProcAddress("glUniform3fv");
		if(!glUniform4fv) glUniform4fv = (PFNGLUNIFORM4FVPROC) wglGetProcAddress("glUniform4fv");
		if(!glTexBuffer) glTexBuffer = (PFNGLTEXBUFFERPROC) wglGetProcAddress("glTexBuffer");
		if(!glTextureBuffer) glTextureBuffer = (PFNGLTEXTUREBUFFERPROC) wglGetProcAddress("glTextureBuffer");
		if(!glBufferSubData) glBufferSubData = (PFNGLBUFFERSUBDATAPROC) wglGetProcAddress("glBufferSubData");
		if(!glGenFramebuffers)glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
		if(!glGenRenderbuffers)glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffers");
		/*if(!glBufferSubData)glViewportIndexedf = (PFNGLVIEWPORTINDEXEDFPROC) wglGetProcAddress("glViewportIndexedf");
		if(!glAddSwapHintRectWIN)glAddSwapHintRectWIN = (PFNGLADDSWAPHINTRECTWINPROC) wglGetProcAddress("glAddSwapHintRectWIN");
		if(!glBindFramebuffer)glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
		if(!glNamedRenderbufferStorage)glNamedRenderbufferStorage = (PFNGLNAMEDRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glNamedRenderbufferStorage");
		if(!glRenderbufferStorage)glRenderbufferStorage = (PFNGLRENDERBUFFERSTORAGEPROC) wglGetProcAddress("glRenderbufferStorage");
		if(!glBindRenderbuffer)glBindRenderbuffer = (PFNGLBINDRENDERBUFFERPROC) wglGetProcAddress("glBindRenderbuffer");
		if(!glFramebufferRenderbuffer)glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC) wglGetProcAddress("glFramebufferRenderbuffer");
		if(!glBlitFramebuffer)glBlitFramebuffer = (PFNGLBLITFRAMEBUFFERPROC) wglGetProcAddress("glBlitFramebuffer");
		if(!glCheckFramebufferStatus)glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC) wglGetProcAddress("glCheckFramebufferStatus");
		if(!glDrawBuffers)glDrawBuffers = (PFNGLDRAWBUFFERSPROC) wglGetProcAddress("glDrawBuffers");
		if(!glBlitNamedFramebuffer) glBlitNamedFramebuffer = (PFNGLBLITNAMEDFRAMEBUFFERPROC) wglGetProcAddress("glBlitNamedFramebuffer");
		if(!glFramebufferTexture)glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)wglGetProcAddress("glFramebufferTexture");
		if(!glFramebufferTexture2D)glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC) wglGetProcAddress("glFramebufferTexture2D");*/
		if(!glReadnPixels)glReadnPixels = (PFNGLREADNPIXELSPROC) wglGetProcAddress("glReadnPixels");
		if(!glUniform2f)glUniform2f = (PFNGLUNIFORM2FPROC) wglGetProcAddress("glUniform2f");
		if(!glUniform2fv)glUniform2fv = (PFNGLUNIFORM2FVPROC) wglGetProcAddress("glUniform2fv");
		if(!glUniform3f)glUniform3f = (PFNGLUNIFORM3FPROC) wglGetProcAddress("glUniform3f");
		if(!glUniform4f)glUniform4f = (PFNGLUNIFORM4FPROC) wglGetProcAddress("glUniform4f");
		if(!wglGetPixelFormatAttribivARB)wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC) wglGetProcAddress("wglGetPixelFormatAttribivARB");
#endif

		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(hglrc);
	}
	
	const int pixelFormatAttribList[] = 
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 32,
		WGL_STENCIL_BITS_ARB, 32,
		//WGL_SWAP_COPY_ARB,GL_TRUE,        //End
		0
	};

	UINT numFormats;

	if(!wglChoosePixelFormatARB(hdc, pixelFormatAttribList, NULL, 1, &pixelFormat, &numFormats))
		MessageBox(0,"wglChoosePixelFormatARB fails","Engine",MB_OK|MB_ICONEXCLAMATION);
	

	const int versionAttribList[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB,1,
		WGL_CONTEXT_MINOR_VERSION_ARB,0, 
		0,        //End
	};

	if(!(hglrc = wglCreateContextAttribsARB(hdc, 0, versionAttribList)))
		MessageBox(0,"wglCreateContextAttribsARB fails","Engine",MB_OK|MB_ICONEXCLAMATION);


	if(hglrc)
		printf("TABCONTAINER: %p, HGLRC: %p, HDC: %p\n",this->tabContainer,hglrc,hdc);
	
	if(!wglMakeCurrent(hdc,hglrc))
		DEBUG_BREAK();

	//if(!vertexArrayObject)
	{
		{
			glGenFramebuffers(1,&frameBuffer);glCheckError();
			
			glGenTextures(1,&textureColorbuffer);glCheckError();
			glGenTextures(1,&textureRenderbuffer);glCheckError();

			glGenRenderbuffers(1,&renderBufferColor);glCheckError();
			glGenRenderbuffers(1,&renderBufferDepth);glCheckError();
		}
		
		glGenVertexArrays(1, &vertexArrayObject);glCheckError();
		glBindVertexArray(vertexArrayObject);glCheckError();

		glGenBuffers(1,&vertexBufferObject);glCheckError();
		glGenBuffers(1,&textureBufferObject);glCheckError();
		/*
		//glGenBuffers(1,&indicesBufferObject);

		glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
		//glBindBuffer(GL_ARRAY_BUFFER,indicesBufferObject);

		glBufferData(GL_ARRAY_BUFFER,100000,0,GL_DYNAMIC_DRAW);*/


		glGenBuffers(1, &pixelBuffer);
	}

	printf("Status: Using GL %s\n", glGetString(GL_VERSION));
	printf("Status: GLSL ver %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));

	this->unlit=ShaderOpenGL::Create("unlit",unlit_vert,unlit_frag);
	this->unlit_color=ShaderOpenGL::Create("unlit_color",unlit_color_vert,unlit_color_frag);
	this->unlit_texture=ShaderOpenGL::Create("unlit_texture",unlit_texture_vs,unlit_texture_fs);
	this->font=ShaderOpenGL::Create("font",font_pixsh,font_frgsh);
	this->shaded_texture=ShaderOpenGL::Create("shaded_texture",texture_vertex_shaded_vert,texture_vertex_shaded_frag);

	this->shaders.push_back(this->unlit);
	this->shaders.push_back(this->unlit_color);
	this->shaders.push_back(this->unlit_texture);
	this->shaders.push_back(this->font);
	this->shaders.push_back(this->shaded_texture);
}


void Renderer3DOpenGL::ChangeContext()
{
	if(!hglrc || !hdc)
	{
		DEBUG_BREAK();
		return;
	}

	if(hglrc != wglGetCurrentContext() || hdc!=wglGetCurrentDC())
	{
		if(!wglMakeCurrent(hdc,hglrc))
			DEBUG_BREAK();
	}
}




void Renderer3DOpenGL::draw(Light*)
{

}

void Renderer3DOpenGL::draw(vec2)
{

}

void Renderer3DOpenGL::draw(Script*)
{

}

void Renderer3DOpenGL::draw(EntityComponent*)
{

}

void Renderer3DOpenGL::draw(Gizmo* gizmo)
{
	this->draw(vec3(0,0,0),vec3(10,0,0),vec3(1,0,0));
	this->draw(vec3(0,0,0),vec3(0,10,0),vec3(0,1,0));
	this->draw(vec3(0,0,0),vec3(0,0,10),vec3(0,0,1));
}



void Renderer3DOpenGL::draw(vec3 point,float psize,vec3 col)
{
	Shader* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	shader->SetSelectionColor(this->picking,0,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glPointSize(psize);

	int ps=shader->GetPositionSlot();
	int uniform_color=shader->GetUniform("color");

	if(uniform_color>=0)
	{glUniform3fv(uniform_color,1,col);glCheckError();}

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,3*sizeof(float),point,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(ps);glCheckError();
	glVertexAttribPointer(ps, 3, GL_FLOAT, GL_FALSE, 0,0);glCheckError();
		
	glDrawArrays(GL_POINTS,0,1);glCheckError();

	glDisableVertexAttribArray(ps);glCheckError();

	glPointSize(1.0f);
	glDisable(GL_PROGRAM_POINT_SIZE);
	glDisable(GL_DEPTH_TEST);
}

void Renderer3DOpenGL::draw(vec4 rect)
{
	Shader* shader=this->unlit_color;

	int position_slot=-1;
	int modelview_slot=-1;

	if(shader)
	{
		shader->Use();
		
		////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());
		int mdl=shader->GetModelviewSlot();
		int view=shader->GetProjectionSlot();
		int ptrclr=shader->GetUniform("ptrclr");

		if(ptrclr>=0)
			glUniform4f(ptrclr,0,0,0,0);

	}
	else
		return;

	if(position_slot<0)
		return;

	float dx=rect[2]-rect[0];
	float dy=rect[3]-rect[1];

	float data[]=
	{
		rect[0],	rect[1],		0,
		rect[0],	rect[1]+dy,		0,
		rect[0]+dx,	rect[1]+dy,		0,
		rect[0]+dx,	rect[1],		0,
		rect[0],	rect[1],		0
	};

	glVertexAttribPointer(position_slot, 3, GL_FLOAT, GL_FALSE, 0,data);
	glEnableVertexAttribArray(position_slot);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}


void Renderer3DOpenGL::draw(mat4 mtx,float size,vec3 color)
{
	Shader* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();
	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	int mdl=shader->GetModelviewSlot();
	int view=shader->GetProjectionSlot();
	int ptrclr=shader->GetUniform("ptrclr");

	if(ptrclr>=0)
		glUniform4f(ptrclr,0,0,0,0);

	/*MatrixStack::Push();
	MatrixStack::Multiply(mtx);*/

	vec3 axes[6];

	vec3 zero=mtx.transform(0,0,0);

	axes[0]=axes[2]=axes[4]=zero;

	mtx.axes(axes[1],axes[3],axes[5]);

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,18*sizeof(float),axes,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,6);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);


	glDisable(GL_DEPTH_TEST);

	

	//MatrixStack::Pop();
}

void Renderer3DOpenGL::draw(AABB aabb,vec3 color)
{
	Shader* shader=this->unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	shader->SetSelectionColor(false,0,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));

	vec3 &a=aabb.a;
	vec3 &b=aabb.b;

	float dx=b.x-a.x;
	float dy=b.y-a.y;
	float dz=b.z-a.z;

	if(dx<0.00000001f && dy<0.00000001f && dz<0.00000001f)
		return;

	float parallelepiped[72]=
	{
		//lower quad
		a.x,a.y,a.z,		a.x+dx,a.y,a.z,
		a.x+dx,a.y,a.z,		a.x+dx,a.y+dy,a.z, 
		a.x+dx,a.y+dy,a.z,	a.x,a.y+dy,a.z,
		a.x,a.y+dy,a.z,     a.x,a.y,a.z,

		//upper quad
		a.x,a.y,a.z+dz,			a.x+dx,a.y,a.z+dz,
		a.x+dx,a.y,a.z+dz,		a.x+dx,a.y+dy,a.z+dz, 
		a.x+dx,a.y+dy,a.z+dz,	a.x,a.y+dy,a.z+dz,
		a.x,a.y+dy,a.z+dz,		a.x,a.y,a.z+dz,

		//staffs
		a.x,a.y,a.z,		a.x,a.y,a.z+dz,
		a.x+dx,a.y,a.z,		a.x+dx,a.y,a.z+dz,
		a.x+dx,a.y+dy,a.z,	a.x+dx,a.y+dy,a.z+dz,
		a.x,a.y+dy,a.z,   	a.x,a.y+dy,a.z+dz
	};

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,72*sizeof(float),parallelepiped,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,24);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);

	glBindBuffer(GL_ARRAY_BUFFER,0);

	glDisable(GL_DEPTH_TEST);
}

void Renderer3DOpenGL::draw(vec3 a,vec3 b,vec3 color)
{
	Shader* shader=this->unlit_color;

	if(!shader)
		return;

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2], 
	};
	
	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelMatrix());

	shader->SetSelectionColor(false,0,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));
	
	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");
	
	glUniform3fv(col,1,color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,6*sizeof(float),line,GL_DYNAMIC_DRAW);
	
	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,2);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);


	glDisable(GL_DEPTH_TEST);

}

void Renderer3DOpenGL::draw(char* text,float x,float y,float width,float height,float sizex,float sizey,float* color4)
{
	Shader* shader=0;//line_color_shader

	if(!shader || !text)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	/*Font* font=FontManager::Instance()->Head()->Data();

	this->draw(font,text,x,y,width,height,sizex,sizey,color4);*/
}


/*
void OpenGLFixedRenderer::draw(Window* w)
{
	line_shader->Use();

	int position_slot=line_shader->GetPositionSlot();

	vec2& pos=w->window_pos;
	vec2& dim=w->window_dim;

	float data[]=
	{
		pos[0]-dim[0]/2,	pos[1]+dim[1]/2,		0,//low-left
		pos[0]+dim[0]/2,	pos[1]+dim[1]/2,		0,//low-right
		pos[0]+dim[0]/2,	pos[1]-dim[1]/2,		0,//up-right
		pos[0]-dim[0]/2,	pos[1]-dim[1]/2,		0,//up-left
		pos[0]-dim[0]/2,	pos[1]+dim[1]/2,		0,//low-left
	};

	glVertexAttribPointer(position_slot, 3, GL_FLOAT, GL_FALSE, 0,data);
	glEnableVertexAttribArray(position_slot);
	glDrawArrays(GL_LINE_STRIP, 0, 5);
}*/

/*
void OpenGLFixedRenderer::draw(WindowManager* windowManager)
{
	this->line_shader->Use();

	mat4 mat;
	mat.ortho(-0.5f,0.5f,-0.5f,0.5f,0,10);
	this->line_shader->SetMatrix4f(this->line_shader->GetProjectionSlot(),mat);


	glDisable(GL_CULL_FACE);

	float width=(float)KernelServer::Instance()->GetApp()->GetWidth();
	float height=(float)KernelServer::Instance()->GetApp()->GetHeight();

	for(Node<Window*> *node=windowManager->Head();node;node=node->Next())
		this->draw(node->Data());

	glEnable(GL_CULL_FACE);
}*/

/*
void OpenGLFixedRenderer::draw(EntityManager& entityManager)
{
	glDisable(GL_CULL_FACE);

	//entityManager.draw();

	glEnable(GL_CULL_FACE);
}
*/



/*
void OpenGLFixedRenderer::draw(Font* font,char* phrase,float x,float y,float width,float height,float sizex,float sizey,float* color4) 
{
	//https://en.wikibooks.org/wiki/OpenGL_Programming/Modern_OpenGL_Tutorial_Text_Rendering_01

	if(!font || !phrase)
		return;

	ShaderInterface* shader=font_shader;

	if(!shader)
		return;

	shader->Use();

	GLuint tid;

	glEnable(GL_BLEND);glCheckError();
	glBlendFunc(GL_CONSTANT_COLOR, GL_ONE_MINUS_SRC_COLOR);

	if(color4)
		glBlendColor(color4[0],color4[1],color4[2],color4[3]);
	else
		glBlendColor(1,1,1,1);

	glActiveTexture(GL_TEXTURE0);glCheckError();
	glGenTextures(1,(GLuint*)&tid);glCheckError();
	glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

	float sx=2.0f/width;
	float sy=2.0f/height;

	char* p=phrase;

	while(*p)
	{
		int err=font->loadchar(*p,sizex,sizey,width,height);

		if(!err)
		{
			unsigned char*	_buf=(unsigned char*)font->GetBuffer();
			int				_width=font->GetWidth();
			int				_rows=font->GetRows();
			float			_left=(float)font->GetLeft();
			float			_top=(float)font->GetTop();
			int				_advx=font->GetAdvanceX();
			int				_advy=font->GetAdvanceY();

			float			x2 = x + _left * sx;
			float			y2 = -y - _top * sy;
			float			w = _width * sx;
			float			h = _rows * sy;

			float box[] = {x2,-y2,0,1,x2 + w,-y2,0,1,x2,-y2-h,0,1,x2+w,-y2-h,0,1};
			float uv[] = {0,0,1,0,0,1,1,1};

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);glCheckError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();

			glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,_width,_rows,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,_buf);glCheckError();

			glUniform1i(shader->GetTextureSlot(), 0);glCheckError();

			glPixelStorei(GL_PACK_ALIGNMENT, 1);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			glEnableVertexAttribArray(shader->GetPositionSlot());glCheckError();
			glEnableVertexAttribArray(shader->GetTexcoordSlot());glCheckError();
			glVertexAttribPointer(shader->GetPositionSlot(), 4, GL_FLOAT, GL_FALSE,0,box);glCheckError();
			glVertexAttribPointer(shader->GetTexcoordSlot(), 2, GL_FLOAT, GL_FALSE,0,uv);glCheckError();
			glDrawArrays(GL_TRIANGLE_STRIP,0,4);glCheckError();
			glDisableVertexAttribArray(shader->GetPositionSlot());glCheckError();
			glDisableVertexAttribArray(shader->GetTexcoordSlot());glCheckError();

			x += (_advx >> 6) * sx;
			y += (_advy >> 6) * sy;
		}

		p++;
	}

	glDeleteTextures(1,(GLuint*)&tid);glCheckError();

	glDisable(GL_BLEND);glCheckError();
}*/


void Renderer3DOpenGL::draw(Texture* _t)
{
	return;
	Shader* shader=0;//unlit_texture

	Texture* texture=(Texture*)_t;
	//TextureFile* texture=(TextureFile*)_t;
	

	if(!shader || !texture->GetBuffer())
		return;

	shader->Use();

	const GLfloat rect_data[] = 
	{
		-1.0f, -1.0f,
		1.0f, -1.0f,
		-1.0f,  1.0f,
		1.0f,  1.0f
	};

	const GLfloat uv_data[] = 
	{
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f,  1.0f,
		0.0f,  0.0f,
	};

	int width=texture->GetWidth();
	int height=texture->GetHeight();
	void* buf=texture->GetBuffer();
	int  size=texture->GetSize();

	if(width<100)
		return;

	GLuint tid;
	glGenTextures(1,&tid);glCheckError();

	glActiveTexture(GL_TEXTURE0);glCheckError();
	glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glCheckError();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,width,height, 0, GL_RGBA, GL_UNSIGNED_BYTE,buf);glCheckError();

	mat4 mat;

	int projection=shader->GetProjectionSlot();
	int modelview=shader->GetModelviewSlot();
	int textureslot=shader->GetTextureSlot();
	int position=shader->GetPositionSlot();
	int texcoord=shader->GetTexcoordSlot();

	mat.ortho(-3,3,-3,3,0,1000);
	glUniformMatrix4fv(projection,1,0,mat);glCheckError();
	mat.identity();
	glUniformMatrix4fv(modelview,1,0,mat);glCheckError();

	glUniform1i(textureslot, 0);glCheckError();
	glEnableVertexAttribArray(position);glCheckError();
	glEnableVertexAttribArray(texcoord);glCheckError();
	glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE,0,rect_data);glCheckError();
	glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE,0,uv_data);glCheckError();
	glDrawArrays(GL_TRIANGLE_STRIP,0,4);glCheckError();

	glDisableVertexAttribArray(position);glCheckError();
	glDisableVertexAttribArray(texcoord);glCheckError();

	glDeleteTextures(1,&tid);glCheckError();
}

void Renderer3DOpenGL::draw(Mesh* mesh,std::vector<GLuint>& textureIndices,int texture_slot,int texcoord_slot)
{
	for(int i=0;i<(int)mesh->materials.size();i++)
	{
		for(int j=0;j<(int)mesh->materials[i]->textures.size() && !textureIndices.size();j++)
		{
			Texture* texture=mesh->materials[i]->textures[j];
			
			int		texture_width=texture->GetWidth();
			int		texture_height=texture->GetHeight();
			void	*texture_buffer=texture->GetBuffer();

			if(texture_buffer)
			{
				GLuint tid;
				glGenTextures(1,&tid);glCheckError();
				textureIndices.push_back(tid);

				glActiveTexture(GL_TEXTURE0);glCheckError();
				glBindTexture(GL_TEXTURE_2D,tid);glCheckError();

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);glCheckError();
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); glCheckError();

				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,texture_width,texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE,texture_buffer);glCheckError();

				glBindBuffer(GL_ARRAY_BUFFER,textureBufferObject);
				glBufferData(GL_ARRAY_BUFFER,mesh->ntexcoord*2*sizeof(float),mesh->texcoord,GL_DYNAMIC_DRAW);

				glUniform1i(texture_slot, 0);glCheckError();
				glEnableVertexAttribArray(texcoord_slot);glCheckError();
				glVertexAttribPointer(texcoord_slot,2,GL_FLOAT,GL_FALSE,0,0);glCheckError();

				glBindBuffer(GL_ARRAY_BUFFER,0);
			}
		}
	}
}

void Renderer3DOpenGL::draw(Mesh* mesh)
{
	drawUnlitTextured(mesh);
}

void Renderer3DOpenGL::drawUnlitTextured(Mesh* mesh)
{
	Shader* shader = mesh->materials.size() ? this->unlit_texture : this->unlit_color;

	if(!shader || !mesh)
		return;

	vec3 lightpos(0,200,-100);

	if(shader==this->shaded_texture)
		this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(mesh->isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CCW);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),mesh->entity->world);


	shader->SetSelectionColor(this->picking,mesh->entity,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));
	
	int position_slot = shader->GetPositionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int normal_slot = shader->GetNormalSlot();

	std::vector<unsigned int> textureIndices;

	if(mesh->materials.size())
	{
		draw(mesh,textureIndices,texture_slot,texcoord_slot);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER,mesh->ncontrolpoints*3*sizeof(float),mesh->controlpoints,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glEnableVertexAttribArray(normal_slot);glCheckError();
	glVertexAttribPointer(position_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();
	if(normal_slot>=0)glVertexAttribPointer(normal_slot,3,GL_FLOAT,GL_FALSE,0,mesh->normals);glCheckError();

	glBindBuffer(GL_ARRAY_BUFFER,0);

	glDrawArrays(GL_TRIANGLES,0,mesh->npolygons*3);glCheckError();
	//glDrawElements(GL_TRIANGLES,mesh->mesh_nvertexindices*3,GL_UNSIGNED_INT,mesh->mesh_vertexindices);glCheckError();


	glDisableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glDisableVertexAttribArray(normal_slot);glCheckError();

	for(int i=textureIndices.size();i>0;i--)
	{
		glDeleteTextures(1,&textureIndices[i-1]);glCheckError();
		glDisableVertexAttribArray(texcoord_slot);glCheckError();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}


void Renderer3DOpenGL::draw(Skin* skin)
{
	Shader* shader = skin->materials.size() ? this->unlit_texture : this->unlit_color;

	if(!skin || !skin->vertexcache || !shader)
	{
		this->drawUnlitTextured(skin);
		return;
	}

	vec3 lightpos(0,200,-100);

	if(shader==this->shaded_texture)
		this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(skin->isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CW);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),skin->entity->local);


	shader->SetSelectionColor(this->picking,skin->entity,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));

	int position_slot = shader->GetPositionSlot();
	int texcoord_slot = shader->GetTexcoordSlot();
	int texture_slot = shader->GetTextureSlot();
	int lightpos_slot = shader->GetLightposSlot();
	int lightdiff_slot = shader->GetLightdiffSlot();
	int lightamb_slot = shader->GetLightambSlot();
	int normal_slot = shader->GetNormalSlot();
	int color_slot = shader->GetColorSlot();
	
	std::vector<GLuint> textureIndices;

	draw(skin,textureIndices,texture_slot,texcoord_slot);

	int uniformTextured=shader->GetUniform("textured");

	glUniform1f(uniformTextured,(GLfloat)textureIndices.size());glCheckError();

	if(lightdiff_slot>=0)glEnableVertexAttribArray(lightdiff_slot);glCheckError();
	if(lightamb_slot>=0)glEnableVertexAttribArray(lightamb_slot);glCheckError();
	if(lightpos_slot>=0)glEnableVertexAttribArray(lightpos_slot);glCheckError();

	float v[3]={255,255,255};

	if(lightdiff_slot>=0)glUniform3f(lightdiff_slot,v[0],v[1],v[2]);glCheckError();
	if(lightamb_slot>=0)glUniform3f(lightamb_slot,v[0]+128,v[1]+128,v[2]+255);glCheckError();
	if(lightpos_slot>=0)glUniform3fv(lightpos_slot,1,lightpos);glCheckError();
	
	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER,skin->ncontrolpoints*3*sizeof(float),skin->vertexcache,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(position_slot);glCheckError();
	if(normal_slot>=0)glEnableVertexAttribArray(normal_slot);glCheckError();
	glVertexAttribPointer(position_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();
	if(normal_slot>=0)glVertexAttribPointer(normal_slot,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_TRIANGLES,0,skin->npolygons*3);glCheckError();
	//glDrawElements(GL_TRIANGLES,skin->mesh_ntriangleindices*3,GL_UNSIGNED_INT,skin->mesh_triangleindices);glCheckError();

	if(lightdiff_slot>=0)glDisableVertexAttribArray(position_slot);glCheckError();
	if(lightdiff_slot>=0)glDisableVertexAttribArray(normal_slot);glCheckError();

	if(lightdiff_slot>=0)glDisableVertexAttribArray(lightdiff_slot);glCheckError();
	if(lightamb_slot>=0)glDisableVertexAttribArray(lightamb_slot);glCheckError();
	if(lightpos_slot>=0)glDisableVertexAttribArray(lightpos_slot);glCheckError();
	
	for(int i=textureIndices.size();i>0;i--)
	{
		glDeleteTextures(1,&textureIndices[i-1]);glCheckError();
		glDisableVertexAttribArray(texcoord_slot);glCheckError();
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
}

void Renderer3DOpenGL::draw(Camera*)
{

}

void Renderer3DOpenGL::draw(Bone* bone)
{
	Shader* shader=this->unlit_color;

	if(!shader)
		return;

	vec3 a=bone->entity->parent->world.position();
	vec3 b=bone->entity->world.position();

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2], 
	};

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),mat4());

	shader->SetSelectionColor(this->picking,bone->entity,vec2(this->tabContainer->mousex/this->tabContainerWin32->windowData->width,this->tabContainer->mousey/this->tabContainerWin32->windowData->height));

	glEnable(GL_DEPTH_TEST);

	int pos=shader->GetPositionSlot();
	int col=shader->GetUniform("color");

	glUniform3fv(col,1,bone->color);

	glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);

	glBufferData(GL_ARRAY_BUFFER,6*sizeof(float),line,GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(pos);glCheckError();

	glVertexAttribPointer(pos,3,GL_FLOAT,GL_FALSE,0,0);glCheckError();

	glDrawArrays(GL_LINES,0,2);glCheckError();
	//glDrawElements(GL_TRIANGLES,3,GL_UNSIGNED_INT,)

	glDisableVertexAttribArray(pos);

	glDisable(GL_DEPTH_TEST);
}

float signof(float num){return (num>0 ? 1.0f : (num<0 ? -1.0f : 0.0f));}

void Renderer3DOpenGL::draw(Entity* iEntity)
{
	iEntity->draw(this);
}


void Renderer3DOpenGL::Render(GuiViewport* viewport,bool force)
{
	vec4 canvas=viewport->rect;
	vec2 mouse(tabContainerWin32->mousex,tabContainerWin32->mousey);

	ID2D1Bitmap*& rBitmap=(ID2D1Bitmap*&)viewport->renderBitmap;
	unsigned char*& rBuffer=(unsigned char*&)viewport->renderBuffer;


	if(!rBitmap || rBitmap->GetSize().width!=canvas.z || rBitmap->GetSize().height!=canvas.w || !viewport->renderBuffer || !viewport->renderBitmap)
	{
		SAFERELEASE(rBitmap);
		SAFEDELETEARRAY(viewport->renderBuffer);

		D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
		bp.pixelFormat=tabContainerWin32->renderer2DWin32->renderer->GetPixelFormat();

		tabContainerWin32->renderer2DWin32->renderer->CreateBitmap(D2D1::SizeU((unsigned int)canvas.z,(unsigned int)canvas.w),bp,&rBitmap);

		int rBufferSize=canvas.z*canvas.w*4;

		rBuffer=new unsigned char[rBufferSize];
	}

	if(viewport->rootEntity)
	{
		viewport->rootEntity->world=viewport->model;

		viewport->rootEntity->update();

		for(std::vector<GuiEntityViewer*>::iterator it=GuiEntityViewer::GetPool().begin();it!=GuiEntityViewer::GetPool().end();it++)
		{
			EditorEntity* eEntity=(EditorEntity*)(*it)->entity;
			if(eEntity && (*it)->tabContainer)
				eEntity->OnPropertiesUpdate((*it)->tabContainer);
		}
	}

	tabContainerWin32->renderer3D->ChangeContext();

	glViewport((int)0,(int)0,(int)canvas.z,(int)canvas.w);glCheckError();
	glScissor((int)0,(int)0,(int)canvas.z,(int)canvas.w);glCheckError();

	glEnable(GL_DEPTH_TEST);

	{
		glClearColor(0.43f,0.43f,0.43f,0.0f);glCheckError();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

		MatrixStack::Push(MatrixStack::PROJECTION,viewport->projection);
		MatrixStack::Push(MatrixStack::VIEW,viewport->view);
		MatrixStack::Push(MatrixStack::MODEL,viewport->model);

		tabContainer->renderer3D->draw(vec3(0,0,0),vec3(1000,0,0),vec3(1,0,0));
		tabContainer->renderer3D->draw(vec3(0,0,0),vec3(0,1000,0),vec3(0,1,0));
		tabContainer->renderer3D->draw(vec3(0,0,0),vec3(0,0,1000),vec3(0,0,1));

		if(viewport->rootEntity)
			this->draw(viewport->rootEntity);

		MatrixStack::Pop(MatrixStack::MODEL);
		MatrixStack::Pop(MatrixStack::VIEW);
		MatrixStack::Pop(MatrixStack::PROJECTION);

		glReadBuffer(GL_BACK);glCheckError();

		glReadPixels((int)0,(int)0,(int)canvas.z,(int)canvas.w,GL_BGRA,GL_UNSIGNED_BYTE,rBuffer);glCheckError();//@mic should implement pbo for performance

		D2D1_RECT_U rBitmapRect={0,0,canvas.z,canvas.w};

		rBitmap->CopyFromMemory(&rBitmapRect,rBuffer,(int)(canvas.z*4));

		tabContainerWin32->renderer2DWin32->renderer->DrawBitmap(rBitmap,D2D1::RectF(canvas.x,canvas.y,canvas.x+canvas.z,canvas.y+canvas.w));
	}

	if(viewport->needsPicking && mouse.y-canvas.y>=0)
	{
		glDisable(GL_DITHER);

		tabContainer->renderer3D->picking=true;

		glClearColor(0.0f,0.0f,0.0f,0.0f);glCheckError();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

		MatrixStack::Push(MatrixStack::PROJECTION,viewport->projection);
		MatrixStack::Push(MatrixStack::VIEW,viewport->view);
		MatrixStack::Push(MatrixStack::MODEL,viewport->model);

		if(viewport->rootEntity)
			this->draw(viewport->rootEntity);

		MatrixStack::Pop(MatrixStack::MODEL);
		MatrixStack::Pop(MatrixStack::VIEW);
		MatrixStack::Pop(MatrixStack::PROJECTION);

		glReadBuffer(GL_BACK);glCheckError();

		viewport->pickedPixel;
		glReadPixels((int)mouse.x,(int)mouse.y-canvas.y,(int)1,(int)1,GL_RGBA,GL_UNSIGNED_BYTE,&viewport->pickedPixel);glCheckError();//@mic should implement pbo for performance

		unsigned int address=0;

		unsigned char* ptrPixel=(unsigned char*)&viewport->pickedPixel;
		unsigned char* ptrAddress=(unsigned char*)&address;

		{
			viewport->pickedEntity=viewport->pickedPixel ? 

				ptrAddress[0]=ptrPixel[3],
				ptrAddress[1]=ptrPixel[2],
				ptrAddress[2]=ptrPixel[1],
				ptrAddress[3]=ptrPixel[0],

				dynamic_cast<EditorEntity*>((EditorEntity*)address)

				: 0 ;
		}

		tabContainer->renderer3D->picking=false;

		glEnable(GL_DITHER);

		viewport->needsPicking=false;
	}
}


void Renderer3DOpenGL::Render()
{
	for(std::list<GuiViewport*>::iterator vport=this->viewports.begin();vport!=this->viewports.end();vport++)
	{
		if(tabContainerWin32->BeginDraw())
		{
			(*vport)->OnPaint(this->tabContainer);
			tabContainerWin32->EndDraw();
		}
	}
}


/*float ratio=this->width/this->height;
	float calcRatio=ratio*tan(45*PI/180.0f);

	draw(vec3(0,1,-1),15);
	draw(vec3(calcRatio*this->RendererViewportInterface_farPlane,0,-this->RendererViewportInterface_farPlane+1),15);*/



/*
class Ray 
{ 
public: 
	Ray(const vec3 &orig, const vec3 &dir) : orig(orig), dir(dir) 
	{ 
		invdir = 1 / dir; 
		sign[0] = (invdir.x < 0); 
		sign[1] = (invdir.y < 0); 
		sign[2] = (invdir.z < 0); 
	} 
	vec3 orig, dir;       // ray orig and dir 
	vec3 invdir; 
	int sign[3]; 
}; 

bool intersect(AABB &bounds,const Ray &r) const 
{ 
	float tmin, tmax, tymin, tymax, tzmin, tzmax; 

	tmin = (bounds[r.sign[0]].x - r.orig.x) * r.invdir.x; 
	tmax = (bounds[1-r.sign[0]].x - r.orig.x) * r.invdir.x; 
	tymin = (bounds[r.sign[1]].y - r.orig.y) * r.invdir.y; 
	tymax = (bounds[1-r.sign[1]].y - r.orig.y) * r.invdir.y; 

	if ((tmin > tymax) || (tymin > tmax)) 
		return false; 
	if (tymin > tmin) 
		tmin = tymin; 
	if (tymax < tmax) 
		tmax = tymax; 

	tzmin = (bounds[r.sign[2]].z - r.orig.z) * r.invdir.z; 
	tzmax = (bounds[1-r.sign[2]].z - r.orig.z) * r.invdir.z; 

	if ((tmin > tzmax) || (tzmin > tmax)) 
		return false; 
	if (tzmin > tmin) 
		tmin = tzmin; 
	if (tzmax < tmax) 
		tmax = tzmax; 

	return true; 
}
*/




/*
void OpenGLRenderer::OnGuiLMouseDown()
{
	/ *float ratio=this->width/this->height;
	float calcRatioX=ratio*tan(45*PI/180.0f);
	float calcRatioY=-1;

	/ *vec3 ndc(2.0f * tabContainer->mousex / this->width -1.0f,-2.0f * (tabContainer->mousey-TabContainer::CONTAINER_HEIGHT) / this->height + 1.0f,0);
	mat4 viewInv=MatrixStack::view.inverse();
	rayStart=viewInv.transform(ndc.x*calcRatioX,ndc.y*calcRatioY,-1);
	rayEnd=viewInv.transform(ndc.x*calcRatioX*this->RendererViewportInterface_farPlane,ndc.y*calcRatioY*this->RendererViewportInterface_farPlane,-this->RendererViewportInterface_farPlane+1);* /
* /

}*/
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiImageWin32///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

//#pragma message(LOCATION " remember to use WindowData::CopyProcedureData to properly synchronize messages by the wndproc and the window herself")

GuiImageWin32::GuiImageWin32():handle(0){}

GuiImageWin32::~GuiImageWin32()
{
	SAFERELEASE(this->handle);
}

void GuiImageWin32::Release()
{
	SAFERELEASE(this->handle);
}

bool GuiImageWin32::Fill(Renderer2D* renderer,unsigned char* iData,float iWidth,float iHeight)
{
	Renderer2DWin32* renderer2DInterfaceWin32=(Renderer2DWin32*)renderer;

	HRESULT result=S_OK;

	if(!this->handle)
	{
		D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
		bp.pixelFormat=renderer2DInterfaceWin32->renderer->GetPixelFormat();
		bp.pixelFormat.alphaMode=D2D1_ALPHA_MODE_PREMULTIPLIED;

		result=renderer2DInterfaceWin32->renderer->CreateBitmap(D2D1::SizeU(iWidth,iHeight),iData,iWidth*4,bp,&this->handle);

		if(S_OK!=result || !this->handle)
			DEBUG_BREAK();
	}
	else
	{
		D2D1_SIZE_F tSize=this->handle->GetSize();

		if(tSize.width!=iWidth || tSize.height!=iHeight)
		{
			SAFEDELETE(this->handle);

			D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
			bp.pixelFormat=renderer2DInterfaceWin32->renderer->GetPixelFormat();
			bp.pixelFormat.alphaMode=D2D1_ALPHA_MODE_PREMULTIPLIED;

			result=renderer2DInterfaceWin32->renderer->CreateBitmap(D2D1::SizeU(iWidth,iHeight),iData,iWidth*4,bp,&this->handle);

			if(S_OK!=result || !this->handle)
				DEBUG_BREAK();
		}
		else
		{
			D2D_RECT_U rBitmapRect={0,0,iWidth,iHeight};

			result=this->handle->CopyFromMemory(&rBitmapRect,iData,iWidth*4);

			if(S_OK!=result)
				DEBUG_BREAK();
		}
	}

	return true;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////TabWin32///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////


TabWin32::TabWin32(float iX,float iY,float iW,float iH,HWND iParentWindow)
	:Tab(iX,iY,iW,iH),
	windowDataWin32((WindowDataWin32*&)windowData),
	editorWindowContainerWin32((ContainerWin32*&)parentWindowContainer),
	renderer2DWin32(0)
{
	windowData=new WindowDataWin32;

	this->windowDataWin32->hwnd=CreateWindow(WC_TABCONTAINERWINDOWCLASS,WC_TABCONTAINERWINDOWCLASS,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,(int)iX,(int)iY,(int)iW,(int)iH,iParentWindow,0,0,this);

	if(!this->windowDataWin32->hwnd)
		DEBUG_BREAK();

	this->iconDown=new GuiImageWin32;
	this->iconUp=new GuiImageWin32;
	this->iconRight=new GuiImageWin32;
	this->iconFolder=new GuiImageWin32;
	this->iconFile=new GuiImageWin32;

	this->renderer2D=this->renderer2DWin32=new Renderer2DWin32(this,this->windowDataWin32->hwnd);

	this->parentWindowContainer=(Container*)GetWindowLongPtr(iParentWindow,GWLP_USERDATA);
	this->parentWindowContainer->tabContainers.push_back(this);

	splitterContainer=this->parentWindowContainer->splitter;

	if(!splitterContainer)
		DEBUG_BREAK();

	Renderer3DOpenGL* _oglRenderer=new Renderer3DOpenGL(this);

	if(!_oglRenderer)
		DEBUG_BREAK();

	this->renderer3D=_oglRenderer;

	this->threadRender=new ThreadWin32;

	Task* tCreateOpenGLContext=this->threadRender->NewTask(std::function<void()>(std::bind(&Renderer3DOpenGL::Create,_oglRenderer,this->windowDataWin32->hwnd)));//_oglRenderer->Create(this->hwnd);
	while(tCreateOpenGLContext->func);
	SAFEDELETE(tCreateOpenGLContext);

	this->taskDraw=this->threadRender->NewTask(std::function<void()>(std::bind(&Tab::Draw,this)),false);
}


int TabWin32::TrackTabMenuPopup()
{
	HMENU root=CreatePopupMenu();
	HMENU create=CreatePopupMenu();

	/*
	#define TAB_MENU_COMMAND_REMOVE	1
	#define TAB_MENU_COMMAND_OPENGLWINDOW 2
	#define TAB_MENU_COMMAND_PROJECTFOLDER 3
	#define TAB_MENU_COMMAND_LOGGER 4
	#define TAB_MENU_COMMAND_SCENEENTITIES 5
	#define TAB_MENU_COMMAND_PROJECTFOLDER2 6
	#define TAB_MENU_COMMAND_SHAREDOPENGLWINDOW 7
	#define TAB_MENU_COMMAND_ENTITYPROPERTIES 8*/

	InsertMenu(root,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)create,"Viewers");
	{
		InsertMenu(create,0,MF_BYPOSITION|MF_STRING,2,"Viewport");
		InsertMenu(create,1,MF_BYPOSITION|MF_STRING,3,"Scene");
		InsertMenu(create,2,MF_BYPOSITION|MF_STRING,4,"Entity");
		InsertMenu(create,3,MF_BYPOSITION|MF_STRING,5,"Project");
		//InsertMenu(create,4,MF_BYPOSITION|MF_STRING,6,"Script");
	}
	InsertMenu(root,1,MF_BYPOSITION|MF_SEPARATOR,0,0);
	InsertMenu(root,2,MF_BYPOSITION|MF_STRING,1,"Remove");

	RECT rc;
	GetWindowRect(windowDataWin32->hwnd,&rc);

	int menuResult=TrackPopupMenu(root,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(this->windowDataWin32->lparam),rc.top+HIWORD(this->windowDataWin32->lparam),0,GetParent(this->windowDataWin32->hwnd),0);

	DestroyMenu(create);
	DestroyMenu(root);

	return menuResult;
}

int TabWin32::TrackGuiSceneViewerPopup(bool iSelected)
{
	HMENU menu=CreatePopupMenu();
	HMENU createEntity=CreatePopupMenu();
	HMENU createComponent=CreatePopupMenu();
	HMENU createMesh=CreatePopupMenu();
	HMENU createScript=CreatePopupMenu();

	if(iSelected)
	{
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,1,"New Entity");
		InsertMenu(menu,1,MF_BYPOSITION|MF_STRING,2,"Delete");
		InsertMenu(menu,2,MF_BYPOSITION|MF_POPUP,(UINT_PTR)createComponent,"Component");
		{
			InsertMenu(createComponent,0,MF_BYPOSITION|MF_STRING,3,"Light");
			InsertMenu(createComponent,1,MF_BYPOSITION|MF_POPUP,(UINT_PTR)createMesh,"Mesh");
			{
			}
			InsertMenu(createComponent,2,MF_BYPOSITION|MF_STRING,5,"Camera");
			InsertMenu(createComponent,3,MF_BYPOSITION|MF_STRING,14,"Script");
		}
	}
	else
	{
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,1,"New Entity");
	}

	RECT rc;
	GetWindowRect(windowDataWin32->hwnd,&rc);

	int result=TrackPopupMenu(menu,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(windowDataWin32->lparam),rc.top+HIWORD(windowDataWin32->lparam),0,GetParent(windowDataWin32->hwnd),0);

	DestroyMenu(menu);
	DestroyMenu(createEntity);
	DestroyMenu(createComponent);
	DestroyMenu(createMesh);
	DestroyMenu(createScript);

	return result;
}

namespace FileViewerActions
{
	const unsigned char Delete=1;
	const unsigned char Create=2;
	const unsigned char Load=3;
}

int TabWin32::TrackProjectFileViewerPopup(ResourceNode* iResourceNode)
{
	HMENU menu=CreatePopupMenu();

	if(iResourceNode)
	{
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,FileViewerActions::Delete,"Delete");

		if(iResourceNode->fileName.Extension() == &EngineIDE::instance->GetSceneExtension()[1])
			InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,FileViewerActions::Load,"Load");
	}
	else
	{
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,FileViewerActions::Create,"Create");
	}

	RECT rc;
	GetWindowRect(windowDataWin32->hwnd,&rc);

	int result=TrackPopupMenu(menu,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(windowDataWin32->lparam),rc.top+HIWORD(windowDataWin32->lparam),0,GetParent(windowDataWin32->hwnd),0);

	DestroyMenu(menu);

	return result;
}


TabWin32::~TabWin32()
{
	if(this->renderer3D)
		delete this->renderer3D;

	if(!this->windowDataWin32->FindAndGrowSibling())
		this->windowDataWin32->UnlinkSibling();

	this->parentWindowContainer->tabContainers.erase(std::find(parentWindowContainer->tabContainers.begin(),parentWindowContainer->tabContainers.end(),this));

	DestroyWindow(this->windowDataWin32->hwnd);
}

bool TabWin32::BeginDraw()
{
	if(!this->isRender)
	{
		if(this->recreateTarget)
		{
			this->OnGuiRecreateTarget();
			this->recreateTarget=0;
		}
		else if(this->resizeTarget)
		{
			HRESULT result=this->renderer2DWin32->renderer->Resize(D2D1::SizeU((int)this->windowDataWin32->width,(int)this->windowDataWin32->height));

			if(S_OK!=result)
				DEBUG_BREAK();

			this->resizeTarget=0;
		}
		
		this->renderer2DWin32->renderer->BeginDraw();
		this->isRender=true;

		return true;
	}
	else
		DEBUG_BREAK();
	
	return false;
	
}
void TabWin32::EndDraw()
{
	if(this->isRender)
	{
		renderer2D->DrawRectangle(0.5f,0.5f,this->windowDataWin32->width-0.5f,this->windowDataWin32->height-0.5f,0xff000000,false);

		HRESULT result=renderer2DWin32->renderer->EndDraw();
		
		this->recreateTarget=(result==D2DERR_RECREATE_TARGET);

		if(result!=0)
		{
			printf("D2D1HwndRenderTarget::EndDraw error: %x\n",result);

			result=renderer2DWin32->renderer->Flush();

			if(result!=0)
				printf("D2D1HwndRenderTarget::Flush error: %x\n",result);
		}

		this->isRender=false;
	}
	else
		DEBUG_BREAK();
}





void TabWin32::OnGuiMouseMove(void* data)
{
	

	this->mousex=(float)LOWORD(this->windowDataWin32->lparam);
	this->mousey=(float)HIWORD(this->windowDataWin32->lparam);

	Tab::OnGuiMouseMove(data);
}


void TabWin32::OnGuiMouseWheel(void* data)
{
	float factor=GET_WHEEL_DELTA_WPARAM(this->windowDataWin32->wparam)>0 ? 1.0f : (GET_WHEEL_DELTA_WPARAM(this->windowDataWin32->wparam)<0 ? -1.0f : 0);

	if(this->mousey>Tab::CONTAINER_HEIGHT)
		this->BroadcastToSelected(&GuiRect::OnMouseWheel,&factor);
}								  
		


void TabWin32::OnGuiRMouseUp(void* data)
{
	float &x=this->mousex;
	float &y=this->mousey;

	if(y<=Tab::CONTAINER_HEIGHT)
	{
		int tabNumberHasChanged=this->tabs.childs.size();

		for(int i=0;i<(int)tabs.childs.size();i++)
		{
			if(x>(i*TAB_WIDTH) && x< (i*TAB_WIDTH+TAB_WIDTH) && y > (CONTAINER_HEIGHT-TAB_HEIGHT) &&  y<CONTAINER_HEIGHT)
			{

				int menuResult=this->TrackTabMenuPopup();

				switch(menuResult)
				{
					case 1:
						if(this->GetPool().size()>1)
						{
							printf("total TabContainer before destroying: %d\n",this->GetPool().size());
							this->~TabWin32();
							printf("total TabContainer after destroying: %d\n",this->GetPool().size());
						}
					break;
					case 2:
						this->tabs.Viewport();
						this->selected=this->tabs.childs.size()-1;
					break;
					case 3:
						this->tabs.SceneViewer();
						this->selected=this->tabs.childs.size()-1;
					break;
					case 4:
						this->tabs.EntityViewer();
						this->selected=this->tabs.childs.size()-1;
					break;
					case 5:
						this->tabs.ProjectViewer();
						this->selected=this->tabs.childs.size()-1;
					break;
					case 6:
						/*this->tabs.ScriptViewer();
						this->selected=this->tabs.childs.size()-1;*/
					break;
				}

				break;
			}
		}

		if(tabNumberHasChanged!=this->tabs.childs.size())
			this->SetDraw(0,true);
	}
	else
	{
		this->BroadcastToSelected(&GuiRect::OnRMouseUp,vec2(this->mousex,this->mousey));
	}
	
}



void TabWin32::DrawFrame()
{
	this->renderer2D->Identity();

	this->renderer2D->DrawRectangle(0,0,(float)this->windowDataWin32->width,(float)CONTAINER_HEIGHT,Renderer2D::COLOR_TAB_BACKGROUND);
	this->renderer2D->DrawRectangle((float)(selected*TAB_WIDTH),
									(float)(CONTAINER_HEIGHT-TAB_HEIGHT),
									(float)(selected*TAB_WIDTH+TAB_WIDTH),
									(float)((CONTAINER_HEIGHT-TAB_HEIGHT)+TAB_HEIGHT),
									Renderer2D::COLOR_GUI_BACKGROUND);

	for(int i=0;i<(int)tabs.childs.size();i++)
		this->renderer2D->DrawText(tabs.childs[i]->name.Buffer(),
									(float)i*TAB_WIDTH,
									(float)CONTAINER_HEIGHT-TAB_HEIGHT,
									(float)i*TAB_WIDTH + (float)TAB_WIDTH,
									(float)(CONTAINER_HEIGHT-TAB_HEIGHT) + (float)TAB_HEIGHT,
									Renderer2D::COLOR_TEXT,
									0.5f,0.5f);
}



void TabWin32::OnGuiPaint(void* data)
{
	this->renderer2D->DrawRectangle(0,Tab::CONTAINER_HEIGHT,this->windowDataWin32->width,this->windowDataWin32->height-Tab::CONTAINER_HEIGHT,Renderer2D::COLOR_GUI_BACKGROUND);

	this->BroadcastToSelected(&GuiRect::OnPaint,data);
}


void TabWin32::OnResizeContainer(void* data)
{
	RECT rc;

	GetClientRect(this->windowDataWin32->hwnd,&rc);
	MapWindowRect(this->windowDataWin32->hwnd,this->editorWindowContainerWin32->windowDataWin32->hwnd,&rc);

	int size=0;
	int diff=0;
	int side=0;


	switch(this->parentWindowContainer->resizeEnumType)
	{
		case 0:
			side=(rc.right-rc.left);
			diff=this->parentWindowContainer->resizeDiffWidth;
			size=side+diff;
			SetWindowPos(this->windowDataWin32->hwnd,0,0,rc.top,size,rc.bottom-rc.top,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER);
			this->parentWindowContainer->resizeCheckWidth+=size;
			break;
		case 1:
			side=rc.bottom;
			diff=this->parentWindowContainer->resizeDiffHeight;
			size=side+diff;
			SetWindowPos(this->windowDataWin32->hwnd,0,rc.left,rc.top,rc.right-rc.left,size,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER);
			this->parentWindowContainer->resizeCheckHeight+=size;
			break;
		case 2:
			side=(rc.right-rc.left);
			diff=this->parentWindowContainer->resizeDiffWidth;
			size=side+diff;
			SetWindowPos(this->windowDataWin32->hwnd,0,rc.left,rc.top,size,rc.bottom-rc.top,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE);
			this->parentWindowContainer->resizeCheckWidth+=size;
			break;
		case 3:
			side=rc.bottom;
			diff=this->parentWindowContainer->resizeDiffHeight;
			size=side+diff;
			SetWindowPos(this->windowDataWin32->hwnd,0,rc.left,rc.top,rc.right-rc.left,size,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE);
			this->parentWindowContainer->resizeCheckHeight+=size;
		break;
		default:
			DEBUG_BREAK();
	}
}

void TabWin32::OnGuiRecreateTarget(void* iData)
{
	if(!this->renderer2DWin32->RecreateTarget(this->windowDataWin32->hwnd))
		DEBUG_BREAK();

	this->iconUp->Release();
	this->iconRight->Release();
	this->iconDown->Release();
	this->iconFolder->Release();
	this->iconFile->Release();
	
	if(!this->iconUp->Fill(this->renderer2D,this->rawUpArrow,CONTAINER_ICON_WH,CONTAINER_ICON_WH))
		DEBUG_BREAK();
	if(!this->iconRight->Fill(this->renderer2D,this->rawRightArrow,CONTAINER_ICON_WH,CONTAINER_ICON_WH))
		DEBUG_BREAK();
	if(!this->iconDown->Fill(this->renderer2D,this->rawDownArrow,CONTAINER_ICON_WH,CONTAINER_ICON_WH))
		DEBUG_BREAK();
	if(!this->iconFolder->Fill(this->renderer2D,this->rawFolder,CONTAINER_ICON_WH,CONTAINER_ICON_WH))
		DEBUG_BREAK();
	if(!this->iconFile->Fill(this->renderer2D,this->rawFile,CONTAINER_ICON_WH,CONTAINER_ICON_WH))
		DEBUG_BREAK();


	this->Tab::OnGuiRecreateTarget(iData);

}


void TabWin32::SetCursor(int iCursorCode)
{
	HCURSOR cursor=0;

	switch(iCursorCode)
	{
		case 0: cursor=LoadCursor(0,IDC_ARROW); break;
		case 1: cursor=LoadCursor(0,IDC_SIZEWE); break;
		case 2: cursor=LoadCursor(0,IDC_SIZENS); break;
	}
	
	if(cursor)
		::SetCursor(cursor);
}





///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////SplitterWin32///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

//#pragma message (LOCATION " beware to new POD initialization (parhentesized or not)")



SplitterWin32::SplitterWin32():
	currentTabContainerWin32((TabWin32*&)currentTabContainer),
	floatingTabRefWin32((TabWin32*&)floatingTabRef),
	floatingTabWin32((TabWin32*&)floatingTab),
	floatingTabTargetWin32((TabWin32*&)floatingTabTarget)

{

}
SplitterWin32::~SplitterWin32(){}

HMENU SplitterWin32::popupMenuRoot=CreatePopupMenu();
HMENU SplitterWin32::popupMenuCreate=CreatePopupMenu();

void SplitterWin32::OnLButtonDown(HWND hwnd,LPARAM lparam)
{
	if(!GetCapture() && !floatingTabRef && splitterCursor!=IDC_ARROW)
	{
		splitterPreviousPos=MAKEPOINTS(lparam);

		SetCursor(LoadCursor(0,splitterCursor));

		int edge1=(splitterCursor==IDC_SIZEWE ? 2 : 3);//right - bottom
		int edge2=(splitterCursor==IDC_SIZEWE ? 0 : 1);//left - top

		RECT rc1,rc2;
		GetClientRect(hittedWindow1,&rc1);
		GetClientRect(hittedWindow2,&rc2);
		MapWindowRect(hittedWindow1,hwnd,&rc1);
		MapWindowRect(hittedWindow2,hwnd,&rc2);

		{
			resizingWindows1=findWindoswAtPos(hwnd,rc1,edge1);
			resizingWindows2=findWindoswAtPos(hwnd,rc2,edge2);
		}

		SetCapture(hwnd);
	}
}

void SplitterWin32::OnLButtonUp(HWND hwnd)
{
	if(floatingTabRef)
	{
		EnableAllChildsDescendants(hwnd,true);

		if(floatingTab)
		{
			if(floatingTabTarget)
			{
				if(floatingTabTargetAnchorPos>=0)
				{
					TabWin32* newTabContainer=0;

					HWND& floatingTabTargetHwnd=floatingTabTargetWin32->windowDataWin32->hwnd;
					HWND& floatingTabRefHwnd=floatingTabRefWin32->windowDataWin32->hwnd;
					HWND& floatingTabHwnd=floatingTabWin32->windowDataWin32->hwnd;

					if(1==(int)floatingTabRef->tabs.childs.size())
					{
						newTabContainer=floatingTabRefWin32;

						if(!newTabContainer->windowData->FindAndGrowSibling())
							DEBUG_BREAK();

						

						RECT floatingTabTargetRc;
						GetClientRect(floatingTabTargetHwnd,&floatingTabTargetRc);
						MapWindowRect(floatingTabTargetHwnd,GetParent(floatingTabTargetHwnd),&floatingTabTargetRc);

						int floatingTabTargetRcWidth=floatingTabTargetRc.right-floatingTabTargetRc.left;
						int floatingTabTargetRcHeight=floatingTabTargetRc.bottom-floatingTabTargetRc.top;


						switch(floatingTabTargetAnchorPos)
						{
						case 0:
							SetWindowPos(floatingTabTargetHwnd,0,floatingTabTargetRc.left+floatingTabTargetRcWidth/2,floatingTabTargetRc.top,floatingTabTargetRc.right-(floatingTabTargetRc.left+floatingTabTargetRcWidth/2),floatingTabTargetRc.bottom-floatingTabTargetRc.top,SWP_SHOWWINDOW);
							SetWindowPos(floatingTabRefHwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top,floatingTabTargetRc.right/2-floatingTabTargetRc.left-splitterSize,floatingTabTargetRc.bottom-floatingTabTargetRc.top,SWP_SHOWWINDOW);
						break;
						case 1:
							SetWindowPos(floatingTabTargetHwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top+floatingTabTargetRcHeight/2,floatingTabTargetRc.right-floatingTabTargetRc.left,floatingTabTargetRc.bottom-(floatingTabTargetRc.top+floatingTabTargetRcHeight/2),SWP_SHOWWINDOW);
							SetWindowPos(floatingTabRefHwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top,floatingTabTargetRc.right-floatingTabTargetRc.left,floatingTabTargetRc.bottom/2-floatingTabTargetRc.top-splitterSize,SWP_SHOWWINDOW);
						break;
						case 2:
							SetWindowPos(floatingTabTargetHwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top,floatingTabTargetRc.right/2-floatingTabTargetRc.left-splitterSize,floatingTabTargetRc.bottom-floatingTabTargetRc.top,SWP_SHOWWINDOW);
							SetWindowPos(floatingTabRefHwnd,0,floatingTabTargetRc.left+floatingTabTargetRcWidth/2,floatingTabTargetRc.top,floatingTabTargetRc.right-(floatingTabTargetRc.left+(floatingTabTargetRcWidth/2)),floatingTabTargetRc.bottom-floatingTabTargetRc.top,SWP_SHOWWINDOW);
						break;
						case 3:
							SetWindowPos(floatingTabTargetHwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top,floatingTabTargetRc.right-floatingTabTargetRc.left,floatingTabTargetRc.bottom/2-floatingTabTargetRc.top-splitterSize,SWP_SHOWWINDOW);
							SetWindowPos(floatingTabRefHwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top+floatingTabTargetRcHeight/2,floatingTabTargetRc.right-floatingTabTargetRc.left,floatingTabTargetRc.bottom-(floatingTabTargetRc.top+(floatingTabTargetRcHeight/2)),SWP_SHOWWINDOW);
						break;
						default:
							DEBUG_BREAK();
						break;
						}
					}
					else
					{
						//newTabContainer=new TabContainerWin32((float)floatingTabRc.left,(float)floatingTabRc.top,(float)(floatingTabRc.right-floatingTabRc.left),(float)(floatingTabRc.bottom-floatingTabRc.top),hwnd);
						
						newTabContainer=this->currentTabContainerWin32->editorWindowContainerWin32->CreateTabContainer((float)floatingTabRc.left,(float)floatingTabRc.top,(float)(floatingTabRc.right-floatingTabRc.left),(float)(floatingTabRc.bottom-floatingTabRc.top));
						
						GuiRect* reparentTab=floatingTabRef->tabs.childs[floatingTabRefTabIdx];
						floatingTabRef->selected>0 ? floatingTabRef->selected-=1 : floatingTabRef->selected=0;
						floatingTabRef->OnGuiActivate();
						reparentTab->SetParent(&newTabContainer->tabs);
						
						newTabContainer->selected=newTabContainer->tabs.childs.size()-1;

						SetWindowPos(floatingTabTargetHwnd,0,floatingTabTargetRc.left,floatingTabTargetRc.top,floatingTabTargetRc.right-floatingTabTargetRc.left,floatingTabTargetRc.bottom-floatingTabTargetRc.top,SWP_SHOWWINDOW);		

						
					}

					floatingTabTarget->windowData->LinkSibling(newTabContainer->windowData,floatingTabTargetAnchorPos);
				}
				else
				{

				}
			}
		}
		this->DestroyFloatingTab();
	}
	else if(GetCapture())
	{
		ReleaseCapture();
		SetCursor(LoadCursor(0,IDC_ARROW));
	}
}

void SplitterWin32::OnMouseMove(HWND hwnd,LPARAM lparam)
{
	POINTS p=MAKEPOINTS(lparam);

	if(floatingTabRef)
	{
		RECT targetRc;
		RECT tmpRc;

		HWND& floatingTabTargetHwnd=floatingTabTargetWin32->windowDataWin32->hwnd;
		HWND& floatingTabRefHwnd=floatingTabRefWin32->windowDataWin32->hwnd;
		HWND& floatingTabHwnd=floatingTabWin32->windowDataWin32->hwnd;

		GetWindowRect(floatingTabRefHwnd,&tmpRc);

		int scalesize=3;

		int floatingTabScaledWidthX=(tmpRc.right-tmpRc.left)/scalesize;
		int floatingTabScaledWidthY=(tmpRc.bottom-tmpRc.top)/scalesize;

		POINT cp={p.x,p.y};

		HWND target=ChildWindowFromPointEx(hwnd,cp,CWP_SKIPDISABLED);

		if(target!=GetParent(floatingTabHwnd) && target!=floatingTabHwnd && /*floatingTabTarget!=floatingTabRef*/(floatingTabRefTabCount==1 ? target!=floatingTabRefHwnd : true))
		{
			floatingTabTarget=(Tab*)GetWindowLongPtr(target,GWLP_USERDATA);

			int hh=Tab::CONTAINER_HEIGHT;

			GetClientRect(target,&targetRc);
			tmpRc=targetRc;
			MapWindowRect(target,hwnd,&tmpRc);

			int w=tmpRc.right-tmpRc.left;
			int h=tmpRc.bottom-tmpRc.top;

			int wd=w/3;
			int hd=h/3;

			floatingTabTargetRc=floatingTabRc=tmpRc;
			int anchor=false;

			/*if(cp.y>tmpRc.top && cp.y<tmpRc.top+hh)
			{
				floatingTabTargetAnchorPos=-1;
				anchor=2;
			}
			else */if(cp.x>tmpRc.left && cp.x<tmpRc.left+wd)
			{
				floatingTabTargetAnchorPos=0;
				floatingTabRc.right=tmpRc.left+wd;
				floatingTabTargetRc.left=floatingTabRc.right+splitterSize;
				anchor=1;
			}
			else if(cp.y>tmpRc.top+hh && cp.y<tmpRc.top+hd)
			{
				floatingTabTargetAnchorPos=1;
				floatingTabRc=tmpRc;
				floatingTabRc.bottom=tmpRc.top+hd;
				floatingTabTargetRc.top=floatingTabRc.bottom+splitterSize;
				anchor=1;
			}
			else if(cp.x>tmpRc.right-wd && cp.x<tmpRc.right)
			{
				floatingTabTargetAnchorPos=2;
				floatingTabRc=tmpRc;
				floatingTabRc.left=tmpRc.right-wd;
				floatingTabTargetRc.right=floatingTabRc.left-splitterSize;
				anchor=1;
			}
			else if(cp.y>tmpRc.bottom-hd && cp.y<tmpRc.bottom)
			{
				floatingTabTargetAnchorPos=3;
				floatingTabRc=tmpRc;
				floatingTabRc.top=tmpRc.bottom-hd;
				floatingTabTargetRc.bottom=floatingTabRc.top-splitterSize;
				anchor=1;
			}
			else
				floatingTabTargetAnchorPos=-1;

			

			if(anchor==2)
			{
				/*ShowWindow(floatingTab,false);
				if(floatingTabTargetAnchorTabIndex<0)
					floatingTabTargetAnchorTabIndex=CreateTabChildren(floatingTabTarget,0,floatingTabRefTabIdx,floatingTab);*/

			}
			else
			{
				/*if(floatingTabTargetAnchorTabIndex>=0)
				{
					SendMessage(floatingTabTarget,TCM_DELETEITEM,floatingTabTargetAnchorTabIndex,0);
					floatingTabTargetAnchorTabIndex=-1;
				}
				ShowWindow(floatingTab,true);*/

				if(anchor==1)
					SetWindowPos(floatingTabHwnd,0,floatingTabRc.left,floatingTabRc.top,floatingTabRc.right-floatingTabRc.left,floatingTabRc.bottom-floatingTabRc.top,SWP_SHOWWINDOW);
				else
					SetWindowPos(floatingTabHwnd,0,p.x-(floatingTabScaledWidthX)/2,p.y-(floatingTabScaledWidthY)/2,floatingTabScaledWidthX,floatingTabScaledWidthY,SWP_SHOWWINDOW);
			}
		}
		else
		{
			//UpdateWindow(floatingTabRef->hwnd);
			SetWindowPos(floatingTabHwnd,0,p.x-(floatingTabScaledWidthX)/2,p.y-(floatingTabScaledWidthY)/2,floatingTabScaledWidthX,floatingTabScaledWidthY,SWP_SHOWWINDOW);
			floatingTabTarget=0;
			floatingTabTargetAnchorPos=-1;
		}
	}
	else
	{
		if(!GetCapture())
		{
			Tab::BroadcastToPool(&Tab::OnGuiMouseMove,(void*)0);

			int pixelDistance=6;

			POINT probePoints[4]={{p.x-pixelDistance,p.y},{p.x+pixelDistance,p.y},{p.x,p.y-pixelDistance},{p.x,p.y+pixelDistance}};

			HWND hittedWindows[4]={0,0,0,0};

			for(int i=0;i<4;i++)
			{
				HWND found=ChildWindowFromPoint(hwnd,probePoints[i]);
				if(found!=0 && hwnd!=found)
					hittedWindows[i]=found;
			}

			if(hittedWindows[0]!=hittedWindows[1] && hittedWindows[0] && hittedWindows[1])
			{
				SetCursor(LoadCursor(0,splitterCursor=IDC_SIZEWE));
				hittedWindow1=hittedWindows[0];
				hittedWindow2=hittedWindows[1];
			}
			else if(hittedWindows[2]!=hittedWindows[3] && hittedWindows[2] && hittedWindows[3])
			{
				SetCursor(LoadCursor(0,splitterCursor=IDC_SIZENS));
				hittedWindow1=hittedWindows[2];
				hittedWindow2=hittedWindows[3];
			}
			else
				SetCursor(LoadCursor(0,splitterCursor=IDC_ARROW));
		}
		else
		{
			POINTS mouseMovedDelta={splitterPreviousPos.x-p.x,splitterPreviousPos.y-p.y};

			if(mouseMovedDelta.x==0 && mouseMovedDelta.y==0)
				return;

			int numwindows=resizingWindows1.size()+resizingWindows2.size();

			HDWP hdwp=BeginDeferWindowPos(numwindows);

			RECT rc;

			DWORD flags1=SWP_NOMOVE|SWP_SHOWWINDOW;
			DWORD flags2=SWP_SHOWWINDOW;
			for(int i=0;i<(int)resizingWindows1.size();i++)
			{
				GetWindowRect(resizingWindows1[i],&rc);
				MapWindowRect(HWND_DESKTOP,hwnd,&rc);

				if(splitterCursor==IDC_SIZEWE)
					DeferWindowPos(hdwp,resizingWindows1[i],0,0,0,(rc.right-rc.left)-mouseMovedDelta.x,rc.bottom-rc.top,flags1);
				else 
					DeferWindowPos(hdwp,resizingWindows1[i],0,0,0,rc.right-rc.left,(rc.bottom-rc.top)-mouseMovedDelta.y,flags1);
			}

			for(int i=0;i<(int)resizingWindows2.size();i++)
			{
				GetWindowRect(resizingWindows2[i],&rc);
				MapWindowRect(HWND_DESKTOP,hwnd,&rc);

				if(splitterCursor==IDC_SIZEWE)
					DeferWindowPos(hdwp,resizingWindows2[i],0,rc.left-mouseMovedDelta.x,rc.top,(rc.right-rc.left)+mouseMovedDelta.x,rc.bottom-rc.top,flags2);
				else
					DeferWindowPos(hdwp,resizingWindows2[i],0,rc.left,rc.top-mouseMovedDelta.y,rc.right-rc.left,(rc.bottom-rc.top)+mouseMovedDelta.y,flags2);
			}

			EndDeferWindowPos(hdwp);
		}

	}

	splitterPreviousPos=p;
}

std::vector<HWND> SplitterWin32::findWindoswAtPos(HWND mainWindow,RECT &srcRect,int rectPosition)
{
	std::vector<HWND> foundWindows;

	int srcRectRef[4]={srcRect.left,srcRect.top,srcRect.right,srcRect.bottom};

	HWND child=0;

	while(child=FindWindowEx(mainWindow,child ? child : 0,0,0))
	{
		RECT rc;
		GetClientRect(child,&rc);
		MapWindowRect(child,mainWindow,&rc);


		int rect[4]={rc.left,rc.top,rc.right,rc.bottom};

		if(rect[rectPosition]==srcRectRef[rectPosition])
			foundWindows.push_back(child);

	}

	return foundWindows;

}


//#pragma message (LOCATION " should SplitterContainer::OnMouseWheel call directly the child OnMouseWheel method?")



void SplitterWin32::CreateFloatingTab(Tab* tab)
{
	if(floatingTabRef)
		return;

	floatingTabRef=tab;
	floatingTabRefTabIdx=tab->selected;
	floatingTabRefTabCount=(int)tab->tabs.childs.size();

	HWND& floatingTabTargetHwnd=floatingTabTargetWin32->windowDataWin32->hwnd;
	HWND& floatingTabRefHwnd=floatingTabRefWin32->windowDataWin32->hwnd;
	HWND& floatingTabHwnd=floatingTabWin32->windowDataWin32->hwnd;

	RECT tmpRc;

	GetWindowRect(floatingTabRefHwnd,&tmpRc);

	floatingTab=new TabWin32((float)tmpRc.left,(float)tmpRc.top,(float)(tmpRc.right-tmpRc.left),(float)(tmpRc.bottom-tmpRc.top),GetParent(floatingTabRefHwnd));

	EnableWindow(floatingTabHwnd,false);

	printf("creating floating TabContainer %p\n",floatingTab);

}

void SplitterWin32::DestroyFloatingTab()
{
	if(floatingTabRef)
	{
		printf("deleting floating TabContainer %p\n",this);

		floatingTab->~Tab();//remove floating window
		floatingTabRef->mouseDown=false;
		floatingTabRef=0;
		floatingTab=0;
	}
}







void SplitterWin32::EnableChilds(HWND hwnd,int enable,int show)
{
	HWND child=0;
	while(child=FindWindowEx(hwnd,child ? child : 0,0,0))
	{
		if(enable>=0)EnableWindow(child,enable);
		if(show>=0)ShowWindow(child,show);
	}
}

void SplitterWin32::EnableAllChildsDescendants(HWND hwnd,int enable,int show)
{
	HWND child=0;
	while(child=FindWindowEx(hwnd,child ? child : 0,0,0))
		EnableChilds(child,enable,show);
}



bool InitSplitter()
{
	bool returnValue=true;

	{
		WNDCLASSEX wc={0};
		wc.cbSize=sizeof(WNDCLASSEX);
		wc.hCursor=LoadCursor(NULL, IDC_ARROW);
		wc.lpszClassName=WC_MAINAPPWINDOW;
		wc.lpfnWndProc=MainContainerWin32::MainWindowProcedure;
		wc.hbrBackground=CreateSolidBrush(Renderer2D::COLOR_MAIN_BACKGROUND);

		if(!RegisterClassEx(&wc))
			DEBUG_BREAK();
	}


	{
		WNDCLASS wc={0};

		wc.lpszClassName=WC_TABCONTAINERWINDOWCLASS;
		wc.lpfnWndProc=TabWin32::TabContainerWindowClassProcedure;
		wc.hbrBackground=CreateSolidBrush(RGB(0,255,0));
		wc.hCursor=(HCURSOR)LoadCursor(0,IDC_ARROW);
		wc.style=CS_VREDRAW|CS_HREDRAW|CS_PARENTDC;

		if(!RegisterClass(&wc))
			DEBUG_BREAK();
	}

	

	return returnValue;
}



///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////SubsystemWin32//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

bool SubsystemWin32::Execute(String iPath,String iCmdLine,String iOutputFile,bool iInput,bool iError,bool iOutput)
{
	if(iPath=="none")
		iPath=EngineIDE::instance->projectFolder;

	STARTUPINFO si={0};
	PROCESS_INFORMATION pi={0};

	HANDLE tFileOutput=0;

	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = true;

	int tOutputFileSize=iOutputFile.Count();

	if(tOutputFileSize && (iInput || iError || iOutput))
	{
		SECURITY_ATTRIBUTES sa={0};

		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = 0;
		sa.bInheritHandle = true;  

		tFileOutput = CreateFile(iOutputFile.Buffer(),FILE_APPEND_DATA,FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL/*|FILE_FLAG_DELETE_ON_CLOSE*/,0);

		if(!tFileOutput)
			DEBUG_BREAK();

		si.dwFlags |= STARTF_USESTDHANDLES;
		si.hStdInput = iInput ? tFileOutput : GetStdHandle(STD_INPUT_HANDLE);
		si.hStdError = iError ? tFileOutput : GetStdHandle(STD_ERROR_HANDLE);
		si.hStdOutput = iOutput ? tFileOutput : GetStdHandle(STD_OUTPUT_HANDLE);
	}

	SetCurrentDirectory(iPath.Buffer());

	String tCommandLine="cmd.exe /V /C " + iCmdLine;

	if(!CreateProcess(0,(char*)tCommandLine.Buffer(),0,0,true,0,0,0,&si,&pi))
		return false;

	WaitForSingleObject( pi.hProcess, INFINITE );

	if(!CloseHandle( pi.hProcess ))
		DEBUG_BREAK();
	if(!CloseHandle( pi.hThread ))
		DEBUG_BREAK();
	if(tFileOutput && !CloseHandle( tFileOutput ))
		DEBUG_BREAK();

	return true;
}


unsigned int SubsystemWin32::FindProcessId(String iProcessName)
{
	PROCESSENTRY32 tProcess;
	tProcess.dwSize = sizeof(PROCESSENTRY32);

	HANDLE tSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if(Process32First(tSnapshot, &tProcess)==TRUE)
	{
		while(Process32Next(tSnapshot, &tProcess) == TRUE)
		{
			if(0==stricmp(tProcess.szExeFile,iProcessName.Buffer()))
				return tProcess.th32ProcessID;
		}
	}

	return 0;
}

unsigned int SubsystemWin32::FindThreadId(unsigned int iProcessId,String iThreadName)
{
	return 0;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////CompilerWin32///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

String CompilerWin32::ComposeMS(Script* iScript)
{
	String tSourceFullPathFileName=EngineIDE::instance->projectFolder + "\\" + iScript->script.path.File();

	String compilerOptions="/nologo /MDd /ZI /EHsc";
	String linkerOptions="/link /MANIFEST:NO /DLL /NOENTRY";
	FilePath outputDLL="/OUT:" + iScript->script.path.Name() + ".dll";
	String engineLibraryFullPathFileName=EngineIDE::instance->ideExecutable.Path() + "\\engineWin32.lib";
	String kernelLib="kernel32.lib";

	return "vcvars32.bat && cl.exe " + compilerOptions + " /I" +  EngineIDE::instance->compiler->ideSourcesPath.Buffer() + " " +  tSourceFullPathFileName + " " + linkerOptions + " " + outputDLL + " " + engineLibraryFullPathFileName + " " + kernelLib;
}

String CompilerWin32::ComposeMingW(Script* iScript)
{
	String tSourceFullPathFileName=EngineIDE::instance->projectFolder + "\\" + iScript->script.path.File();

	String tMingwCompilerExecutable="c:\\sdk\\mingw32\\bin\\i686-w64-mingw32-g++.exe ";

	String tEngineLibraryPath=" -Lc:\\sdk\\mingw32\\x86_64-w64-mingw32\\lib -L" + EngineIDE::instance->compiler->ideLibraryPath;
	String tMingwCommandLine="-O0 -g -I " + EngineIDE::instance->compiler->ideSourcesPath + tEngineLibraryPath + " -static -shared -o " + iScript->script.path.Name() + ".dll " + tSourceFullPathFileName ;

	String primitives=" " + EngineIDE::instance->ideExecutable.Path() + "\\primitives.o";
	String entities=" " + EngineIDE::instance->ideExecutable.Path() + "\\entities.o";
	String jpg=" " + EngineIDE::instance->ideExecutable.Path() + "\\imgjpg.o";
	String tga=" " + EngineIDE::instance->ideExecutable.Path() + "\\imgtga.o";
	String png=" " + EngineIDE::instance->ideExecutable.Path() + "\\imgpng.o";
	String engineWin32=" " + EngineIDE::instance->ideExecutable.Path() + "\\engineWin32.dll";
	String engineMingW=" " + EngineIDE::instance->ideExecutable.Path() + "\\engineMingW.dll";

	return tMingwCompilerExecutable + tMingwCommandLine + engineMingW;
}


String gfCreateRandomString(int iCount)
{
	const char tSymbolsAlphabet[]={"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"};

	char* tRandomString=new char[iCount+1];

	unsigned int tFeed=*(unsigned int*)&tRandomString;
	unsigned int tSymbol=sizeof(tSymbolsAlphabet);

	srand(tFeed);

	for(int i=0;i<iCount;i++)
	{
		tRandomString[i]=tSymbolsAlphabet[std::rand() % tSymbol];
	}

	tRandomString[iCount]='\0';
	
	String tReturnString(tRandomString);

	SAFEDELETEARRAY(tRandomString);

	return tReturnString;
}


String CompilerWin32::CreateRandomDir(String iDirWhere)
{
	String tRandomWorkingDirectoryName;
	String tRandomWorkingDirectory;

	DWORD res;

	while(true)
	{
		tRandomWorkingDirectoryName=gfCreateRandomString(8);

		String tFullRandomDirName=iDirWhere + "\\" + tRandomWorkingDirectoryName;

		DWORD res=::GetFileAttributes(tFullRandomDirName.Buffer());

		if((res & INVALID_FILE_ATTRIBUTES) || (res & FILE_ATTRIBUTE_DIRECTORY))
			break;
	}

	tRandomWorkingDirectory=iDirWhere + "\\" + tRandomWorkingDirectoryName;

	if(!::CreateDirectory(tRandomWorkingDirectory.Buffer(),0))
	{
		DWORD lastError=GetLastError();
		DEBUG_BREAK();
	}

	if(!(FILE_ATTRIBUTE_DIRECTORY & GetFileAttributes(tRandomWorkingDirectory.Buffer())))
		DEBUG_BREAK();

	return tRandomWorkingDirectory;
}

bool CompilerWin32::Compile(Script* iScript)
{
	if(iScript->runtime)
		return false;

	bool retVal;

	String tSourceFileContent=iScript->script.All();;
	File tCompilerTextOutput=EngineIDE::instance->applicationDataFolder + "\\error.output";
	
	//delete error.output

	if(tCompilerTextOutput.Exist())
	{
		if(!tCompilerTextOutput.Delete())
			DEBUG_BREAK();
	}

	//create random directory for the module if not exist yet

	if(!iScript->modulePath.Count())
		iScript->modulePath=this->CreateRandomDir(EngineIDE::instance->applicationDataFolder);

	//append exports to source

	if(iScript->script.Open("ab"))
	{
		String tExporterClassDeclaration="\n\nextern \"C\" __declspec(dllexport) EntityScript* Create(){return new " + iScript->entity->name + "_;}";
		String tExporterDeleterDeclaration="\n\nextern \"C\" __declspec(dllexport) void Destroy(EntityScript* iDestroy){SAFEDELETE(iDestroy);}";

		iScript->script.Write((void*)tExporterClassDeclaration.Buffer(),tExporterClassDeclaration.Count(),1);
		iScript->script.Write((void*)tExporterDeleterDeclaration.Buffer(),tExporterDeleterDeclaration.Count(),1);
		iScript->script.Close();
	}

	//compose the compiler command line

	String tCommandLineMingW=this->ComposeMingW(iScript);
	String tCommandLineWin32=this->ComposeMS(iScript);

	//execute compilation

	bool tExecuteWithSuccess=EngineIDE::instance->subsystem->Execute(iScript->modulePath,tCommandLineMingW,tCompilerTextOutput.path,true,true,true);

	if(!tExecuteWithSuccess)
		DEBUG_BREAK();

	//unroll exports

	if(iScript->script.Open("wb"))
	{
		iScript->script.Write((void*)tSourceFileContent.Buffer(),tSourceFileContent.Count(),1);
		iScript->script.Close();
	}

	//convert compiler output to readable locale

	wchar_t* tWideCharCompilationOutput=0;

	if(tCompilerTextOutput.Open("rb"))
	{
		int size=tCompilerTextOutput.Size();

		String tString=tCompilerTextOutput.All();

		tWideCharCompilationOutput=new wchar_t[tString.Count()+1];
		int nCharsWritten=MultiByteToWideChar(CP_OEMCP,MB_ERR_INVALID_CHARS,tString.Buffer(),tString.Count(),tWideCharCompilationOutput,tString.Count());
		tWideCharCompilationOutput[nCharsWritten]=L'\0';

		tCompilerTextOutput.Close();

		if(!tCompilerTextOutput.Delete())
			DEBUG_BREAK();
	}

	//extract and parse breakpoint line addresses

	File tLineAddressesOutput=iScript->modulePath + "\\laddrss.output";

	String tSharedObjectFileName=iScript->script.path.Name() + ".dll";
	String tSharedObjectSourceName=iScript->script.path.Name() + ".cpp";

	String tParseLineAddressesCommandLine="objdump --dwarf=decodedline " + tSharedObjectFileName + " | find \""+ tSharedObjectSourceName + "\" | find /V \":\"";

	tExecuteWithSuccess=EngineIDE::instance->subsystem->Execute(iScript->modulePath,tParseLineAddressesCommandLine,tLineAddressesOutput.path,true,true,true);

	if(!tExecuteWithSuccess)
		DEBUG_BREAK();

	if(tLineAddressesOutput.Open("rb"))
	{
		EditorScript* tEditorScript=(EditorScript*)iScript;

		int tNumberOfLines=tLineAddressesOutput.CountOccurrences('\n');

		char c[500];
		unsigned int line;

		for(int i=0;i<tNumberOfLines;i++)
		{
			fscanf((FILE*)tLineAddressesOutput.data,"%s",c);
			fscanf((FILE*)tLineAddressesOutput.data,"%u",&line);
			fscanf((FILE*)tLineAddressesOutput.data,"%s",&c);

			if(i>7)//skip source exports
			{
				Debugger::Breakpoint tLineAddress;

				sscanf(c,"%lx", &tLineAddress.address);
				tLineAddress.line=line;
				tLineAddress.script=iScript;


				EngineIDE::instance->debugger->allAvailableBreakpoints.push_back(tLineAddress);
			}			
		}

		tLineAddressesOutput.Close();

		if(!tLineAddressesOutput.Delete())
			DEBUG_BREAK();
	}

	//spawn a compilerViewer and show it if errors  @mic best to send message to the guicompilerviewer

	GuiCompilerViewer* guiCompilerViewer=!GuiCompilerViewer::GetPool().empty() ? GuiCompilerViewer::GetPool().front() : 0;

	if(!guiCompilerViewer)
	{
		EditorScript* tEditorScript=(EditorScript*)iScript;
		
		Tab* tabContainer=tEditorScript->scriptViewer ? tEditorScript->scriptViewer->GetRootRect()->tabContainer : Tab::GetPool().front();

		if(!tabContainer)
			DEBUG_BREAK();

		guiCompilerViewer=tabContainer->tabs.CompilerViewer();

		tabContainer->SetDraw(0,true,0);
	}

	Tab* tabContainer=guiCompilerViewer->GetRootRect()->tabContainer;

	bool noErrors=guiCompilerViewer->ParseCompilerOutputFile(tWideCharCompilationOutput);

	/*
	guiCompilerViewer->OnSize(tabContainer);
	guiCompilerViewer->OnActivate(tabContainer);

	if(false==noErrors)
		tabContainer->SetSelection(guiCompilerViewer);
	*/

	printf("%s on compiling %s\n",noErrors ? "OK" : "ERROR",iScript->script.path.Buffer());

	SAFEDELETEARRAY(tWideCharCompilationOutput);

	return retVal;
}




bool CompilerWin32::LoadScript(Script* iScript)
{
	if(!iScript->modulePath.Count())
		return false;

	HMODULE*		tModule=0;
	String			tFullModuleName;
	EntityScript*	(*tCreateMoculeClassFunction)()=0;

	tModule=this->ideScriptSourceModules.find(iScript)!=this->ideScriptSourceModules.end() ? this->ideScriptSourceModules[iScript] : 0;

	if(!tModule)
	{
		tModule=new HMODULE;
		this->ideScriptSourceModules.insert(std::pair<Script*,HMODULE*>(iScript,tModule));
	}

	tFullModuleName=iScript->modulePath + "\\" + iScript->entity->name + ".dll";

	*tModule=(HMODULE)LoadLibrary(tFullModuleName.Buffer());

	if(!*tModule)
		return false;

	tCreateMoculeClassFunction=(EntityScript* (*)())GetProcAddress(*tModule,"Create");

	if(tCreateMoculeClassFunction)
	{
		iScript->runtime=tCreateMoculeClassFunction();
		iScript->runtime->entity=iScript->entity;

		EntityScript* es=iScript->runtime;

		printf("entityscriptptr 0x%p\nentityptr 0x%p\nentityparentptr 0x%p\nentitychildsptr 0x%p\nentitylocalptr 0x%p\nentityworldptr 0x%p\nentitynameptr 0x%p\nentitybboxptr 0x%p\nentitycomponentsptr 0x%p\n",
			es,es->entity,es->entity->parent,&es->entity->childs,&es->entity->local,&es->entity->world,&es->entity->name,&es->entity->bbox,&es->entity->components);

		//iScript->runtime->init();
		EngineIDE::instance->debugger->RunDebuggeeFunction(iScript,0);

		return true;
	}
	else
	{
		SAFEDELETEARRAY(iScript->runtime);

		if(*tModule && FreeLibrary(*tModule))
			DEBUG_BREAK();
			

		SAFEDELETE(tModule);
		this->ideScriptSourceModules.erase(iScript);
	}

	return false;
}

bool CompilerWin32::UnloadScript(Script* iScript)
{
	const bool tDestroyInTheDLL=false;

	if(iScript->runtime)
	{
		HMODULE* tModule=this->ideScriptSourceModules.find(iScript)!=this->ideScriptSourceModules.end() ? this->ideScriptSourceModules[iScript] : 0;

		if(!tModule)
			return false;

		//iScript->runtime->deinit();
		EngineIDE::instance->debugger->RunDebuggeeFunction(iScript,2);

		if(!FreeLibrary(*tModule))
			return false;
		else
		{
			SAFEDELETE(tModule);
			this->ideScriptSourceModules.erase(iScript);
		}

		if(tDestroyInTheDLL)
		{
			void (*DestroyScript)(EntityScript*)=(void(*)(EntityScript*))GetProcAddress(*tModule,"Destroy");

			Tab* tabContainerRunninUpdater=GuiViewport::GetPool()[0]->GetRootRect()->tabContainer;

			tabContainerRunninUpdater->taskDraw->pause=true;

			while(tabContainerRunninUpdater->taskDraw->executing);

			DestroyScript(iScript->runtime);
			iScript->runtime=0;

			tabContainerRunninUpdater->taskDraw->pause=false;
		}
		else
			iScript->runtime=0;

		return true;
	}

	return true;
}


String gCompileAndroidSource(FilePath& iScriptSource,String& iAndroidTargetDirectory)
{
	String tLibName=iScriptSource.Name();

	String tAndroidMK=	"LOCAL_PATH := $(call my-dir)\n" 
						"include $(CLEAR_VARS)\n\n"

						"DSTDIR := " + iAndroidTargetDirectory + "\n"
						"LOCAL_C_INCLUDES := " + EngineIDE::instance->compiler->ideSourcesPath + "\n"
						"LOCAL_STATIC_LIBRARIES := -lEngine\n"
						"LOCAL_MODULE := " + tLibName + "\n"
						"LOCAL_SRC_FILES := " + iScriptSource + "\n"
						"LOCAL_CPPFLAGS := -std=gnu++0x -Wall -fPIE\n"
						"LOCAL_LDLIBS := -L" + EngineIDE::instance->compiler->ideLibraryPath + " \n\n"

						"include $(BUILD_SHARED_LIBRARY)\n\n"
						"all: $(DSTDIR)/$(notdir $(LOCAL_BUILT_MODULE))\n\n"

						"$(DSTDIR)/$(notdir $(LOCAL_BUILT_MODULE)): $(LOCAL_BUILT_MODULE)\n"
						"	move /Y \"$(subst /,\\,$<)\" \"$(subst /,\\,$@)\"\n";

	FILE* tAndroidMKFile=fopen(iAndroidTargetDirectory + "\\project\\jni\\Android.mk","w");

	if(tAndroidMKFile)
	{
		fwrite(tAndroidMK.Buffer(),tAndroidMK.Count(),1,tAndroidMKFile);
		fclose(tAndroidMKFile);
	}

	EngineIDE::instance->subsystem->Execute(iAndroidTargetDirectory + "\\project","ndk-build");

	return iAndroidTargetDirectory + "\\lib" + tLibName + ".so";
}

void gPackResourceDir(String& iCurrentDirectory,ResourceNodeDir* iResDir,File& iPackFile,File& iTableFile,String& iAndroidTargetDirectory)
{
	//store current dir

	if(iResDir->parent)
	{
		iCurrentDirectory+=iResDir->fileName.Buffer();
		iCurrentDirectory+="\\";
	}

	//if node contains files, process them, later process other dir nodes

	for(std::list<ResourceNode*>::iterator tResFile=iResDir->files.begin();tResFile!=iResDir->files.end();tResFile++)
	{
		File	tDataFile(EngineIDE::instance->projectFolder + iCurrentDirectory + (*tResFile)->fileName);
		int		tDataFileStart;
		int		tDataFileSize;
		String	tFinalFileName;

		if(tDataFile.path.Extension()=="cpp")
		{
			//compile source, put it into target directory
			tDataFile.path=gCompileAndroidSource(tDataFile.path,iAndroidTargetDirectory);
		}

		tDataFileSize=tDataFile.Size();

		/*if(!tDataFileSize)
			continue;*/

		if(tDataFile.Open())
		{
			tDataFileStart=ftell((FILE*)iPackFile.data);

			if(tDataFileSize)
				fwrite(&tDataFile.data,tDataFileSize,1,(FILE*)iPackFile.data);

			tDataFile.Close();
		}
		else
			DEBUG_BREAK();

		tFinalFileName=iCurrentDirectory + tDataFile.path.File(); 

		fprintf((FILE*)iTableFile.data,"%s %d %d\n",tFinalFileName.Buffer(),tDataFileStart,tDataFileSize);
		//printf("adding to package: %s %d %d\n",tFinalFileName.Buffer(),tDataFileStart,tDataFileSize);
	}

	for(std::list<ResourceNodeDir*>::iterator tResNodeDir=iResDir->dirs.begin();tResNodeDir!=iResDir->dirs.end();tResNodeDir++)
		gPackResourceDir(iCurrentDirectory,*tResNodeDir,iPackFile,iTableFile,iAndroidTargetDirectory);
}

bool CompilerWin32::CreateAndroidTarget()
{
	String tAndroidTargetDirectory=EngineIDE::instance->projectFolder.PathUp(1) + "\\android";
	String tAndroidProjectDirectory=tAndroidTargetDirectory + "\\project";
	String tAndroidProjectJniDirectory=tAndroidProjectDirectory + "\\jni";

	std::vector<std::string>		tSceneScripts;
	File							tAssetFile(tAndroidTargetDirectory + "\\asset.mp3");
	int								tPackFileSize=0;
	int								tTableFileSize=0;
	String							tResourceDirectory="\\";

	{
		SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),0,false};

		//create android project directory

		::CreateDirectory(tAndroidProjectDirectory.Buffer(),&sa);

		//create android jni project directory

		::CreateDirectory(tAndroidProjectJniDirectory.Buffer(),&sa);

		//create jni\Application.mk

		String tApplicationMK=	"APP_STL := c++_static\n";

		FILE* tApplicationMKFile=fopen(tAndroidProjectJniDirectory + "\\Application.mk","w");

		if(tApplicationMKFile)
		{
			fwrite(tApplicationMK.Buffer(),tApplicationMK.Count(),1,tApplicationMKFile);
			fclose(tApplicationMKFile);
		}
	}

	//pack data and table

	File tPackFile(tAndroidTargetDirectory + "\\pack.pck");
	File tTableFile(tAndroidTargetDirectory + "\\ptbl.txt");

	tPackFile.Open("wb");
	tTableFile.Open("wb");

	if(tPackFile.IsOpen() && tTableFile.IsOpen())
	{
		//iterate resources

		gPackResourceDir(tResourceDirectory,&GuiProjectViewer::GetPool()[0]->rootResource,tPackFile,tTableFile,tAndroidTargetDirectory);

		tPackFile.Close();
		tTableFile.Close();
	}

	//write asset

	tPackFileSize=tPackFile.Size();
	tTableFileSize=tTableFile.Size();

	tAssetFile.Open("wb");
	tPackFile.Open("rb");
	tTableFile.Open("rb");

	if(tAssetFile.IsOpen() && tPackFile.IsOpen() && tTableFile.IsOpen())
	{
		fwrite(&tTableFileSize,sizeof(int),1,(FILE*)tAssetFile.data);
		fwrite(&tPackFileSize,sizeof(int),1,(FILE*)tAssetFile.data);
		fwrite(&tTableFile.data,tTableFileSize,1,(FILE*)tAssetFile.data);
		fwrite(&tPackFile.data,tPackFileSize,1,(FILE*)tAssetFile.data);

		tAssetFile.Close();
		tPackFile.Close();
		tTableFile.Close();
	}

	File::Delete(tPackFile.path.Buffer());
	File::Delete(tTableFile.path.Buffer());

	return true;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////DebuggerWin32/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

const char* ExceptionString(unsigned int iCode)
{
	switch(iCode)
	{
		case EXCEPTION_ACCESS_VIOLATION        : return "EXCEPTION_ACCESS_VIOLATION"		; break;
		case EXCEPTION_DATATYPE_MISALIGNMENT   : return "EXCEPTION_DATATYPE_MISALIGNMENT";	; break;
		case EXCEPTION_BREAKPOINT              : return "EXCEPTION_BREAKPOINT"  			; break;
		case EXCEPTION_SINGLE_STEP             : return "EXCEPTION_SINGLE_STEP"  			; break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED   : return "EXCEPTION_ARRAY_BOUNDS_EXCEEDED"  	; break;
		case EXCEPTION_FLT_DENORMAL_OPERAND    : return "EXCEPTION_FLT_DENORMAL_OPERAND"  	; break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO      : return "EXCEPTION_FLT_DIVIDE_BY_ZERO"  	; break;
		case EXCEPTION_FLT_INEXACT_RESULT      : return "EXCEPTION_FLT_INEXACT_RESULT"  	; break;
		case EXCEPTION_FLT_INVALID_OPERATION   : return "EXCEPTION_FLT_INVALID_OPERATION"  	; break;
		case EXCEPTION_FLT_OVERFLOW            : return "EXCEPTION_FLT_OVERFLOW"  			; break;
		case EXCEPTION_FLT_STACK_CHECK         : return "EXCEPTION_FLT_STACK_CHECK"  		; break;
		case EXCEPTION_FLT_UNDERFLOW           : return "EXCEPTION_FLT_UNDERFLOW"  			; break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO      : return "EXCEPTION_INT_DIVIDE_BY_ZERO"  	; break;
		case EXCEPTION_INT_OVERFLOW            : return "EXCEPTION_INT_OVERFLOW"  			; break;
		case EXCEPTION_PRIV_INSTRUCTION        : return "EXCEPTION_PRIV_INSTRUCTION"  		; break;
		case EXCEPTION_IN_PAGE_ERROR           : return "EXCEPTION_IN_PAGE_ERROR"  			; break;
		case EXCEPTION_ILLEGAL_INSTRUCTION     : return "EXCEPTION_ILLEGAL_INSTRUCTION"  	; break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "EXCEPTION_NONCONTINUABLE_EXCEPTION"; break;  
		case EXCEPTION_STACK_OVERFLOW          : return "EXCEPTION_STACK_OVERFLOW"  		; break;
		case EXCEPTION_INVALID_DISPOSITION     : return "EXCEPTION_INVALID_DISPOSITION"  	; break;
		case EXCEPTION_GUARD_PAGE              : return "EXCEPTION_GUARD_PAGE"  			; break;
		case EXCEPTION_INVALID_HANDLE          : return "EXCEPTION_INVALID_HANDLE"  		; break;

		default:
			return "NONE";
	}
}

void DebuggerWin32::PrintThreadContext(void* iThreadContext)
{
	CONTEXT tc=*(CONTEXT*)iThreadContext;

	printf("{\ncflags 0x%p\neflags 0x%p\ndr0 0x%p\ndr1 0x%p\ndr2 0x%p\ndr3 0x%p\ndr7 0x%p\n}\n",
		tc.ContextFlags,
		tc.EFlags,
		tc.Dr0,
		tc.Dr1,
		tc.Dr2,
		tc.Dr3,
		tc.Dr7
		);
}

void DebuggerWin32::SuspendDebuggee()
{
	if(!this->threadSuspendend)
	{
		DWORD tSuspended=0;

		while(!tSuspended)
			tSuspended=SuspendThread(this->debuggeeThread);

		if(tSuspended==(DWORD)0xffffffff)
			printf("error suspending debuggee\n");
		else
			this->threadSuspendend=true;
	}
}

void DebuggerWin32::ResumeDebuggee()
{
	if(this->threadSuspendend)
	{
		DWORD tSuspended=0x00000002;

		while(tSuspended>0x00000001)
			tSuspended=ResumeThread(this->debuggeeThread);

		if(tSuspended==(DWORD)0xffffffff)
			printf("error resuming debuggee\n");
		else
			this->threadSuspendend=false;
	}
}

DWORD WINAPI debuggeeThreadFunc(LPVOID iDebuggerWin32)
{
	printf("debuggeeThreadFunc started\n");

	DebuggerWin32* tDebuggerWin32=(DebuggerWin32*)iDebuggerWin32;

	while(true)
	{
		if(tDebuggerWin32->runningScript)
		{
			switch(tDebuggerWin32->runningScriptFunction)
			{
				case 0: tDebuggerWin32->runningScript->runtime->init();break;
				case 1: tDebuggerWin32->runningScript->runtime->update();break;
				case 2: tDebuggerWin32->runningScript->runtime->deinit();break;
			};

			tDebuggerWin32->runningScriptFunction=0;
			tDebuggerWin32->runningScript=0;
		}
	}

	return 0;
}

int DebuggerWin32::HandleHardwareBreakpoint(void* iException)
{
	LPEXCEPTION_POINTERS exceptionInfo=(LPEXCEPTION_POINTERS)iException;

	this->threadContext=exceptionInfo->ContextRecord;

	this->PrintThreadContext(exceptionInfo->ContextRecord);

	if(this->lastBreakedAddress==exceptionInfo->ExceptionRecord->ExceptionAddress)
	{
		exceptionInfo->ContextRecord->EFlags|=0x10000;
		this->lastBreakedAddress=0;
		printf("skipping breakpoint %p\n",exceptionInfo->ExceptionRecord->ExceptionAddress);
	}
	else
	{
		std::vector<Debugger::Breakpoint>& tBreakpoints=this->breakpointSet;

		Debugger::Breakpoint* tbBreakpoint=0;

		for(size_t i=0;i<tBreakpoints.size();i++)
		{
			if(exceptionInfo->ExceptionRecord->ExceptionAddress==tBreakpoints[i].address)
			{
				this->BreakDebuggee(tBreakpoints[i]);
				break;
			}
		}

		if(this->breaked)
			while(this->breaked);
		else
		{
			printf("%s on address 0x%p without breakpoint\n",ExceptionString(exceptionInfo->ExceptionRecord->ExceptionCode),exceptionInfo->ExceptionRecord->ExceptionAddress);

			exceptionInfo->ContextRecord->Dr0=0;
			exceptionInfo->ContextRecord->Dr1=0;
			exceptionInfo->ContextRecord->Dr2=0;
			exceptionInfo->ContextRecord->Dr3=0;
			exceptionInfo->ContextRecord->Dr7=0;
		}
	}

	

	return EXCEPTION_CONTINUE_EXECUTION;
}

LONG WINAPI UnhandledException(LPEXCEPTION_POINTERS exceptionInfo)  
{  
	DebuggerWin32* debuggerWin32=(DebuggerWin32*)EngineIDEWin32::instance->debugger;

	return debuggerWin32->HandleHardwareBreakpoint(exceptionInfo);
}


void DebuggerWin32::SetHardwareBreakpoint(Breakpoint& iLineAddress,bool iSet)
{
	const unsigned int NUMBER_OF_BREAKPOINT_REGISTERS=4;

	DWORD* rBreakpointRegisters[NUMBER_OF_BREAKPOINT_REGISTERS]=
	{
		&this->threadContext->Dr0,
		&this->threadContext->Dr1,
		&this->threadContext->Dr2,
		&this->threadContext->Dr3
	};

	for(size_t i=0;i<NUMBER_OF_BREAKPOINT_REGISTERS;i++)
	{
		if(iSet)
		{
			if(!*rBreakpointRegisters[i])
			{
				*rBreakpointRegisters[i]=(DWORD)iLineAddress.address;
				this->threadContext->Dr7|=1UL << i*2;
				break;
			}
		}
		else
		{
			if(*rBreakpointRegisters[i]==(DWORD)iLineAddress.address)
			{
				*rBreakpointRegisters[i]=0;
				this->threadContext->Dr7 &= ~ (1UL << i*2);
				break;
			}
		}
	}
}

void DebuggerWin32::SetBreakpoint(Breakpoint& iLineAddress,bool iSet)
{
	if(!this->breaked)
	{
		this->SuspendDebuggee();

		this->threadContext->ContextFlags=CONTEXT_ALL|CONTEXT_DEBUG_REGISTERS;

		if(!GetThreadContext(this->debuggeeThread,this->threadContext))
			DEBUG_BREAK();
	}

	this->SetHardwareBreakpoint(iLineAddress,iSet);

	if(!this->breaked)
	{
		if(!SetThreadContext(this->debuggeeThread,this->threadContext))
			DEBUG_BREAK();
		else
			printf("%s breakpoint on source 0x%p at line %d address 0x%p\n",iSet ? "adding" : "removing",iLineAddress.script,iLineAddress.line,iLineAddress.address);

		this->ResumeDebuggee();
	}

	this->PrintThreadContext(this->threadContext);
}

void DebuggerWin32::BreakDebuggee(Breakpoint& iBreakpoint)
{
	this->breaked=true;
	this->currentBreakpoint=&iBreakpoint;
	this->lastBreakedAddress=iBreakpoint.address;

	iBreakpoint.breaked=true;

	EditorScript* tEditorScript=(EditorScript*)iBreakpoint.script;

	tEditorScript->scriptViewer->GetRootRect()->tabContainer->SetDraw(2,0,tEditorScript->scriptViewer);

	printf("breakpoint on 0x%p at line %d address 0x%p\n",iBreakpoint.script,iBreakpoint.line,iBreakpoint.address);
}
void DebuggerWin32::ContinueDebuggee()
{
	this->breaked=false;
	this->currentBreakpoint->breaked=false;
	this->currentBreakpoint=0;

	printf("resuming\n");
}

DebuggerWin32::DebuggerWin32()
{
	this->threadContext=new CONTEXT;

	SetUnhandledExceptionFilter(UnhandledException);

	this->debuggeeThread=CreateThread(0,0,debuggeeThreadFunc,this,/*CREATE_SUSPENDED*/0,(DWORD*)(int*)&this->debuggeeThreadId);
	
	printf("Debugger: debuggee thread id is %d\n",this->debuggeeThreadId);

	
}

void DebuggerWin32::RunDebuggeeFunction(Script* iDebuggee,unsigned char iFunctionIndex)
{
	if(!this->breaked)
	{
		/*if(!iDebuggee)
			DEBUG_BREAK();

		if(this->script)
			DEBUG_BREAK();*/

		this->runningScriptFunction=iFunctionIndex;
		this->runningScript=iDebuggee;

		while(this->runningScript && !this->breaked);
	}
	
}



///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////main  entry point/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////






//int WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
int main()
{
	

	EngineIDE *app = new EngineIDEWin32;

	app->Initialize();

	app->Run();

	return 0;
}

///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////