// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "win32.h"

D2D1::Matrix3x2F identity(){return D2D1::Matrix3x2F::Identity();}

ID2D1Factory*		Direct2DGuiBase::factory=0;
IWICImagingFactory*	Direct2DGuiBase::imager=0;
IDWriteFactory*		Direct2DGuiBase::writer=0;
IDWriteTextFormat*	Direct2DGuiBase::texter=0;

void Direct2DGuiBase::Init(wchar_t* fontName,float fontSize)
{
	if(!factory)
	{
		HRESULT res=S_OK;

		res=CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,(LPVOID*)&imager);
		if(S_OK!=res)
			__debugbreak();

		res=D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
		if(S_OK!=res)
			__debugbreak();

		res=DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(writer),reinterpret_cast<IUnknown **>(&writer));
		if(S_OK!=res)
			__debugbreak();

		res=writer->CreateTextFormat(fontName,NULL,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,fontSize,L"",&texter);
		if(S_OK!=res)
			__debugbreak();

		

		// Center the text horizontally and vertically.
		//textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		res=texter->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		if(S_OK!=res)
			__debugbreak();
	}
}

void Direct2DGuiBase::Release()
{
	SAFERELEASE(factory);
	SAFERELEASE(imager);
	SAFERELEASE(writer);
	SAFERELEASE(texter);
};

ID2D1HwndRenderTarget* Direct2DGuiBase::InitHWNDRenderer(HWND hwnd)
{
	ID2D1HwndRenderTarget* renderer=0;

	if (hwnd!=0)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top);

		D2D1_RENDER_TARGET_PROPERTIES rtp=D2D1::RenderTargetProperties();

		HRESULT res=factory->CreateHwndRenderTarget(rtp,D2D1::HwndRenderTargetProperties(hwnd, size,D2D1_PRESENT_OPTIONS_IMMEDIATELY),&renderer);
		if(S_OK!=res)
			__debugbreak();
	}

	return renderer;
}

ID2D1DCRenderTarget* Direct2DGuiBase::InitDCRenderer()
{
	ID2D1DCRenderTarget* renderer=0;

	D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(
		DXGI_FORMAT_B8G8R8A8_UNORM,
		D2D1_ALPHA_MODE_IGNORE),
		0,
		0,
		D2D1_RENDER_TARGET_USAGE_NONE,
		D2D1_FEATURE_LEVEL_DEFAULT
		);

	if(S_OK!=factory->CreateDCRenderTarget(&rtp,&renderer))
		__debugbreak();

	return renderer;
}

ID2D1TransformedGeometry* Direct2DGuiBase::CreateTransformedGeometry(ID2D1Geometry* geometry)
{
	ID2D1TransformedGeometry* transformedGeometry;
	factory->CreateTransformedGeometry(geometry,identity(),&transformedGeometry);
	return transformedGeometry;
}

ID2D1RectangleGeometry* Direct2DGuiBase::CreateRectangle(float x,float y,float w,float h)
{
	ID2D1RectangleGeometry* rectangleGeometry;
	D2D1_RECT_F rect={x,y,w,h};
	factory->CreateRectangleGeometry(rect,&rectangleGeometry);
	return rectangleGeometry;
}
ID2D1RoundedRectangleGeometry* Direct2DGuiBase::CreateRoundRectangle(float x,float y,float w,float h,float rx,float ry)
{
	ID2D1RoundedRectangleGeometry* roundRectangleGeometry;
	D2D1_ROUNDED_RECT roundedRect={x,y,w,h,rx,ry};
	factory->CreateRoundedRectangleGeometry(roundedRect,&roundRectangleGeometry);
	return roundRectangleGeometry;
}

ID2D1EllipseGeometry* Direct2DGuiBase::CreateEllipse(float x,float y,float rx,float ry)
{
	ID2D1EllipseGeometry* ellipseGeometry;
	D2D1_ELLIPSE ellipse={x,y,rx,ry};
	factory->CreateEllipseGeometry(ellipse,&ellipseGeometry);
	return ellipseGeometry;
}

ID2D1PathGeometry* Direct2DGuiBase::CreatePathGeometry()
{
	ID2D1PathGeometry* pathGeometry;
	factory->CreatePathGeometry(&pathGeometry);
	return pathGeometry;
}

ID2D1Bitmap* Direct2DGuiBase::CreateRawBitmap(ID2D1RenderTarget*renderer,const wchar_t* fname)
{
	ID2D1Bitmap *bitmap=0;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,(LPVOID*)&imager);

	if (!SUCCEEDED(hr))
		__debugbreak();

	hr = imager->CreateDecoderFromFilename(fname,NULL,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&pDecoder);

	if (!SUCCEEDED(hr))
		__debugbreak();

	unsigned int frameCount;
	pDecoder->GetFrameCount(&frameCount);

	hr = pDecoder->GetFrame(0, &pSource);

	if (!SUCCEEDED(hr))
		__debugbreak();

	hr = imager->CreateFormatConverter(&pConverter);

	if (!SUCCEEDED(hr))
		__debugbreak();

	hr = pConverter->Initialize(pSource,GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,NULL,0.f,WICBitmapPaletteTypeMedianCut);
				

	if (!SUCCEEDED(hr))
		__debugbreak();

	hr = renderer->CreateBitmapFromWicBitmap(pConverter,NULL,&bitmap);
				
	if (SUCCEEDED(hr))
	{
		SAFERELEASE(imager);
		SAFERELEASE(pDecoder);
		SAFERELEASE(pSource);
		SAFERELEASE(pStream);
		SAFERELEASE(pConverter);
		SAFERELEASE(pScaler);
	}

	

	return bitmap;
}

void Direct2DGuiBase::CreateRawBitmap(const wchar_t* fname,unsigned char*& buffer,float &width,float &height)
{
	ID2D1Bitmap *bitmap=0;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICStream *pStream = NULL;
	IWICFormatConverter *pConverter = NULL;
	IWICBitmapScaler *pScaler = NULL;

	HRESULT hr;/*= CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,(LPVOID*)&imager);

	if (!SUCCEEDED(hr))
		__debugbreak();*/

	hr = imager->CreateDecoderFromFilename(fname,NULL,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&pDecoder);

	if (!SUCCEEDED(hr))
		__debugbreak();

	unsigned int frameCount;
	pDecoder->GetFrameCount(&frameCount);

	hr = pDecoder->GetFrame(0, &pSource);

	if (!SUCCEEDED(hr))
		__debugbreak();

	hr = imager->CreateFormatConverter(&pConverter);

	if (!SUCCEEDED(hr))
		__debugbreak();

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
		__debugbreak();

	if (SUCCEEDED(hr))
	{
		SAFERELEASE(pDecoder);
		SAFERELEASE(pSource);
		SAFERELEASE(pStream);
		SAFERELEASE(pConverter);
		SAFERELEASE(pScaler);
	}
}

void Direct2DGuiBase::Draw(ID2D1RenderTarget*renderer,ID2D1Geometry* geometry,ID2D1Brush* brush,bool filled)
{
	filled ? renderer->FillGeometry(geometry,brush) : renderer->DrawGeometry(geometry,brush);
}

void Direct2DGuiBase::DrawRectangle(ID2D1RenderTarget* renderer,float x,float y,float w,float h,ID2D1Brush* brush,bool filled)
{
	const D2D1_RECT_F rect={x,y,x+w,y+h};
	filled ? renderer->FillRectangle(&rect,brush) : renderer->DrawRectangle(&rect,brush);
}

void Direct2DGuiBase::DrawRoundRectangle(ID2D1RenderTarget*renderer,float x,float y,float w,float h,float rx,float ry,ID2D1Brush* brush,bool filled)
{
	const D2D1_ROUNDED_RECT roundedRect={x,y,w,h,rx,ry};
	filled ? renderer->DrawRoundedRectangle(&roundedRect,brush) : renderer->FillRoundedRectangle(&roundedRect,brush);
}

void Direct2DGuiBase::DrawEllipse(ID2D1RenderTarget*renderer,float x,float y, float rx,float ry,ID2D1Brush* brush,bool filled)
{
	const D2D1_ELLIPSE ellipse={x,y,rx,ry};
	filled ? renderer->DrawEllipse(&ellipse,brush) : renderer->FillEllipse(&ellipse,brush);
}

void Direct2DGuiBase::DrawLine(ID2D1RenderTarget*renderer,float x1,float y1, float x2,float y2,ID2D1Brush* brush)
{
	const D2D1_POINT_2F line[2]={x1,y1,x2,y2};
	renderer->DrawLine(line[0],line[1],brush);
}

void Direct2DGuiBase::DrawBitmap(ID2D1RenderTarget*renderer,ID2D1Bitmap* bitmap,float x,float y, float w,float h)
{
	const D2D1_RECT_F rect={x,y,w,h};
	renderer->DrawBitmap(bitmap,rect);
}

void Direct2DGuiBase::DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* inputText,float x1,float y1, float x2,float y2,float ax,float ay)
{
	if(!inputText)
		return;
	size_t retLen=0;
	wchar_t retText[CHAR_MAX];
	mbstowcs_s(&retLen,retText,CHAR_MAX,inputText,strlen(inputText));

	renderer->PushAxisAlignedClip(D2D1::RectF(x1,y1,x2,y2),D2D1_ANTIALIAS_MODE_ALIASED);

	if(ax<0 && ay<0)
		renderer->DrawText(retText,retLen,texter,D2D1::RectF(x1,y1,x2,y2),brush,D2D1_DRAW_TEXT_OPTIONS_NONE,DWRITE_MEASURING_MODE_GDI_CLASSIC);
	else
	{
		ID2D1HwndRenderTarget* hwndRenderer=(ID2D1HwndRenderTarget*)(renderer);
		
		if(!hwndRenderer)
			__debugbreak();

		SIZE tSize;
		GetTextExtentPoint32(GetDC(hwndRenderer->GetHwnd()),inputText,strlen(inputText),&tSize);

		vec4 rect(-tSize.cx/2.0f,-tSize.cy/2.0f,(float)tSize.cx,(float)tSize.cy);

		float ww=x2-x1;
		float hh=y2-y1;

		ax>=0 ? rect.x+=x1+ax*ww : rect.x=x1;
		ay>=0 ? rect.y+=y1+ay*hh : rect.y=y1;

		/*rect.x = x > rect.x ? x : (x+w < rect.x+rect.z ? rect.x - (rect.x+rect.z - (x+w)) - tSize.cx/2.0f: rect.x);
		rect.y = y > rect.y ? y : (y+h < rect.y+rect.w ? rect.y - (rect.y+rect.w - (y+h)) - tSize.c/2.0f: rect.y);*/


		renderer->DrawText(retText,retLen,texter,D2D1::RectF(rect.x,rect.y,rect.x+rect.z,rect.y+rect.w),brush,D2D1_DRAW_TEXT_OPTIONS_NONE,DWRITE_MEASURING_MODE_GDI_CLASSIC);
	}

	renderer->PopAxisAlignedClip();
}


bool Direct2DGuiBase::OnSize(ID2D1Geometry* geometry,D2D1::Matrix3x2F& mtx,float x,float y)
{
	static BOOL hitted=false;

	geometry->FillContainsPoint(D2D1::Point2F(x,y),mtx,&hitted);
	return hitted!=0;
}

bool Direct2DGuiBase::OnSize(ID2D1Geometry* geometry,float x,float y)
{
	static D2D1::Matrix3x2F matrix=D2D1::Matrix3x2F::Identity();
	static BOOL hitted=false;

	geometry->FillContainsPoint(D2D1::Point2F(x,y),matrix,&hitted);
	return hitted!=0;
}


ID2D1SolidColorBrush* Direct2DGuiBase::SolidColorBrush(ID2D1RenderTarget* renderer,float r,float g,float b,float a)
{
	ID2D1SolidColorBrush* solidColorBrush;
	renderer->CreateSolidColorBrush(D2D1::ColorF(r,g,b,a),&solidColorBrush);
	return solidColorBrush;
}

ID2D1LinearGradientBrush* Direct2DGuiBase::LinearGradientBrush(ID2D1RenderTarget* renderer,float x1,float y1,float x2,float y2,float position,float r,float g,float b,float a,D2D1::Matrix3x2F mtx)
{
	ID2D1LinearGradientBrush* linearGradientBrush;
	ID2D1GradientStopCollection* gradientStopCollection;
	D2D1_GRADIENT_STOP gradientStop={position,r,g,b,a};
	renderer->CreateGradientStopCollection(&gradientStop,1,&gradientStopCollection);
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientBrushProperties={x1,y1,x2,y2};
	D2D1_BRUSH_PROPERTIES brushProperties={a,mtx};
	renderer->CreateLinearGradientBrush(&gradientBrushProperties,&brushProperties,gradientStopCollection,&linearGradientBrush);
	return linearGradientBrush;
}

ID2D1BitmapBrush* Direct2DGuiBase::BitmapBrush()
{
	return 0;
}

ID2D1RadialGradientBrush* Direct2DGuiBase::RadialGradientBrush(ID2D1RenderTarget* renderer,float x1,float y1,float x2,float y2,float position,float r,float g,float b,float a,D2D1::Matrix3x2F mtx)
{
	ID2D1RadialGradientBrush* radialGradientBrush;
	ID2D1GradientStopCollection* gradientStopCollection;
	D2D1_GRADIENT_STOP gradientStop={position,r,g,b,a};
	renderer->CreateGradientStopCollection(&gradientStop,1,&gradientStopCollection);
	D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES gradientBrushProperties={x1,y1,x2,y2};
	D2D1_BRUSH_PROPERTIES brushProperties={a,mtx};
	renderer->CreateRadialGradientBrush(&gradientBrushProperties,&brushProperties,gradientStopCollection,&radialGradientBrush);
	return radialGradientBrush;
}



GuiScrollBar::GuiScrollBar():
guiRect(0),
scrollerPressed(-1),
scrollerPosition(0),
scrollerRatio(1),
parentAlignRectX(0)
{
	this->name="ScrollBar";
}
GuiScrollBar::~GuiScrollBar()
{
	
}
	

bool GuiScrollBar::SetScrollerRatio(float contentHeight,float containerHeight)
{
	float oldScrollerRatio=this->scrollerRatio;

	this->scrollerRatio = (contentHeight<containerHeight) ? 1.0f : containerHeight/contentHeight;

	if(oldScrollerRatio!=this->scrollerRatio)
		SetScrollerPosition(this->scrollerPosition);

	return oldScrollerRatio!=this->scrollerRatio;
}

bool GuiScrollBar::SetScrollerPosition(float positionPercent)
{
	float oldScrollerPosition=this->scrollerPosition;

	float scrollerContainerHeight=this->GetContainerHeight();
	float scrollerHeight=this->GetScrollerHeight();

	if(positionPercent+scrollerRatio>1)
		this->scrollerPosition=(scrollerContainerHeight-scrollerHeight)/scrollerContainerHeight;
	else
		this->scrollerPosition = positionPercent < 0 ? 0 : positionPercent;

	return oldScrollerPosition!=this->scrollerPosition;
}

bool GuiScrollBar::Scroll(float upOrDown)
{
	float rowHeightRatio=this->scrollerRatio/GuiSceneViewer::TREEVIEW_ROW_HEIGHT;

	float amount=this->scrollerPosition + (upOrDown<0 ? rowHeightRatio : -rowHeightRatio);

	return this->SetScrollerPosition(amount);
}

bool GuiScrollBar::IsVisible()
{
	return this->scrollerRatio<1.0f;
}

float GuiScrollBar::GetContainerHeight()
{
	return this->rect.w-2.0f*SCROLLBAR_TIP_HEIGHT;
}
float GuiScrollBar::GetScrollerTop()
{
	return this->GetContainerTop()+this->scrollerPosition*this->GetContainerHeight();
}
float GuiScrollBar::GetScrollerBottom()
{
	return this->GetScrollerTop()+this->scrollerRatio*this->GetContainerHeight();
}
float GuiScrollBar::GetScrollerHeight()
{
	return this->GetScrollerBottom()-this->GetScrollerTop();
}
float GuiScrollBar::GetContainerTop()
{
	return this->rect.y+SCROLLBAR_TIP_HEIGHT;
}
float GuiScrollBar::GetContainerBottom()
{
	return this->rect.y+this->rect.w-SCROLLBAR_TIP_HEIGHT;
}

void GuiScrollBar::OnLMouseUp(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseUp(tabContainer);

	this->scrollerPressed=-1;
}

void GuiScrollBar::OnLMouseDown(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseDown(tabContainer,data);

	if(!this->hovering || scrollerRatio==1.0f)
		return;

	vec2& mpos=*(vec2*)data;

	if(mpos.y<this->GetContainerTop())
		this->Scroll(1);
	else if(mpos.y<this->GetContainerBottom())
	{
		this->scrollerPressed=0;

		if(mpos.y>=this->GetScrollerTop() && mpos.y<=this->GetScrollerBottom())
			this->scrollerPressed=((mpos.y-this->GetScrollerTop())/this->GetScrollerHeight())*this->scrollerRatio;
		else
			SetScrollerPosition((mpos.y-this->GetContainerTop())/this->GetContainerHeight());
	}
	else this->Scroll(-1);

	if(this->parent)
		this->parent->OnPaint(tabContainer);
}

void GuiScrollBar::OnMouseMove(TabContainer* tabContainer,void* data)
{
	GuiRect::OnMouseMove(tabContainer,data);
	
	if(this->scrollerRatio==1.0f || this->scrollerPressed<0)
		return;

	vec2& mpos=*(vec2*)data;

	if(mpos.y>this->GetContainerTop() && mpos.y<this->GetContainerBottom())
	{
		float mouseContainerY=(mpos.y-this->GetContainerTop())/this->GetContainerHeight();

		this->SetScrollerPosition(mouseContainerY-this->scrollerPressed);
		
		if(this->parent)
			this->parent->OnPaint(tabContainer);
	}
}


void GuiScrollBar::OnPaint(TabContainer* tabContainer,void* data)
{
	if(this->scrollerRatio==1.0f)
		return;

	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+GuiScrollBar::SCROLLBAR_WIDTH,this->rect.y+GuiScrollBar::SCROLLBAR_TIP_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_GUI_BACKGROUND));
	tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,this->rect.y+this->rect.w-GuiScrollBar::SCROLLBAR_TIP_HEIGHT,this->rect.x+GuiScrollBar::SCROLLBAR_WIDTH,this->rect.y+this->rect.w),tabContainer->SetColor(TabContainer::COLOR_GUI_BACKGROUND));

	tabContainer->renderTarget->DrawBitmap(tabContainer->iconUp,D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+GuiScrollBar::SCROLLBAR_WIDTH,this->rect.y+GuiScrollBar::SCROLLBAR_TIP_HEIGHT));
	tabContainer->renderTarget->DrawBitmap(tabContainer->iconDown,D2D1::RectF(this->rect.x,this->rect.y+this->rect.w-GuiScrollBar::SCROLLBAR_TIP_HEIGHT,this->rect.x+GuiScrollBar::SCROLLBAR_WIDTH,this->rect.y+this->rect.w));

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,this->GetContainerTop(),this->rect.x+GuiScrollBar::SCROLLBAR_WIDTH,this->GetContainerBottom()),tabContainer->SetColor(TabContainer::COLOR_GUI_BACKGROUND));

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,this->GetScrollerTop(),this->rect.x+GuiScrollBar::SCROLLBAR_WIDTH,this->GetScrollerBottom()),tabContainer->SetColor(D2D1::ColorF::Black));

	this->BroadcastToChilds(&GuiRect::OnPaint,tabContainer);

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}

void GuiSceneViewer::OnMouseWheel(TabContainer* tabContainer,void* data)
{
	GuiRect::OnMouseWheel(tabContainer,data);

	if(this->hovering)
	{
		float wheelFactor=*(float*)data;

		this->scrollBar->Scroll(wheelFactor);

		this->OnPaint(tabContainer);
	}
}

void GuiSceneViewer::OnRMouseUp(TabContainer* tabContainer,void* data)
{
	GuiRect::OnRMouseUp(tabContainer,data);

	HMENU menu=CreatePopupMenu();
	HMENU createEntity=CreatePopupMenu();
	HMENU createComponent=CreatePopupMenu();

	InsertMenu(menu,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)createEntity,"Entity");
	{
		InsertMenu(createEntity,0,MF_BYPOSITION|MF_STRING,1,"Entity");
		InsertMenu(createEntity,1,MF_BYPOSITION|MF_STRING,2,"Camera");
	}
	InsertMenu(menu,1,MF_BYPOSITION|MF_POPUP,(UINT_PTR)createComponent,"Component");
	{

	}
	InsertMenu(menu,2,MF_BYPOSITION|MF_STRING,3,"Delete");

	RECT rc;
	GetWindowRect(tabContainer->hwnd,&rc);

	int menuResult=TrackPopupMenu(menu,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(tabContainer->lparam),rc.top+HIWORD(tabContainer->lparam),0,GetParent(tabContainer->hwnd),0);

	switch(menuResult)
	{
		case 1:
			{

			
			Entity* newEntity=new Entity;

			newEntity->name="Entity";

			newEntity->SetParent(this->entityRoot);
			newEntity->bbox.a.make(-1,-1,-1);
			newEntity->bbox.b.make(1,1,1);

			this->UpdateNodes(this->entityRoot);

			this->OnSize(tabContainer);
			this->OnPaint(tabContainer);
			}
		break;
		case 2:
			{
			Entity* newEntity=new Entity;

			newEntity->name="Camera";

			newEntity->SetParent(this->entityRoot);
			newEntity->bbox.a.make(-1,-1,-1);
			newEntity->bbox.b.make(1,1,1);

			newEntity->CreateComponent<Camera>();

			this->UpdateNodes(this->entityRoot);

			this->OnSize(tabContainer);
			this->OnPaint(tabContainer);
			}
		break;
		case 3:
			{

			}
			break;
	}

	DestroyMenu(menu);
}


GuiSceneViewer::GuiSceneViewer():
entityRoot(0)
{
	this->entityRoot=new Entity;
	this->entityRoot->name="EntitySceneRoot";
	this->entityRoot->expanded=true;

	this->name="Scene";
}

void GuiSceneViewer::OnRecreateTarget(TabContainer* tabContainer,void* data)
{
	GuiRect::OnRecreateTarget(tabContainer);
}

GuiSceneViewer::~GuiSceneViewer()
{
	printf("destroying treeview %p\n",this);
}
			


void GuiSceneViewer::OnEntitiesChange(TabContainer* tabContainer,void* data)
{
	Entity* entity=(Entity*)data;

	if(entity)
		entity->SetParent(this->entityRoot);

	this->contentHeight=-(float)TREEVIEW_ROW_HEIGHT+this->UpdateNodes(this->entityRoot);

	scrollBar->SetScrollerRatio(this->contentHeight,this->rect.w);

	if(this->active)
	{
		this->OnSize(tabContainer);
		this->OnPaint(tabContainer);
	}

	this->BroadcastToChilds(&GuiRect::OnEntitiesChange,tabContainer);
}

void GuiSceneViewer::UnselectNodes(Entity* node)
{
	if(!node)
		return;

	node->selected=false;

	for(std::list<Entity*>::iterator nCh=node->childs.begin();nCh!=node->childs.end();nCh++)
		this->UnselectNodes(*nCh);
}

bool GuiSceneViewer::ProcessMouseInput(vec2& mpos,vec2& pos,Entity* node,Entity*& expChanged,Entity*& selChanged)
{
	float drawFromHeight=this->scrollBar->scrollerPosition*this->contentHeight;

	if(pos.y+TREEVIEW_ROW_HEIGHT>=drawFromHeight && pos.y<=drawFromHeight+this->rect.w)
	{
		float relativeY=this->rect.y+pos.y-drawFromHeight;

		float xCursor=this->rect.x+pos.x+TREEVIEW_ROW_ADVANCE*node->level;

		bool hittedRow=mpos.y>relativeY && mpos.y<relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT;
		bool hittedExpandos= node->childs.size() && (mpos.x>xCursor && mpos.x<xCursor+TREEVIEW_ROW_ADVANCE);

		if(hittedRow)
		{
			if(!hittedExpandos)
			{
				this->UnselectNodes(this->entityRoot);

				if(!node->selected)
				{
					node->selected=true;
					selChanged=node;

					return true;
				}
			}
			else
			{
				node->expanded=!node->expanded;
				expChanged=node;
				return true;
			}
		}

	}

	pos.y+=TREEVIEW_ROW_HEIGHT;

	if(node->expanded)
	{
		for(std::list<Entity*>::iterator nCh=node->childs.begin();nCh!=node->childs.end();nCh++)
		{
			if(this->ProcessMouseInput(mpos,pos,*nCh,expChanged,selChanged))
				return true;
		}
	}

	return 0;
}


void GuiSceneViewer::OnLMouseDown(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseDown(tabContainer,data);

	if(!this->hovering)
		return;

	vec2& mpos=*(vec2*)data;

	if(mpos.x<this->rect.x+this->width)
	{
		Entity* expChanged=0;
		Entity* selChanged=0;

		if(this->ProcessMouseInput(mpos,vec2(-TREEVIEW_ROW_ADVANCE,-TREEVIEW_ROW_HEIGHT),this->entityRoot,expChanged,selChanged))
		{
			if(expChanged)
			{
				this->UpdateNodes(this->entityRoot);
				this->OnSize(tabContainer);
			}

			if(selChanged)
			{
				if(!tabContainer->buttonControlDown)
					this->selection.clear();

				this->selection.push_back(selChanged);

				TabContainer::BroadcastToPool(&TabContainer::OnGuiEntitySelected,this->selection[0]);
			}
		}	

		
		this->OnPaint(tabContainer);
	}
}


int GuiSceneViewer::UpdateNodes(Entity* node)
{
	if(!node)
		return 0;

	if(node==this->entityRoot)
		this->contentHeight=0;
	else
	this->contentHeight += TREEVIEW_ROW_HEIGHT;

	if(node->expanded)
	{
		for(std::list<Entity*>::iterator nCh=node->childs.begin();nCh!=node->childs.end();nCh++)
			this->UpdateNodes(*nCh);
	}

	return this->contentHeight;
}

void GuiSceneViewer::DrawNodes(TabContainer* tabContainer,Entity* node,vec2& pos)
{
	if(!node)
		return;

	float drawFromHeight=this->scrollBar->scrollerPosition*this->contentHeight;

	if(pos.y+TREEVIEW_ROW_HEIGHT>=drawFromHeight && pos.y<=drawFromHeight+this->rect.w)
	{
		float relativeY=this->rect.y+pos.y-drawFromHeight;

		if(node->selected)
			tabContainer->renderTarget->FillRectangle(D2D1::RectF(0,(float)relativeY,(float)this->width,(float)relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

		float xCursor=this->rect.x+pos.x+TREEVIEW_ROW_ADVANCE*node->level;

		if(node->childs.size())
			tabContainer->renderTarget->DrawBitmap(node->expanded ? tabContainer->iconDown : tabContainer->iconRight,D2D1::RectF(xCursor,relativeY,xCursor+TabContainer::CONTAINER_ICON_WH,relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT));

		xCursor+=TREEVIEW_ROW_ADVANCE;

		Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),node->name,xCursor,relativeY,xCursor+this->width,relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT,-1,0.5f);
	}
	else if(pos.y>drawFromHeight)
		return;

	pos.y+=TREEVIEW_ROW_HEIGHT;

	if(node->expanded)
	{
		for(std::list<Entity*>::iterator nCh=node->childs.begin();nCh!=node->childs.end();nCh++)
			this->DrawNodes(tabContainer,*nCh,pos);
	}

}


void GuiSceneViewer::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)this->rect.x+this->width,(float)this->rect.y+this->rect.w),tabContainer->SetColor(TabContainer::COLOR_GUI_BACKGROUND));

	tabContainer->renderTarget->PushAxisAlignedClip(D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+this->width,this->rect.y+this->rect.w),D2D1_ANTIALIAS_MODE_ALIASED);

	this->DrawNodes(tabContainer,this->entityRoot,vec2(-TREEVIEW_ROW_ADVANCE,-TREEVIEW_ROW_HEIGHT));

	tabContainer->renderTarget->PopAxisAlignedClip();

	this->BroadcastToChilds(&GuiRect::OnPaint,tabContainer);

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}









GuiEntityViewer::GuiEntityViewer():
	entity(0)
{
	this->name="Entity";
};

GuiEntityViewer::~GuiEntityViewer()
{
	printf("destroying properties %p\n",this);
}

void GuiEntityViewer::OnEntitySelected(TabContainer* tabContainer,void* data)
{
	GuiRect::OnEntitySelected(tabContainer);

	Entity* iEntity=static_cast<Entity*>(data);

	if(iEntity)
	{
		if(this->entity)
		{
			this->BroadcastToChilds(&GuiRect::OnDeactivate,tabContainer);
			this->entity->properties->SetParent(0);
			this->entity->properties->SetClip(0);
			this->scrollBar->SetParent(0);
		}
		
		if(!iEntity->properties)
		{
			iEntity->properties=new GuiRect();

			std::vector<GuiRect*> lvl(50);

			lvl[0]=iEntity->properties->Container("Entity");

			lvl[0]->Property("Name",iEntity->name);
			lvl[0]->Property("Position",iEntity->world.position());
			lvl[1]=lvl[0]->Container("AABB");
			lvl[1]->Property("min",iEntity->bbox.a);
			lvl[1]->Property("max",iEntity->bbox.b);
			lvl[1]->Property("Volume",iEntity->bbox.b-iEntity->bbox.a);
			lvl[0]->Property("Child Num",String((int)iEntity->childs.size()));

			Bone* bone=iEntity->findComponent<Bone>();
			Mesh* mesh=iEntity->findComponent<Mesh>();
			Skin* skin=iEntity->findComponent<Skin>();
			Light* light=iEntity->findComponent<Light>();
			Animation* anim=iEntity->findComponent<Animation>();
			AnimationController* animcont=iEntity->findComponent<AnimationController>();


			if(bone)
			{
				lvl[0]=iEntity->properties->Container("Bone");
			}
			if(mesh)
			{
				lvl[0]=iEntity->properties->Container("Mesh");
				lvl[0]->Property("Controlpoints",String(mesh->mesh_ncontrolpoints));
				lvl[0]->Property("Normals",String(mesh->mesh_nnormals));
				lvl[0]->Property("Polygons",String(mesh->mesh_npolygons));
				lvl[0]->Property("Texcoord",String(mesh->mesh_ntexcoord));
				lvl[0]->Property("Vertexindices",String(mesh->mesh_nvertexindices));
			}
			if(skin)
			{
				lvl[0]=iEntity->properties->Container("Skin");
				lvl[0]->Property("Clusters",String(skin->skin_nclusters));
				lvl[0]->Property("Textures",String(skin->skin_ntextures));
			}
			if(light)
			{
				lvl[0]=iEntity->properties->Container("Light");

			}
			if(anim)
			{
				lvl[0]=iEntity->properties->Container("Animation");
				lvl[0]->Property("IsBone",String(anim->entity->findComponent<Bone>() ? "true" : "false"));
				lvl[0]->Property("Duration",String(anim->end-anim->start));
				lvl[0]->Property("Begin",String(anim->start));
				lvl[0]->Property("End",String(anim->end));
			}
			if(animcont)
			{
				lvl[0]=iEntity->properties->Container("AnimationController");
				lvl[0]->Property("Number of nodes",String((int)animcont->animations.size()));
				lvl[0]->Slider("Velocity",&animcont->speed);
				lvl[0]->Property("Duration",String(animcont->end-animcont->start));
				lvl[0]->Property("Begin",String(animcont->start));
				lvl[0]->Property("End",String(animcont->end));
				lvl[0]->PropertyAnimControl(animcont);
			}
		}

		this->entity=iEntity;
		this->entity->properties->SetParent(this);

		this->scrollBar->SetParent(this);
		this->entity->properties->SetClip(this);

		this->entity->properties->OnSize(tabContainer);
		this->entity->properties->OnActivate(tabContainer);

		this->OnPaint(tabContainer);
	}
}

void GuiEntityViewer::OnExpandos(TabContainer* tabContainer,void* data)
{
	this->UpdateNodes(this->entity->properties);
	this->OnSize(tabContainer);
}

void GuiEntityViewer::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	if(this->entity && this->entity->properties)
		this->entity->properties->OnPaint(tabContainer);

	this->scrollBar->OnPaint(tabContainer);

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}

void GuiEntityViewer::OnActivate(TabContainer* tabContainer,void* data)
{
	GuiRect::OnActivate(tabContainer);
}

void GuiEntityViewer::OnLMouseDown(TabContainer* tabContainer,void* data)
{
	vec2& mpos=*(vec2*)data;

	if(this->entity && this->entity->properties)
		this->entity->properties->OnLMouseDown(tabContainer,vec2(mpos.x,mpos.y+this->scrollBar->scrollerPosition*this->contentHeight));

	this->scrollBar->OnLMouseDown(tabContainer,data);
}

void GuiEntityViewer::OnMouseWheel(TabContainer* tabContainer,void* data)
{
	if(this->_contains(this->rect,vec2(tabContainer->mousex,tabContainer->mousey)))
	{
		this->scrollBar->Scroll(*(float*)data);
		this->OnPaint(tabContainer);
	}

	GuiScrollRect::OnMouseWheel(tabContainer,data);
}

void GuiEntityViewer::OnMouseMove(TabContainer* tabContainer,void* data)
{
	vec2& mpos=*(vec2*)data;

	if(this->entity && this->entity->properties)
		this->entity->properties->OnMouseMove(tabContainer,vec2(mpos.x,mpos.y+this->scrollBar->scrollerPosition*this->contentHeight));

	this->scrollBar->OnMouseMove(tabContainer,data);
}

bool GuiEntityViewer::ProcessMouseInput(vec2&,vec2&,GuiRect* node)
{
	return false;
}
void GuiEntityViewer::DrawNodes(TabContainer*,GuiRect* node,vec2&)
{

}
int GuiEntityViewer::UpdateNodes(GuiRect* node)
{
	if(!node && this->entity)
		return 0;

	this->contentHeight=0;

	for(std::vector<GuiRect*>::iterator nCh=this->entity->properties->childs.begin();nCh!=this->entity->properties->childs.end();nCh++)
	{
		GuiRect* prop=(*nCh);
		this->contentHeight+=prop->rect.w;
	}

	return this->contentHeight;
}





GuiProjectViewer::GuiProjectViewer():
lMouseDown(false),
splitterMoving(false)
{
	this->name="Project";

	left.Set(this,0,0,-1,0,0,0,0,0,0,0.5f,1.0f);
	right.Set(this,&this->left,0,-1,0,0,0,0,0,0,0.5f,1.0f);

	this->rootResource.fileName=App::instance->projectFolder;
	this->rootResource.expanded=true;
	this->rootResource.isDir=true;

	left.rootResource=&this->rootResource;
	right.rootResource=&this->rootResource;

	left.selectedDirs.push_back(&this->rootResource);
}

GuiProjectViewer::~GuiProjectViewer()
{
	printf("destroying resources %p\n",this);
}

GuiProjectViewer::ResourceNode::ResourceNode():
level(0),
isDir(0),
selectedLeft(0),
selectedRight(0)
{}
GuiProjectViewer::ResourceNode::~ResourceNode(){}

GuiProjectViewer::ResourceNodeDir::ResourceNodeDir():expanded(0){}
GuiProjectViewer::ResourceNodeDir::~ResourceNodeDir(){}

void GuiProjectViewer::OnActivate(TabContainer* tabContainer,void* data)
{
	GuiRect::OnActivate(tabContainer);

	this->ScanDir(this->rootResource.fileName);
	this->CreateNodes(this->rootResource.fileName,&this->rootResource);

	this->left.CalcNodesHeight(&this->rootResource);
	this->right.CalcNodesHeight(&this->rootResource);

	this->BroadcastToChilds(&GuiRect::OnSize,tabContainer,data);
}

void GuiProjectViewer::OnMouseMove(TabContainer* tabContainer,void* data)
{
	GuiRect::OnMouseMove(tabContainer,data);

	if(this->hovering)
	{
		if(this->splitterMoving)
		{
		}
		else
		{
		}

		SetCursor(LoadCursor(0,IDC_SIZEWE));
	}
}



void GuiProjectViewer::OnLMouseUp(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseUp(tabContainer);

	lMouseDown=false;
	splitterMoving=false;
}

void GuiProjectViewer::OnLMouseDown(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseDown(tabContainer,data);

	if(!this->hovering)
		return;

	lMouseDown=true;

}

void GuiProjectViewer::OnReparent(TabContainer* tabContainer,void* data)
{
	GuiRect::OnReparent(tabContainer);

	tabContainer->OnGuiSize();
	tabContainer->OnGuiRecreateTarget();
}


void GuiProjectViewer::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)tabContainer->width,(float)tabContainer->height),tabContainer->SetColor(TabContainer::COLOR_MAIN_BACKGROUND));

	this->BroadcastToChilds(&GuiRect::OnPaint,tabContainer);

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}





void GuiProjectViewer::ScanDir(String dir)
{
	HANDLE handle;
	WIN32_FIND_DATA data;

	String scanDir=dir;
	scanDir+="\\*";

	handle=FindFirstFile(scanDir,&data); //. dir

	//printf("GuiProjectViewer::ScanDir(%s)\n",scanDir);

	if(!handle || INVALID_HANDLE_VALUE == handle)
	{
		__debugbreak();
		return;
	}
	else
		FindNextFile(handle,&data);

	while(FindNextFile(handle,&data))
	{
		if(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			continue;

		if(strstr(data.cFileName,".engine"))
		{
			String s = dir + "\\" + String(data.cFileName);

			if(!PathFileExists(s.Buf()))
			{
				//delete .engine file
				continue;
			}
		}
		else
		{
			String engineFile = dir + "\\" + String(data.cFileName) + ".engine";

			if(!PathFileExists(engineFile.Buf()))
			{
				FILE* f=fopen(engineFile.Buf(),"w");

				if(f)
				{
					unsigned char _true=1;
					unsigned char _false=0;

					fwrite(&_false,1,1,f);//selectedLeft
					fwrite(&_false,1,1,f);//selectedRight
					fwrite((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)? &_true : &_false,1,1,f);//isDir
					fwrite(&_false,1,1,f);//expanded
					fclose(f);

					if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
						this->ScanDir(dir +"\\"+data.cFileName);
					
				}
			}
		}
	}

	FindClose(handle);
	handle=0;
}

void GuiProjectViewer::CreateNodes(String dir,ResourceNodeDir* iParent)
{
	HANDLE handle;
	WIN32_FIND_DATA data;

	String scanDir=dir;
	scanDir+="\\*";

	handle=FindFirstFile(scanDir,&data); //. dir

	//printf("GuiProjectViewer::CreateNodes(%s)\n",scanDir);

	if(!handle || INVALID_HANDLE_VALUE == handle)
	{
		__debugbreak();
		return;
	}
	else
		FindNextFile(handle,&data);

	while(FindNextFile(handle,&data))
	{
		if(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			continue;

		if(strstr(data.cFileName,".engine"))
		{
			String engineFile=dir + "\\" + String(data.cFileName);

			bool selectedLeft;
			bool selectedRight;
			bool isDir;
			bool expanded;

			{
				FILE* f=fopen(engineFile.Buf(),"r");

				if(f)
				{
					fread(&selectedLeft,1,1,f);
					fread(&selectedRight,1,1,f);
					fread(&isDir,1,1,f);
					fread(&expanded,1,1,f);

					fclose(f);
				}
			}

			std::string fileName(data.cFileName,&data.cFileName[strlen(data.cFileName)-7]);

			if(isDir)
			{
				ResourceNodeDir* dirNode=new ResourceNodeDir;

				iParent->dirs.push_back(dirNode);
				dirNode->parent=iParent;
				dirNode->fileName=fileName.c_str();
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
				fileNode->fileName=fileName.c_str();
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






void GuiProjectViewer::GuiDirView::OnLMouseDown(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseDown(tabContainer,data);

	if(!this->hovering)
		return;

	vec2& mpos=*(vec2*)data;

	ResourceNodeDir* expChanged=0;
	ResourceNodeDir* selChanged=0;

	float drawFromHeight=this->scrollBar->scrollerPosition*this->contentHeight;

	if(this->ProcessMouseInput(mpos,vec2(),drawFromHeight,this->rootResource,this->rootResource,expChanged,selChanged))
	{
		if(expChanged)
		{
			this->CalcNodesHeight(this->rootResource);
			this->OnSize(tabContainer);
		}

		if(selChanged)
		{
			if(!tabContainer->buttonControlDown)
				this->selectedDirs.clear();

			this->selectedDirs.push_back(selChanged);

			//TabContainer::BroadcastToPool(&TabContainer::OnGuiEntitySelected,this->selection[0]);
		}
	}	

	this->OnPaint(tabContainer);
}

void GuiProjectViewer::GuiFileView::OnLMouseDown(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseDown(tabContainer,data);

	if(!this->hovering)
		return;

	vec2& mpos=*(vec2*)data;

	ResourceNodeDir* expChanged=0;
	ResourceNode* selChanged=0;

	float drawFromHeight=this->scrollBar->scrollerPosition*this->contentHeight;

	if(this->ProcessMouseInput(mpos,vec2(),drawFromHeight,this->rootResource,this->rootResource,expChanged,selChanged))
	{
		if(expChanged)
		{
			this->CalcNodesHeight(this->rootResource);
			this->OnSize(tabContainer);
		}

		if(selChanged)
		{
			if(!tabContainer->buttonControlDown)
				this->selectedFiles.clear();

			this->selectedFiles.push_back(selChanged);

			//TabContainer::BroadcastToPool(&TabContainer::OnGuiEntitySelected,this->selection[0]);
		}

		this->OnPaint(tabContainer);
	}	
}



void GuiProjectViewer::GuiDirView::DrawNodes(TabContainer* tabContainer,ResourceNodeDir* node,vec2& pos,bool& terminated)
{
	if(terminated)
		return;

	float drawFromHeight=this->scrollBar->scrollerPosition*this->contentHeight;

	if(pos.y+TREEVIEW_ROW_HEIGHT>=drawFromHeight && pos.y<=drawFromHeight+this->rect.w)
	{
		float relativeY=this->rect.y+pos.y-drawFromHeight;

		if(node->selectedLeft)
			tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,relativeY,this->rect.x+this->width,(float)relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

		float xCursor=this->rect.x+TREEVIEW_ROW_ADVANCE*node->level;

		if(node->dirs.size())
			tabContainer->renderTarget->DrawBitmap(node->expanded ? tabContainer->iconDown : tabContainer->iconRight,D2D1::RectF(xCursor,relativeY,xCursor+TabContainer::CONTAINER_ICON_WH,relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT));

		xCursor+=TREEVIEW_ROW_ADVANCE;

		tabContainer->renderTarget->DrawBitmap(tabContainer->iconFolder,D2D1::RectF(xCursor,relativeY,xCursor+TabContainer::CONTAINER_ICON_WH,relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT));

		xCursor+=TREEVIEW_ROW_ADVANCE;

		Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),node->fileName,xCursor,relativeY,this->width,relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT,-1,0.5f);
	
		
	}

	pos.y+=TREEVIEW_ROW_HEIGHT;
	
	if(pos.y>drawFromHeight+this->rect.w)
	{
		terminated=true;
		return;
	}

	if(node->expanded)
	{
		for(std::list<ResourceNodeDir*>::iterator nCh=node->dirs.begin();nCh!=node->dirs.end();nCh++)
			this->DrawNodes(tabContainer,*nCh,pos,terminated);
	}

	return;
}

void GuiProjectViewer::GuiFileView::DrawNodes(TabContainer* tabContainer,ResourceNodeDir* _node,vec2& pos)
{
	float drawFromHeight=this->scrollBar->scrollerPosition*this->contentHeight;

	for(std::list<ResourceNodeDir*>::iterator nCh=_node->dirs.begin();nCh!=_node->dirs.end();nCh++)
	{
		if(pos.y+TREEVIEW_ROW_HEIGHT>=drawFromHeight && pos.y<=drawFromHeight+this->rect.w)
		{
			float relativeY=this->rect.y+pos.y-drawFromHeight;

			ResourceNodeDir* node=(*nCh);

			if(node->selectedRight)
				tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,relativeY,this->rect.x+this->width,(float)relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

			float xCursor=this->rect.x;

			tabContainer->renderTarget->DrawBitmap(tabContainer->iconFolder,D2D1::RectF(xCursor,relativeY,xCursor+TabContainer::CONTAINER_ICON_WH,relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT));

			xCursor+=TREEVIEW_ROW_ADVANCE;

			Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),node->fileName,xCursor,relativeY,xCursor+this->width,relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT,-1,0.5f);
		}
		else if(pos.y>drawFromHeight)
			return;

		pos.y+=TREEVIEW_ROW_HEIGHT;
	}

	for(std::list<ResourceNode*>::iterator nCh=_node->files.begin();nCh!=_node->files.end();nCh++)
	{
		if(pos.y+TREEVIEW_ROW_HEIGHT>=drawFromHeight && pos.y<=drawFromHeight+this->rect.w)
		{
			float relativeY=this->rect.y+pos.y-drawFromHeight;

			ResourceNode* node=(*nCh);

			if(node->selectedRight)
				tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,relativeY,this->rect.x+this->width,relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

			float xCursor=this->rect.x;
			tabContainer->renderTarget->DrawBitmap(tabContainer->iconFile,D2D1::RectF(xCursor,relativeY,xCursor+TabContainer::CONTAINER_ICON_WH,relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT));

			xCursor+=TREEVIEW_ROW_ADVANCE;

			Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),node->fileName,xCursor,relativeY,xCursor+rect.z,relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT,-1,0.5f);
		}
		else if(pos.y>drawFromHeight)
			return;

		pos.y+=TREEVIEW_ROW_HEIGHT;
	}

	return;
}

int GuiProjectViewer::GuiDirView::CalcNodesHeight(ResourceNodeDir* node)
{
	if(!node)
		return 0;

	if(node==this->rootResource)
		this->contentHeight=0;

	this->contentHeight += node->isDir  ? TREEVIEW_ROW_HEIGHT : 0;

	if(node->expanded)
	{
		for(std::list<ResourceNodeDir*>::iterator nCh=node->dirs.begin();nCh!=node->dirs.end();nCh++)
			this->CalcNodesHeight(*nCh);
	}

	return this->contentHeight;
}

int GuiProjectViewer::GuiFileView::CalcNodesHeight(ResourceNodeDir* node)
{
	if(!node)
		return 0;

	return this->contentHeight=(node->dirs.size() + node->files.size())*TREEVIEW_ROW_HEIGHT;
}


void GuiProjectViewer::GuiDirView::UnselectNodes(ResourceNodeDir* node)
{
	if(!node)
		return;

	node->selectedLeft=false;

	for(std::list<ResourceNode*>::iterator nCh=node->files.begin();nCh!=node->files.end();nCh++)
		(*nCh)->selectedLeft=0;

	for(std::list<ResourceNodeDir*>::iterator nCh=node->dirs.begin();nCh!=node->dirs.end();nCh++)
		this->UnselectNodes(*nCh);
}

void GuiProjectViewer::GuiFileView::UnselectNodes(ResourceNodeDir* node)
{
	if(!node)
		return;

	node->selectedRight=false;

	for(std::list<ResourceNode*>::iterator nCh=node->files.begin();nCh!=node->files.end();nCh++)
		(*nCh)->selectedRight=0;

	for(std::list<ResourceNodeDir*>::iterator nCh=node->dirs.begin();nCh!=node->dirs.end();nCh++)
		this->UnselectNodes(*nCh);
}

bool GuiProjectViewer::GuiDirView::ProcessMouseInput(vec2& mpos,vec2& pos,float& drawFromHeight,ResourceNodeDir* root,ResourceNodeDir* node,ResourceNodeDir*& expChanged,ResourceNodeDir*& selChanged)
{
	if(pos.y+TREEVIEW_ROW_HEIGHT>=drawFromHeight && pos.y<=drawFromHeight+this->rect.w)
	{
		float relativeY=this->rect.y+pos.y-drawFromHeight;

		float xCursor=this->rect.x+TREEVIEW_ROW_ADVANCE*node->level;

		bool hittedRow=mpos.y>relativeY && mpos.y<relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT;
		bool hittedExpandos= node->dirs.size() && (mpos.x>xCursor && mpos.x<xCursor+TREEVIEW_ROW_ADVANCE);

		if(hittedRow)
		{
			if(!hittedExpandos)
			{
				this->UnselectNodes(root);

				if(!node->selectedLeft)
				{
					node->selectedLeft=true;
					selChanged=node;

					return true;
				}
			}
			else
			{
				node->expanded=!node->expanded;
				expChanged=node;
				return true;
			}
		}

	}

	pos.y+=TREEVIEW_ROW_HEIGHT;

	if(node->expanded)
	{
		for(std::list<ResourceNodeDir*>::iterator nCh=node->dirs.begin();nCh!=node->dirs.end();nCh++)
		{
			if(this->ProcessMouseInput(mpos,pos,drawFromHeight,root,*nCh,expChanged,selChanged))
				return true;
		}
	}

	return 0;
}

bool GuiProjectViewer::GuiFileView::ProcessMouseInput(vec2& mpos,vec2& pos,float& drawFromHeight,ResourceNodeDir* root,ResourceNodeDir* _node,ResourceNodeDir*& expChanged,ResourceNode*& selChanged)
{
	for(std::list<ResourceNodeDir*>::iterator dir=_node->dirs.begin();dir!=_node->dirs.end();dir++)
	{
		if(pos.y+TREEVIEW_ROW_HEIGHT>=drawFromHeight && pos.y<=drawFromHeight+this->rect.w)
		{
			float relativeY=this->rect.y+pos.y-drawFromHeight;

			ResourceNodeDir* node=(*dir);

			float xCursor=this->rect.x+TREEVIEW_ROW_ADVANCE*node->level;

			bool hittedRow=mpos.y>relativeY && mpos.y<relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT;

			if(hittedRow)
			{
				this->UnselectNodes(root);

				if(!node->selectedRight)
				{
					node->selectedRight=true;
					selChanged=node;

					return true;
				}
			}
		}

		pos.y+=TREEVIEW_ROW_HEIGHT;
	}

	for(std::list<ResourceNode*>::iterator file=_node->files.begin();file!=_node->files.end();file++)
	{
		if(pos.y+TREEVIEW_ROW_HEIGHT>=drawFromHeight && pos.y<=drawFromHeight+this->rect.w)
		{
			float relativeY=this->rect.y+pos.y-drawFromHeight;

			ResourceNode* node=(*file);

			float xCursor=this->rect.x+TREEVIEW_ROW_ADVANCE*node->level;

			bool hittedRow=mpos.y>relativeY && mpos.y<relativeY+GuiSceneViewer::TREEVIEW_ROW_HEIGHT;

			if(hittedRow)
			{
				this->UnselectNodes(root);

				if(!node->selectedRight)
				{
					node->selectedRight=true;
					selChanged=node;

					return true;
				}
			}
		}

		pos.y+=TREEVIEW_ROW_HEIGHT;
	}

	return false;
}



void GuiProjectViewer::GuiDirView::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+this->width,this->rect.y+this->rect.w),tabContainer->SetColor(TabContainer::COLOR_GUI_BACKGROUND));

	tabContainer->renderTarget->PushAxisAlignedClip(D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+this->width,this->rect.y+this->rect.w),D2D1_ANTIALIAS_MODE_ALIASED);

	bool terminated=false;
	this->DrawNodes(tabContainer,this->rootResource,vec2(),terminated);

	tabContainer->renderTarget->PopAxisAlignedClip();

	this->BroadcastToChilds(&GuiRect::OnPaint,tabContainer,data);

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}


void GuiProjectViewer::GuiFileView::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+this->width,this->rect.y+this->rect.w),tabContainer->SetColor(TabContainer::COLOR_GUI_BACKGROUND));

	tabContainer->renderTarget->PushAxisAlignedClip(D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+this->width,this->rect.y+this->rect.w),D2D1_ANTIALIAS_MODE_ALIASED);

	this->DrawNodes(tabContainer,this->rootResource,vec2());

	tabContainer->renderTarget->PopAxisAlignedClip();

	this->BroadcastToChilds(&GuiRect::OnPaint,tabContainer,data);

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}