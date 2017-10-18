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

void Direct2DGuiBase::DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* inputText,float x,float y, float w,float h,float ax,float ay)
{
	if(!inputText)
		return;
	size_t retLen=0;
	wchar_t retText[CHAR_MAX];
	mbstowcs_s(&retLen,retText,CHAR_MAX,inputText,strlen(inputText));

	if(ax<0 && ay<0)
		renderer->DrawText(retText,retLen,texter,D2D1::RectF(x,y,w,h),brush,D2D1_DRAW_TEXT_OPTIONS_NONE,DWRITE_MEASURING_MODE_GDI_CLASSIC);
	else
	{
		ID2D1HwndRenderTarget* hwndRenderer=(ID2D1HwndRenderTarget*)(renderer);
		
		if(!hwndRenderer)
			__debugbreak();

		SIZE tSize;
		GetTextExtentPoint32(GetDC(hwndRenderer->GetHwnd()),inputText,strlen(inputText),&tSize);

		vec4 rect(-tSize.cx/2.0f,-tSize.cy/2.0f,tSize.cx,tSize.cy);

		w-=x;
		h-=y;

		ax>=0 ? rect.x+=x+ax*w : rect.x=x;
		ay>=0 ? rect.y+=y+ay*h : rect.y=y;

		/*rect.x = x > rect.x ? x : (x+w < rect.x+rect.z ? rect.x - (rect.x+rect.z - (x+w)) - tSize.cx/2.0f: rect.x);
		rect.y = y > rect.y ? y : (y+h < rect.y+rect.w ? rect.y - (rect.y+rect.w - (y+h)) - tSize.c/2.0f: rect.y);*/


		renderer->DrawText(retText,retLen,texter,D2D1::RectF(rect.x,rect.y,rect.x+rect.z,rect.y+rect.w),brush,D2D1_DRAW_TEXT_OPTIONS_NONE,DWRITE_MEASURING_MODE_GDI_CLASSIC);
	}
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
reference(0)
{
	this->name="ScrollBar";
	this->scroller=0;
	this->scrollerClick=-1;
	this->scrollerFactor=1;
}
GuiScrollBar::~GuiScrollBar()
{
	
}


void GuiScrollBar::SetScrollerFactor(float contentHeight,float containerHeight)
{
	this->scrollerFactor = (contentHeight<containerHeight) ? 1.0f : containerHeight/contentHeight;
}

void GuiScrollBar::SetScrollerPosition(float position)
{
	if(scrollerFactor==1.0f)
		return;

	scroller+=position>0 ? GuiScrollBar::SCROLLBAR_AMOUNT : -GuiScrollBar::SCROLLBAR_AMOUNT;

	scroller<this->rect.y ? scroller=this->rect.y : 0;

	float scrollerRun=(this->rect.w-this->rect.y)-(GuiScrollBar::SCROLLBAR_TIP_HEIGHT*2);
	float scrollerHeight=scrollerRun*scrollerFactor;

	scroller+scrollerHeight>this->rect.y+this->rect.w ? scroller=(this->rect.y+this->rect.w)-scrollerHeight : 0;
}

float GuiScrollBar::GetScrollValue()
{
	return 1.0f/this->scrollerFactor * scroller;;
}

void GuiScrollBar::OnLMouseUp(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseUp(tabContainer);

	if(this->scrollerClick>=0)
		ReleaseCapture();
	this->scrollerClick=-1;
	
}


void GuiScrollBar::OnLMouseDown(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseDown(tabContainer);

	if(scrollerFactor==1.0f)
		return ;//scroller;

	float scrollerRun=(this->rect.w-this->rect.y)-(GuiScrollBar::SCROLLBAR_TIP_HEIGHT*2);
	float scrollerHeight=scrollerRun*scrollerFactor;

	if(tabContainer->mousey>TabContainer::CONTAINER_HEIGHT && tabContainer->mousey<(TabContainer::CONTAINER_HEIGHT+GuiScrollBar::SCROLLBAR_TIP_HEIGHT))
	{
		scroller=(scroller-GuiScrollBar::SCROLLBAR_AMOUNT < 0 ? 0 : scroller-GuiScrollBar::SCROLLBAR_AMOUNT);
	}
	else if(tabContainer->mousey>(scroller+this->rect.y+GuiScrollBar::SCROLLBAR_TIP_HEIGHT) && tabContainer->mousey<((scroller+this->rect.y+GuiScrollBar::SCROLLBAR_TIP_HEIGHT)+scrollerHeight))
	{
		scrollerClick=tabContainer->mousey-this->rect.y-GuiScrollBar::SCROLLBAR_TIP_HEIGHT-scroller;
		SetCapture(tabContainer->hwnd);
	}
	else if(tabContainer->mousey>(this->rect.w-GuiScrollBar::SCROLLBAR_TIP_HEIGHT))
	{
		scroller+=GuiScrollBar::SCROLLBAR_AMOUNT;
	}

	//return scroller;
}

void GuiScrollBar::OnMouseMove(TabContainer* tabContainer,void* data)
{
	GuiRect::OnMouseMove(tabContainer);

	if(scrollerFactor==1.0f || scrollerClick<0)
		return ;//false;

	float scrollerRun=(this->rect.w-this->rect.y)-(GuiScrollBar::SCROLLBAR_TIP_HEIGHT*2);
	float scrollerHeight=scrollerRun*scrollerFactor;

	float absScrollerY=tabContainer->mousey-this->rect.y-GuiScrollBar::SCROLLBAR_TIP_HEIGHT-scrollerClick;

	if(absScrollerY>=0 && tabContainer->mousey+(scrollerHeight-scrollerClick)<=(this->rect.w-GuiScrollBar::SCROLLBAR_TIP_HEIGHT))
		scroller=absScrollerY;
	else
		printf("");

	//return true;
}


void GuiScrollBar::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=!tabContainer->isRender;

	if(selfRender)
		tabContainer->BeginDraw();

	float scrollerRun=this->rect.w-GuiScrollBar::SCROLLBAR_TIP_HEIGHT*2.0f;

	float scrollerHeight=scrollerRun*scrollerFactor;

	tabContainer->renderTarget->DrawBitmap(tabContainer->iconUp,D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+GuiScrollBar::SCROLLBAR_WIDTH,this->rect.y+GuiScrollBar::SCROLLBAR_TIP_HEIGHT));
	tabContainer->renderTarget->DrawBitmap(tabContainer->iconDown,D2D1::RectF(this->rect.x,this->rect.y+this->rect.w-GuiScrollBar::SCROLLBAR_TIP_HEIGHT,this->rect.x+GuiScrollBar::SCROLLBAR_WIDTH,this->rect.y+this->rect.w));

	float scrollerY=scroller+this->rect.y+GuiScrollBar::SCROLLBAR_TIP_HEIGHT;
	float scrollerY2=scrollerY+scrollerHeight;

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,scrollerY,this->rect.x+GuiScrollBar::SCROLLBAR_WIDTH,scrollerY2),tabContainer->SetColor(D2D1::ColorF::Black));

	this->BroadcastToChilds(&GuiRect::OnPaint,tabContainer);

	if(selfRender)
		tabContainer->EndDraw();

}

void GuiSceneViewer::OnRMouseUp(TabContainer* tabContainer,void* data)
{
	HMENU menu=CreatePopupMenu();

	InsertMenu(menu,0,MF_BYPOSITION|MF_POPUP,(UINT_PTR)menu,"New");
	{
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,1,"Entity");
	}
	InsertMenu(menu,1,MF_BYPOSITION|MF_SEPARATOR,0,0);
	InsertMenu(menu,2,MF_BYPOSITION|MF_STRING,TAB_MENU_COMMAND_REMOVE,"Remove Tab");

	RECT rc;
	GetWindowRect(tabContainer->hwnd,&rc);

	int menuResult=TrackPopupMenu(menu,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,rc.left+LOWORD(tabContainer->lparam),rc.top+HIWORD(tabContainer->lparam),0,GetParent(tabContainer->hwnd),0);

	switch(menuResult)
	{
		case 1:
			//the parent entity is a selected entity or the root entity if one
			Entity* entityParent = 0;//!Editor::selection.empty() ? Editor::selection[0]->entity : (!Entity::pool.empty() ? Entity::pool.front() : 0);

			Entity* newEntity=new Entity;

			if(entityParent)
			{
				newEntity->SetParent(entityParent);

				newEntity->bbox.a.make(-1,-1,-1);
				newEntity->bbox.b.make(1,1,1);
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
	scrollBar.Set(this,0,0,-1,0,0,20,0,1,0,-1,1);
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

	scrollBar.SetScrollerFactor(bitmapHeight,this->frameHeight);

	if(this->active)
	{
		this->OnSize(tabContainer);
		this->OnPaint(tabContainer);
	}

	this->BroadcastToChilds(&GuiRect::OnEntitiesChange,tabContainer);
}

void GuiSceneViewer::OnSize(TabContainer* tabContainer,void* data)
{
	frameWidth=tabContainer->width-GuiScrollBar::SCROLLBAR_WIDTH;
	frameHeight=tabContainer->height;

	GuiRect::OnSize(tabContainer);
}

void GuiSceneViewer::UnselectNodes(Entity* node)
{
	if(!node)
		return;

	node->selected=false;

	for(std::list<Entity*>::iterator nCh=node->childs.begin();nCh!=node->childs.end();nCh++)
		this->UnselectNodes(*nCh);
}

bool GuiSceneViewer::ProcessNodes(vec2& mpos,vec2& pos,Entity* node,Entity*& expChanged,Entity*& selChanged)
{
	float xCursor=pos.x+TREEVIEW_ROW_ADVANCE*node->level;

	bool hittedRow=mpos.y>pos.y && mpos.y<pos.y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT;
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
			return node;
		}
	}

	if(node->expanded)
	{
		for(std::list<Entity*>::iterator nCh=node->childs.begin();nCh!=node->childs.end();nCh++)
		{
			pos.y+=TREEVIEW_ROW_HEIGHT;

			if(this->ProcessNodes(mpos,pos,*nCh,expChanged,selChanged))
				return true;
		}
	}

	return 0;
}


void GuiSceneViewer::OnLMouseDown(TabContainer* tabContainer,void* data)
{
	if(tabContainer->mousex>frameWidth)
	{
		scrollBar.OnLMouseDown(tabContainer);
		this->OnPaint(tabContainer);
	}
	else
	{
		Entity* expChanged=0;
		Entity* selChanged=0;

		if(this->ProcessNodes(vec2(tabContainer->mousex,tabContainer->mousey-TabContainer::CONTAINER_HEIGHT),vec2(-TREEVIEW_ROW_ADVANCE,-TREEVIEW_ROW_HEIGHT),this->entityRoot,expChanged,selChanged))
		{
			//this->ScanEntities(tabContainer,this->entitySceneRoot,&this->entityNodeRoot,bitmapWidth=0,bitmapHeight=0,0);
			scrollBar.SetScrollerFactor(bitmapHeight,this->frameHeight);

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


void GuiSceneViewer::OnReparent(TabContainer* tabContainer,void* data)
{
	this->scrollBar.OnReparent(tabContainer);
	this->OnSize(tabContainer);
}

void GuiSceneViewer::DrawNodes(TabContainer* tabContainer,Entity* node,vec2& pos)
{
	if(!node)
		return;

	if(node->selected)
		tabContainer->renderTarget->FillRectangle(D2D1::RectF(0,(float)pos.y,(float)this->rect.z,(float)pos.y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));


	float xCursor=pos.x+TREEVIEW_ROW_ADVANCE*node->level;

	if(node->childs.size())
		tabContainer->renderTarget->DrawBitmap(node->expanded ? tabContainer->iconDown : tabContainer->iconRight,D2D1::RectF(xCursor,pos.y,xCursor+TabContainer::CONTAINER_ICON_WH,pos.y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT));

	xCursor=pos.x+TREEVIEW_ROW_ADVANCE*(node->level+1);

	Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),node->name,xCursor,pos.y,xCursor+this->rect.z,pos.y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT,-1,0.5f);

	if(node->expanded)
	{
		pos.y+=TREEVIEW_ROW_HEIGHT;

		for(std::list<Entity*>::iterator nCh=node->childs.begin();nCh!=node->childs.end();nCh++)
			this->DrawNodes(tabContainer,*nCh,pos);
	}
	else
		pos.y+=TREEVIEW_ROW_HEIGHT;
}


void GuiSceneViewer::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=!tabContainer->isRender;

	if(selfRender)
		tabContainer->BeginDraw();

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)frameWidth,(float)this->rect.y+this->rect.w),tabContainer->SetColor(TabContainer::COLOR_GUI_BACKGROUND));

	tabContainer->renderTarget->PushAxisAlignedClip(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)frameWidth,(float)this->rect.y+this->rect.w),D2D1_ANTIALIAS_MODE_ALIASED);

	tabContainer->renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0,(float)TabContainer::CONTAINER_HEIGHT-scrollBar.scroller));

	this->DrawNodes(tabContainer,this->entityRoot,vec2(-TREEVIEW_ROW_ADVANCE,-TREEVIEW_ROW_HEIGHT));

	//tabContainer->renderer->DrawRectangle(D2D1::RectF(0,0,bitmapWidth,bitmapHeight),tabContainer->SetColor(TabContainer::COLOR_TEXT));

	tabContainer->renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	tabContainer->renderTarget->PopAxisAlignedClip();

	this->BroadcastToChilds(&GuiRect::OnPaint,tabContainer);

	if(selfRender)
		tabContainer->EndDraw();
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

		this->entity->properties->OnSize(tabContainer);
		this->entity->properties->OnActivate(tabContainer);

		this->OnPaint(tabContainer);
	}
};

void GuiEntityViewer::OnActivate(TabContainer* tabContainer,void* data)
{
	GuiRect::OnActivate(tabContainer);
}




void GuiProjectViewer::ResourceNode::clear()
{
	this->parent=0;
	this->x=0;
	this->y=0;
	this->expanded=0;
	this->selected=0;
	this->textWidth=0;
	this->level=0;
	this->nChilds=0;
	this->isDir=false;

	for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
		nCh->clear();

	this->childsDirs.clear();
}

bool GuiProjectViewer::ResourceNode::ScanDir(const char* dir,HANDLE& handle,WIN32_FIND_DATA& data,int opt)
{
	if(!handle)//extract dir and skip first-two win32 directory hardlinks /. and /..
	{
		String scanDir=dir;
		scanDir+="\\*";

		handle=FindFirstFile(scanDir,&data); //. dir

		printf("scanning dir %s\n",scanDir);

		if(!handle || INVALID_HANDLE_VALUE == handle)
		{
			__debugbreak();
			return false;
		}
		else
			FindNextFile(handle,&data);
	}

	while(FindNextFile(handle,&data))
	{
		if(data.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			continue;

		//printf("filename %s\n",data.cFileName);

		if(opt<0)		//returns data with no filter
			return true;

		if(opt==0 && (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return true;


		if(opt==1 && !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return true;

		return true;
	}

	FindClose(handle);
	handle=0;
	return false;
}

GuiProjectViewer::ResourceNode* tempResourceNode=0;

void GuiProjectViewer::ResourceNode::insertDirectory(String &fullPath,HANDLE parentHandle,WIN32_FIND_DATA found,HDC hdc,float& width,float& height,ResourceNode* parent,int expandUntilLevel)
{
	tempResourceNode=!tempResourceNode ? this : tempResourceNode;

	fileName= !parent ? fullPath : found.cFileName;

	height+= (parent && !parent->expanded) ? 0 : GuiSceneViewer::TREEVIEW_ROW_HEIGHT;
	

	this->parent=parent;
	this->x=parent ? parent->x+GuiSceneViewer::TREEVIEW_ROW_ADVANCE : GuiSceneViewer::TREEVIEW_ROW_ADVANCE;
	this->y=height-GuiSceneViewer::TREEVIEW_ROW_HEIGHT;
	this->level=parent ? parent->level+1 : 0;
	this->expanded=this->level<expandUntilLevel ? true : false;
	this->selected=false;
	this->nChilds = 0;
	this->isDir = true;

	parent ? parent->nChilds++ : 0;

	const char* pText=!parent ? fullPath.Buf() : found.cFileName;

	SIZE tSize={0,0};

	if(pText)
	GetTextExtentPoint32(hdc,pText,strlen(pText),&tSize);

	this->textWidth=tSize.cx;

	if(!parent || parent && parent->expanded)
		width=this->textWidth+this->x > width ? this->textWidth+this->x : width;

	while(ScanDir(fullPath,parentHandle,found))
	{
		if(found.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
			continue;

		if(found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			/*if(tempResourceNode!=0 && parent && tempResourceNode==parent->parent)
			{
				
			}
			else*/
			{
				String dir=fullPath + "\\" + found.cFileName;

				this->childsDirs.push_back(ResourceNode());
				this->childsDirs.back().insertDirectory(dir,0,found,hdc,width,height,this,expandUntilLevel);
			}
		}

		
	}
}


void GuiProjectViewer::ResourceNode::insertFiles(GuiProjectViewer::ResourceNode& directory,HDC hdc,float& width,float& height)
{
	this->clear();

	HANDLE handle=0;
	WIN32_FIND_DATA found;

	String fullPath;

	//get fileName full path
	{ 
		std::vector<String> vFullPath;

		vFullPath.push_back(directory.fileName);
		ResourceNode* pNode=directory.parent;
		while(pNode)
		{
			vFullPath.push_back(pNode->fileName);
			pNode=pNode->parent;
		}

		int vSize=(int)vFullPath.size();

		for(int i=vSize-1;i>=0;i--)
		{
			if(i != vSize-1)
				fullPath+="\\";
			fullPath+=vFullPath[i];
			

		}
	}

	while(ScanDir(fullPath,handle,found))
	{
		this->childsDirs.push_back(ResourceNode());
		ResourceNode& node=this->childsDirs.back();


		node.fileName= found.cFileName;

		height+=GuiSceneViewer::TREEVIEW_ROW_HEIGHT;

		node.parent=&directory;
		node.x=0;
		node.y=height-GuiSceneViewer::TREEVIEW_ROW_HEIGHT;
		node.level=0;
		node.expanded=false;
		node.selected=false;
		node.nChilds = this->nChilds++;
		node.isDir = (found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? true : false;

		SIZE tSize;
		GetTextExtentPoint32(hdc,node.fileName,node.fileName.Count(),&tSize);

		node.textWidth=tSize.cx;

		width=node.textWidth+node.x > width ? node.textWidth+node.x : width;
	}
}


void GuiProjectViewer::ResourceNode::update(float& width,float& height)
{
	height+=this->parent ? (this->parent->expanded ? GuiSceneViewer::TREEVIEW_ROW_HEIGHT : 0) : GuiSceneViewer::TREEVIEW_ROW_HEIGHT;

	this->x=this->parent ? this->parent->x+GuiSceneViewer::TREEVIEW_ROW_ADVANCE : GuiSceneViewer::TREEVIEW_ROW_ADVANCE;
	this->y=height-GuiSceneViewer::TREEVIEW_ROW_HEIGHT;

	width=this->parent ? (this->parent->expanded ? (this->textWidth+this->x > width ? this->textWidth+this->x : width) : width) : this->textWidth+this->x;

	if(this->expanded)
	{
		for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
			nCh->update(width,height);
	}
}
void GuiProjectViewer::ResourceNode::drawdirlist(TabContainer* tabContainer,GuiProjectViewer* resNode)
{
	float scrollY=resNode->leftScrollBar.GetScrollValue();

	if(this->y >= scrollY && this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT <= scrollY+resNode->frameHeight)
	{
		size_t resLen=0;
		wchar_t resText[CHAR_MAX];
		mbstowcs_s(&resLen,resText,CHAR_MAX,this->fileName,this->fileName.Count());

		if(this->selected)
			tabContainer->renderTarget->FillRectangle(D2D1::RectF(0,(float)this->y,(float)resNode->leftFrameWidth-GuiScrollBar::SCROLLBAR_WIDTH-4,(float)this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

		if(this->nChilds)
			tabContainer->renderTarget->DrawBitmap(this->expanded ? tabContainer->iconDown : tabContainer->iconRight,D2D1::RectF(this->x-GuiSceneViewer::TREEVIEW_ROW_ADVANCE,this->y,this->x,this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT));

		tabContainer->renderTarget->DrawBitmap(tabContainer->iconFolder,D2D1::RectF(this->x,this->y,this->x + GuiSceneViewer::TREEVIEW_ROW_ADVANCE,this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT));

		tabContainer->renderTarget->DrawText(resText,resLen,Direct2DGuiBase::texter,D2D1::RectF(this->x + GuiSceneViewer::TREEVIEW_ROW_ADVANCE,this->y,this->x + GuiSceneViewer::TREEVIEW_ROW_ADVANCE + this->textWidth,this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TEXT));
	}

	if(this->expanded)
	{
		for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
			nCh->drawdirlist(tabContainer,resNode);
	}
}

void GuiProjectViewer::ResourceNode::drawfilelist(TabContainer* tabContainer,GuiProjectViewer* resNode)
{
	float scrollY=resNode->rightScrollBar.GetScrollValue();

	if(this->parent && (this->y >= scrollY && this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT <= scrollY+resNode->frameHeight))
	{
		size_t resLen=0;
		wchar_t resText[CHAR_MAX];
		mbstowcs_s(&resLen,resText,CHAR_MAX,this->fileName,this->fileName.Count());

		if(this->selected)
			tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->x,this->y,tabContainer->width,this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

		if(this->nChilds)
			tabContainer->renderTarget->DrawBitmap(this->expanded ? tabContainer->iconDown : tabContainer->iconRight,D2D1::RectF(this->x-GuiSceneViewer::TREEVIEW_ROW_ADVANCE,this->y,this->x,this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT));

		tabContainer->renderTarget->DrawBitmap(this->isDir ? tabContainer->iconFolder : tabContainer->iconFile,D2D1::RectF(this->x,this->y,this->x + GuiSceneViewer::TREEVIEW_ROW_ADVANCE,this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT));

		tabContainer->renderTarget->DrawText(resText,resLen,Direct2DGuiBase::texter,D2D1::RectF(this->x + GuiSceneViewer::TREEVIEW_ROW_ADVANCE,this->y,this->x + GuiSceneViewer::TREEVIEW_ROW_ADVANCE + this->textWidth,this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TEXT));
	}

	for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
		nCh->drawfilelist(tabContainer,resNode);
}





GuiProjectViewer::ResourceNode* GuiProjectViewer::ResourceNode::onmousepressedLeftPane(TabContainer* tabContainer,GuiProjectViewer* tv,float& x,float& y,float& width,float& height)
{
	if(y>this->y && y<this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT)
	{
		if(this->nChilds && x>this->x-GuiSceneViewer::TREEVIEW_ROW_ADVANCE && x<this->x)
		{
			this->expanded=!this->expanded;
			return this;
		}
		else
		{
			this->selected=true;
			tv->rightElements.insertFiles(*this,GetDC(tabContainer->hwnd),tv->rightBitmapWidth=0,tv->rightBitmapHeight=0);
			tv->rightScrollBar.SetScrollerFactor(tv->rightBitmapHeight,tv->frameHeight);
		}
	}
	else
		this->selected=false;

	if(this->expanded)
	{
		ResourceNode* resNode=0;

		for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
		{
			if((resNode=nCh->onmousepressedLeftPane(tabContainer,tv,x,y,width,height)))  //((a=b))
				return resNode;
		}
	}

	return 0;
}

GuiProjectViewer::ResourceNode* GuiProjectViewer::ResourceNode::onmousepressedRightPane(TabContainer* tabContainer,GuiProjectViewer* tv,float& x,float& y,float& width,float& height)
{
	if(y>this->y && y<this->y+GuiSceneViewer::TREEVIEW_ROW_HEIGHT)
	{
		this->selected=true;
	}
	else
		this->selected=false;

	ResourceNode* resNode=0;

	for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
	{
		if((resNode=nCh->onmousepressedRightPane(tabContainer,tv,x,y,width,height)))  //((a=b))
			return resNode;
	}

	return 0;
}

GuiProjectViewer::GuiProjectViewer():
leftBitmapWidth(0),
leftBitmapHeight(0),
rightBitmapWidth(0),
rightBitmapHeight(0),
leftFrameWidth(100),
lMouseDown(false),
splitterMoving(false)
{
	this->name="Project";

	leftScrollBar.Set(this,0,0,-1,0,0,20,0,1,0,-1,1);
	rightScrollBar.Set(this,0,0,-1,0,0,20,0,1,0,-1,1);

	
}

void GuiProjectViewer::SetLeftScrollBar()
{
	leftScrollBar.rect.x=this->leftFrameWidth-4-20;
	leftScrollBar.alignPos.x=-1;
	leftScrollBar.SetScrollerFactor(leftBitmapHeight,frameHeight);
}
void GuiProjectViewer::SetRightScrollBar()
{
	rightScrollBar.SetScrollerFactor(rightBitmapHeight,frameHeight);
}
void GuiProjectViewer::OnActivate(TabContainer* tabContainer,void* data)
{
	GuiRect::OnActivate(tabContainer);

	if(!leftElements.fileName.Count())
	{
		HDC hdc=GetDC(tabContainer->hwnd);
		int nLevels=0;

		leftElements.insertDirectory(App::instance->projectFolder,0,WIN32_FIND_DATA(),GetDC(tabContainer->hwnd),leftBitmapWidth=0,leftBitmapHeight=0,0);
		rightElements.insertFiles(leftElements,GetDC(tabContainer->hwnd),rightBitmapWidth=0,rightBitmapHeight=0);

		this->SetLeftScrollBar();
		this->SetRightScrollBar();
	}
}

GuiProjectViewer::~GuiProjectViewer()
{
	printf("destroying resources %p\n",this);
}

void GuiProjectViewer::OnSize(TabContainer* tabContainer,void* data)
{
	GuiRect::OnSize(tabContainer);

	frameHeight=tabContainer->height-TabContainer::CONTAINER_HEIGHT;

	if(this->leftFrameWidth>tabContainer->width)
		this->leftFrameWidth=(tabContainer->width-GuiScrollBar::SCROLLBAR_WIDTH)/2.0f;

	this->SetLeftScrollBar();
	this->SetRightScrollBar();
}

void GuiProjectViewer::OnMouseMove(TabContainer* tabContainer,void* data)
{
	GuiRect::OnMouseMove(tabContainer);

	if(this->splitterMoving)
	{
		this->leftFrameWidth=tabContainer->mousex;
		this->SetLeftScrollBar();
		tabContainer->OnGuiPaint();
	}
	else
	{
		if(tabContainer->mousex>=this->leftFrameWidth-4 && tabContainer->mousex<=this->leftFrameWidth)
		{
			SetCursor(LoadCursor(0,IDC_SIZEWE));
		}
		else
		{
			SetCursor(LoadCursor(0,IDC_ARROW));
		}
	}
}

void GuiProjectViewer::OnMouseWheel(TabContainer* tabContainer,void* data)
{
	GuiRect::OnMouseWheel(tabContainer);

	short int delta = GET_WHEEL_DELTA_WPARAM(tabContainer->wparam);

	if(tabContainer->mousex<(this->leftFrameWidth-4))
	{
		leftScrollBar.SetScrollerPosition((float)-delta);
	}
	else if(tabContainer->mousex>(this->leftFrameWidth+4))
	{
		rightScrollBar.SetScrollerPosition((float)-delta);
	}

	tabContainer->OnGuiPaint();
}

void GuiProjectViewer::OnLMouseUp(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseUp(tabContainer);

	lMouseDown=false;
	splitterMoving=false;
}

void GuiProjectViewer::OnLMouseDown(TabContainer* tabContainer,void* data)
{
	GuiRect::OnLMouseDown(tabContainer);

	float &mx=tabContainer->mousex;
	float &my=tabContainer->mousey;

	lMouseDown=true;

	if(mx<(this->leftFrameWidth-4-GuiScrollBar::SCROLLBAR_WIDTH))
	{
		my-=TabContainer::CONTAINER_HEIGHT-leftScrollBar.scroller;

		ResourceNode* expandedModified=leftElements.onmousepressedLeftPane(tabContainer,this,mx,my,leftBitmapWidth,leftBitmapHeight);

		if(expandedModified)
		{
			leftElements.update(leftBitmapWidth=0,leftBitmapHeight=0);
			leftScrollBar.SetScrollerFactor(leftBitmapHeight,this->frameHeight);
		}	
	
		tabContainer->OnGuiPaint();
	}
	else if(mx<this->leftFrameWidth)//splitter
	{
		printf("on the splitter\n");
		splitterMoving=true;
	}
	else if(mx<(tabContainer->width-GuiScrollBar::SCROLLBAR_WIDTH))//right pane
	{
		my-=TabContainer::CONTAINER_HEIGHT-rightScrollBar.scroller;

		printf("on the right pane\n");
		rightElements.onmousepressedRightPane(tabContainer,this,mx,my,rightBitmapWidth,rightBitmapHeight);
		tabContainer->OnGuiPaint();
	}	
}

void GuiProjectViewer::OnReparent(TabContainer* tabContainer,void* data)
{
	GuiRect::OnReparent(tabContainer);

	this->leftScrollBar.OnReparent(tabContainer);
	this->rightScrollBar.OnReparent(tabContainer);
	tabContainer->OnGuiSize();
	tabContainer->OnGuiRecreateTarget();
}


void GuiProjectViewer::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=!tabContainer->isRender;

	if(selfRender)
		tabContainer->BeginDraw();

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)tabContainer->width,(float)tabContainer->height),tabContainer->SetColor(TabContainer::COLOR_MAIN_BACKGROUND));

	//dirs hierarchy

	tabContainer->renderTarget->PushAxisAlignedClip(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)this->leftFrameWidth-4,(float)tabContainer->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)this->leftFrameWidth-4,(float)tabContainer->height),tabContainer->SetColor(TabContainer::COLOR_GUI_BACKGROUND));

	tabContainer->renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0,(float)TabContainer::CONTAINER_HEIGHT-leftScrollBar.GetScrollValue()));

	leftElements.drawdirlist(tabContainer,this);

	tabContainer->renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	tabContainer->renderTarget->PopAxisAlignedClip();

	//files hierarchy

	tabContainer->renderTarget->PushAxisAlignedClip(D2D1::RectF(this->leftFrameWidth,(float)TabContainer::CONTAINER_HEIGHT,(float)tabContainer->width,(float)tabContainer->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->leftFrameWidth,(float)TabContainer::CONTAINER_HEIGHT,(float)tabContainer->width,(float)tabContainer->height),tabContainer->SetColor(TabContainer::COLOR_GUI_BACKGROUND));

	tabContainer->renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(this->leftFrameWidth,(float)TabContainer::CONTAINER_HEIGHT-rightScrollBar.GetScrollValue()));

	rightElements.drawfilelist(tabContainer,this);

	tabContainer->renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

	tabContainer->renderTarget->PopAxisAlignedClip();

	this->BroadcastToChilds(&GuiRect::OnPaint,tabContainer);

	if(selfRender)
		tabContainer->EndDraw();
}

