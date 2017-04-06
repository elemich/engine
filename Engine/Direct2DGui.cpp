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

		if(S_OK!=CoCreateInstance(CLSID_WICImagingFactory,NULL,CLSCTX_INPROC_SERVER,IID_IWICImagingFactory,(LPVOID*)&imager))
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

		D2D1_RENDER_TARGET_PROPERTIES rtp=D2D1::RenderTargetProperties();
		//rtp.pixelFormat.format=DXGI_FORMAT_R8G8B8A8_UINT;
		
		//rtp.pixelFormat=D2D1::PixelFormat( DXGI_FORMAT_R8G8B8A8_UINT, D2D1_ALPHA_MODE_);

		HRESULT res=factory->CreateHwndRenderTarget(rtp,D2D1::HwndRenderTargetProperties(hwnd, size,D2D1_PRESENT_OPTIONS_IMMEDIATELY),&renderer);
		if(S_OK!=res)
			__debugbreak();

		/*D2D1_PIXEL_FORMAT pf=D2D1::PixelFormat();

		if(S_OK!=factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE,pf,96.f,96.f,D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE,D2D1_FEATURE_LEVEL_DEFAULT),D2D1::HwndRenderTargetProperties(hwnd, size),&renderer))
			__debugbreak();*/
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
		if(imager)imager->Release();
		if(pDecoder)pDecoder->Release();
		if(pSource)pSource->Release();
		if(pStream)pStream->Release();
		if(pConverter)pConverter->Release();
		if(pScaler)pScaler->Release();
	}

	

	return bitmap;
}

void Direct2DGuiBase::CreateRawBitmap(const wchar_t* fname,unsigned char*& buffer)
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

	if(buffer)
	delete [] buffer;

	UINT width,height;
	pSource->GetSize(&width,&height);

	buffer=new unsigned char[width*height*4];

	hr=pSource->CopyPixels(0,width*4,width*height*4,buffer);

	if (!SUCCEEDED(hr))
		__debugbreak();

	if (SUCCEEDED(hr))
	{
		if(pDecoder)pDecoder->Release();
		if(pSource)pSource->Release();
		if(pStream)pStream->Release();
		if(pConverter)pConverter->Release();
		if(pScaler)pScaler->Release();
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

void Direct2DGuiBase::DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* inputText,float x,float y, float w,float h)
{
	size_t retLen=0;
	wchar_t retText[CHAR_MAX];
	mbstowcs_s(&retLen,retText,CHAR_MAX,inputText,strlen(inputText));
	renderer->DrawText(retText,retLen,texter,D2D1::RectF(x,y,x+w,y+h),brush);
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


TreeView::TreeViewNode::TreeViewNode()
{
	clear(*this);
}
TreeView::TreeViewNode::~TreeViewNode()
{
	clear(*this);
}

void TreeView::TreeViewNode::insert(TreeViewNode& node,Entity* entity,HDC hdc,float& width,float& height,TreeViewNode* parent,int expandUntilLevel)
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
		node.childs.push_back(TreeViewNode());
		insert(node.childs.back(),*eCh,hdc,width,height,&node,expandUntilLevel);
	}
}

void TreeView::TreeViewNode::update(TreeViewNode& node,float& width,float& height)
{
	if(!node.entity)
		return;

	height+=node.parent ? (node.parent->expanded ? TREEVIEW_ROW_HEIGHT : 0) : TREEVIEW_ROW_HEIGHT;

	node.x=node.parent ? node.parent->x+TREEVIEW_ROW_ADVANCE : TREEVIEW_ROW_ADVANCE;
	node.y=height-TREEVIEW_ROW_HEIGHT;

	width=node.parent ? (node.parent->expanded ? (node.textWidth+node.x > width ? node.textWidth+node.x : width) : width) : node.textWidth+node.x;
	
	if(node.expanded)
	{
		for(std::list<TreeViewNode>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
			update(*nCh,width,height);
	}
}
void TreeView::TreeViewNode::drawlist(TreeViewNode& node,TreeView* tv)
{
	if(!node.entity)
		return;

	size_t resLen=0;
	wchar_t resText[CHAR_MAX];
	mbstowcs_s(&resLen,resText,CHAR_MAX,node.entity->entity_name,node.entity->entity_name.Count());

	if(node.hasChilds)
		tv->bitmaprenderer->DrawBitmap(node.expanded ? tv->downArrow : tv->rightArrow,D2D1::RectF(node.x-TREEVIEW_ROW_ADVANCE,node.y,node.x,node.y+TREEVIEW_ROW_HEIGHT));

	tv->bitmaprenderer->DrawText(resText,resLen,Direct2DGuiBase::texter,D2D1::RectF(node.x,node.y,node.x+node.textWidth,node.y+TREEVIEW_ROW_HEIGHT),tv->tab->SetColor(TabContainer::COLOR_TEXT));

	if(node.expanded)
	{
		for(std::list<TreeViewNode>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
			drawlist(*nCh,tv);
	}
}

void TreeView::TreeViewNode::drawselection(TreeViewNode& node,TreeView* tv)
{
	TabContainer* tabContainer=tv->tab;

	if(node.selected)
		tabContainer->renderer->FillRectangle(D2D1::RectF(0,(float)node.y,(float)tabContainer->width,(float)node.y+TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

	if(node.expanded)
	{
		for(std::list<TreeViewNode>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
			drawselection(*nCh,tv);
	}
}



bool TreeView::TreeViewNode::onmousepressed(TreeViewNode& node,TreeView* tv,float& x,float& y,float& width,float& height)
{
	if(y>node.y && y<node.y+TREEVIEW_ROW_HEIGHT)
	{
		if(node.hasChilds && x>node.x-TREEVIEW_ROW_ADVANCE && x<node.x)
		{
			node.expanded=!node.expanded;
			TreeViewNode::update(tv->elements,width=0,height=0);
			return true;
		}
		else
			node.selected=true;
	}
	else
		node.selected=false;

	if(node.expanded)
	{
		for(std::list<TreeViewNode>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
		{
			if(onmousepressed(*nCh,tv,x,y,width,height))
				return true;
		}
	}

	return false;
}



void TreeView::TreeViewNode::clear(TreeViewNode& node)
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

	for(std::list<TreeViewNode>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
		clear(*nCh);

	node.childs.clear();
}





TreeView::TreeView(TabContainer* tc):
bitmaprenderer(0),
bitmap(0),
bitmapWidth((float)tc->width),
bitmapHeight((float)tc->height),
scrollY(0),
rightArrow(0),
downArrow(0)
{
	tab=tc;

	frameWidth=tab->width-20;
	frameHeight=tab->height-30;

	this->name="Entities";

	this->RecreateTarget();

	

	OnEntitiesChange();
}

void TreeView::RecreateTarget()
{
	if(rightArrow)rightArrow->Release();
	if(downArrow)downArrow->Release();


	DrawItems();
}


TreeView::~TreeView()
{
	if(rightArrow)rightArrow->Release();
	if(downArrow)downArrow->Release();
}

void TreeView::OnEntitiesChange()
{
	HDC hdc=GetDC(tab->hwnd);
	int nLevels=0;

	TreeViewNode::clear(elements);

	Entity* rootEntity=Entity::pool.size() ? Entity::pool.front() : 0;

	TreeViewNode::insert(elements,rootEntity,GetDC(tab->hwnd),bitmapWidth=0,bitmapHeight=0,0);

	this->DrawItems();
}

void TreeView::OnSize()
{
	frameWidth=tab->width-ScrollBar::SCROLLBAR_WIDTH;
	frameHeight=tab->height-TabContainer::CONTAINER_HEIGHT;
}

void TreeView::OnLMouseDown()
{
	float &mx=tab->mousex;
	float &my=tab->mousey;

	if(mx>frameWidth)
	{
		if(my>TabContainer::CONTAINER_HEIGHT && my<(TabContainer::CONTAINER_HEIGHT+ScrollBar::SCROLLBAR_TIP_HEIGHT))
			scrollY=(scrollY-ScrollBar::SCROLLBAR_AMOUNT < 0 ? 0 : scrollY-ScrollBar::SCROLLBAR_AMOUNT);
		else if(my>(TabContainer::CONTAINER_HEIGHT+ScrollBar::SCROLLBAR_TIP_HEIGHT) && my<frameHeight)
		{

		}
		else if(my>frameHeight)
			scrollY+=ScrollBar::SCROLLBAR_AMOUNT;

		this->OnPaint();
	}
	else
	{
		my-=TabContainer::CONTAINER_HEIGHT-scrollY;

		if(TreeViewNode::onmousepressed(elements,this,mx,my,bitmapWidth,bitmapHeight))
			this->DrawItems();
		
		this->OnPaint();
	}
}

void TreeView::OnRun()
{
	if(tab->mouseDown)
		this->OnLMouseDown();
}

void TreeView::OnReparent()
{
	this->OnSize();
	this->RecreateTarget();
	this->DrawItems();
}

void TreeView::DrawItems()
{
	if(bitmaprenderer)
		bitmaprenderer->Release();

	tab->renderer->CreateCompatibleRenderTarget(D2D1::SizeF(bitmapWidth,bitmapHeight),&bitmaprenderer);

	if(!bitmaprenderer)
		bitmaprenderer->Release();

	D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
	bp.pixelFormat=bitmaprenderer->GetPixelFormat();


	bitmaprenderer->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ARROW_WH,TabContainer::CONTAINER_ARROW_WH),TabContainer::rawRightArrow,TabContainer::CONTAINER_ARROW_STRIDE,bp,&rightArrow);

	if(!rightArrow)
		__debugbreak();

	bitmaprenderer->CreateBitmap(D2D1::SizeU(TabContainer::CONTAINER_ARROW_WH,TabContainer::CONTAINER_ARROW_WH),TabContainer::rawDownArrow,TabContainer::CONTAINER_ARROW_STRIDE,bp,&downArrow);

	if(!downArrow)
		__debugbreak();

	bitmaprenderer->BeginDraw();

	TreeViewNode::drawlist(elements,this);

	bitmaprenderer->DrawRectangle(D2D1::RectF(0,0,bitmapWidth,bitmapHeight),tab->SetColor(TabContainer::COLOR_TEXT));

	bitmaprenderer->EndDraw();

	LRESULT result=bitmaprenderer->GetBitmap(&bitmap);

	if(result!=S_OK)
		__debugbreak();

	this->OnPaint();
}

void TreeView::OnPaint()
{
	if(!tab->isRender)
		tab->renderer->BeginDraw();


	tab->renderer->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)frameWidth,(float)tab->height),this->tab->SetColor(GuiInterface::COLOR_GUI_BACKGROUND));

	tab->renderer->PushAxisAlignedClip(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)frameWidth,(float)tab->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tab->renderer->SetTransform(D2D1::Matrix3x2F::Translation(0,(float)TabContainer::CONTAINER_HEIGHT-scrollY));

	TreeViewNode::drawselection(elements,this);

	tab->renderer->DrawBitmap(bitmap);

	tab->renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	tab->renderer->PopAxisAlignedClip();

	tab->renderer->FillRectangle(D2D1::RectF((float)frameWidth,(float)TabContainer::CONTAINER_HEIGHT,(float)tab->width,(float)TabContainer::CONTAINER_HEIGHT+ScrollBar::SCROLLBAR_TIP_HEIGHT),this->tab->SetColor(TabContainer::COLOR_TAB_SELECTED));
	tab->renderer->FillRectangle(D2D1::RectF((float)frameWidth,(float)(tab->height-ScrollBar::SCROLLBAR_TIP_HEIGHT),(float)(tab->width),(float)(tab->height)),this->tab->SetColor(TabContainer::COLOR_TAB_SELECTED));

	if(!tab->isRender)
		tab->renderer->EndDraw();
}



