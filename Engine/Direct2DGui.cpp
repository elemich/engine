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
		SAFERELEASE(imager);
		SAFERELEASE(pDecoder);
		SAFERELEASE(pSource);
		SAFERELEASE(pStream);
		SAFERELEASE(pConverter);
		SAFERELEASE(pScaler);
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

	UINT width,height;
	pSource->GetSize(&width,&height);

	buffer=new unsigned char[width*height*4];

	hr=pSource->CopyPixels(0,width*4,width*height*4,buffer);

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



ScrollBar::ScrollBar(TabContainer* tc)
{
	this->tab=tc;
	this->Set(0,0,10,10);
	this->scroller=0;
	this->scrollerClick=-1;
}
ScrollBar::~ScrollBar()
{
	
}

void ScrollBar::Set(float _x,float _y,float _width,float _height)
{
	x=_x,y=_y,width=_width,height=_height;
}

void ScrollBar::SetScrollerFactor(float contentHeight,float containerHeight)
{
	this->scrollerFactor = (contentHeight<containerHeight) ? 1.0f : containerHeight/contentHeight;
}

void ScrollBar::SetScrollerPosition(float position)
{
	if(scrollerFactor==1.0f)
		return;

	scroller+=position>0 ? ScrollBar::SCROLLBAR_AMOUNT : -ScrollBar::SCROLLBAR_AMOUNT;
}

float ScrollBar::GetScrollValue()
{
	return 1.0f/this->scrollerFactor * scroller;;
}

void ScrollBar::OnReleased()
{
	if(this->scrollerClick>=0)
		ReleaseCapture();
	this->scrollerClick=-1;
	
}


float ScrollBar::OnPressed()
{
	if(scrollerFactor==1.0f)
		return scroller;

	float scrollerRun=(height-y)-(ScrollBar::SCROLLBAR_TIP_HEIGHT*2);
	float scrollerHeight=scrollerRun*scrollerFactor;

	if(this->tab->mousey>TabContainer::CONTAINER_HEIGHT && this->tab->mousey<(TabContainer::CONTAINER_HEIGHT+ScrollBar::SCROLLBAR_TIP_HEIGHT))
	{
		scroller=(scroller-ScrollBar::SCROLLBAR_AMOUNT < 0 ? 0 : scroller-ScrollBar::SCROLLBAR_AMOUNT);
	}
	else if(this->tab->mousey>(scroller+y+ScrollBar::SCROLLBAR_TIP_HEIGHT) && this->tab->mousey<((scroller+y+ScrollBar::SCROLLBAR_TIP_HEIGHT)+scrollerHeight))
	{
		scrollerClick=this->tab->mousey-y-ScrollBar::SCROLLBAR_TIP_HEIGHT-scroller;
		SetCapture(this->tab->hwnd);
	}
	else if(this->tab->mousey>(height-ScrollBar::SCROLLBAR_TIP_HEIGHT))
	{
		scroller+=ScrollBar::SCROLLBAR_AMOUNT;
	}

	return scroller;
}

bool ScrollBar::OnScrolled()
{
	if(scrollerFactor==1.0f || scrollerClick<0)
		return false;

	float scrollerRun=(height-y)-(ScrollBar::SCROLLBAR_TIP_HEIGHT*2);
	float scrollerHeight=scrollerRun*scrollerFactor;

	float mouseY=this->tab->mousey-y-ScrollBar::SCROLLBAR_TIP_HEIGHT;

	float newPosition=mouseY-scrollerClick;

	if(newPosition>=0 && newPosition<(height-ScrollBar::SCROLLBAR_TIP_HEIGHT)+scrollerHeight)
		scroller=mouseY-scrollerClick;
	else
		printf("");

	return true;
}
void ScrollBar::OnPaint()
{
	float scrollerRun=(height-y)-(ScrollBar::SCROLLBAR_TIP_HEIGHT*2);

	float scrollerHeight=scrollerRun*scrollerFactor;

	tab->renderer->DrawBitmap(tab->iconUp,D2D1::RectF(x,y,x+ScrollBar::SCROLLBAR_WIDTH,y+ScrollBar::SCROLLBAR_TIP_HEIGHT));

	float scrollerY=scroller+y+ScrollBar::SCROLLBAR_TIP_HEIGHT;
	float scrollerY2=scrollerY+scrollerHeight;

	tab->renderer->FillRectangle(D2D1::RectF(x,scrollerY,x+ScrollBar::SCROLLBAR_WIDTH,scrollerY2),this->tab->SetColor(D2D1::ColorF::Black));

	tab->renderer->DrawBitmap(tab->iconDown,D2D1::RectF(x,height-ScrollBar::SCROLLBAR_TIP_HEIGHT,x+ScrollBar::SCROLLBAR_WIDTH,height));
}







TreeView::TreeViewNode::TreeViewNode()
{
	clear();
}
TreeView::TreeViewNode::~TreeViewNode()
{
	clear();
}

void TreeView::TreeViewNode::insert(Entity* entity,HDC hdc,float& width,float& height,TreeView::TreeViewNode* parent,int expandUntilLevel)
{
	if(!entity)
		return;

	height+= (parent && !parent->expanded) ? 0 : TreeView::TREEVIEW_ROW_HEIGHT;

	this->parent=parent;
	this->entity=entity;
	this->x=parent ? parent->x+TreeView::TREEVIEW_ROW_ADVANCE: TreeView::TREEVIEW_ROW_ADVANCE;
	this->y=height-TreeView::TREEVIEW_ROW_HEIGHT;
	this->level=parent ? parent->level+1 : 0;
	this->expanded=this->level<expandUntilLevel ? true : false;
	this->selected=false;
	
	this->hasChilds=(int)entity->entity_childs.size();

	SIZE tSize;
	GetTextExtentPoint32(hdc,entity->entity_name,entity->entity_name.Count(),&tSize);

	this->textWidth=tSize.cx;

	if(!parent || parent && parent->expanded)
		width=this->textWidth+this->x > width ? this->textWidth+this->x : width;
	
	for(std::list<Entity*>::iterator eCh=entity->entity_childs.begin();eCh!=entity->entity_childs.end();eCh++)
	{	
		this->childs.push_back(TreeViewNode());
		this->childs.back().insert(*eCh,hdc,width,height,this,expandUntilLevel);
	}
}

void TreeView::TreeViewNode::update(float& width,float& height)
{
	if(!this->entity)
		return;

	height+=this->parent ? (this->parent->expanded ? TreeView::TREEVIEW_ROW_HEIGHT : 0) : TreeView::TREEVIEW_ROW_HEIGHT;

	this->x=this->parent ? this->parent->x+TreeView::TREEVIEW_ROW_ADVANCE : TreeView::TREEVIEW_ROW_ADVANCE;
	this->y=height-TreeView::TREEVIEW_ROW_HEIGHT;

	width=this->parent ? (this->parent->expanded ? (this->textWidth+this->x > width ? this->textWidth+this->x : width) : width) : this->textWidth+this->x;
	
	if(this->expanded)
	{
		for(std::list<TreeViewNode>::iterator nCh=this->childs.begin();nCh!=this->childs.end();nCh++)
			nCh->update(width,height);
	}
}
void TreeView::TreeViewNode::drawlist(TreeView* tv)
{
	if(!this->entity)
		return;

	size_t resLen=0;
	wchar_t resText[CHAR_MAX];
	mbstowcs_s(&resLen,resText,CHAR_MAX,this->entity->entity_name,this->entity->entity_name.Count());

	if(this->hasChilds)
		tv->bitmaprenderer->DrawBitmap(this->expanded ? tv->tab->iconDown : tv->tab->iconRight,D2D1::RectF(this->x-TreeView::TREEVIEW_ROW_ADVANCE,this->y,this->x,this->y+TreeView::TREEVIEW_ROW_HEIGHT));

	tv->bitmaprenderer->DrawText(resText,resLen,Direct2DGuiBase::texter,D2D1::RectF(this->x,this->y,this->x+this->textWidth,this->y+TreeView::TREEVIEW_ROW_HEIGHT),tv->tab->SetColor(TabContainer::COLOR_TEXT));

	if(this->expanded)
	{
		for(std::list<TreeViewNode>::iterator nCh=this->childs.begin();nCh!=this->childs.end();nCh++)
			nCh->drawlist(tv);
	}
}

void TreeView::TreeViewNode::drawselection(TreeView* tv)
{
	TabContainer* tabContainer=tv->tab;

	if(this->selected)
		tabContainer->renderer->FillRectangle(D2D1::RectF(0,(float)this->y,(float)tabContainer->width,(float)this->y+TreeView::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

	if(this->expanded)
	{
		for(std::list<TreeViewNode>::iterator nCh=this->childs.begin();nCh!=this->childs.end();nCh++)
			nCh->drawselection(tv);
	}
}



TreeView::TreeViewNode* TreeView::TreeViewNode::onmousepressed(TreeView* tv,float& x,float& y,float& width,float& height)
{
	if(y>this->y && y<this->y+TreeView::TREEVIEW_ROW_HEIGHT)
	{
		if(this->hasChilds && x>this->x-TreeView::TREEVIEW_ROW_ADVANCE && x<this->x)
		{
			this->expanded=!this->expanded;
			return this;
		}
		else
			this->selected=true;
	}
	else
		this->selected=false;

	if(this->expanded)
	{
		for(std::list<TreeViewNode>::iterator nCh=this->childs.begin();nCh!=this->childs.end();nCh++)
		{
			TreeViewNode* childNode=nCh->onmousepressed(tv,x,y,width,height);
			if(childNode)
				return childNode;
		}
	}

	return 0;
}



void TreeView::TreeViewNode::clear()
{
	this->parent=0;
	this->entity=0;
	this->x=0;
	this->y=0;
	this->expanded=0;
	this->selected=0;
	this->textWidth=0;
	this->level=0;
	this->hasChilds=0;

	for(std::list<TreeViewNode>::iterator nCh=this->childs.begin();nCh!=this->childs.end();nCh++)
		nCh->clear();

	this->childs.clear();
}





TreeView::TreeView(TabContainer* tc):
bitmaprenderer(0),
bitmap(0),
bitmapWidth((float)tc->width),
bitmapHeight((float)tc->height),
scrollBar(tc)
{
	tab=tc;

	frameWidth=tab->width-20;
	frameHeight=tab->height-30;

	this->name="Entities";

	this->RecreateTarget();

	scrollBar.SetScrollerFactor(bitmapHeight,frameHeight);

	OnEntitiesChange();
}

void TreeView::RecreateTarget()
{
	DrawItems();
}


TreeView::~TreeView()
{
	printf("destrouying treeview %p\n",this);
}

void TreeView::OnEntitiesChange()
{
	HDC hdc=GetDC(tab->hwnd);
	int nLevels=0;

	elements.clear();

	Entity* rootEntity=Entity::pool.size() ? Entity::pool.front() : 0;

	elements.insert(rootEntity,GetDC(tab->hwnd),bitmapWidth=0,bitmapHeight=0,0);

	scrollBar.SetScrollerFactor(bitmapHeight,this->frameHeight);

	this->DrawItems();
	this->OnPaint();
}

void TreeView::OnSize()
{
	frameWidth=tab->width-ScrollBar::SCROLLBAR_WIDTH;
	frameHeight=tab->height-TabContainer::CONTAINER_HEIGHT;

	scrollBar.Set(frameWidth-ScrollBar::SCROLLBAR_WIDTH,(float)TabContainer::CONTAINER_HEIGHT,(frameWidth-ScrollBar::SCROLLBAR_WIDTH)+ScrollBar::SCROLLBAR_WIDTH,frameHeight);
}

void TreeView::OnLMouseDown()
{
	float &mx=tab->mousex;
	float &my=tab->mousey;

	if(mx>frameWidth) //mouarse is on the scrollb
	{
		scrollBar.OnPressed();
		this->OnPaint();
	}
	else// if(mx>TabContainer::CONTAINER_HEIGHT && mx<(TabContainer::CONTAINER_HEIGHT+frameHeight))//mouse is on the frame
	{
		my-=TabContainer::CONTAINER_HEIGHT-scrollBar.scroller;

		TreeViewNode* updatedNode=elements.onmousepressed(this,mx,my,bitmapWidth,bitmapHeight);

		if(updatedNode)
		{
			elements.update(bitmapWidth=0,bitmapHeight=0);
			scrollBar.SetScrollerFactor(bitmapHeight,this->frameHeight);
			this->DrawItems();
		}	
	
		this->OnPaint();

		/*for(int i=0;i<(int)guiInterfacesPool.size();i++)
			guiInterfacesPool[i]->OnSelected(sel);*/
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
	SAFERELEASE(bitmaprenderer);
	

	tab->renderer->CreateCompatibleRenderTarget(D2D1::SizeF(bitmapWidth,bitmapHeight),&bitmaprenderer);

	D2D1_PIXEL_FORMAT pf=bitmaprenderer->GetPixelFormat();

	bitmaprenderer->BeginDraw();

	elements.drawlist(this);

	bitmaprenderer->DrawRectangle(D2D1::RectF(0,0,bitmapWidth,bitmapHeight),tab->SetColor(TabContainer::COLOR_TEXT));

	bitmaprenderer->EndDraw();

	LRESULT result=bitmaprenderer->GetBitmap(&bitmap);

	if(result!=S_OK)
		__debugbreak();
}

void TreeView::OnPaint()
{
	if(!tab->isRender)
		tab->renderer->BeginDraw();

	tab->renderer->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)frameWidth,(float)tab->height),this->tab->SetColor(Gui::COLOR_GUI_BACKGROUND));

	tab->renderer->PushAxisAlignedClip(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)frameWidth,(float)tab->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tab->renderer->SetTransform(D2D1::Matrix3x2F::Translation(0,(float)TabContainer::CONTAINER_HEIGHT-scrollBar.scroller));

	elements.drawselection(this);

	tab->renderer->DrawBitmap(bitmap);

	tab->renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	tab->renderer->PopAxisAlignedClip();

	scrollBar.OnPaint();

	if(!tab->isRender)
		tab->renderer->EndDraw();
}












void Properties::PropertiesNode::insert(Entity* entity,HDC hdc,float& width,float& height,PropertiesNode* parent,int expandUntilLevel)
{
	if(!entity)
		return;

	this->entity=entity;
	this->parent=parent;

	this->update(width,height);

	for(std::list<Entity*>::iterator eCh=entity->entity_childs.begin();eCh!=entity->entity_childs.end();eCh++)
	{	
		this->childs.push_back(PropertiesNode());
		this->childs.back().insert(*eCh,hdc,width,height,this,expandUntilLevel);
	}
}

void Properties::PropertiesNode::update(float& width,float& height)
{
	this->panels.push_back(PanelData("Entity"));
	PanelData& pdEntity=panels.back();

	if(entity->GetBone())
	{
		this->panels.push_back(PanelData("Bone"));
		PanelData& pdBone=panels.back();
	}
	if(entity->GetMesh())
	{
		this->panels.push_back(PanelData("Mesh"));
		PanelData& pdMesh=panels.back();
	}
	if(entity->GetLight())
	{
		this->panels.push_back(PanelData("Light"));
		PanelData& pdLight=panels.back();
	}
	if(entity->GetSkin())
	{
		this->panels.push_back(PanelData("Skin"));
		PanelData& pdSkin=panels.back();
	}
}

void Properties::PropertiesNode::draw(Properties* tv)
{
	if(!entity)
		return;

	//tv->tab->renderer->BeginDraw();



	tv->tab->renderer->PushAxisAlignedClip(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)tv->tab->width,(float)tv->tab->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tv->tab->renderer->SetTransform(D2D1::Matrix3x2F::Translation(0,(float)TabContainer::CONTAINER_HEIGHT));

	for(int i=0;i<(int)panels.size();i++)
	{
		tv->tab->renderer->DrawBitmap(panels[i].expanded ? tv->tab->iconDown : tv->tab->iconRight,D2D1::RectF(0,i*20.0f,20.0f,20.0f));
		Direct2DGuiBase::DrawText(tv->tab->renderer,tv->tab->SetColor(TabContainer::COLOR_TEXT),panels[i].name,20.0f,i*20.0f,(float)tv->tab->width,20.0f);
	}

	tv->tab->renderer->PopAxisAlignedClip();

	//tv->tab->renderer->EndDraw();
}
void Properties::PropertiesNode::drawselection(Properties* tv)
{

}
void Properties::PropertiesNode::clear()
{
	this->parent=0;
	this->entity=0;

	for(std::list<PropertiesNode>::iterator nCh=this->childs.begin();nCh!=this->childs.end();nCh++)
		nCh->clear();

	this->childs.clear();
}

Properties::PropertiesNode* Properties::PropertiesNode::onmousepressed(Properties* tv,float& x,float& y,float& width,float& height)
{
	return 0;
}

Properties::Properties(TabContainer* tc)
{
	this->tab=tc;
	this->name="Properties";

	RecreateTarget();
};

Properties::~Properties()
{
	printf("destrouying properties %p\n",this);
}

void Properties::OnPaint()
{
	if(!tab->isRender)
		tab->renderer->BeginDraw();

	tab->renderer->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)tab->width,(float)tab->height),tab->SetColor(Gui::COLOR_GUI_BACKGROUND));

	this->DrawItems();

	if(!tab->isRender)
		tab->renderer->EndDraw();

};

void Properties::OnEntitiesChange()
{
	elements.clear();

	Entity* rootEntity=Entity::pool.size() ? Entity::pool.front() : 0;

	float w,h;
	elements.insert(rootEntity,0,w=0,h=0,0,0);

	this->OnPaint();
};

void Properties::DrawItems()
{
	elements.draw(this);
};



void Resources::ResourceNode::clear()
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

bool Resources::ResourceNode::ScanDir(const char* dir,HANDLE& handle,WIN32_FIND_DATA& data,int opt)
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

		printf("filename %s\n",data.cFileName);

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

void Resources::ResourceNode::insertDirectory(String &fullPath,HANDLE parentHandle,WIN32_FIND_DATA found,HDC hdc,float& width,float& height,ResourceNode* parent,int expandUntilLevel)
{
	fileName= !parent ? fullPath : found.cFileName;

	height+= (parent && !parent->expanded) ? 0 : TreeView::TREEVIEW_ROW_HEIGHT;
	

	this->parent=parent;
	this->x=parent ? parent->x+TreeView::TREEVIEW_ROW_ADVANCE : TreeView::TREEVIEW_ROW_ADVANCE;
	this->y=height-TreeView::TREEVIEW_ROW_HEIGHT;
	this->level=parent ? parent->level+1 : 0;
	this->expanded=this->level<expandUntilLevel ? true : false;
	this->selected=false;
	this->nChilds = 0;
	this->isDir = true;

	parent ? parent->nChilds++ : 0;

	const char* pText=!parent ? fullPath.Buf() : found.cFileName;

	SIZE tSize;
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
			String dir=fullPath + "\\" + found.cFileName;

			this->childsDirs.push_back(ResourceNode());
			this->childsDirs.back().insertDirectory(dir,0,found,hdc,width,height,this,expandUntilLevel);
		}

		
	}
}


void Resources::ResourceNode::insertFiles(Resources::ResourceNode& directory,HDC hdc,float& width,float& height)
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

		height+=TreeView::TREEVIEW_ROW_HEIGHT;

		node.parent=&directory;
		node.x=0;
		node.y=height-TreeView::TREEVIEW_ROW_HEIGHT;
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


void Resources::ResourceNode::update(float& width,float& height)
{
	height+=this->parent ? (this->parent->expanded ? TreeView::TREEVIEW_ROW_HEIGHT : 0) : TreeView::TREEVIEW_ROW_HEIGHT;

	this->x=this->parent ? this->parent->x+TreeView::TREEVIEW_ROW_ADVANCE : TreeView::TREEVIEW_ROW_ADVANCE;
	this->y=height-TreeView::TREEVIEW_ROW_HEIGHT;

	width=this->parent ? (this->parent->expanded ? (this->textWidth+this->x > width ? this->textWidth+this->x : width) : width) : this->textWidth+this->x;

	if(this->expanded)
	{
		for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
			nCh->update(width,height);
	}
}
void Resources::ResourceNode::drawdirlist(Resources* tv)
{
	size_t resLen=0;
	wchar_t resText[CHAR_MAX];
	mbstowcs_s(&resLen,resText,CHAR_MAX,this->fileName,this->fileName.Count());

	if(this->nChilds)
		tv->leftbitmaprenderer->DrawBitmap(this->expanded ? tv->tab->iconDown : tv->tab->iconRight,D2D1::RectF(this->x-TreeView::TREEVIEW_ROW_ADVANCE,this->y,this->x,this->y+TreeView::TREEVIEW_ROW_HEIGHT));
		
	tv->leftbitmaprenderer->DrawBitmap(tv->tab->iconFolder,D2D1::RectF(this->x,this->y,this->x + TreeView::TREEVIEW_ROW_ADVANCE,this->y+TreeView::TREEVIEW_ROW_HEIGHT));

	tv->leftbitmaprenderer->DrawText(resText,resLen,Direct2DGuiBase::texter,D2D1::RectF(this->x + TreeView::TREEVIEW_ROW_ADVANCE,this->y,this->x + TreeView::TREEVIEW_ROW_ADVANCE + this->textWidth,this->y+TreeView::TREEVIEW_ROW_HEIGHT),tv->tab->SetColor(TabContainer::COLOR_TEXT));

	if(this->expanded)
	{
		for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
			nCh->drawdirlist(tv);
	}
}

void Resources::ResourceNode::drawfilelist(Resources* tv)
{
	if(this->parent)
	{
		size_t resLen=0;
		wchar_t resText[CHAR_MAX];
		mbstowcs_s(&resLen,resText,CHAR_MAX,this->fileName,this->fileName.Count());

		if(this->nChilds)
			tv->rightbitmaprenderer->DrawBitmap(this->expanded ? tv->tab->iconDown : tv->tab->iconRight,D2D1::RectF(this->x-TreeView::TREEVIEW_ROW_ADVANCE,this->y,this->x,this->y+TreeView::TREEVIEW_ROW_HEIGHT));

		tv->rightbitmaprenderer->DrawBitmap(this->isDir ? tv->tab->iconFolder : tv->tab->iconFile,D2D1::RectF(this->x,this->y,this->x + TreeView::TREEVIEW_ROW_ADVANCE,this->y+TreeView::TREEVIEW_ROW_HEIGHT));

		tv->rightbitmaprenderer->DrawText(resText,resLen,Direct2DGuiBase::texter,D2D1::RectF(this->x + TreeView::TREEVIEW_ROW_ADVANCE,this->y,this->x + TreeView::TREEVIEW_ROW_ADVANCE + this->textWidth,this->y+TreeView::TREEVIEW_ROW_HEIGHT),tv->tab->SetColor(TabContainer::COLOR_TEXT));
	}

	for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
		nCh->drawfilelist(tv);
}

void Resources::ResourceNode::drawdirselection(Resources* tv)
{
	TabContainer* tabContainer=tv->tab;

	if(this->selected)
		tabContainer->renderer->FillRectangle(D2D1::RectF(0,(float)this->y,(float)tv->leftFrameWidth-ScrollBar::SCROLLBAR_WIDTH-4,(float)this->y+TreeView::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

	if(this->expanded)
	{
		for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
			nCh->drawdirselection(tv);
	}
}


void Resources::ResourceNode::drawfileselection(Resources* tv)
{
	TabContainer* tabContainer=tv->tab;

	if(this->selected)
		tabContainer->renderer->FillRectangle(D2D1::RectF(tv->leftFrameWidth,this->y,tabContainer->width-ScrollBar::SCROLLBAR_WIDTH,this->y+TreeView::TREEVIEW_ROW_HEIGHT),tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

	for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
		nCh->drawfileselection(tv);
}



Resources::ResourceNode* Resources::ResourceNode::onmousepressedLeftPane(Resources* tv,float& x,float& y,float& width,float& height)
{
	if(y>this->y && y<this->y+TreeView::TREEVIEW_ROW_HEIGHT)
	{
		if(this->nChilds && x>this->x-TreeView::TREEVIEW_ROW_ADVANCE && x<this->x)
		{
			this->expanded=!this->expanded;
			return this;
		}
		else
		{
			this->selected=true;
			tv->rightElements.insertFiles(*this,GetDC(tv->tab->hwnd),tv->rightBitmapWidth=0,tv->rightBitmapHeight=0);
			tv->rightScrollBar.SetScrollerFactor(tv->rightBitmapHeight,tv->frameHeight);
			tv->DrawRightItems();
		}
	}
	else
		this->selected=false;

	if(this->expanded)
	{
		ResourceNode* resNode=0;

		for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
		{
			if((resNode=nCh->onmousepressedLeftPane(tv,x,y,width,height)))  //((a=b))
				return resNode;
		}
	}

	return 0;
}

Resources::ResourceNode* Resources::ResourceNode::onmousepressedRightPane(Resources* tv,float& x,float& y,float& width,float& height)
{
	if(y>this->y && y<this->y+TreeView::TREEVIEW_ROW_HEIGHT)
	{
		this->selected=true;
	}
	else
		this->selected=false;

	ResourceNode* resNode=0;

	for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
	{
		if((resNode=nCh->onmousepressedRightPane(tv,x,y,width,height)))  //((a=b))
			return resNode;
	}

	return 0;
}

Resources::Resources(TabContainer* tc):
leftbitmaprenderer(0),
rightbitmaprenderer(0),
leftBitmap(0),
rightBitmap(0),
leftBitmapWidth(0),
leftBitmapHeight(0),
rightBitmapWidth(0),
rightBitmapHeight(0),
leftFrameWidth(100),
lMouseDown(false),
splitterMoving(false),
leftScrollBar(tc),
rightScrollBar(tc)
{
	tab=tc;

	leftFrameWidth=(tab->width-20)/2;
	frameHeight=tab->height-30;

	

	this->SetLeftScrollBar();
	this->SetRightScrollBar();
	

	this->name="Project";

	this->OnEntitiesChange();

	this->RecreateTarget();

	this->DrawItems();
}

void Resources::RecreateTarget()
{
	DrawItems();
}

void Resources::SetLeftScrollBar()
{
	leftScrollBar.Set(this->leftFrameWidth-4-ScrollBar::SCROLLBAR_WIDTH,(float)TabContainer::CONTAINER_HEIGHT,this->leftFrameWidth-4,this->tab->height);
	leftScrollBar.SetScrollerFactor(leftBitmapHeight,frameHeight);
}
void Resources::SetRightScrollBar()
{
	rightScrollBar.Set(this->tab->width-ScrollBar::SCROLLBAR_WIDTH,(float)TabContainer::CONTAINER_HEIGHT,this->tab->width,this->tab->height);
	rightScrollBar.SetScrollerFactor(rightBitmapHeight,frameHeight);
}


Resources::~Resources()
{
	printf("destrouying resources %p\n",this);
}

void Resources::OnEntitiesChange()
{
	HDC hdc=GetDC(tab->hwnd);
	int nLevels=0;

	leftElements.clear();

	Entity* rootEntity=Entity::pool.size() ? Entity::pool.front() : 0;

	leftElements.insertDirectory(App::instance->projectFolder,0,WIN32_FIND_DATA(),GetDC(tab->hwnd),leftBitmapWidth=0,leftBitmapHeight=0,0);
	rightElements.insertFiles(leftElements,GetDC(tab->hwnd),rightBitmapWidth=0,rightBitmapHeight=0);

	this->SetLeftScrollBar();
	this->SetRightScrollBar();

	this->DrawItems();
	this->OnPaint();
}

void Resources::OnSize()
{
	frameHeight=tab->height-TabContainer::CONTAINER_HEIGHT;

	if(this->leftFrameWidth>this->tab->width)
		this->leftFrameWidth=(this->tab->width-ScrollBar::SCROLLBAR_WIDTH)/2.0f;

	this->SetLeftScrollBar();
	this->SetRightScrollBar();
}

void Resources::OnMouseMove()
{
	if(this->splitterMoving)
	{
		this->leftFrameWidth=this->tab->mousex;
		this->SetLeftScrollBar();
		this->OnPaint();
	}
	else
	{
		if(tab->mousex>=this->leftFrameWidth-4 && tab->mousex<=this->leftFrameWidth)
		{
			SetCursor(LoadCursor(0,IDC_SIZEWE));
		}
		else
		{
			SetCursor(LoadCursor(0,IDC_ARROW));
		}
	}

	if(leftScrollBar.OnScrolled() || rightScrollBar.OnScrolled())
		this->OnPaint();
}

void Resources::OnMouseWheel()
{
	short int delta = GET_WHEEL_DELTA_WPARAM(tab->wparam);

	if(this->tab->mousex<(this->leftFrameWidth-4))
	{
		leftScrollBar.SetScrollerPosition(-delta);
	}
	else if(this->tab->mousex>(this->leftFrameWidth+4))
	{
		rightScrollBar.SetScrollerPosition(-delta);
	}

	this->OnPaint();
}

void Resources::OnLMouseUp()
{
	lMouseDown=false;
	splitterMoving=false;
	leftScrollBar.OnReleased();
	rightScrollBar.OnReleased();
}

void Resources::OnLMouseDown()
{
	float &mx=tab->mousex;
	float &my=tab->mousey;

	lMouseDown=true;

	if(mx<(this->leftFrameWidth-4-ScrollBar::SCROLLBAR_WIDTH))
	{
		my-=TabContainer::CONTAINER_HEIGHT-leftScrollBar.scroller;

		ResourceNode* expandedModified=leftElements.onmousepressedLeftPane(this,mx,my,leftBitmapWidth,leftBitmapHeight);

		if(expandedModified)
		{
			leftElements.update(leftBitmapWidth=0,leftBitmapHeight=0);
			leftScrollBar.SetScrollerFactor(leftBitmapHeight,this->frameHeight);
			this->DrawLeftItems();
		}	
	
		this->OnPaint();
	}
	else if(mx<this->leftFrameWidth-4)//left scrollbar
	{
		leftScrollBar.OnPressed();
		this->OnPaint();
	}
	else if(mx<this->leftFrameWidth)//splitter
	{
		printf("on the splitter\n");
		splitterMoving=true;
	}
	else if(mx<(this->tab->width-ScrollBar::SCROLLBAR_WIDTH))//right pane
	{
		my-=TabContainer::CONTAINER_HEIGHT-rightScrollBar.scroller;

		printf("on the right pane\n");
		rightElements.onmousepressedRightPane(this,mx,my,rightBitmapWidth,rightBitmapHeight);
		this->OnPaint();
	}
	else//right scrollbar
	{
		rightScrollBar.OnPressed();
		this->OnPaint();
	}
	
}

void Resources::OnRun()
{
	if(tab->mouseDown)
		this->OnLMouseDown();
}

void Resources::OnReparent()
{
	this->OnSize();
	this->RecreateTarget();
	this->DrawItems();
}

void Resources::DrawLeftItems()
{
	LRESULT result;

	SAFERELEASE(leftbitmaprenderer);

	result=tab->renderer->CreateCompatibleRenderTarget(D2D1::SizeF(leftBitmapWidth,leftBitmapHeight),&leftbitmaprenderer);

	if(result!=S_OK)
		__debugbreak();

	D2D1_PIXEL_FORMAT pf=leftbitmaprenderer->GetPixelFormat();

	//draw left pane

	leftbitmaprenderer->BeginDraw();

	leftElements.drawdirlist(this);

	leftbitmaprenderer->DrawRectangle(D2D1::RectF(0,0,leftBitmapWidth,leftBitmapHeight),tab->SetColor(TabContainer::COLOR_TEXT));

	leftbitmaprenderer->EndDraw();

	result=leftbitmaprenderer->GetBitmap(&leftBitmap);

	if(result!=S_OK)
		__debugbreak();
}

void Resources::DrawRightItems()
{
	LRESULT result;
	//draw right pane
	SAFERELEASE(rightbitmaprenderer);

	result=tab->renderer->CreateCompatibleRenderTarget(D2D1::SizeF(rightBitmapWidth,rightBitmapHeight),&rightbitmaprenderer);

	if(result!=S_OK)
		__debugbreak();

	rightbitmaprenderer->BeginDraw();

	rightElements.drawfilelist(this);

	rightbitmaprenderer->DrawRectangle(D2D1::RectF(0,0,rightBitmapWidth,rightBitmapHeight),tab->SetColor(TabContainer::COLOR_TEXT));

	rightbitmaprenderer->EndDraw();

	result=rightbitmaprenderer->GetBitmap(&rightBitmap);

	if(result!=S_OK)
		__debugbreak();
}

void Resources::DrawItems()
{
	this->DrawLeftItems();
	this->DrawRightItems();
}

void Resources::OnPaint()
{
	

	if(!tab->isRender)
		tab->renderer->BeginDraw();

	tab->renderer->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)tab->width,(float)tab->height),this->tab->SetColor(Gui::COLOR_MAIN_BACKGROUND));

	//dirs hierarchy

	tab->renderer->PushAxisAlignedClip(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)this->leftFrameWidth-4,(float)tab->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tab->renderer->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)this->leftFrameWidth-4,(float)tab->height),this->tab->SetColor(Gui::COLOR_GUI_BACKGROUND));

	tab->renderer->SetTransform(D2D1::Matrix3x2F::Translation(0,(float)TabContainer::CONTAINER_HEIGHT-leftScrollBar.GetScrollValue()));

	leftElements.drawdirselection(this);

	tab->renderer->DrawBitmap(leftBitmap);

	tab->renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	tab->renderer->PopAxisAlignedClip();

	leftScrollBar.OnPaint();

	//files hierarchy

	tab->renderer->PushAxisAlignedClip(D2D1::RectF(this->leftFrameWidth,(float)TabContainer::CONTAINER_HEIGHT,(float)this->tab->width,(float)tab->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tab->renderer->FillRectangle(D2D1::RectF(this->leftFrameWidth,(float)TabContainer::CONTAINER_HEIGHT,(float)this->tab->width,(float)tab->height),this->tab->SetColor(Gui::COLOR_GUI_BACKGROUND));

	tab->renderer->SetTransform(D2D1::Matrix3x2F::Translation(0,(float)TabContainer::CONTAINER_HEIGHT-rightScrollBar.GetScrollValue()));

	rightElements.drawfileselection(this);

	tab->renderer->DrawBitmap(rightBitmap,D2D1::RectF(this->leftFrameWidth,0,this->leftFrameWidth+this->rightBitmapWidth,this->rightBitmapHeight));

	tab->renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	tab->renderer->PopAxisAlignedClip();

	rightScrollBar.OnPaint();

	if(!tab->isRender)
	{
		if(tab->renderer->EndDraw() & D2DERR_RECREATE_TARGET)
			tab->recreateTarget=true;
	}
}

