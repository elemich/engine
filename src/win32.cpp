#include "win32.h"

#ifdef _MSC_VER
	#define AGGREGATECALL(Func,Arg) Arg=Func()
#else
	#define AGGREGATECALL(Func,Arg) Func(&Arg)
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
		for(std::list<Task*>::iterator taskIter=pThread->tasks.begin();taskIter!=pThread->tasks.end();)
		{
			Task* pTask=(*taskIter);

			if(!pThread->pause && !pTask->pause && pTask->owner==pThread)
			{
				pTask->executing=true;
				pThread->executing=pTask;
				
				if(pTask->func)
				{
					pTask->func();
				}
				if(pTask->remove)
				{
					pTask->func=nullptr;
					taskIter=pThread->tasks.erase(taskIter);
				}
				else
				{
					pTask->executing=false;
					taskIter++;
				}

				pThread->executing=0;
				
			}
			else
				taskIter++;
		}

		::Sleep(pThread->sleep);

		if(pThread->exit)
			break;
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
	this->exit=true;

	WaitForSingleObject(this->handle,INFINITE);
	CloseHandle(this->handle);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////Direct2DBase//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
ID2D1Factory*			Direct2D::factory=0;
IWICImagingFactory*		Direct2D::imager=0;
IDWriteFactory*			Direct2D::writer=0;

void Direct2D::Release()
{
	SAFERELEASE(factory);
	SAFERELEASE(imager);
	SAFERELEASE(writer);
};


void Direct2D::Init()
{
	if(!factory)
	{
		wprintf(L"Initializing Direct2D\n");

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
	}
}

extern GuiFont* globalDefaultFont;
extern std::vector<GuiFont*> globalFontPool;

GuiFont* Direct2D::CreateFont(String iFontName,float iFontSize)
{
	IDWriteFontCollection*	tFontCollection=0;
	IDWriteFontFamily*		tFontFamily=0;
	IDWriteFontFace*		tFontFace=0;
	IDWriteFontFile*		tFontFile=0;
	IDWriteFont*			tFont=0;
	IDWriteTextFormat*		tTexter=0;

	HRESULT res=S_OK;

	res=Direct2D::writer->GetSystemFontCollection(&tFontCollection,false);
	if(S_OK!=res || !tFontCollection)
		DEBUG_BREAK();

	BOOL tFontExist;
	UINT32 tFontIndex;
	tFontCollection->FindFamilyName(iFontName.c_str(),&tFontIndex,&tFontExist);

	if(!tFontExist || tFontExist==UINT_MAX)
		DEBUG_BREAK();

	res=tFontCollection->GetFontFamily(tFontIndex,&tFontFamily);
	if(S_OK!=res || !tFontFamily)
		DEBUG_BREAK();

	res=tFontFamily->GetFirstMatchingFont(DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STRETCH_NORMAL,DWRITE_FONT_STYLE_NORMAL,&tFont);

	if(S_OK!=res || !tFont)
		DEBUG_BREAK();

	res=tFont->CreateFontFace(&tFontFace);

	if(S_OK!=res || !tFontFace)
		DEBUG_BREAK();

	wchar_t*				tGlyphs=L" \t!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~\0";//" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_'abcdefghijklmnopqrstuvwxyz{|}~";
	
	DWRITE_FONT_METRICS		tFontMetrics;
	
	UINT32					tGlyphsCount=wcslen(tGlyphs);
	UINT32*					tGlyphsCodePoints=new UINT32[tGlyphsCount];
	UINT16*					tGlyphsIndices=new UINT16[tGlyphsCount];
	DWRITE_GLYPH_METRICS*	tGlyphMetrics=new DWRITE_GLYPH_METRICS[tGlyphsCount];

	//get metrics

	tFontFace->GetMetrics(&tFontMetrics);

	for(int i=0;i<tGlyphsCount;i++)
		tGlyphsCodePoints[i]=tGlyphs[i];

	res = tFontFace->GetGlyphIndices((UINT32*)tGlyphsCodePoints,tGlyphsCount,tGlyphsIndices);

	if(S_OK!=res)
		DEBUG_BREAK();

	res = tFontFace->GetDesignGlyphMetrics(tGlyphsIndices,tGlyphsCount,tGlyphMetrics,FALSE);

	if(S_OK!=res)
		DEBUG_BREAK();

	res=Direct2D::writer->CreateTextFormat(iFontName.c_str(),NULL,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,iFontSize,L"",&tTexter);
	if(S_OK!=res || !tTexter)
		DEBUG_BREAK();

	res=tTexter->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
	if(S_OK!=res)
		DEBUG_BREAK();

	/*res=tTexter->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_DEFAULT,13.333334f,Direct2D::fontLogicSize);

	if(S_OK!=res)
		DEBUG_BREAK();*/

	SAFERELEASE(tFontCollection);
	SAFERELEASE(tFontFamily);
	SAFERELEASE(tFontFace);
	SAFERELEASE(tFontFile);
	SAFERELEASE(tFont);

	GuiFontWin32* tGuiFont=new GuiFontWin32;

	tGuiFont->name=iFontName;
	tGuiFont->texter=tTexter;

	float tXHeight=tTexter->GetFontSize() * (float)tFontMetrics.xHeight/(float)tFontMetrics.designUnitsPerEm;
	float tAscent=tTexter->GetFontSize() * (float)tFontMetrics.ascent/(float)tFontMetrics.designUnitsPerEm;
	float tDescent=tTexter->GetFontSize() * (float)tFontMetrics.descent/(float)tFontMetrics.designUnitsPerEm;

	tGuiFont->height=(tAscent+tDescent);

	for(int i=0;i<255;i++)
		tGuiFont->widths[i]=-1;

	for(int i=0;i<tGlyphsCount;i++)
	{	
		float tAdvanceWidth=(float)tGlyphMetrics[i].advanceWidth/(float)tFontMetrics.designUnitsPerEm;
	
		tGuiFont->widths[tGlyphs[i]]=tTexter->GetFontSize() * tAdvanceWidth;
	}

	tGuiFont->widths['\t']=tGuiFont->widths[' ']*tGuiFont->tabSpaces;

	tTexter->SetIncrementalTabStop(tGuiFont->widths['\t']);

	globalFontPool.push_back(tGuiFont);

	if(!globalDefaultFont)
		globalDefaultFont=tGuiFont;

	SAFEDELETE(tGlyphsCodePoints);
	SAFEDELETE(tGlyphsIndices);
	SAFEDELETE(tGlyphMetrics);

	return tGuiFont;
}

void dump(unsigned char* t)
{
	wprintf(L"\n");
	for(int i=0;i<1600;i++)
	{
		wprintf(L"0x%x,",t[i]);
	}
	wprintf(L"\n");
	system("pause");
}

bool Direct2D::CreateRawBitmap(const wchar_t* fname,bool iAction,ID2D1RenderTarget* renderer,ID2D1Bitmap* iBitmap,unsigned char** iBuffer,float* iWidth,float* iHeight)
{
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRESULT hr;

	hr = imager->CreateDecoderFromFilename(fname,NULL,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&pDecoder);

	if (hr!=S_OK || !pDecoder)
		DEBUG_BREAK();

	unsigned int frameCount;
	pDecoder->GetFrameCount(&frameCount);

	hr = pDecoder->GetFrame(0, &pSource);

	if (hr!=S_OK || !pSource)
		DEBUG_BREAK();

	hr = imager->CreateFormatConverter(&pConverter);

	if (hr!=S_OK || !pConverter)
		DEBUG_BREAK();

	hr = pConverter->Initialize(pSource,GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,NULL,0.f,WICBitmapPaletteTypeMedianCut);

	if(iAction==0)//Create the bitmap
	{
		hr = renderer->CreateBitmapFromWicBitmap(pConverter,NULL,&iBitmap);

		if (hr!=S_OK || !iBitmap)
			DEBUG_BREAK();
	}
	else if(iBuffer) //Copy the data
	{
		if(*iBuffer)
			delete [] *iBuffer;

		UINT tWidth,tHeight;
		pSource->GetSize(&tWidth,&tHeight);

		*iBuffer=new unsigned char[tWidth*tHeight*4];

		hr=pSource->CopyPixels(0,tWidth*4,tWidth*tHeight*4,*iBuffer);

		if (hr!=S_OK || !*iBuffer)
			DEBUG_BREAK();

		*iWidth=(float)tWidth;
		*iHeight=(float)tHeight;

		dump(*iBuffer);
	}

	if (SUCCEEDED(hr))
	{
		SAFERELEASE(pDecoder);
		SAFERELEASE(pSource);
		SAFERELEASE(pStream);
		SAFERELEASE(pConverter);
		SAFERELEASE(pScaler);
	}

	return true;
}


void Direct2D::DrawText(Renderer2D* iRenderer,const GuiFont* iFont,unsigned int iColor,const String& iText,float x1,float y1, float x2,float y2,float iAlignPosX,float iAlignPosY,bool iClip)
{
	Renderer2DWin32* tRenderer=(Renderer2DWin32*)iRenderer;
	GuiFontWin32* tFont=(GuiFontWin32*)iFont;

	tRenderer->renderer->DrawText(iText.c_str(),iText.size(),tFont->texter,D2D1::RectF(x1,y1,x2,y2),tRenderer->SetColorWin32(iColor),D2D1_DRAW_TEXT_OPTIONS_NONE,DWRITE_MEASURING_MODE_NATURAL);
}

void Direct2D::DrawLine(ID2D1RenderTarget*renderer,ID2D1Brush* brush,vec2 p1,vec2 p2,float iWidth,float iOpacity)
{
	renderer->DrawLine(D2D1::Point2F(p1.x,p1.y),D2D1::Point2F(p2.x,p2.y),brush,iWidth);
}

void Direct2D::DrawRectangle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y, float w,float h,bool fill,float op)
{
	brush->SetOpacity(op);
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

bool Direct2D::LoadBitmapRef(ID2D1RenderTarget* renderer,ID2D1Bitmap*& iHandle,unsigned char* iData,float iWidth,float iHeight)
{
	HRESULT result=S_OK;

	if(!iHandle)
	{
		D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
		//renderer2DInterfaceWin32->renderer->GetPixelFormat(&bp.pixelFormat);
		AGGREGATECALL(renderer->GetPixelFormat,bp.pixelFormat);
		bp.pixelFormat.alphaMode=D2D1_ALPHA_MODE_PREMULTIPLIED;

		result=renderer->CreateBitmap(D2D1::SizeU(iWidth,iHeight),iData,iWidth*4,bp,&iHandle);

		if(S_OK!=result || !iHandle)
			DEBUG_BREAK();
	}
	else
	{
		D2D1_SIZE_F tSize;
		//this->handle->GetSize(&tSize);
		AGGREGATECALL(iHandle->GetSize,tSize);

		if(tSize.width!=iWidth || tSize.height!=iHeight)
		{
			SAFEDELETE(iHandle);

			D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
			//renderer2DInterfaceWin32->renderer->GetPixelFormat(&bp.pixelFormat);
			AGGREGATECALL(renderer->GetPixelFormat,bp.pixelFormat);
			bp.pixelFormat.alphaMode=D2D1_ALPHA_MODE_PREMULTIPLIED;

			result=renderer->CreateBitmap(D2D1::SizeU(iWidth,iHeight),iData,iWidth*4,bp,&iHandle);

			if(S_OK!=result || !iHandle)
				DEBUG_BREAK();
		}
		else
		{
			D2D_RECT_U rBitmapRect={0,0,iWidth,iHeight};

			result=iHandle->CopyFromMemory(&rBitmapRect,iData,iWidth*4);

			if(S_OK!=result)
				DEBUG_BREAK();
		}
	}

	return true;
}

bool Direct2D::LoadBitmapFile(ID2D1RenderTarget* renderer,String iFilename,ID2D1Bitmap*& iHandle,float& iWidth,float& iHeight)
{
	return Direct2D::CreateRawBitmap(iFilename.c_str(),false,renderer,iHandle,0,0,0);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////GuiFontWin32///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiFontWin32::GuiFontWin32():texter(0){};
GuiFontWin32::~GuiFontWin32()
{
	SAFERELEASE(this->texter);
}

GuiFont* GuiFont::CreateFont(String iFontName,float iFontSize)
{
	return Direct2D::CreateFont(iFontName,iFontSize);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////Renderer2DWin32///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

Renderer2DWin32::Renderer2DWin32(Tab* iTabContainer,HWND handle):Renderer2D(iTabContainer),brush(0),renderer(0)
{
	Direct2D::Init();
}
Renderer2DWin32::~Renderer2DWin32()
{
	SAFERELEASE(this->brush);
	SAFERELEASE(this->renderer);

}



bool Renderer2DWin32::RecreateTarget(HWND iHandle)
{
	SAFERELEASE(this->renderer);
	SAFERELEASE(this->brush);

	HRESULT result=S_OK;

	{
		RECT rc;
		GetClientRect(iHandle, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top);

		D2D1_RENDER_TARGET_PROPERTIES rRenderTargetProperties=D2D1::RenderTargetProperties();

		rRenderTargetProperties.type=D2D1_RENDER_TARGET_TYPE_HARDWARE;
		rRenderTargetProperties.usage=D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE;//we wants resource sharing
		/*rRenderTargetProperties.pixelFormat.format=DXGI_FORMAT_UNKNOWN;
		rRenderTargetProperties.pixelFormat.alphaMode=D2D1_ALPHA_MODE_PREMULTIPLIED;*/

		result=Direct2D::factory->CreateHwndRenderTarget(rRenderTargetProperties,D2D1::HwndRenderTargetProperties(iHandle,size,D2D1_PRESENT_OPTIONS_IMMEDIATELY),&renderer);
	}

	if(S_OK!=result || !this->renderer)
		DEBUG_BREAK();

	result=this->renderer->CreateSolidColorBrush(D2D1::ColorF(Tab::COLOR_BACK),&brush);

	if(S_OK!=result || !this->brush)
		DEBUG_BREAK();

	return true;
}

void Renderer2DWin32::DrawText(const String& iText,float left,float top, float right,float bottom,unsigned int iColor,const GuiFont* iFont)
{
	Direct2D::DrawText(this,iFont,iColor,iText,left,top,right,bottom);
}

void Renderer2DWin32::DrawText(const String& iText,float left,float top, float right,float bottom,vec2 iSpot,vec2 iAlign,unsigned int iColor,const GuiFont* iFont)
{
	vec4 tRect(left,top,right-left,bottom-top);

	vec2 tTextSize=iFont->MeasureText(iText.c_str());

	float tLeft=tRect.x + (tRect.z*iAlign.x) - (tTextSize.x * iSpot.x);
	float tTop=tRect.y + (tRect.w*iAlign.y) - (tTextSize.y * iSpot.y);

	Direct2D::DrawText(this,iFont,iColor,iText,tLeft,tTop,tLeft + tTextSize.x,tTop + tTextSize.y);
}

void Renderer2DWin32::DrawLine(vec2 p1,vec2 p2,unsigned int iColor,float iWidth,float iOpacity)
{
	Direct2D::DrawLine(this->renderer,this->SetColorWin32(iColor),p1,p2,iWidth,iOpacity);
}

void Renderer2DWin32::DrawRectangle(float iX,float iY, float iW,float iH,unsigned int iColor,bool iFill,float op)
{
	Direct2D::DrawRectangle(this->renderer,this->SetColorWin32(iColor),iX,iY,iW,iH,iFill,op);
}

void Renderer2DWin32::DrawRectangle(vec4& iXYWH,unsigned int iColor,bool iFill)
{
	Direct2D::DrawRectangle(this->renderer,this->SetColorWin32(iColor),iXYWH.x,iXYWH.y,iXYWH.x+iXYWH.z,iXYWH.y+iXYWH.w,iFill);
}

void Renderer2DWin32::DrawBitmap(Picture* iBitmap,float iX,float iY, float iW,float iH)
{
	Direct2D::DrawBitmap(this->renderer,(ID2D1Bitmap*)iBitmap->handle,iX,iY,iW,iH);
}

bool Renderer2DWin32::LoadBitmap(Picture* iPicture)
{
	if(!iPicture->handle)
	{
		PictureRef* tPictureRef   = dynamic_cast<PictureRef*>(iPicture);
		PictureFile* tPictureFile = dynamic_cast<PictureFile*>(iPicture);

		if(tPictureRef)
			return Direct2D::LoadBitmapRef(this->renderer,(ID2D1Bitmap*&)tPictureRef->handle,tPictureRef->refData,tPictureRef->width,tPictureRef->height);

		if(tPictureFile)
			return Direct2D::LoadBitmapFile(this->renderer,tPictureFile->fileName,(ID2D1Bitmap*&)tPictureFile->handle,tPictureFile->width,tPictureFile->height);
	}
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


ID2D1Brush* Renderer2DWin32::SetColorWin32(unsigned int color,float opaque)
{
	this->brush->SetColor(D2D1::ColorF(color));
	this->brush->SetOpacity(opaque);
	return this->brush;
}



///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////ContainerWin32////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////



ContainerWin32::ContainerWin32():
	windowDataWin32((WindowDataWin32*&)windowData),
	splitterContainerWin32((SplitterWin32*&)splitter)

{
	windowData=new WindowDataWin32;
	splitter=new SplitterWin32;
}

ContainerWin32::~ContainerWin32()
{
	SAFEDELETE(this->windowData);
	SAFEDELETE(this->splitter);
}

int ContainerWin32::GetWindowHandle()
{
	return (int)this->windowDataWin32->hwnd;
}

TabWin32* ContainerWin32::CreateTab(float x,float y,float w,float h)
{
	TabWin32* result=new TabWin32(x,y,w,h,this->windowDataWin32->hwnd);
	return result;
}

TabWin32* ContainerWin32::CreateModalTab(float x,float y,float w,float h)
{
	TabWin32* result=new TabWin32(x,y,w,h,this->windowDataWin32->hwnd,true);

	DWORD tLong=(DWORD)GetWindowLongPtr(result->windowDataWin32->hwnd,GWL_STYLE);

	tLong ^= WS_CAPTION;

	SetWindowLongPtr(result->windowDataWin32->hwnd,GWL_STYLE,(LONG)tLong);

	SetParent(result->windowDataWin32->hwnd,0);

	Ide::GetInstance()->mainAppWindow->mainContainer->BroadcastToSelectedTabRects(&GuiRect::OnSize);
	Ide::GetInstance()->mainAppWindow->mainContainer->BroadcastToSelectedTabRects(&GuiRect::OnActivate);

	return result;
}

void ContainerWin32::DestroyTab(Tab* tTab)
{
	tTab->Destroy();

	PostMessage(this->windowDataWin32->hwnd,(WPARAM)WM_ENABLE,true,0);
}


void ContainerWin32::OnSizing()
{
	LPRECT sz=(LPRECT)this->windowDataWin32->lparam;

	RECT trc;
	GetClientRect(this->windowDataWin32->hwnd,&trc);

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
	//this->OnSize();
	//SplitterContainer::OnSize(this->hwnd,this->wparam,this->lparam);

	if(this->resizeEnumType<0)
		return;

	this->resizeCheckWidth=0;
	this->resizeCheckHeight=0;

	for(std::vector<Tab*>::iterator i=this->tabs.begin();i!=this->tabs.end();i++)
	{
		TabWin32* tab=static_cast<TabWin32*>(*(i));

		if(tab && tab->windowData->siblings[this->resizeEnumType].empty())
			tab->OnResizeContainer();
	}
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////MenuInterface/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

HMENU ____MainMenu=0;
static int ____MenuInterfaceIds=0;

int DLLBUILD GetMenuId()
{
	return ____MenuInterfaceIds;
}

int DLLBUILD IncrementMenuId()
{
	return ____MenuInterfaceIds++;
}

int MenuInterface::GetMenuId()
{
	return ::GetMenuId();
}

int MenuInterface::IncrementMenuId()
{
	return ::IncrementMenuId();
}

int MenuInterface::Menu(String iName,bool tPopup)
{
	HMENU tMenu=____MainMenu;

	wchar_t*  tBase=&iName[0];
	wchar_t* tBegin=tBase;
	wchar_t* tEnd=0;

	int tItemId=-1;

	int tFirstLevel=0;
	for (int i=0;i<iName.size();i++)tFirstLevel+=iName[i]==L'\\' ? 1 : 0;

	tFirstLevel=!(bool)tFirstLevel;

	bool tFirstLevelQuestionMark=false;

	while(tBegin)
	{
		tEnd=wcsstr(tBegin,L"\\");

		size_t pos=tBegin-tBase;
		size_t len=tEnd-tBegin;
		std::wstring searchMenu=iName.substr(pos,tEnd ? len : std::wstring::npos);	
		int menuIdx=0;

		wchar_t menuName[CHAR_MAX]={0};
		bool found=false;
		
		while(GetMenuString(tMenu,menuIdx,menuName,CHAR_MAX,MF_BYPOSITION))
		{
			if(String(menuName)==String(L"?"))
				tFirstLevelQuestionMark=true;

			if(searchMenu==menuName)
			{
				tMenu=GetSubMenu(tMenu,menuIdx);
				found=true;
				break;
			}

			menuIdx++;
		}

		wchar_t* tNextMenuNameExists=wcsstr(tBegin,L"\\");

		if(!found && !tNextMenuNameExists)
		{
			if(!menuName[0])
			{
				HMENU tNewMenu=tPopup ? CreatePopupMenu() : 0;

				MENUITEMINFO mitem={0};

				mitem.cbSize=sizeof(MENUITEMINFO);
				mitem.fMask=MIIM_DATA|MIIM_TYPE|MIIM_SUBMENU;
				mitem.fType=MFT_STRING;
				mitem.dwItemData=(ULONG_PTR)this;
				mitem.dwTypeData=(wchar_t*)searchMenu.c_str();
				mitem.cch=searchMenu.size();
				mitem.hSubMenu=tNewMenu;
				mitem.fMask|=MIIM_ID;
				mitem.wID=tItemId=this->IncrementMenuId();

				::InsertMenuItem(tMenu,tFirstLevelQuestionMark ? menuIdx-1 : menuIdx,true,&mitem);
				::DrawMenuBar((HWND)Ide::GetInstance()->mainAppWindow->mainContainer->GetWindowHandle());
			}
		}

		tBegin = tEnd ? tEnd+1 : 0;
	}

	return tItemId;
}


///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////MainContainerWin32////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

MainContainerWin32::MainContainerWin32():mainContainerWin32(0)
{
	GuiFont::CreateFont(L"Verdana",10);
	GuiFont::CreateFont(L"Verdana",12);
	GuiFont::CreateFont(L"Verdana",14);
	GuiFont::CreateFont(L"Verdana",16);
}

MainContainerWin32::~MainContainerWin32()
{
	this->Deintialize();
}

void MainContainerWin32::Initialize()
{
	this->mainContainer=this->mainContainerWin32=new ContainerWin32;

	this->containers.push_back(this->mainContainer);

	____MainMenu=CreateMenu();
	::InsertMenu(____MainMenu,0,MF_BYPOSITION,0,0);

	if(____MainMenu)
	{
		MENUINFO mi={0};

		mi.cbSize=sizeof(MENUINFO);
		mi.fMask=MIM_STYLE|MIM_APPLYTOSUBMENUS;
		mi.dwStyle=MNS_NOTIFYBYPOS;

		SetMenuInfo(____MainMenu,&mi);
	}

	this->mainContainerWin32->windowDataWin32->hwnd=CreateWindow(WC_MAINAPPWINDOW,WC_MAINAPPWINDOW,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,0,0,1024,768,0,____MainMenu,0,this->mainContainerWin32);

	this->MenuFile=this->Menu(L"File",true);
	this->MenuActionExit=this->Menu(L"File\\Exit",false);
	this->MenuBuild=this->Menu(L"Build",true);
	this->MenuActionBuildPC=this->Menu(L"Build\\PC",false);
	this->MenuPlugins=this->Menu(L"Plugins",true);
	this->MenuActionConfigurePlugin=this->Menu(L"Plugins\\Configure",false);
	this->MenuInfo=this->Menu(L"?",true);
	this->MenuActionProgramInfo=this->Menu(L"?\\Info",false);

	RECT rc;
	GetClientRect(this->mainContainerWin32->windowDataWin32->hwnd,&rc);

	TabWin32* tabContainer1=this->mainContainerWin32->CreateTab(0.0f,0.0f,(float)300,(float)200);
	TabWin32* tabContainer2=this->mainContainerWin32->CreateTab(0.0f,204.0f,(float)300,(float)200);
	TabWin32* tabContainer3=this->mainContainerWin32->CreateTab(0.0f,408.0f,(float)300,(float)rc.bottom-(rc.top+408));
	TabWin32* tabContainer4=this->mainContainerWin32->CreateTab(304.0f,0.0f,(float)rc.right-(rc.left+304),(float)rc.bottom-rc.top);



	GuiSceneViewer* tScene=tabContainer1->CreateSingletonViewer<GuiSceneViewer>();
	tabContainer2->CreateViewer<GuiEntityViewer>();
	tabContainer3->CreateSingletonViewer<GuiProjectViewer>();
	GuiViewport* tViewport=tabContainer4->CreateViewer<GuiViewport>(new GuiViewport(vec3(100,100,100),vec3(0,0,0),vec3(0,0,1),true));

	tViewport->rootEntity=tScene->entityRoot;

	this->mainContainer->BroadcastToSelectedTabRects(&GuiRect::OnSize);
	this->mainContainer->BroadcastToSelectedTabRects(&GuiRect::OnActivate);

	ShowWindow(this->mainContainerWin32->windowDataWin32->hwnd,true);
}

void MainContainerWin32::Deintialize()
{

}

ContainerWin32* MainContainerWin32::CreateContainer()
{
	ContainerWin32* container=new ContainerWin32;

	container->windowDataWin32->hwnd=CreateWindow(WC_MAINAPPWINDOW,WC_MAINAPPWINDOW,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,this->mainContainerWin32->windowDataWin32->hwnd,0,0,container);

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
			wprintf(L"main painting\n");
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
			return 0;
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
		case WM_MENUCOMMAND:
		{
			int itemIdx=wparam;
			HMENU hMenu=(HMENU)lparam;

			MENUITEMINFO mii={0};
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_DATA|MIIM_ID;

			if(GetMenuItemInfo(hMenu,itemIdx,true,&mii))
			{
				MenuInterface* tMenuInterface=(MenuInterface*)mii.dwItemData;

				if(tMenuInterface)
					tMenuInterface->OnMenuPressed(mii.wID);
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
	TabWin32* tabWin32=(TabWin32*)GetWindowLongPtr(hwnd,GWLP_USERDATA);
	Tab*	  tab=(Tab*)tabWin32;

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
				tab->OnPaint();*/
			return 1;
		case WM_SIZE:
			tabWin32->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));
			tab->OnGuiSize();
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&Tab::OnGuiSize,tab,(void*)0)));
		break;
		case WM_WINDOWPOSCHANGED:
			tabWin32->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));
			tab->OnWindowPosChanging();
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&Tab::OnWindowPosChanging,tab,(void*)0)));
		break;
		case WM_LBUTTONDOWN:
			{
				tabWin32->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
				//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));

				InputManager::mouseInput.left=true;

				unsigned int tOldTime=InputManager::mouseInput.lastLeft;
				InputManager::mouseInput.lastLeft=Ide::GetInstance()->timer->GetCurrent();

				if(tabWin32->windowDataWin32->hwnd!=::GetFocus())
					::SetFocus(tabWin32->windowDataWin32->hwnd);

				if(InputManager::mouseInput.lastLeft-tOldTime<1000/6.0f)
					tab->OnGuiDLMouseDown();
					//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&Tab::OnGuiDLMouseDown,tab,(void*)0)));
				else
					tab->OnGuiLMouseDown();
					//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&Tab::OnGuiLMouseDown,tab,(void*)0)));
			}
		break;
		case WM_MOUSEWHEEL:
			{
				tabWin32->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
				//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));
				float wheelValue=GET_WHEEL_DELTA_WPARAM(wparam)>0 ? 1.0f : (GET_WHEEL_DELTA_WPARAM(wparam)<0 ? -1.0f : 0);
				tab->OnGuiMouseWheel(&wheelValue);
				//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&Tab::OnGuiMouseWheel,tab,(void*)&wheelValue)));
			}
		break;
		case WM_MOUSEMOVE:
			tabWin32->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));
			tab->OnGuiMouseMove();
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&Tab::OnGuiMouseMove,tab,(void*)0)));
		break;
		case WM_NCHITTEST:
			if(tabWin32->splitterContainer->floatingTabRef)
				return HTTRANSPARENT;
			else
				result=DefWindowProc(hwnd,msg,wparam,lparam);
			break;
		case WM_LBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			tabWin32->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));
			InputManager::mouseInput.left=false;
			tab->OnGuiLMouseUp();
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&Tab::OnGuiLMouseUp,tab,(void*)0)));
		break;
		case WM_RBUTTONUP:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			tabWin32->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));
			tab->OnGuiRMouseUp();
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&Tab::OnGuiRMouseUp,tab,(void*)0)));
		break;
		case WM_CLOSE:
			SAFEDELETE(tabWin32);
			DestroyWindow(hwnd);
			return 0;
		break;
		case WM_KEYDOWN:
			{
				result=DefWindowProc(hwnd,msg,wparam,lparam);
				tabWin32->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
				//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));
				MSG _msg;
				PeekMessage(&_msg, NULL, 0, 0, PM_NOREMOVE);

				if(_msg.message==WM_CHAR)
				{
					TranslateMessage(&_msg);
					DefWindowProc(_msg.hwnd,_msg.message,_msg.wParam,_msg.lParam);
					tab->OnGuiKeyDown(&_msg.wParam);
					//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&Tab::OnGuiKeyDown,tab,(void*)&_msg.wParam)));
				}
				else
					tab->OnGuiKeyDown();
					//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&Tab::OnGuiKeyDown,tab,(void*)0)));
			}
			break;
		case WM_PAINT:
		{
			tabWin32->windowDataWin32->CopyProcedureData(hwnd,msg,wparam,lparam);
			//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			//tab->OnGuiPaint();
			EndPaint(hwnd,&ps);
			tab->SetDraw();
			result=0;
		}
		break;
		default:
			{
				result=DefWindowProc(hwnd,msg,wparam,lparam);
				/*if(tab)
					tab->Draw();*/
			}
			

	}

	if(tabWin32)
	{
		//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));
		tabWin32->windowDataWin32->msg=0;
		tabWin32->windowDataWin32->wparam;
		tabWin32->windowDataWin32->lparam=0;
	}

	if(tab && tab->concurrentInstances.size())
	{
		for(std::list< std::function<void()> >::iterator iterConcurrentInstances=tab->concurrentInstances.begin();iterConcurrentInstances!=tab->concurrentInstances.end();)
		{
			(*iterConcurrentInstances)();
			iterConcurrentInstances=tab->concurrentInstances.erase(iterConcurrentInstances);
		}
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

void WindowDataWin32::Enable(bool iValue)
{
	::EnableWindow(this->hwnd,iValue);
}
bool WindowDataWin32::IsEnabled()
{
	return ::IsWindowEnabled(this->hwnd);
}

vec2 WindowDataWin32::Size()
{
	RECT tRect;
	vec2 tSize;
	
	::GetWindowRect(this->hwnd,&tRect);

	tSize.make(tRect.right-tRect.left,tRect.bottom-tRect.top);

	return tSize;
}

vec2 WindowDataWin32::Pos()
{
	RECT tRect;

	::GetWindowRect(this->hwnd,&tRect);

	return vec2(tRect.left,tRect.top);
}

void WindowDataWin32::Show(bool iShow)
{
	::ShowWindow(this->hwnd,iShow ? SW_SHOWNORMAL : SW_HIDE);
}

bool WindowDataWin32::IsVisible()
{
	return ::IsWindowVisible(this->hwnd);
}

void WindowDataWin32::Resize(float iWidth,float iHeight)
{
	::SetWindowPos(this->hwnd,0,0,0,iWidth,iHeight,SWP_NOREPOSITION|SWP_NOMOVE);
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
	vec2 iThisWSize=this->Size();

	for(int i=0;i<4;i++)
	{
		for(std::list<WindowData*>::iterator it=this->siblings[i].begin();it!=this->siblings[i].end();it++)
		{
			WindowDataWin32* window=static_cast<WindowDataWin32*>(*it);

			vec2 iWSize=window->Size();

			if(iWSize.x==iThisWSize.x || iWSize.y==iThisWSize.y)
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



IdeWin32::IdeWin32():
processThreadHandle(0),
processHandle(0),
projectDirHasChanged(false)
{
	HRESULT result;

	{
		this->processHandle=::GetCurrentProcess();
		this->processId=(unsigned int)::GetProcessId(this->processHandle);
		this->processThreadHandle=::GetCurrentThread();
		this->processThreadId=(unsigned int)::GetCurrentThreadId();

		wprintf(L"Engine: process id is %d\n",this->processId);
		wprintf(L"Engine: process main thread id is %d\n",this->processThreadId);
	}

	result=CoInitialize(0);

	if(S_OK!=result)
		DEBUG_BREAK();

	{//projectFolder
		wchar_t _pszDisplayName[MAX_PATH]=L"";

		BROWSEINFO bi={0};
		bi.pszDisplayName=_pszDisplayName;
		bi.lpszTitle=L"Select Project Directory";

		PIDLIST_ABSOLUTE tmpProjectFolder=SHBrowseForFolder(&bi);

		DWORD err=GetLastError();

		if(tmpProjectFolder)
		{
			wchar_t path[MAX_PATH];

			if(SHGetPathFromIDList(tmpProjectFolder,path))
			{
				this->folderProject=path;
				wprintf(L"User project folder: %s\n",this->folderProject.c_str());
			}
		}
	}

	{//exeFolder
		wchar_t ch[5000];
		if(!GetModuleFileName(0,ch,5000))
			DEBUG_BREAK();

		this->pathExecutable=ch;
		this->folderPlugins=this->pathExecutable.Path() + L"\\plugins";

		String pathExecutablePath=this->pathExecutable.Path();

		if(!SetDllDirectory(pathExecutablePath.c_str()))
			wprintf(L"failed to add dll search path\n");

		wprintf(L"Application folder: %s\n",pathExecutablePath.c_str());
	}

	this->subsystem=new SubsystemWin32;
	this->compiler=new Compiler;

	this->debugger=new DebuggerWin32;

	{//applicationDataFolder
		wchar_t ch[5000];
		if(S_OK!=SHGetFolderPath(0,CSIDL_APPDATA,0,SHGFP_TYPE_CURRENT,ch))
			DEBUG_BREAK();

		this->folderAppData=String(ch) + L"\\EngineAppData";

		if(!PathFileExists(this->folderAppData.c_str()))
		{
			SECURITY_ATTRIBUTES sa;
			CreateDirectory(this->folderAppData.c_str(),&sa);
		}
		else
		{
			this->subsystem->Execute(this->folderAppData,L"del /F /Q *.*");
			this->subsystem->Execute(this->folderAppData,L"FOR /D %p IN (*.*) DO rmdir \"%p\" /s /q");
		}

		wprintf(L"Application data folder: %s\n",this->folderAppData.c_str());
	}

	Direct2D::Init();

	int error=ERROR_SUCCESS;

	InitSplitter();

	this->stringEditor=new StringEditor;

	{
		this->timer=new TimerWin32;

		this->timerThread=new ThreadWin32;
		this->timerThread->NewTask(std::function<void()>(std::bind(&Timer::update,this->timer)),false);
	}

	this->projectDirChangedThread=new ThreadWin32;
	this->projectDirChangedThread->NewTask(std::function<void()>(std::bind(&Ide::ProjectDirHasChangedFunc,this)),false);

	this->mainAppWindow=new MainContainerWin32;

	this->mainAppWindow->Initialize();

	this->pluginSystem=new PluginSystemWin32;

	this->pluginSystem->ScanPluginsDirectory();

	wprintf(L"sizeof(wchar_t): %d,sizeof(bool): %d",sizeof(wchar_t),sizeof(bool));
}

IdeWin32::~IdeWin32()
{
	SAFEDELETE(this->mainAppWindow);
	SAFEDELETE(this->subsystem);
	SAFEDELETE(this->compiler);
	SAFEDELETE(this->debugger);

	SetDllDirectory(0);
	Direct2D::Release();
	CoUninitialize();
}


void IdeWin32::ScanDir(String iDirectory,ResourceNodeDir* iParent)
{
	HANDLE			tHandle;
	WIN32_FIND_DATA tData;

	String tScanDir=iDirectory + L"\\*";

	tHandle=FindFirstFile(tScanDir.c_str(),&tData); //. dir

	if(!tHandle || INVALID_HANDLE_VALUE == tHandle)
	{
		DEBUG_BREAK();
		return;
	}
	else
		FindNextFile(tHandle,&tData);

	int tEngineExtensionCharSize=Ide::GetInstance()->GetEntityExtension().size();

	File tFile;

	bool tSelectedLeft;
	bool tSelectedRight;
	bool tIsDir;
	bool tExpanded;

	const unsigned char _true=1;
	const unsigned char _false=0;

	bool tCreateNode;

	String tCreateNodeFilename;

	while(FindNextFile(tHandle,&tData))
	{
		if(tData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			continue;

		tCreateNode=false;

		if(wcsstr(tData.cFileName,Ide::GetInstance()->GetEntityExtension().c_str()))
		{
			//if filename not exists delete it

			String tOriginalFullFileName=iDirectory + L"\\" + String(tData.cFileName,wcslen(tData.cFileName)-tEngineExtensionCharSize);
			String tEngineFileName=iDirectory + L"\\" + String(tData.cFileName);

			DWORD tAttr=::GetFileAttributes(tOriginalFullFileName.c_str());

			if(tAttr == INVALID_FILE_ATTRIBUTES)
			{
				File::Delete(tEngineFileName.c_str());
				continue;
			}

			tFile=tEngineFileName;

			if(tFile.Open(L"r"))
			{
				fread(&tSelectedLeft,1,1,tFile);
				fread(&tSelectedRight,1,1,tFile);
				fread(&tIsDir,1,1,tFile);
				fread(&tExpanded,1,1,tFile);

				tFile.Close();
			}

			tCreateNode=true;
			tCreateNodeFilename=String(tData.cFileName,wcslen(tData.cFileName)-tEngineExtensionCharSize);
		}
		else
		{
			String engineFile = iDirectory + L"\\" + String(tData.cFileName) + Ide::GetInstance()->GetEntityExtension();

			if(!PathFileExists(engineFile.c_str()))
			{
				tSelectedLeft=false;
				tSelectedRight=false;
				tIsDir=(tData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;
				tExpanded=false;

				tFile=engineFile;

				if(tFile.Open(L"w"))
				{
					fwrite(&_false,1,1,tFile);//selectedLeft
					fwrite(&_false,1,1,tFile);//selectedRight
					fwrite(tIsDir? &_true : &_false,1,1,tFile);//isDir
					fwrite(&_false,1,1,tFile);//expanded
					
					tFile.Close();
				}

				DWORD fileAttribute=GetFileAttributes(engineFile.c_str());
				SetFileAttributes(engineFile.c_str(),FILE_ATTRIBUTE_HIDDEN|fileAttribute);

				tCreateNode=true;
				tCreateNodeFilename=tData.cFileName;
			}
		}

		if(tCreateNode)
		{
			if(tIsDir)
			{
				ResourceNodeDir* dirNode=new ResourceNodeDir;

				iParent->dirs.push_back(dirNode);
				dirNode->parent=iParent;
				dirNode->fileName=tCreateNodeFilename;
				dirNode->level = iParent ? iParent->level+1 : 0;
				dirNode->selectedLeft=tSelectedLeft;
				dirNode->selectedRight=tSelectedRight;
				dirNode->expanded=tExpanded;
				dirNode->isDir=tIsDir;

				ResourceNodeDir* dirNodeParent=(ResourceNodeDir*)dirNode->parent;

				dirNode->directoryViewerRow.SetStringMode(dirNode->fileName,true);
				dirNode->directoryViewerRow.rowData=dirNode;
				dirNodeParent->directoryViewerRow.Insert(&dirNode->directoryViewerRow);

				this->ScanDir(iDirectory + L"\\"+ tCreateNodeFilename,dirNode);
			}
			else
			{
				ResourceNode* fileNode=new ResourceNode;

				iParent->files.push_back(fileNode);
				fileNode->parent=iParent;
				fileNode->fileName=tCreateNodeFilename;
				fileNode->level = iParent ? iParent->level+1 : 0;
				fileNode->selectedLeft=tSelectedLeft;
				fileNode->selectedRight=tSelectedRight;
				fileNode->isDir=tIsDir;
			}
		}
	}

	FindClose(tHandle);
	tHandle=0;
}


void IdeWin32::Run()
{
	MSG msg;

	while(true)
	{
		if(GetMessage(&msg,0,0,0)!=WM_QUIT)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(this->projectDirHasChanged)
		{
			this->projectDirHasChanged=false;

			Tab* tTab=GuiProjectViewer::GetInstance()->GetRootRect()->tab;

			tTab->DrawBlock(true);

			GuiEvent tChangeDirEvent(tTab,0);

			GuiProjectViewer::GetInstance()->OnDeactivate(tChangeDirEvent);
			GuiProjectViewer::GetInstance()->OnActivate(tChangeDirEvent);

			tTab->DrawBlock(false);

			tTab->SetDraw();
		}
	}
}

void IdeWin32::Sleep(int iMilliseconds)
{
	::Sleep(iMilliseconds);
}

void IdeWin32::ProjectDirHasChangedFunc()
{
	HANDLE tProjectDirChangeHandle=FindFirstChangeNotification(this->folderProject.c_str(),true,
		FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_ATTRIBUTES|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_SECURITY);
	DWORD tProjectDirectoryIsChanged=WaitForSingleObject(tProjectDirChangeHandle,INFINITE); 

	this->projectDirHasChanged = WAIT_OBJECT_0==tProjectDirectoryIsChanged ? true : false;

	FindCloseChangeNotification(tProjectDirChangeHandle);
}


///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////TimerWin32//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void TimerWin32::update()
{
	lastFrameTime=currentFrameTime;
	currentFrameTime=timeGetTime();
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

		wprintf(L"OPENGL ERROR %d, HGLRC: %p, HDC: %p\n",err,currentContext,currentContextDC);

		DEBUG_BREAK();
	}
}






///////////////////////////////////////////////
///////////////////////////////////////////////
//////////////////ShaderOpenGL//////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

int Renderer3DOpenGL::CreateShader(const char* name,int shader_type, const char* shader_src)
{
	GLint compile_success = 0;
	GLchar message[1024];
	int len=0;
	int shader_id;

	shader_id = glCreateShader(shader_type);glCheckError();

	if(!shader_id)
	{
		wprintf(L"glCreateShader error for %s,%s\n",shader_type,shader_src);glCheckError();
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
		MessageBoxA(0,message,"Engine",MB_OK|MB_ICONEXCLAMATION);
		DEBUG_BREAK();
	}

	return shader_id;
}


Shader* Renderer3DOpenGL::CreateProgram(const char* name,const char* vertexsh,const char* fragmentsh)
{
	GLint tLinkSuccess=0;
	GLint tProgram=0;
	GLint tVertexShader=0;
	GLint tFragmentShader=0;
	GLchar tMessage[1024]={0};
	GLint tLength=0;

	tProgram = glCreateProgram();glCheckError();

	if(!tProgram)
	{
		wprintf(L"glCreateProgram error for %s,%s\n",vertexsh,fragmentsh);
		DEBUG_BREAK();
		return 0;
	}

	tVertexShader=CreateShader(name,GL_VERTEX_SHADER, vertexsh);
	tFragmentShader=CreateShader(name,GL_FRAGMENT_SHADER, fragmentsh);

	glAttachShader(tProgram, tVertexShader);glCheckError();
	glAttachShader(tProgram, tFragmentShader);glCheckError();
	glLinkProgram(tProgram);glCheckError();
	glGetProgramiv(tProgram, GL_LINK_STATUS, &tLinkSuccess);glCheckError();

	if (GL_FALSE==tLinkSuccess)
	{
		wprintf(L"glLinkProgram error for %s\n",tMessage);
		DEBUG_BREAK();
	}

	glGetProgramInfoLog(tProgram,sizeof(tMessage),&tLength,tMessage);glCheckError();

	glDetachShader(tProgram,tVertexShader);
	glDetachShader(tProgram,tFragmentShader);

	glDeleteShader(tVertexShader);
	glDeleteShader(tFragmentShader);

	ShaderOpenGL* ___shader=0;

	if(tProgram && GL_FALSE!=tLinkSuccess)
	{
		___shader=new ShaderOpenGL(this);

		___shader->name=StringUtils::ToWide(name);
		___shader->programId=tProgram;
		___shader->vertexShaderId=tVertexShader;
		___shader->fragmentShaderId=tFragmentShader;

		this->shaders.push_back(___shader);
	}

	return ___shader;
}



Shader* Renderer3DOpenGL::CreateShaderProgram(const char* name,const char* pix,const char* frag)
{
	Shader* shader=this->CreateProgram(name,pix,frag);

	if(shader)
	{
		shader->Use();
		shader->init();
	}
	else
		wprintf(L"error creating shader %s\n",name);

	return shader;
}

ShaderOpenGL::ShaderOpenGL(Renderer3DOpenGL* iRenderer):Shader(iRenderer),renderer(iRenderer){}

ShaderOpenGL::~ShaderOpenGL()
{
	renderer->glDeleteProgram(this->programId);
}



unsigned int& ShaderOpenGL::GetBufferObject()
{
	return vbo;
}

int ShaderOpenGL::GetUniform(int slot,char* var)
{
	return renderer->glGetUniformLocation(slot,var);glCheckError();
}
int ShaderOpenGL::GetAttrib(int slot,char* var)
{
	return renderer->glGetAttribLocation(slot,var);glCheckError();
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
	renderer->glUseProgram(programId);glCheckError();
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
	int location=renderer->glGetAttribLocation(programId,attrib);glCheckError();
	return location;
}

int ShaderOpenGL::GetUniform(const char* uniform)
{
	int location=renderer->glGetUniformLocation(programId,uniform);glCheckError();
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
int ShaderOpenGL::GetPointSize()
{
	return GetUniform("pointsize");
}

void ShaderOpenGL::SetSelectionColor(bool pick,void* ptr,vec2 iMpos,vec2 iRectSize)
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

			renderer->glUniform4f(_ptrclr,fcx,fcy,fcz,fcw);
		}
		else renderer->glUniform4f(_ptrclr,0,0,0,0);

		if(_mousepos>=0)
			renderer->glUniform2f(_mousepos,iMpos.x/iRectSize.x,iMpos.y/iRectSize.y);
	}
}

bool ShaderOpenGL::SetMatrix4f(int slot,float* mtx)
{
	if(slot<0)
		return false;

	renderer->glUniformMatrix4fv(slot,1,0,mtx);glCheckError();
	return true;
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


Renderer3DOpenGL::Renderer3DOpenGL(TabWin32* iTab):
	Renderer3D(iTab),
	hglrc(0),
	tab((TabWin32*&)iTab)
{}

Renderer3DOpenGL::~Renderer3DOpenGL()
{
	
	
}

char* Renderer3DOpenGL::Name()
{
	return 0;
}

void Renderer3DOpenGL::Deinitialize()
{
	this->ChangeContext();

	SAFEDELETE(this->shader_font);
	SAFEDELETE(this->shader_shaded_texture);
	SAFEDELETE(this->shader_unlit);
	SAFEDELETE(this->shader_unlit_color);
	SAFEDELETE(this->shader_unlit_texture);

	glDeleteFramebuffers(1,&this->frameBuffer);
	glDeleteTextures(1,&this->textureColorbuffer);
	glDeleteTextures(1,&this->textureRenderbuffer);
	glDeleteRenderbuffers(1,&this->renderBufferColor);
	glDeleteRenderbuffers(1,&this->renderBufferDepth);

	glDeleteVertexArrays(1,&this->vertexArrayObject);

	glDeleteBuffers(1,&this->vertexBufferObject);
	glDeleteBuffers(1,&this->textureBufferObject);
	glDeleteBuffers(1,&this->pixelBuffer);

	wglMakeCurrent(this->hdc,0);
	
	wglDeleteContext(this->hglrc);
	ReleaseDC(this->tab->windowDataWin32->hwnd,this->hdc);
}

void Renderer3DOpenGL::Initialize()
{
	this->hdc=GetDC(this->tab->windowDataWin32->hwnd);

	RECT r;
	GetClientRect(this->tab->windowDataWin32->hwnd,&r);

	int width=(int)(r.right-r.left);
	int height=(int)(r.bottom-r.top);

	DWORD error=0;

	if(!hdc)
		MessageBox(0,L"Getting Device Context",L"GetDC",MB_OK|MB_ICONEXCLAMATION);

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

	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATEXTPROC) wglGetProcAddress("wglChoosePixelFormatARB");
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");

	glAttachShader = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
	glBindBuffer = (PFNGLBINDBUFFERPROC) wglGetProcAddress("glBindBuffer");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC) wglGetProcAddress("glBindVertexArray");
	glBufferData = (PFNGLBUFFERDATAPROC) wglGetProcAddress("glBufferData");
	glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
	glCreateShader = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC) wglGetProcAddress("glDeleteBuffers");
	glDeleteProgram = (PFNGLDELETEPROGRAMPROC) wglGetProcAddress("glDeleteProgram");
	glDeleteShader = (PFNGLDELETESHADERPROC) wglGetProcAddress("glDeleteShader");
	glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC) wglGetProcAddress("glDeleteFramebuffers");
	glDeleteRenderbuffers = (PFNGLDELETERENDERBUFFERSPROC) wglGetProcAddress("glDeleteRenderbuffers");
	glDeleteVertexArrays = (PFNGLDELETEVERTEXARRAYSPROC) wglGetProcAddress("glDeleteVertexArrays");
	glDetachShader = (PFNGLDETACHSHADERPROC) wglGetProcAddress("glDetachShader");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glEnableVertexAttribArray");
	glEnableVertexArrayAttrib = (PFNGLENABLEVERTEXARRAYATTRIBPROC) wglGetProcAddress("glEnableVertexArrayAttrib");
	glGenBuffers = (PFNGLGENBUFFERSPROC) wglGetProcAddress("glGenBuffers");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC) wglGetProcAddress("glGenVertexArrays");
	glGetAttribLocation = (PFNGLGETATTRIBLOCATIONPROC) wglGetProcAddress("glGetAttribLocation");
	glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
	glGetProgramiv = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
	glGetShaderiv = (PFNGLGETSHADERIVPROC) wglGetProcAddress("glGetShaderiv");
	glLinkProgram = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
	glShaderSource = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
	glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC) wglGetProcAddress("glVertexAttribPointer");
	glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC) wglGetProcAddress("glBindAttribLocation");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC) wglGetProcAddress("glUniformMatrix4fv");
	glActiveTexture = (PFNGLACTIVETEXTUREPROC) wglGetProcAddress("glActiveTexture");
	glUniform1i = (PFNGLUNIFORM1IPROC) wglGetProcAddress("glUniform1i");
	glUniform1f = (PFNGLUNIFORM1FPROC) wglGetProcAddress("glUniform1f");
	glUniform3f = (PFNGLUNIFORM3FPROC) wglGetProcAddress("glUniform3f");
	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC) wglGetProcAddress("glGenerateMipmap");
	glDisableVertexAttribArray = (PFNGLDISABLEVERTEXATTRIBARRAYPROC) wglGetProcAddress("glDisableVertexAttribArray");
	glUniform3fv = (PFNGLUNIFORM3FVPROC) wglGetProcAddress("glUniform3fv");
	glUniform4fv = (PFNGLUNIFORM4FVPROC) wglGetProcAddress("glUniform4fv");
	glTexBuffer = (PFNGLTEXBUFFERPROC) wglGetProcAddress("glTexBuffer");
	glTextureBuffer = (PFNGLTEXTUREBUFFERPROC) wglGetProcAddress("glTextureBuffer");
	glBufferSubData = (PFNGLBUFFERSUBDATAPROC) wglGetProcAddress("glBufferSubData");
	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
	glGenRenderbuffers = (PFNGLGENRENDERBUFFERSPROC) wglGetProcAddress("glGenRenderbuffers");
	glReadnPixels = (PFNGLREADNPIXELSPROC) wglGetProcAddress("glReadnPixels");
	glUniform2f = (PFNGLUNIFORM2FPROC) wglGetProcAddress("glUniform2f");
	glUniform2fv = (PFNGLUNIFORM2FVPROC) wglGetProcAddress("glUniform2fv");
	glUniform3f = (PFNGLUNIFORM3FPROC) wglGetProcAddress("glUniform3f");
	glUniform4f = (PFNGLUNIFORM4FPROC) wglGetProcAddress("glUniform4f");
	wglGetPixelFormatAttribivARB = (PFNWGLGETPIXELFORMATATTRIBIVARBPROC) wglGetProcAddress("wglGetPixelFormatAttribivARB");

	wglMakeCurrent(this->hdc, NULL);
	wglDeleteContext(hglrc);

	ReleaseDC(this->tab->windowDataWin32->hwnd,this->hdc);
	
	const int pixelFormatAttribList[] =
	{
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_DEPTH_BITS_ARB, 32,
		WGL_STENCIL_BITS_ARB, 32,
		//WGL_SWAP_COPY_ARB,GL_TRUE,        //End
		0
	};

	this->hdc=GetDC(this->tab->windowDataWin32->hwnd);

	UINT numFormats;

	if(!wglChoosePixelFormatARB(hdc, pixelFormatAttribList, NULL, 1, &pixelFormat, &numFormats))
		MessageBox(0,L"wglChoosePixelFormatARB fails",L"Engine",MB_OK|MB_ICONEXCLAMATION);


	const int versionAttribList[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB,1,
		WGL_CONTEXT_MINOR_VERSION_ARB,0,
		0,        //End
	};

	if(!(hglrc = wglCreateContextAttribsARB(hdc, 0, versionAttribList)))
		MessageBox(0,L"wglCreateContextAttribsARB fails",L"Engine",MB_OK|MB_ICONEXCLAMATION);

	if(hglrc)
		wprintf(L"TABCONTAINER: %p, HGLRC: %p, HDC: %p\n",this->tab,hglrc,hdc);

	if(!wglMakeCurrent(hdc,hglrc))
		DEBUG_BREAK();

	/*if(!vertexArrayObject)
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
		/ *
		//glGenBuffers(1,&indicesBufferObject);

		glBindBuffer(GL_ARRAY_BUFFER,vertexBufferObject);
		//glBindBuffer(GL_ARRAY_BUFFER,indicesBufferObject);

		glBufferData(GL_ARRAY_BUFFER,100000,0,GL_DYNAMIC_DRAW);* /


		glGenBuffers(1, &pixelBuffer);
	}*/

	wprintf(L"Status: Using GL %s\n", StringUtils::ToWide((const char*)glGetString(GL_VERSION)).c_str());
	wprintf(L"Status: GLSL ver %s\n",StringUtils::ToWide((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)).c_str());

	this->shader_unlit=this->CreateShaderProgram("unlit",unlit_vert,unlit_frag);
	this->shader_unlit_color=this->CreateShaderProgram("unlit_color",unlit_color_vert,unlit_color_frag);
	this->shader_unlit_texture=this->CreateShaderProgram("unlit_texture",unlit_texture_vs,unlit_texture_fs);
	this->shader_font=this->CreateShaderProgram("font",font_pixsh,font_frgsh);
	this->shader_shaded_texture=this->CreateShaderProgram("shaded_texture",texture_vertex_shaded_vert,texture_vertex_shaded_frag);
}


void Renderer3DOpenGL::ChangeContext()
{
	if(!this->hglrc || !this->hdc)
	{
		DEBUG_BREAK();
		return;
	}

	if(this->hglrc != wglGetCurrentContext() || this->hdc!=wglGetCurrentDC())
	{
		if(!wglMakeCurrent(this->hdc,this->hglrc))
		{
			glCheckError();
			DEBUG_BREAK();
		}
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
	Shader* shader=this->shader_unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	shader->SetSelectionColor(this->picking,0,this->tab->mouse,this->tab->Size());

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_PROGRAM_POINT_SIZE);
	/*glPointSize(psize);*/

	int ps=shader->GetPositionSlot();
	int uniform_color=shader->GetUniform("color");
	int tPointSize=shader->GetPointSize();

	if(tPointSize)
		glUniform1f(tPointSize,psize);

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
	Shader* shader=this->shader_unlit_color;

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
	Shader* shader=this->shader_unlit_color;

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
	Shader* shader=this->shader_unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	shader->SetSelectionColor(false,0,this->tab->mouse,this->tab->Size());

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
	Shader* shader=this->shader_unlit_color;

	if(!shader)
		return;

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2],
	};

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelMatrix());

	shader->SetSelectionColor(false,0,this->tab->mouse,this->tab->Size());

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
	Shader* shader = mesh->materials.size() ? this->shader_unlit_texture : this->shader_unlit_color;

	if(!shader || !mesh)
		return;

	vec3 lightpos(0,200,-100);

	if(shader==this->shader_shaded_texture)
		this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(mesh->isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CCW);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),mesh->entity->world);


	shader->SetSelectionColor(this->picking,mesh->entity,this->tab->mouse,this->tab->Size());

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
	Shader* shader = skin->materials.size() ? this->shader_unlit_texture : this->shader_unlit_color;

	if(!skin || !skin->vertexcache || !shader)
	{
		this->drawUnlitTextured(skin);
		return;
	}

	vec3 lightpos(0,200,-100);

	if(shader==this->shader_shaded_texture)
		this->draw(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(skin->isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CW);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),skin->entity->local);


	shader->SetSelectionColor(this->picking,skin->entity,this->tab->mouse,this->tab->Size());

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
	Shader* shader=this->shader_unlit_color;

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

	shader->SetSelectionColor(this->picking,bone->entity,this->tab->mouse,this->tab->Size());

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

	/ *vec3 ndc(2.0f * tabContainer->mouse.x / this->width -1.0f,-2.0f * (tabContainer->mouse.y-TabContainer::CONTAINER_HEIGHT) / this->height + 1.0f,0);
	mat4 viewInv=MatrixStack::view.inverse();
	rayStart=viewInv.transform(ndc.x*calcRatioX,ndc.y*calcRatioY,-1);
	rayEnd=viewInv.transform(ndc.x*calcRatioX*this->RendererViewportInterface_farPlane,ndc.y*calcRatioY*this->RendererViewportInterface_farPlane,-this->RendererViewportInterface_farPlane+1);* /
* /

}*/
///////////////////////////////////////////////
///////////////////////////////////////////////
///////////////////GuiImage///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void Picture::Release()
{
	SAFERELEASE((ID2D1Bitmap*&)this->handle);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////////TabWin32///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

void TabWin32::Destroy()
{
	PostMessage(this->windowDataWin32->hwnd,WM_CLOSE,0,0);
}

TabWin32::~TabWin32()
{
	SetWindowLongPtr(this->windowDataWin32->hwnd,GWLP_USERDATA,(LONG_PTR)0);

	if(!this->windowDataWin32->FindAndGrowSibling())
		this->windowDataWin32->UnlinkSibling();

	SAFEDELETE(this->iconDown);
	SAFEDELETE(this->iconUp);
	SAFEDELETE(this->iconLeft);
	SAFEDELETE(this->iconRight);
	SAFEDELETE(this->iconFolder);
	SAFEDELETE(this->iconFile);

	//destroy the tab draw task

	this->threadRenderWin32->Block(true);

	for(std::list<DrawInstance*>::iterator it=this->drawInstances.begin();it!=this->drawInstances.end();it++)
		SAFEDELETE(*it);

	this->threadRenderWin32->Block(false);

	
	this->threadRenderWin32->DestroyTask(this->drawTask);

	this->Destroy3DRenderer();

	SAFEDELETE(this->renderer3DOpenGL);
	SAFEDELETE(this->threadRenderWin32);
	SAFEDELETE(this->renderer2DWin32);
	SAFEDELETE(this->windowDataWin32);
}

void TabWin32::Create3DRenderer()
{
#if ENABLE_RENDERER
	if(!this->renderer3D)
	{
		this->renderer3D=new Renderer3DOpenGL(this);

		if(!this->renderer3D)
			DEBUG_BREAK();

		Task* tCreateOpenGLContext=this->thread->NewTask(std::function<void()>(std::bind(&Renderer3DOpenGL::Initialize,this->renderer3DOpenGL)));
		while(tCreateOpenGLContext->func);
		SAFEDELETE(tCreateOpenGLContext);
	}
#endif
}

void TabWin32::Destroy3DRenderer()
{
#if ENABLE_RENDERER
	if(this->renderer3D)
	{
		Task* tDeleteOpenGLContext=this->thread->NewTask(std::function<void()>(std::bind(&Renderer3DOpenGL::Deinitialize,this->renderer3DOpenGL)));
		while(tDeleteOpenGLContext->func);
		SAFEDELETE(tDeleteOpenGLContext);
	}
#endif

}

TabWin32::TabWin32(float iX,float iY,float iW,float iH,HWND iParentWindow,bool iModal)
	:Tab(iX,iY,iW,iH),
	windowDataWin32((WindowDataWin32*&)windowData),
	containerWin32((ContainerWin32*&)container),
	renderer2DWin32((Renderer2DWin32*&)renderer2D),
	renderer3DOpenGL((Renderer3DOpenGL*&)renderer3D),
	threadRenderWin32((ThreadWin32*&)thread)
{
	this->isModal=iModal;

	windowData=new WindowDataWin32;

	DWORD tStyle=WS_CLIPCHILDREN|WS_CLIPSIBLINGS;

	if(!this->isModal)
		tStyle=tStyle|WS_CHILD;

	this->windowDataWin32->hwnd=CreateWindow(WC_TABCONTAINERWINDOWCLASS,WC_TABCONTAINERWINDOWCLASS,tStyle,(int)iX,(int)iY,(int)iW,(int)iH,iParentWindow,0,0,this);

	if(!this->windowDataWin32->hwnd)
		DEBUG_BREAK();

	wprintf(L"Tab size(%d,%d,%d,%d)\n",(int)iX,(int)iY,(int)iW,(int)iH);

	this->thread=new ThreadWin32;

	this->drawTask=this->thread->NewTask(std::function<void()>(std::bind(&Tab::Draw,this)),false);

	this->iconDown=new PictureRef(Tab::rawDownArrow,Tab::ICON_WH,Tab::ICON_WH);
	this->iconUp=new PictureRef(Tab::rawUpArrow,Tab::ICON_WH,Tab::ICON_WH);
	this->iconLeft=new PictureRef(Tab::rawLeftArrow,Tab::ICON_WH,Tab::ICON_WH);
	this->iconRight=new PictureRef(Tab::rawRightArrow,Tab::ICON_WH,Tab::ICON_WH);
	this->iconFolder=new PictureRef(Tab::rawFolder,Tab::ICON_WH,Tab::ICON_WH);
	this->iconFile=new PictureRef(Tab::rawFile,Tab::ICON_WH,Tab::ICON_WH);

	this->renderer2D=this->renderer2DWin32=new Renderer2DWin32(this,this->windowDataWin32->hwnd);

	this->OnGuiRecreateTarget();

	this->container=(Container*)GetWindowLongPtr(iParentWindow,GWLP_USERDATA);
	this->container->tabs.push_back(this);

	splitterContainer=this->container->splitter;

	if(!splitterContainer)
		DEBUG_BREAK();

	ShowWindow(this->windowDataWin32->hwnd,true);
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

	InsertMenu(root,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)create,L"Viewers");
	{
		InsertMenu(create,0,MF_BYPOSITION|MF_STRING,2,L"Viewport");
		InsertMenu(create,1,MF_BYPOSITION|MF_STRING,3,L"Scene");
		InsertMenu(create,2,MF_BYPOSITION|MF_STRING,4,L"Entity");
		InsertMenu(create,3,MF_BYPOSITION|MF_STRING,5,L"Project");
		//InsertMenu(create,4,MF_BYPOSITION|MF_STRING,6,"Script");
	}
	InsertMenu(root,1,MF_BYPOSITION|MF_SEPARATOR,0,0);
	InsertMenu(root,2,MF_BYPOSITION|MF_STRING,1,L"Remove");

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
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,1,L"New Entity");
		InsertMenu(menu,1,MF_BYPOSITION|MF_STRING,2,L"Delete");
		InsertMenu(menu,2,MF_BYPOSITION|MF_POPUP,(UINT_PTR)createComponent,L"Component");
		{
			InsertMenu(createComponent,0,MF_BYPOSITION|MF_STRING,3,L"Light");
			InsertMenu(createComponent,1,MF_BYPOSITION|MF_POPUP,(UINT_PTR)createMesh,L"Mesh");
			{
			}
			InsertMenu(createComponent,2,MF_BYPOSITION|MF_STRING,5,L"Camera");
			InsertMenu(createComponent,3,MF_BYPOSITION|MF_STRING,14,L"Script");
		}
	}
	else
	{
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,1,L"New Entity");
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
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,FileViewerActions::Delete,L"Delete");

		if(iResourceNode->fileName.PointedExtension() == Ide::GetInstance()->GetSceneExtension())
			InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,FileViewerActions::Load,L"Load");
	}
	else
	{
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,FileViewerActions::Create,L"Create");
	}

	RECT rc;
	GetWindowRect(windowDataWin32->hwnd,&rc);

	int result=TrackPopupMenu(menu,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(windowDataWin32->lparam),rc.top+HIWORD(windowDataWin32->lparam),0,GetParent(windowDataWin32->hwnd),0);

	DestroyMenu(menu);

	return result;
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
			vec2 tTabSize=this->Size();

			HRESULT result=this->renderer2DWin32->renderer->Resize(D2D1::SizeU(tTabSize.x,tTabSize.y));

			if(S_OK!=result)
				DEBUG_BREAK();
		}

		this->renderer2DWin32->renderer->BeginDraw();
		this->isRender=true;

		if(this->resizeTarget)
		{
			this->PaintBackground();
			this->resizeTarget=0;
		}

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
		HRESULT result=renderer2DWin32->renderer->EndDraw();

		this->recreateTarget=(result==D2DERR_RECREATE_TARGET);

		if(result!=0)
		{
			DEBUG_BREAK();

			wprintf(L"D2D1HwndRenderTarget::EndDraw error: %x\n",result);

			result=renderer2DWin32->renderer->Flush();

			if(result!=0)
				wprintf(L"D2D1HwndRenderTarget::Flush error: %x\n",result);
		}

		this->isRender=false;
	}
	else
		DEBUG_BREAK();
}





void TabWin32::OnGuiMouseMove(void* data)
{
	this->mouse.x=(float)LOWORD(this->windowDataWin32->lparam);
	this->mouse.y=(float)HIWORD(this->windowDataWin32->lparam);

	Tab::OnGuiMouseMove(data);
}


void TabWin32::OnGuiMouseWheel(void* data)
{
	float factor=GET_WHEEL_DELTA_WPARAM(this->windowDataWin32->wparam)>0 ? 1.0f : (GET_WHEEL_DELTA_WPARAM(this->windowDataWin32->wparam)<0 ? -1.0f : 0);

	if(!this->hasFrame || this->mouse.y>Tab::BAR_HEIGHT)
		this->BroadcastToSelected(&GuiRect::OnMouseWheel,&factor);
}



void TabWin32::OnGuiRMouseUp(void* data)
{
	if(this->hasFrame && this->mouse.y<=Tab::BAR_HEIGHT)
	{
		float &x=this->mouse.x;
		float &y=this->mouse.y;

		int tabNumberHasChanged=this->rects.childs.size();

		for(int i=0;i<(int)rects.childs.size();i++)
		{
			if(x>(i*LABEL_WIDTH) && x< (i*LABEL_WIDTH+LABEL_WIDTH) && y > (BAR_HEIGHT-LABEL_HEIGHT) &&  y<BAR_HEIGHT)
			{

				int menuResult=this->TrackTabMenuPopup();

				switch(menuResult)
				{
					case 1:
						if(this->container->tabs.size()>1)
						{
							wprintf(L"total TabContainer before destroying: %d\n",this->container->tabs.size());
							this->Destroy();
							wprintf(L"total TabContainer after destroying: %d\n",this->container->tabs.size());
						}
					break;
					case 2:
						this->CreateViewer<GuiViewport>();
						this->selected=this->rects.childs.size()-1;
					break;
					case 3:
						this->CreateSingletonViewer<GuiSceneViewer>();
						this->selected=this->rects.childs.size()-1;
					break;
					case 4:
						this->CreateViewer<GuiEntityViewer>();
						this->selected=this->rects.childs.size()-1;
					break;
					case 5:
						this->CreateSingletonViewer<GuiProjectViewer>();
						this->selected=this->rects.childs.size()-1;
					break;
					case 6:
						/*this->tabs.ScriptViewer();
						this->selected=this->tabs.childs.size()-1;*/
					break;
				}

				break;
			}
		}

		if(tabNumberHasChanged!=this->rects.childs.size())
			this->SetDraw(0,true);
	}
	else
	{
		this->BroadcastToSelected(&GuiRect::OnRMouseUp,this->mouse);
	}
}






void TabWin32::OnResizeContainer(void* data)
{
	RECT rc;

	GetClientRect(this->windowDataWin32->hwnd,&rc);
	MapWindowRect(this->windowDataWin32->hwnd,this->containerWin32->windowDataWin32->hwnd,&rc);

	int size=0;
	int diff=0;
	int side=0;


	switch(this->container->resizeEnumType)
	{
		case 0:
			side=(rc.right-rc.left);
			diff=this->container->resizeDiffWidth;
			size=side+diff;
			SetWindowPos(this->windowDataWin32->hwnd,0,0,rc.top,size,rc.bottom-rc.top,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER);
			this->container->resizeCheckWidth+=size;
			break;
		case 1:
			side=rc.bottom;
			diff=this->container->resizeDiffHeight;
			size=side+diff;
			SetWindowPos(this->windowDataWin32->hwnd,0,rc.left,rc.top,rc.right-rc.left,size,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER);
			this->container->resizeCheckHeight+=size;
			break;
		case 2:
			side=(rc.right-rc.left);
			diff=this->container->resizeDiffWidth;
			size=side+diff;
			SetWindowPos(this->windowDataWin32->hwnd,0,rc.left,rc.top,size,rc.bottom-rc.top,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE);
			this->container->resizeCheckWidth+=size;
			break;
		case 3:
			side=rc.bottom;
			diff=this->container->resizeDiffHeight;
			size=side+diff;
			SetWindowPos(this->windowDataWin32->hwnd,0,rc.left,rc.top,rc.right-rc.left,size,/*SWP_NOREDRAW|*/SWP_NOZORDER|SWP_NOOWNERZORDER|SWP_NOMOVE);
			this->container->resizeCheckHeight+=size;
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
	this->iconLeft->Release();
	this->iconDown->Release();
	this->iconFolder->Release();
	this->iconFile->Release();

	if(!this->renderer2D->LoadBitmap(this->iconUp))
		DEBUG_BREAK();
	if(!this->renderer2D->LoadBitmap(this->iconRight))
		DEBUG_BREAK();
	if(!this->renderer2D->LoadBitmap(this->iconLeft))
		DEBUG_BREAK();
	if(!this->renderer2D->LoadBitmap(this->iconDown))
		DEBUG_BREAK();
	if(!this->renderer2D->LoadBitmap(this->iconFolder))
		DEBUG_BREAK();
	if(!this->renderer2D->LoadBitmap(this->iconFile))
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
////////////////GuiViewportWin32///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

extern std::list<GuiViewport*> globalViewports;

GuiViewport::~GuiViewport()
{
	globalViewports.remove(this);

	SAFERELEASE((ID2D1Bitmap*&)this->renderBitmap);
}

void GuiViewport::DrawBuffer(Tab* iTab,vec4& iVec)
{
	if(this->renderBuffer)
	{
		Renderer2DWin32* renderer2DWin32=(Renderer2DWin32*)iTab->renderer2D;
		renderer2DWin32->renderer->DrawBitmap((ID2D1Bitmap*&)this->renderBitmap,D2D1::RectF(iVec.x,iVec.y,iVec.x+iVec.z,iVec.y+iVec.w));
		iTab->renderer2D->DrawRectangle(iVec.x,iVec.y,iVec.x+iVec.z,iVec.y+iVec.w,0xff0000,false);
	}
}
void GuiViewport::Render(Tab* iTab)
{
	vec4 tCanvas=this->rect;
	vec2 tMouse(iTab->mouse);

	Renderer2DWin32* renderer2DWin32=(Renderer2DWin32*)iTab->renderer2D;

	ID2D1Bitmap*& rBitmap=(ID2D1Bitmap*&)this->renderBitmap;
	unsigned char*& rBuffer=(unsigned char*&)this->renderBuffer;

	D2D1_SIZE_F tSize;

	if(rBitmap)
		//rBitmap->GetSize(&tSize);
		AGGREGATECALL(rBitmap->GetSize,tSize);

	if(!rBitmap || tSize.width!=tCanvas.z || tSize.height!=tCanvas.w || !this->renderBuffer || !this->renderBitmap)
	{
		SAFERELEASE(rBitmap);
		SAFEDELETEARRAY(this->renderBuffer);

		D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
		//tabContainerWin32->renderer2DWin32->renderer->GetPixelFormat(&bp.pixelFormat);
		AGGREGATECALL(renderer2DWin32->renderer->GetPixelFormat,bp.pixelFormat);

		renderer2DWin32->renderer->CreateBitmap(D2D1::SizeU((unsigned int)tCanvas.z,(unsigned int)tCanvas.w),bp,&rBitmap);

		int rBufferSize=tCanvas.z*tCanvas.w*4;

		rBuffer=new unsigned char[rBufferSize];
	}

	if(this->rootEntity)
	{
		this->rootEntity->world=this->model;

		this->rootEntity->update();

		std::vector<GuiEntityViewer*> tGuiEntityViewer;

		Ide::GetInstance()->mainAppWindow->GetTabRects<GuiEntityViewer>(tGuiEntityViewer);

		EditorEntity* tEditorEntity;

		for(std::vector<GuiEntityViewer*>::iterator it=tGuiEntityViewer.begin();it!=tGuiEntityViewer.end();it++)
		{
			tEditorEntity=(EditorEntity*)(*it)->entity;

			if(tEditorEntity && (*it)->tabContainer)
				tEditorEntity->OnPropertiesUpdate((*it)->tabContainer);
		}
	}

	iTab->renderer3D->ChangeContext();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

	glViewport((int)tCanvas.x,(int)tCanvas.y,(int)tCanvas.x+tCanvas.z,(int)tCanvas.y+tCanvas.w);glCheckError();
	glScissor((int)tCanvas.x,(int)tCanvas.y,(int)tCanvas.x+tCanvas.z,(int)tCanvas.y+tCanvas.w);glCheckError();

	{
		int tGuiRectColorBack=GuiRect::COLOR_BACK;
		char* pGuiRectColorBack=(char*)&tGuiRectColorBack;

		glClearColor(pGuiRectColorBack[2]/255.0f,pGuiRectColorBack[1]/255.0f,pGuiRectColorBack[0]/255.0f,0.0f);glCheckError();
		//glClearColor(1,0,0,0);glCheckError();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

		MatrixStack::Push(MatrixStack::PROJECTION,this->projection);
		MatrixStack::Push(MatrixStack::VIEW,this->view);
		MatrixStack::Push(MatrixStack::MODEL,this->model);

		iTab->renderer3D->draw(vec3(0,0,0),vec3(1000,0,0),vec3(1,0,0));
		iTab->renderer3D->draw(vec3(0,0,0),vec3(0,1000,0),vec3(0,1,0));
		iTab->renderer3D->draw(vec3(0,0,0),vec3(0,0,1000),vec3(0,0,1));

		if(this->rootEntity)
			iTab->renderer3D->draw(this->rootEntity);

		MatrixStack::Pop(MatrixStack::MODEL);
		MatrixStack::Pop(MatrixStack::VIEW);
		MatrixStack::Pop(MatrixStack::PROJECTION);

		glReadBuffer(GL_BACK);glCheckError();

		glReadPixels((int)tCanvas.x,(int)tCanvas.y,(int)tCanvas.x+tCanvas.z,(int)tCanvas.y+tCanvas.w,GL_BGRA,GL_UNSIGNED_BYTE,rBuffer);glCheckError();//@mic should implement pbo for performance

		D2D1_RECT_U rBitmapRect={0,0,tCanvas.z,tCanvas.w};

		rBitmap->CopyFromMemory(&rBitmapRect,rBuffer,(int)(tCanvas.z*4));

		this->DrawBuffer(iTab,tCanvas);
	}

	if(this->needsPicking && tMouse.y-tCanvas.y>=0)
	{
		glDisable(GL_DITHER);

		iTab->renderer3D->picking=true;

		glClearColor(0.0f,0.0f,0.0f,0.0f);glCheckError();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

		MatrixStack::Push(MatrixStack::PROJECTION,this->projection);
		MatrixStack::Push(MatrixStack::VIEW,this->view);
		MatrixStack::Push(MatrixStack::MODEL,this->model);

		if(this->rootEntity)
			iTab->renderer3D->draw(this->rootEntity);

		MatrixStack::Pop(MatrixStack::MODEL);
		MatrixStack::Pop(MatrixStack::VIEW);
		MatrixStack::Pop(MatrixStack::PROJECTION);

		glReadBuffer(GL_BACK);glCheckError();

		this->pickedPixel;
		glReadPixels((int)tMouse.x,(int)tMouse.y-tCanvas.y,(int)1,(int)1,GL_RGBA,GL_UNSIGNED_BYTE,&this->pickedPixel);glCheckError();//@mic should implement pbo for performance

		unsigned int address=0;

		unsigned char* ptrPixel=(unsigned char*)&this->pickedPixel;
		unsigned char* ptrAddress=(unsigned char*)&address;

		{
			this->pickedEntity=this->pickedPixel ?

				ptrAddress[0]=ptrPixel[3],
				ptrAddress[1]=ptrPixel[2],
				ptrAddress[2]=ptrPixel[1],
				ptrAddress[3]=ptrPixel[0],

				dynamic_cast<EditorEntity*>((EditorEntity*)address)

				: 0 ;
		}

		iTab->renderer3D->picking=false;

		glEnable(GL_DITHER);

		this->needsPicking=false;
	}
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

					if(1==(int)floatingTabRef->rects.childs.size())
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

						newTabContainer=this->currentTabContainerWin32->containerWin32->CreateTab((float)floatingTabRc.left,(float)floatingTabRc.top,(float)(floatingTabRc.right-floatingTabRc.left),(float)(floatingTabRc.bottom-floatingTabRc.top));

						GuiRect* reparentTab=floatingTabRef->rects.childs[floatingTabRefTabIdx];
						floatingTabRef->selected>0 ? floatingTabRef->selected-=1 : floatingTabRef->selected=0;
						floatingTabRef->OnGuiActivate();
						reparentTab->SetParent(&newTabContainer->rects);

						newTabContainer->selected=newTabContainer->rects.childs.size()-1;

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

			int hh=Tab::BAR_HEIGHT;

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
			Ide::GetInstance()->mainAppWindow->mainContainer->BroadcastToTabs(&Tab::OnGuiMouseMove,(void*)0);

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
	floatingTabRefTabCount=(int)tab->rects.childs.size();

	HWND& floatingTabTargetHwnd=floatingTabTargetWin32->windowDataWin32->hwnd;
	HWND& floatingTabRefHwnd=floatingTabRefWin32->windowDataWin32->hwnd;
	HWND& floatingTabHwnd=floatingTabWin32->windowDataWin32->hwnd;

	RECT tmpRc;

	GetWindowRect(floatingTabRefHwnd,&tmpRc);

	floatingTab=new TabWin32((float)tmpRc.left,(float)tmpRc.top,(float)(tmpRc.right-tmpRc.left),(float)(tmpRc.bottom-tmpRc.top),GetParent(floatingTabRefHwnd));

	EnableWindow(floatingTabHwnd,false);

	wprintf(L"creating floating TabContainer %p\n",floatingTab);

}

void SplitterWin32::DestroyFloatingTab()
{
	if(floatingTabRef)
	{
		wprintf(L"deleting floating TabContainer %p\n",this);

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
		wc.hbrBackground=CreateSolidBrush(MainContainer::COLOR_BACK);

		if(!RegisterClassEx(&wc))
			DEBUG_BREAK();
	}


	{
		WNDCLASS wc={0};

		wc.lpszClassName=WC_TABCONTAINERWINDOWCLASS;
		wc.lpfnWndProc=TabWin32::TabContainerWindowClassProcedure;
		wc.hbrBackground=CreateSolidBrush(RGB(0,255,0));
		wc.hCursor=(HCURSOR)LoadCursor(0,IDC_ARROW);
		wc.style=CS_VREDRAW|CS_HREDRAW|CS_OWNDC;

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

bool SubsystemWin32::Execute(String iPath,String iCmdLine,String iOutputFile,bool iInput,bool iError,bool iOutput,bool iNewConsole)
{
	if(iPath==L"none")
		iPath=Ide::GetInstance()->folderProject;

	STARTUPINFO si={0};
	PROCESS_INFORMATION pi={0};

	HANDLE tFileOutput=0;

	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = true;

	const int tOldCurrentPathSize=1024;
	wchar_t tOldCurrentPath[tOldCurrentPathSize];

	GetCurrentDirectory(tOldCurrentPathSize,tOldCurrentPath);
	SetCurrentDirectory(iPath.c_str());

	String tCommandLine=L"cmd.exe /V /C " + iCmdLine;

	int tOutputFileSize=iOutputFile.size();

	if(tOutputFileSize && (iInput || iError || iOutput))
	{
		SECURITY_ATTRIBUTES sa={0};

		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = 0;
		sa.bInheritHandle = !iNewConsole;

		tFileOutput = CreateFile(iOutputFile.c_str(),FILE_APPEND_DATA,FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL/*|FILE_FLAG_DELETE_ON_CLOSE*/,0);

		if(!tFileOutput)
			DEBUG_BREAK();

		si.dwFlags |= STARTF_USESTDHANDLES;
		si.hStdInput = iInput ? tFileOutput : GetStdHandle(STD_INPUT_HANDLE);
		si.hStdError = iError ? tFileOutput : GetStdHandle(STD_ERROR_HANDLE);
		si.hStdOutput = iOutput ? tFileOutput : GetStdHandle(STD_OUTPUT_HANDLE);
	}

	if(!CreateProcess(0,(wchar_t*)tCommandLine.c_str(),0,0,!iNewConsole,0,0,0,&si,&pi))
		return false;

	WaitForSingleObject( pi.hProcess, INFINITE );

	if(!CloseHandle( pi.hProcess ))
		DEBUG_BREAK();
	if(!CloseHandle( pi.hThread ))
		DEBUG_BREAK();
	if(tFileOutput && !CloseHandle( tFileOutput ))
		DEBUG_BREAK();

	SetCurrentDirectory(tOldCurrentPath);

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
			if(iProcessName==tProcess.szExeFile)
				return tProcess.th32ProcessID;
		}
	}

	return 0;
}

unsigned int SubsystemWin32::FindThreadId(unsigned int iProcessId,String iThreadName)
{
	return 0;
}

String SubsystemWin32::DirectoryChooser(String iDescription,String iExtension)
{
	wchar_t _pszDisplayName[MAX_PATH]=L"";

	BROWSEINFO bi={0};
	bi.hwndOwner=(HWND)Ide::GetInstance()->mainAppWindow->mainContainer->GetWindowHandle();
	bi.pszDisplayName=_pszDisplayName;
	bi.lpszTitle=L"Select Directory";

	PIDLIST_ABSOLUTE tmpFolder=SHBrowseForFolder(&bi);

	DWORD err=GetLastError();

	if(tmpFolder)
	{
		wchar_t path[MAX_PATH];

		if(SHGetPathFromIDList(tmpFolder,path))
		{
			return path;
		}
	}

	return L"";
}

String SubsystemWin32::FileChooser(String iDescription,String iExtension)
{
	wchar_t charpretval[5000]={0};

	OPENFILENAME openfilename={0};
	openfilename.lStructSize=sizeof(OPENFILENAME);
	openfilename.hwndOwner=(HWND)Ide::GetInstance()->mainAppWindow->mainContainer->GetWindowHandle();
	openfilename.lpstrFilter=(iDescription + L"\0" + iExtension + L"\0\0").c_str();
	openfilename.nFilterIndex=1;
	openfilename.lpstrFile=charpretval;
	openfilename.nMaxFile=5000;

	GetOpenFileName(&openfilename);

	return openfilename.lpstrFile;
}

std::vector<String> SubsystemWin32::ListDirectories(String iDir)
{
	std::vector<String> tResult;

	HANDLE			tHandle;
	WIN32_FIND_DATA tData;

	String tScanDir=iDir + L"\\*";

	tHandle=FindFirstFile(tScanDir.c_str(),&tData); //. dir

	if(!tHandle || INVALID_HANDLE_VALUE == tHandle)
		tResult;
	else
		FindNextFile(tHandle,&tData);

	while(FindNextFile(tHandle,&tData))
	{
		if(!(tData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		tResult.push_back(tData.cFileName);
	}

	FindClose(tHandle);
	tHandle=0;

	return tResult;
}

bool SubsystemWin32::CreateDirectory(String iDir)
{
	SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),0,false};

	return ::CreateDirectory(iDir.c_str(),&sa);
}

bool SubsystemWin32::DirectoryExist(String iDir)
{
	DWORD tFileAttributes=GetFileAttributes(iDir.c_str());

	return (tFileAttributes!=INVALID_FILE_ATTRIBUTES && (tFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

void* SubsystemWin32::LoadLibrary(String iLibName)
{
	void* tModule=::LoadLibrary(iLibName.c_str());

	if(!tModule)
		wprintf(L"SubsystemWin32::LoadLibrary: error %d loading module %s\n",GetLastError(),iLibName.c_str());

	return tModule;
}

bool SubsystemWin32::FreeLibrary(void* iModule)
{
	return ::FreeLibrary((HMODULE)iModule);
}

void* SubsystemWin32::GetProcAddress(void* iModule,String iAddress)
{
	return (void*)::GetProcAddress((HMODULE)iModule,StringUtils::ToChar(iAddress).c_str());
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////////DebuggerWin32/////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

String ExceptionString(unsigned int iCode)
{
	switch(iCode)
	{
		case EXCEPTION_ACCESS_VIOLATION        : return L"EXCEPTION_ACCESS_VIOLATION"		; break;
		case EXCEPTION_DATATYPE_MISALIGNMENT   : return L"EXCEPTION_DATATYPE_MISALIGNMENT";	; break;
		case EXCEPTION_BREAKPOINT              : return L"EXCEPTION_BREAKPOINT"  			; break;
		case EXCEPTION_SINGLE_STEP             : return L"EXCEPTION_SINGLE_STEP"  			; break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED   : return L"EXCEPTION_ARRAY_BOUNDS_EXCEEDED"  	; break;
		case EXCEPTION_FLT_DENORMAL_OPERAND    : return L"EXCEPTION_FLT_DENORMAL_OPERAND"  	; break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO      : return L"EXCEPTION_FLT_DIVIDE_BY_ZERO"  	; break;
		case EXCEPTION_FLT_INEXACT_RESULT      : return L"EXCEPTION_FLT_INEXACT_RESULT"  	; break;
		case EXCEPTION_FLT_INVALID_OPERATION   : return L"EXCEPTION_FLT_INVALID_OPERATION"  	; break;
		case EXCEPTION_FLT_OVERFLOW            : return L"EXCEPTION_FLT_OVERFLOW"  			; break;
		case EXCEPTION_FLT_STACK_CHECK         : return L"EXCEPTION_FLT_STACK_CHECK"  		; break;
		case EXCEPTION_FLT_UNDERFLOW           : return L"EXCEPTION_FLT_UNDERFLOW"  			; break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO      : return L"EXCEPTION_INT_DIVIDE_BY_ZERO"  	; break;
		case EXCEPTION_INT_OVERFLOW            : return L"EXCEPTION_INT_OVERFLOW"  			; break;
		case EXCEPTION_PRIV_INSTRUCTION        : return L"EXCEPTION_PRIV_INSTRUCTION"  		; break;
		case EXCEPTION_IN_PAGE_ERROR           : return L"EXCEPTION_IN_PAGE_ERROR"  			; break;
		case EXCEPTION_ILLEGAL_INSTRUCTION     : return L"EXCEPTION_ILLEGAL_INSTRUCTION"  	; break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return L"EXCEPTION_NONCONTINUABLE_EXCEPTION"; break;
		case EXCEPTION_STACK_OVERFLOW          : return L"EXCEPTION_STACK_OVERFLOW"  		; break;
		case EXCEPTION_INVALID_DISPOSITION     : return L"EXCEPTION_INVALID_DISPOSITION"  	; break;
		case EXCEPTION_GUARD_PAGE              : return L"EXCEPTION_GUARD_PAGE"  			; break;
		case EXCEPTION_INVALID_HANDLE          : return L"EXCEPTION_INVALID_HANDLE"  		; break;

		default:
			return L"NONE";
	}
}

void DebuggerWin32::PrintThreadContext(void* iThreadContext)
{
	CONTEXT tc=*(CONTEXT*)iThreadContext;

	wprintf(L"{\ncflags 0x%p\neflags 0x%p\ndr0 0x%p\ndr1 0x%p\ndr2 0x%p\ndr3 0x%p\ndr7 0x%p\n}\n",
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
			wprintf(L"error suspending debuggee\n");
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
			wprintf(L"error resuming debuggee\n");
		else
			this->threadSuspendend=false;
	}
}

DWORD WINAPI debuggeeThreadFunc(LPVOID iDebuggerWin32)
{
	wprintf(L"debuggeeThreadFunc started\n");

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
		
		::Sleep(tDebuggerWin32->sleep);
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
		wprintf(L"skipping breakpoint %p\n",exceptionInfo->ExceptionRecord->ExceptionAddress);
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
			wprintf(L"%s on address 0x%p without breakpoint\n",ExceptionString(exceptionInfo->ExceptionRecord->ExceptionCode),exceptionInfo->ExceptionRecord->ExceptionAddress);

			exceptionInfo->ContextRecord->Dr0=0;
			exceptionInfo->ContextRecord->Dr1=0;
			exceptionInfo->ContextRecord->Dr2=0;
			exceptionInfo->ContextRecord->Dr3=0;
			exceptionInfo->ContextRecord->Dr7=0;
		}
	}

	return EXCEPTION_CONTINUE_EXECUTION;
}

//LONG WINAPI (*SystemUnhandledException)(LPEXCEPTION_POINTERS exceptionInfo)=0;

LONG WINAPI UnhandledException(LPEXCEPTION_POINTERS exceptionInfo)
{
	DebuggerWin32* debuggerWin32=(DebuggerWin32*)Ide::GetInstance()->debugger;

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
			wprintf(L"%s breakpoint on source 0x%p at line %d address 0x%p\n",iSet ? "adding" : "removing",iLineAddress.script,iLineAddress.line,iLineAddress.address);

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

	tEditorScript->scriptViewer->GetRootRect()->tab->SetDraw(tEditorScript->scriptViewer);

	wprintf(L"breakpoint on 0x%p at line %d address 0x%p\n",iBreakpoint.script,iBreakpoint.line,iBreakpoint.address);
}
void DebuggerWin32::ContinueDebuggee()
{
	this->breaked=false;
	this->currentBreakpoint->breaked=false;
	this->currentBreakpoint=0;

	wprintf(L"resuming\n");
}

DebuggerWin32::DebuggerWin32()
{
	this->threadContext=new CONTEXT;

	/*SystemUnhandledException=*/SetUnhandledExceptionFilter(UnhandledException);

	this->debuggeeThread=CreateThread(0,0,debuggeeThreadFunc,this,/*CREATE_SUSPENDED*/0,(DWORD*)(int*)&this->debuggeeThreadId);

	wprintf(L"Debugger: debuggee thread id is %d\n",this->debuggeeThreadId);
}

DebuggerWin32::~DebuggerWin32()
{
    //SetUnhandledExceptionFilter(SystemUnhandledException);
    SAFEDELETE(this->threadContext);
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
/////////////////PluginSystemWin32/////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

PluginSystemWin32::PluginSystemWin32()
{
}

void PluginSystemWin32::ScanPluginsDirectory()
{
	Ide::GetInstance()->projectDirChangedThread->Block(true);

	File tPluginsFile=Ide::GetInstance()->folderPlugins + L"\\plugins.cfg";

	std::vector<String> tPluginStates;

	if(tPluginsFile.Open(L"rb"))
	{
		const unsigned int _strDim=1024;
		char _str[_strDim];

		while(!feof(tPluginsFile.data) && fgets(_str,_strDim,tPluginsFile.data))
		{
			char* _Del=strstr(_str,"\n");

			_Del ? *_Del=0 : 0;

			_Del=strstr(_str,"\r");

			_Del ? *_Del=0 : 0;

			String _Wstr=StringUtils::ToWide(_str);
			tPluginStates.push_back(_Wstr);
		}

		tPluginsFile.Close();
	}

	PluginSystem::Plugin* (*ptfGetPluginPrototypeFunction)(PluginSystem*)=0;

	HANDLE			tScanHandle;
	WIN32_FIND_DATA tScanResult;
	DWORD			terr=0;
	HMODULE			tPluginDll=0;

	Plugin*			tPlugin=0;

	String tTargetDir=FilePath(Ide::GetInstance()->pathExecutable).Path() + L"\\plugins\\*";

	tScanHandle=FindFirstFile(tTargetDir.c_str(),&tScanResult); //. dir

	if(!tScanHandle || INVALID_HANDLE_VALUE == tScanHandle)
	{
		DEBUG_BREAK();
		return;
	}
	else
		FindNextFile(tScanHandle,&tScanResult);

	while(FindNextFile(tScanHandle,&tScanResult))
	{
		if(tScanResult.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			continue;

		String tFilename=FilePath(tTargetDir).PathUp(1) + L"\\" + tScanResult.cFileName;

		SetDllDirectory(FilePath(tTargetDir).PathUp(1).c_str());

		tPluginDll=LoadLibrary(tFilename.c_str());

		terr=GetLastError();

		if(tPluginDll)
		{
			ptfGetPluginPrototypeFunction=(PluginSystem::Plugin* (*)(PluginSystem*))GetProcAddress(tPluginDll,"GetPlugin");

			if(ptfGetPluginPrototypeFunction)
			{
				tPlugin=ptfGetPluginPrototypeFunction(this);

				if(tPlugin)
				{
					this->tPluginDlls.push_back(tPluginDll);
					this->plugins.push_back(tPlugin);

					for(size_t i=0;i<tPluginStates.size();i++)
					{
						if(tPlugin->name==tPluginStates[i])
							tPlugin->Load();
					}

					wprintf(L"%s plugin loaded\n",tPlugin->name.c_str());
				}
			}

			if(!ptfGetPluginPrototypeFunction || !tPlugin)
				FreeLibrary(tPluginDll);
		}
	}

	Ide::GetInstance()->projectDirChangedThread->Block(false);
}







int _DLL_PROCESS_ATTACH=0;
int _DLL_PROCESS_DETACH=0;
int _DLL_THREAD_ATTACH=0;
int _DLL_THREAD_DETACH=0;
int _DLL_THREAD_ERROR=0;

BOOL DllMain(HINSTANCE,DWORD iReason,LPVOID)
{
	wprintf(L"\nWIN32DLLMAIN:");

	BOOL retVal=FALSE;

    switch(iReason)
    {
		case DLL_PROCESS_ATTACH:wprintf(L"DLL_PROCESS_ATTACH %d:",++_DLL_PROCESS_ATTACH);retVal=TRUE;break;
		case DLL_PROCESS_DETACH:wprintf(L"DLL_PROCESS_DETACH %d:",++_DLL_PROCESS_DETACH);retVal=TRUE;break;
		case DLL_THREAD_ATTACH:wprintf(L"DLL_THREAD_ATTACH %d:",++_DLL_THREAD_ATTACH);retVal=TRUE;break;
        case DLL_THREAD_DETACH:wprintf(L"DLL_THREAD_DETACH %d:",++_DLL_THREAD_DETACH);retVal=TRUE;break;

		default:
			wprintf(L"DLL_THREAD_ERROR %d:",++_DLL_THREAD_ERROR);retVal=FALSE;
    }

	wprintf(L"\n");

    return retVal;
}

