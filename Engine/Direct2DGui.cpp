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
		if(S_OK!=D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory))
			__debugbreak();

		if(S_OK!=DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(writer),reinterpret_cast<IUnknown **>(&writer)))
			__debugbreak();

		if(S_OK!=writer->CreateTextFormat(fontName,NULL,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,fontSize,L"",&texter))
			__debugbreak();


		// Center the text horizontally and vertically.
		//textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

		if(S_OK!=texter->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER))
			__debugbreak();
	}
}

void Direct2DGuiBase::Release()
{
	if(factory)factory->Release();
	if(imager)imager->Release();
	if(writer)writer->Release();
	if(texter)texter->Release();
};

ID2D1HwndRenderTarget* Direct2DGuiBase::InitHWNDRenderer(HWND hwnd)
{
	ID2D1HwndRenderTarget* renderer=0;

	if (hwnd!=0)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left,rc.bottom - rc.top);

		if(S_OK!=factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),D2D1::HwndRenderTargetProperties(hwnd, size),&renderer))
			__debugbreak();
	}

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

ID2D1Bitmap* Direct2DGuiBase::CreateBitmap(ID2D1RenderTarget*renderer,const wchar_t* fname)
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
				
	if (!SUCCEEDED(hr))
	{
		pDecoder->Release();
		pSource->Release();
		pStream->Release();
		pConverter->Release();
		pScaler->Release();
	}

	

	return bitmap;
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

void Direct2DGuiBase::DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* text,float x,float y, float w,float h)
{
	wchar_t t[CHAR_MAX];
	mbstowcs(t,text,CHAR_MAX);
	renderer->DrawText(t,wcslen(t),texter,D2D1::RectF(x,y,x+w,y+h),brush);
}

bool Direct2DGuiBase::OnSize(ID2D1Geometry* geometry,D2D1::Matrix3x2F& mtx,float x,float y)
{
	static BOOL hitted=false;

	geometry->FillContainsPoint(D2D1::Point2F(x,y),mtx,&hitted);
	return hitted;
}

bool Direct2DGuiBase::OnSize(ID2D1Geometry* geometry,float x,float y)
{
	static D2D1::Matrix3x2F matrix=D2D1::Matrix3x2F::Identity();
	static BOOL hitted=false;

	geometry->FillContainsPoint(D2D1::Point2F(x,y),matrix,&hitted);
	return hitted;
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


TreeView::TVNODE::TVNODE()
{
	clear(*this);
}
TreeView::TVNODE::~TVNODE()
{
	clear(*this);
}

void TreeView::TVNODE::insert(TVNODE& node,Entity* entity,HDC hdc,float& width,float& height,TVNODE* parent,int expandUntilLevel)
{
	if(!entity)
		return;

	height+= (parent && !parent->expanded) ? 0 : TREEVIEW_ROW_HEIGHT;

	node.parent=parent;
	node.entity=entity;
	node.x=parent ? parent->x+TREEVIEW_ROW_ADVANCE: TREEVIEW_ROW_ADVANCE;
	node.y=height-TREEVIEW_ROW_HEIGHT;
	node.level=parent ? parent->level+1 : 0;
	node.expanded=node.level<expandUntilLevel ? true : false;
	node.selected=false;
	
	node.hasChilds=(int)entity->entity_childs.size();

	SIZE tSize;
	GetTextExtentPoint32(hdc,entity->entity_name,entity->entity_name.Count(),&tSize);

	node.textWidth=tSize.cx;

	if(!parent || parent && parent->expanded)
		width=node.textWidth+node.x > width ? node.textWidth+node.x : width;
	
	for(std::list<Entity*>::iterator eCh=entity->entity_childs.begin();eCh!=entity->entity_childs.end();eCh++)
	{	
		node.childs.push_back(TVNODE());
		insert(node.childs.back(),*eCh,hdc,width,height,&node,expandUntilLevel);
	}
}

void TreeView::TVNODE::update(TVNODE& node,float& width,float& height)
{
	if(!node.entity)
		return;

	height+=node.parent ? (node.parent->expanded ? TREEVIEW_ROW_HEIGHT : 0) : TREEVIEW_ROW_HEIGHT;

	node.x=node.parent ? node.parent->x+TREEVIEW_ROW_ADVANCE : TREEVIEW_ROW_ADVANCE;
	node.y=height-TREEVIEW_ROW_HEIGHT;

	width=node.parent ? (node.parent->expanded ? (node.textWidth+node.x > width ? node.textWidth+node.x : width) : width) : node.textWidth+node.x;
	
	if(node.expanded)
	{
		for(std::list<TVNODE>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
			update(*nCh,width,height);
	}
}
void TreeView::TVNODE::drawlist(TVNODE& node,TreeView* tv)
{
	if(!node.entity)
		return;

	wchar_t text[CHAR_MAX];
	int textLen=(int)mbstowcs(text,node.entity->entity_name,CHAR_MAX);

	if(node.hasChilds)
		tv->treeview_bitmapRenderTarget->DrawBitmap(node.expanded ? tv->treeview_downArrow : tv->treeview_rightArrow,D2D1::RectF(node.x-TREEVIEW_ROW_ADVANCE,node.y,node.x,node.y+TREEVIEW_ROW_HEIGHT));

	tv->treeview_bitmapRenderTarget->DrawText(text,textLen,Direct2DGuiBase::texter,D2D1::RectF(node.x,node.y,node.x+node.textWidth,node.y+TREEVIEW_ROW_HEIGHT),tv->m_pTextBrush);

	if(node.expanded)
	{
		for(std::list<TVNODE>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
			drawlist(*nCh,tv);
	}
}

void TreeView::TVNODE::drawselection(TVNODE& node,TreeView* tv)
{
	if(node.selected)
		tv->tabContainer->tabcontainer_renderer->FillRectangle(D2D1::RectF(0,node.y,tv->tabContainer->width,node.y+TREEVIEW_ROW_HEIGHT),tv->tabContainer->tabcontainer_tabBrush);

	if(node.expanded)
	{
		for(std::list<TVNODE>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
			drawselection(*nCh,tv);
	}
}



bool TreeView::TVNODE::onmousepressed(TVNODE& node,TreeView* tv,float& x,float& y,float& width,float& height)
{
	if(y>node.y && y<node.y+TREEVIEW_ROW_HEIGHT)
	{
		if(node.hasChilds && x>node.x-TREEVIEW_ROW_ADVANCE && x<node.x)
		{
			node.expanded=!node.expanded;
			TVNODE::update(tv->elements,width=0,height=0);
			return true;
		}
		else
			node.selected=true;
	}
	else
		node.selected=false;

	if(node.expanded)
	{
		for(std::list<TVNODE>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
		{
			if(onmousepressed(*nCh,tv,x,y,width,height))
				return true;
		}
	}

	return false;
}

ID2D1Bitmap* TreeView::treeview_rightArrow=0;
ID2D1Bitmap* TreeView::treeview_downArrow=0;

ID2D1SolidColorBrush* TreeView::colorGray=0;
ID2D1SolidColorBrush* TreeView::colorGraySel=0;
ID2D1SolidColorBrush* TreeView::m_pFeatureBrush=0;
ID2D1SolidColorBrush* TreeView::m_pFeatureBrushSelection=0;
ID2D1SolidColorBrush* TreeView::m_pTextBrush=0;

void TreeView::TVNODE::clear(TVNODE& node)
{
	node.parent=0;
	node.entity=0;
	node.x=0;
	node.y=0;
	node.expanded=0;
	node.selected=0;
	node.textWidth=0;
	node.level=0;
	node.hasChilds=0;

	for(std::list<TVNODE>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
		clear(*nCh);

	node.childs.clear();
}


TreeView::TreeView(TabContainer* tc):
tabContainer(tc),
treeview_bitmapRenderTarget(0),
treeview_bitmap(0),
treeview_tvWidth(tc->width),
treeview_tvHeight(tc->height),
treeview_scroll(0),
treeview_framex(tabContainer->width-20),
treeview_framey(tabContainer->height-30)
{

	if(!colorGray)tabContainer->tabcontainer_renderer->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray),&colorGray);
	if(!colorGraySel)tabContainer->tabcontainer_renderer->CreateSolidColorBrush(D2D1::ColorF(0x858585),&colorGraySel);
	if(!m_pFeatureBrush)tabContainer->tabcontainer_renderer->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BlueViolet),&m_pFeatureBrush);
	if(!m_pFeatureBrushSelection)tabContainer->tabcontainer_renderer->CreateSolidColorBrush(D2D1::ColorF(0xffffff),&m_pFeatureBrushSelection);
	if(!m_pTextBrush)tabContainer->tabcontainer_renderer->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&m_pTextBrush);

	if(!treeview_rightArrow)treeview_rightArrow=Direct2DGuiBase::CreateBitmap(tabContainer->tabcontainer_renderer,L"rightarrow.png");
	if(!treeview_downArrow)treeview_downArrow=Direct2DGuiBase::CreateBitmap(tabContainer->tabcontainer_renderer,L"downarrow.png");

	OnEntitiesChange();
}


TreeView::~TreeView()
{
	colorGray->Release();
	colorGraySel->Release();
	m_pFeatureBrush->Release();
	m_pFeatureBrushSelection->Release();
	m_pTextBrush->Release();
}

void TreeView::OnEntitiesChange()
{
	HDC hdc=GetDC(tabContainer->hwnd);
	int nLevels=0;

	TVNODE::clear(elements);

	Entity* rootEntity=Entity::pool.size() ? Entity::pool.front() : 0;

	TVNODE::insert(elements,rootEntity,GetDC(tabContainer->hwnd),treeview_tvWidth=0,treeview_tvHeight=0,0);

	this->DrawItems();
}

void TreeView::OnSize()
{
	treeview_framex=tabContainer->width-ScrollBar::SCROLLBAR_WIDTH;
	treeview_framey=tabContainer->height-TabContainer::CONTAINER_HEIGHT;
}

void TreeView::OnLMouseDown()
{
	float &mx=tabContainer->tabcontainer_mousex;
	float &my=tabContainer->tabcontainer_mousey;

	if(mx>treeview_framex)
	{
		if(my>TabContainer::CONTAINER_HEIGHT && my<(TabContainer::CONTAINER_HEIGHT+ScrollBar::SCROLLBAR_TIP_HEIGHT))
			treeview_scroll=(treeview_scroll-ScrollBar::SCROLLBAR_AMOUNT < 0 ? 0 : treeview_scroll-ScrollBar::SCROLLBAR_AMOUNT);
		else if(my>(TabContainer::CONTAINER_HEIGHT+ScrollBar::SCROLLBAR_TIP_HEIGHT) && my<treeview_framey)
		{

		}
		else if(my>treeview_framey)
			treeview_scroll+=ScrollBar::SCROLLBAR_AMOUNT;

		this->OnPaint();
	}
	else
	{
		my-=TabContainer::CONTAINER_HEIGHT-treeview_scroll;

		if(TVNODE::onmousepressed(elements,this,mx,my,treeview_tvWidth,treeview_tvHeight))
			this->DrawItems();
		
		this->OnPaint();
	}
}

void TreeView::OnRun()
{
	if(tabContainer->tabcontainer_mouseDown)
		this->OnLMouseDown();
}

void TreeView::DrawItems()
{
	if(treeview_bitmapRenderTarget)
		treeview_bitmapRenderTarget->Release();

	tabContainer->tabcontainer_renderer->CreateCompatibleRenderTarget(D2D1::SizeF(treeview_tvWidth,treeview_tvHeight),&treeview_bitmapRenderTarget);

	treeview_bitmapRenderTarget->BeginDraw();

	TVNODE::drawlist(elements,this);

	treeview_bitmapRenderTarget->DrawRectangle(D2D1::RectF(0,0,treeview_tvWidth,treeview_tvHeight),m_pTextBrush);

	treeview_bitmapRenderTarget->EndDraw();

	treeview_bitmapRenderTarget->GetBitmap(&treeview_bitmap);

	this->OnPaint();
}

void TreeView::OnPaint()
{
	tabContainer->tabcontainer_renderer->BeginDraw();

	tabContainer->tabcontainer_renderer->FillRectangle(D2D1::RectF(0,TabContainer::CONTAINER_HEIGHT,treeview_framex,tabContainer->height),this->tabContainer->tabcontainer_backgroundBrush);

	tabContainer->tabcontainer_renderer->PushAxisAlignedClip(D2D1::RectF(0,TabContainer::CONTAINER_HEIGHT,treeview_framex,tabContainer->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tabContainer->tabcontainer_renderer->SetTransform(D2D1::Matrix3x2F::Translation(0,TabContainer::CONTAINER_HEIGHT-treeview_scroll));

	TVNODE::drawselection(elements,this);

	tabContainer->tabcontainer_renderer->DrawBitmap(treeview_bitmap);

	tabContainer->tabcontainer_renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	tabContainer->tabcontainer_renderer->PopAxisAlignedClip();

	tabContainer->tabcontainer_renderer->FillRectangle(D2D1::RectF(treeview_framex,TabContainer::CONTAINER_HEIGHT,tabContainer->width,TabContainer::CONTAINER_HEIGHT+ScrollBar::SCROLLBAR_TIP_HEIGHT),this->m_pFeatureBrush);
	tabContainer->tabcontainer_renderer->FillRectangle(D2D1::RectF(treeview_framex,treeview_framey,tabContainer->width,tabContainer->height),this->m_pFeatureBrush);

	tabContainer->tabcontainer_renderer->EndDraw();
}



