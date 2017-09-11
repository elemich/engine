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

void Direct2DGuiBase::DrawText(ID2D1RenderTarget*renderer,ID2D1Brush* brush,const char* inputText,float x,float y, float w,float h)
{
	if(!inputText)
		return;
	size_t retLen=0;
	wchar_t retText[CHAR_MAX];
	mbstowcs_s(&retLen,retText,CHAR_MAX,inputText,strlen(inputText));
	renderer->DrawText(retText,retLen,texter,D2D1::RectF(x,y,w,h),brush,D2D1_DRAW_TEXT_OPTIONS_NONE,DWRITE_MEASURING_MODE_GDI_CLASSIC);
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



ScrollBar::ScrollBar(TabContainer* _tab,float ix, float iy, float iw,float ih):
tab(_tab)
{
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

	scroller<this->y ? scroller=this->y : 0;

	float scrollerRun=(height-y)-(ScrollBar::SCROLLBAR_TIP_HEIGHT*2);
	float scrollerHeight=scrollerRun*scrollerFactor;

	scroller+scrollerHeight>this->y+this->height ? scroller=(this->y+this->height)-scrollerHeight : 0;
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

	if(tab->mousey>TabContainer::CONTAINER_HEIGHT && tab->mousey<(TabContainer::CONTAINER_HEIGHT+ScrollBar::SCROLLBAR_TIP_HEIGHT))
	{
		scroller=(scroller-ScrollBar::SCROLLBAR_AMOUNT < 0 ? 0 : scroller-ScrollBar::SCROLLBAR_AMOUNT);
	}
	else if(tab->mousey>(scroller+y+ScrollBar::SCROLLBAR_TIP_HEIGHT) && tab->mousey<((scroller+y+ScrollBar::SCROLLBAR_TIP_HEIGHT)+scrollerHeight))
	{
		scrollerClick=tab->mousey-y-ScrollBar::SCROLLBAR_TIP_HEIGHT-scroller;
		SetCapture(tab->hwnd);
	}
	else if(tab->mousey>(height-ScrollBar::SCROLLBAR_TIP_HEIGHT))
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

	float absScrollerY=tab->mousey-y-ScrollBar::SCROLLBAR_TIP_HEIGHT-scrollerClick;

	if(absScrollerY>=0 && tab->mousey+(scrollerHeight-scrollerClick)<=(this->height-ScrollBar::SCROLLBAR_TIP_HEIGHT))
		scroller=absScrollerY;
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

	tab->renderer->FillRectangle(D2D1::RectF(x,scrollerY,x+ScrollBar::SCROLLBAR_WIDTH,scrollerY2),tab->SetColor(D2D1::ColorF::Black));

	tab->renderer->DrawBitmap(tab->iconDown,D2D1::RectF(x,height-ScrollBar::SCROLLBAR_TIP_HEIGHT,x+ScrollBar::SCROLLBAR_WIDTH,height));
}







SceneEntityNode::SceneEntityNode()
{
	clear();
}
SceneEntityNode::~SceneEntityNode()
{
	clear();
}

void SceneEntityNode::insert(Entity* entity,HDC hdc,float& width,float& height,SceneEntityNode* parent,int expandUntilLevel)
{
	if(!entity)
		return;

	height+= (parent && !parent->expanded) ? 0 : SceneViewer::TREEVIEW_ROW_HEIGHT;

	this->parent=parent;
	this->entity=entity;
	this->x=parent ? parent->x+SceneViewer::TREEVIEW_ROW_ADVANCE: SceneViewer::TREEVIEW_ROW_ADVANCE;
	this->y=height-SceneViewer::TREEVIEW_ROW_HEIGHT;
	this->level=parent ? parent->level+1 : 0;
	this->expanded=this->level<expandUntilLevel ? true : false;
	this->selected=false;
	
	this->hasChilds=(int)entity->entity_childs.size();

	SIZE tSize;
	GetTextExtentPoint32(hdc,entity->entity_name,entity->entity_name.Count(),&tSize);

	this->textWidth=tSize.cx;

	if(!parent || parent && parent->expanded)
		width=this->textWidth+this->x > width ? this->textWidth+this->x : width;

	float pwidth=0,pheight=0;
	this->properties.insert(entity,hdc,pwidth,pheight,0,1);
	
	for(std::list<Entity*>::iterator eCh=entity->entity_childs.begin();eCh!=entity->entity_childs.end();eCh++)
	{	
		this->childs.push_back(SceneEntityNode());
		this->childs.back().insert(*eCh,hdc,width,height,this,expandUntilLevel);
	}
}

void SceneEntityNode::update(float& width,float& height)
{
	if(!this->entity)
		return;

	height+=this->parent ? (this->parent->expanded ? SceneViewer::TREEVIEW_ROW_HEIGHT : 0) : SceneViewer::TREEVIEW_ROW_HEIGHT;

	this->x=this->parent ? this->parent->x+SceneViewer::TREEVIEW_ROW_ADVANCE : SceneViewer::TREEVIEW_ROW_ADVANCE;
	this->y=height-SceneViewer::TREEVIEW_ROW_HEIGHT;

	width=this->parent ? (this->parent->expanded ? (this->textWidth+this->x > width ? this->textWidth+this->x : width) : width) : this->textWidth+this->x;
	
	if(this->expanded)
	{
		for(std::list<SceneEntityNode>::iterator nCh=this->childs.begin();nCh!=this->childs.end();nCh++)
			nCh->update(width,height);
	}
}









void SceneEntityNode::clear()
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

	for(std::list<SceneEntityNode>::iterator nCh=this->childs.begin();nCh!=this->childs.end();nCh++)
		nCh->clear();

	this->childs.clear();
}





SceneViewer::SceneViewer(TabContainer* tc):
GuiTab(tc),
bitmapWidth((float)tc->width),
bitmapHeight((float)tc->height),
scrollBar(tc)
{
	this->guiTabRootElement.width=0,this->guiTabRootElement.height=0;

	this->tabContainer=tc;

	this->frameWidth=this->tabContainer->width-20;
	this->frameHeight=this->tabContainer->height-30;

	this->name="Entities";

	this->OnRecreateTarget();

	scrollBar.Set(frameWidth,0,20,frameHeight);
	scrollBar.SetScrollerFactor(bitmapHeight,frameHeight);

	OnEntitiesChange();
}

void SceneViewer::OnRecreateTarget()
{
}


SceneViewer::~SceneViewer()
{
	printf("destroying treeview %p\n",this);
}

void SceneViewer::OnEntitiesChange()
{
	HDC hdc=GetDC(tabContainer->hwnd);
	int nLevels=0;

	elements.clear();

	Entity* rootEntity=Entity::pool.size() ? Entity::pool.front() : 0;

	elements.insert(rootEntity,GetDC(tabContainer->hwnd),bitmapWidth=0,bitmapHeight=0,0);

	scrollBar.SetScrollerFactor(bitmapHeight,this->frameHeight);

	this->OnPaint();
}

void SceneViewer::OnSize()
{
	frameWidth=tabContainer->width-ScrollBar::SCROLLBAR_WIDTH;
	frameHeight=tabContainer->height-TabContainer::CONTAINER_HEIGHT;

	scrollBar.Set(frameWidth-ScrollBar::SCROLLBAR_WIDTH,(float)TabContainer::CONTAINER_HEIGHT,(frameWidth-ScrollBar::SCROLLBAR_WIDTH)+ScrollBar::SCROLLBAR_WIDTH,frameHeight);
}

SceneEntityNode* SceneViewer::OnNodePressed(SceneEntityNode& node)
{
	if(this->tabContainer->mousey>node.y && this->tabContainer->mousey<node.y+SceneViewer::TREEVIEW_ROW_HEIGHT)
	{
		if(node.hasChilds && this->tabContainer->mousex>node.x-SceneViewer::TREEVIEW_ROW_ADVANCE && this->tabContainer->mousex<node.x)
		{
			node.expanded=!node.expanded;
			return &node;
		}
		else
		{
			node.selected=true;
			Editor::selection.push_back(&node);
		}
	}
	else
		node.selected=false;

	if(node.expanded)
	{
		for(std::list<SceneEntityNode>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
		{
			SceneEntityNode* childNode=this->OnNodePressed(*nCh);
			if(childNode)
				return childNode;
		}
	}

	return 0;
}


void SceneViewer::OnLMouseDown()
{
	float &mx=tabContainer->mousex;
	float &my=tabContainer->mousey;

	if(mx>frameWidth) //mouarse is on the scrollb
	{
		scrollBar.OnPressed();
		this->OnPaint();
	}
	else// if(mx>TabContainer::CONTAINER_HEIGHT && mx<(TabContainer::CONTAINER_HEIGHT+frameHeight))//mouse is on the frame
	{
		my-=TabContainer::CONTAINER_HEIGHT-scrollBar.scroller;

		if(!tabContainer->buttonControlDown)
			Editor::selection.clear();

		SceneEntityNode* updatedNode=this->OnNodePressed(elements);

		if(updatedNode)
		{
			elements.update(bitmapWidth=0,bitmapHeight=0);
			scrollBar.SetScrollerFactor(bitmapHeight,this->frameHeight);
		}	

		GuiInterface::Broadcast(&GuiInterface::OnSelected);

		this->OnPaint();
	}
}

void SceneViewer::OnUpdate()
{
	if(tabContainer->mouseDown)
		this->OnLMouseDown();

}

void SceneViewer::OnReparent()
{
	this->OnSize();
	this->OnRecreateTarget();
}

void SceneViewer::DrawNodeSelectionRecursive(SceneEntityNode& node)
{
	if(node.selected)
		this->tabContainer->renderer->FillRectangle(D2D1::RectF(0,(float)node.y,(float)this->tabContainer->width,(float)node.y+SceneViewer::TREEVIEW_ROW_HEIGHT),this->tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

	if(node.expanded)
	{
		for(std::list<SceneEntityNode>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
			this->DrawNodeSelectionRecursive(*nCh);
	}
}

void SceneViewer::DrawNodeRecursive(SceneEntityNode& node)
{
	if(!node.entity)
		return;

	size_t resLen=0;
	wchar_t resText[CHAR_MAX];
	mbstowcs_s(&resLen,resText,CHAR_MAX,node.entity->entity_name,node.entity->entity_name.Count());

	if(node.selected)
		this->tabContainer->renderer->FillRectangle(D2D1::RectF(0,(float)node.y,(float)this->tabContainer->width,(float)node.y+SceneViewer::TREEVIEW_ROW_HEIGHT),this->tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

	if(node.hasChilds)
		this->tabContainer->renderer->DrawBitmap(node.expanded ? this->tabContainer->iconDown : this->tabContainer->iconRight,D2D1::RectF(node.x-SceneViewer::TREEVIEW_ROW_ADVANCE,node.y,node.x,node.y+SceneViewer::TREEVIEW_ROW_HEIGHT));

	this->tabContainer->renderer->DrawText(resText,resLen,Direct2DGuiBase::texter,D2D1::RectF(node.x,node.y,node.x+node.textWidth,node.y+SceneViewer::TREEVIEW_ROW_HEIGHT),this->tabContainer->SetColor(GuiInterface::COLOR_TEXT));

	if(node.expanded)
	{
		for(std::list<SceneEntityNode>::iterator nCh=node.childs.begin();nCh!=node.childs.end();nCh++)
			this->DrawNodeRecursive(*nCh);
	}
}


void SceneViewer::OnPaint()
{
	tabContainer->BeginDraw();

	tabContainer->renderer->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)frameWidth,(float)tabContainer->height),this->tabContainer->SetColor(GuiTab::COLOR_GUI_BACKGROUND));

	tabContainer->renderer->PushAxisAlignedClip(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)frameWidth,(float)tabContainer->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tabContainer->renderer->SetTransform(D2D1::Matrix3x2F::Translation(0,(float)TabContainer::CONTAINER_HEIGHT-scrollBar.scroller));

	this->DrawNodeRecursive(elements);

	//tabContainer->renderer->DrawRectangle(D2D1::RectF(0,0,bitmapWidth,bitmapHeight),tabContainer->SetColor(GuiInterface::COLOR_TEXT));

	tabContainer->renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	tabContainer->renderer->PopAxisAlignedClip();

	scrollBar.OnPaint();

	tabContainer->EndDraw();
}












void SceneEntityPropertyNode::insert(Entity* _entity,HDC hdc,float& width,float& height,SceneEntityPropertyNode* _parent,int expandUntilLevel)
{
	if(!_entity)
		return;

	this->entity=_entity;
	this->parent=_parent;

	this->root._reset();


	this->root.name=this->entity->entity_name + " PropertiesPanel";
	this->root.text=this->entity->entity_name + " Properties";

	this->root.alignRect.make(1,1);
	this->root.alignPos.make(0.5f,0.5f);
	this->root.alignText.make(0.5f,0);

	GuiTabElement* panel;
	GuiTabElement* container1;	
	GuiTabElement* container2;	

	container1=this->root.CreateTabElementContainer(0,"Entity");

	panel=container1->CreateTabElementRow(0,"Name",this->entity->entity_name);
	container2=container1->CreateTabElementContainer(panel,"AABB");
	panel=container2->CreateTabElementRow(0,"min",_entity->entity_bbox.a);
	panel=container2->CreateTabElementRow(panel,"max",_entity->entity_bbox.b);
	panel=container1->CreateTabElementRow(container2,"Child Num",String((int)_entity->entity_childs.size()));

	if(_entity->GetBone())
	{
		container1=this->root.CreateTabElementContainer(container1,"Bone");
	}
	if(_entity->GetMesh())
	{
		container1=this->root.CreateTabElementContainer(container1,"Mesh");
	}
	if(_entity->GetLight())
	{
		container1=this->root.CreateTabElementContainer(container1,"Light");
		
	}
	if(_entity->GetSkin())
	{
		container1=this->root.CreateTabElementContainer(container1,"Skin");
		
	}
}

void SceneEntityPropertyNode::update(float& width,float& height)
{
	
}


void SceneEntityPropertyNode::clear()
{
	this->parent=0;
	this->entity=0;
}



Properties::Properties(TabContainer* tc):
	GuiTab(tc)
{
	this->name="Properties";
	this->guiTabRootElement.name+="_Properties";

	OnRecreateTarget();

	this->OnSelected();
};

Properties::~Properties()
{
	printf("destroying properties %p\n",this);
}

void Properties::OnPaint()
{
	GuiTab::OnPaint();
};

void Properties::OnSelected()
{
	this->guiTabRootElement.childs.clear();

	if(Editor::selection.size())
	{

		GuiTabElement* properties=&Editor::selection[0]->properties.root;

		this->guiTabRootElement.childs.push_back(properties);
		properties->parent=&this->guiTabRootElement;
	}
	else
	{
	}

	this->OnUpdate();
	this->OnPaint();
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

Resources::ResourceNode* tempResourceNode=0;

void Resources::ResourceNode::insertDirectory(String &fullPath,HANDLE parentHandle,WIN32_FIND_DATA found,HDC hdc,float& width,float& height,ResourceNode* parent,int expandUntilLevel)
{
	tempResourceNode=!tempResourceNode ? this : tempResourceNode;

	fileName= !parent ? fullPath : found.cFileName;

	height+= (parent && !parent->expanded) ? 0 : SceneViewer::TREEVIEW_ROW_HEIGHT;
	

	this->parent=parent;
	this->x=parent ? parent->x+SceneViewer::TREEVIEW_ROW_ADVANCE : SceneViewer::TREEVIEW_ROW_ADVANCE;
	this->y=height-SceneViewer::TREEVIEW_ROW_HEIGHT;
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

		height+=SceneViewer::TREEVIEW_ROW_HEIGHT;

		node.parent=&directory;
		node.x=0;
		node.y=height-SceneViewer::TREEVIEW_ROW_HEIGHT;
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
	height+=this->parent ? (this->parent->expanded ? SceneViewer::TREEVIEW_ROW_HEIGHT : 0) : SceneViewer::TREEVIEW_ROW_HEIGHT;

	this->x=this->parent ? this->parent->x+SceneViewer::TREEVIEW_ROW_ADVANCE : SceneViewer::TREEVIEW_ROW_ADVANCE;
	this->y=height-SceneViewer::TREEVIEW_ROW_HEIGHT;

	width=this->parent ? (this->parent->expanded ? (this->textWidth+this->x > width ? this->textWidth+this->x : width) : width) : this->textWidth+this->x;

	if(this->expanded)
	{
		for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
			nCh->update(width,height);
	}
}
void Resources::ResourceNode::drawdirlist(Resources* tv)
{
	float scrollY=tv->leftScrollBar.GetScrollValue();

	if(this->y >= scrollY && this->y+SceneViewer::TREEVIEW_ROW_HEIGHT <= scrollY+tv->frameHeight)
	{
		size_t resLen=0;
		wchar_t resText[CHAR_MAX];
		mbstowcs_s(&resLen,resText,CHAR_MAX,this->fileName,this->fileName.Count());

		if(this->selected)
			tv->tabContainer->renderer->FillRectangle(D2D1::RectF(0,(float)this->y,(float)tv->leftFrameWidth-ScrollBar::SCROLLBAR_WIDTH-4,(float)this->y+SceneViewer::TREEVIEW_ROW_HEIGHT),tv->tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

		if(this->nChilds)
			tv->tabContainer->renderer->DrawBitmap(this->expanded ? tv->tabContainer->iconDown : tv->tabContainer->iconRight,D2D1::RectF(this->x-SceneViewer::TREEVIEW_ROW_ADVANCE,this->y,this->x,this->y+SceneViewer::TREEVIEW_ROW_HEIGHT));

		tv->tabContainer->renderer->DrawBitmap(tv->tabContainer->iconFolder,D2D1::RectF(this->x,this->y,this->x + SceneViewer::TREEVIEW_ROW_ADVANCE,this->y+SceneViewer::TREEVIEW_ROW_HEIGHT));

		tv->tabContainer->renderer->DrawText(resText,resLen,Direct2DGuiBase::texter,D2D1::RectF(this->x + SceneViewer::TREEVIEW_ROW_ADVANCE,this->y,this->x + SceneViewer::TREEVIEW_ROW_ADVANCE + this->textWidth,this->y+SceneViewer::TREEVIEW_ROW_HEIGHT),tv->tabContainer->SetColor(GuiInterface::COLOR_TEXT));
	}

	if(this->expanded)
	{
		for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
			nCh->drawdirlist(tv);
	}
}

void Resources::ResourceNode::drawfilelist(Resources* tv)
{
	float scrollY=tv->rightScrollBar.GetScrollValue();

	if(this->parent && (this->y >= scrollY && this->y+SceneViewer::TREEVIEW_ROW_HEIGHT <= scrollY+tv->frameHeight))
	{
		size_t resLen=0;
		wchar_t resText[CHAR_MAX];
		mbstowcs_s(&resLen,resText,CHAR_MAX,this->fileName,this->fileName.Count());

		if(this->selected)
			tv->tabContainer->renderer->FillRectangle(D2D1::RectF(this->x,this->y,tv->tabContainer->width,this->y+SceneViewer::TREEVIEW_ROW_HEIGHT),tv->tabContainer->SetColor(TabContainer::COLOR_TAB_SELECTED));

		if(this->nChilds)
			tv->tabContainer->renderer->DrawBitmap(this->expanded ? tv->tabContainer->iconDown : tv->tabContainer->iconRight,D2D1::RectF(this->x-SceneViewer::TREEVIEW_ROW_ADVANCE,this->y,this->x,this->y+SceneViewer::TREEVIEW_ROW_HEIGHT));

		tv->tabContainer->renderer->DrawBitmap(this->isDir ? tv->tabContainer->iconFolder : tv->tabContainer->iconFile,D2D1::RectF(this->x,this->y,this->x + SceneViewer::TREEVIEW_ROW_ADVANCE,this->y+SceneViewer::TREEVIEW_ROW_HEIGHT));

		tv->tabContainer->renderer->DrawText(resText,resLen,Direct2DGuiBase::texter,D2D1::RectF(this->x + SceneViewer::TREEVIEW_ROW_ADVANCE,this->y,this->x + SceneViewer::TREEVIEW_ROW_ADVANCE + this->textWidth,this->y+SceneViewer::TREEVIEW_ROW_HEIGHT),tv->tabContainer->SetColor(GuiInterface::COLOR_TEXT));
	}

	for(std::list<ResourceNode>::iterator nCh=this->childsDirs.begin();nCh!=this->childsDirs.end();nCh++)
		nCh->drawfilelist(tv);
}





Resources::ResourceNode* Resources::ResourceNode::onmousepressedLeftPane(Resources* tv,float& x,float& y,float& width,float& height)
{
	if(y>this->y && y<this->y+SceneViewer::TREEVIEW_ROW_HEIGHT)
	{
		if(this->nChilds && x>this->x-SceneViewer::TREEVIEW_ROW_ADVANCE && x<this->x)
		{
			this->expanded=!this->expanded;
			return this;
		}
		else
		{
			this->selected=true;
			tv->rightElements.insertFiles(*this,GetDC(tv->tabContainer->hwnd),tv->rightBitmapWidth=0,tv->rightBitmapHeight=0);
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
	if(y>this->y && y<this->y+SceneViewer::TREEVIEW_ROW_HEIGHT)
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
GuiTab(tc),
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

	leftFrameWidth=(tabContainer->width-20)/2;
	frameHeight=tabContainer->height-30;

	

	this->SetLeftScrollBar();
	this->SetRightScrollBar();
	

	this->name="Project";

	this->OnEntitiesChange();

	this->OnRecreateTarget();

	this->DrawItems();
}

void Resources::OnRecreateTarget()
{
	DrawItems();
}

void Resources::SetLeftScrollBar()
{
	leftScrollBar.Set(this->leftFrameWidth-4-ScrollBar::SCROLLBAR_WIDTH,(float)TabContainer::CONTAINER_HEIGHT,this->leftFrameWidth-4,this->tabContainer->height);
	leftScrollBar.SetScrollerFactor(leftBitmapHeight,frameHeight);
}
void Resources::SetRightScrollBar()
{
	rightScrollBar.Set(this->tabContainer->width-ScrollBar::SCROLLBAR_WIDTH,(float)TabContainer::CONTAINER_HEIGHT,this->tabContainer->width,this->tabContainer->height);
	rightScrollBar.SetScrollerFactor(rightBitmapHeight,frameHeight);
}


Resources::~Resources()
{
	printf("destroying resources %p\n",this);
}

void Resources::OnEntitiesChange()
{
	HDC hdc=GetDC(tabContainer->hwnd);
	int nLevels=0;

	leftElements.clear();

	Entity* rootEntity=Entity::pool.size() ? Entity::pool.front() : 0;

	leftElements.insertDirectory(App::instance->projectFolder,0,WIN32_FIND_DATA(),GetDC(tabContainer->hwnd),leftBitmapWidth=0,leftBitmapHeight=0,0);
	rightElements.insertFiles(leftElements,GetDC(tabContainer->hwnd),rightBitmapWidth=0,rightBitmapHeight=0);

	this->SetLeftScrollBar();
	this->SetRightScrollBar();

	this->DrawItems();
	this->OnPaint();
}

void Resources::OnSize()
{
	frameHeight=tabContainer->height-TabContainer::CONTAINER_HEIGHT;

	if(this->leftFrameWidth>this->tabContainer->width)
		this->leftFrameWidth=(this->tabContainer->width-ScrollBar::SCROLLBAR_WIDTH)/2.0f;

	this->SetLeftScrollBar();
	this->SetRightScrollBar();
}

void Resources::OnMouseMove()
{
	if(this->splitterMoving)
	{
		this->leftFrameWidth=this->tabContainer->mousex;
		this->SetLeftScrollBar();
		this->OnPaint();
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

	if(leftScrollBar.OnScrolled() || rightScrollBar.OnScrolled())
		this->OnPaint();
}

void Resources::OnMouseWheel()
{
	short int delta = GET_WHEEL_DELTA_WPARAM(tabContainer->wparam);

	if(this->tabContainer->mousex<(this->leftFrameWidth-4))
	{
		leftScrollBar.SetScrollerPosition((float)-delta);
	}
	else if(this->tabContainer->mousex>(this->leftFrameWidth+4))
	{
		rightScrollBar.SetScrollerPosition((float)-delta);
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
	float &mx=tabContainer->mousex;
	float &my=tabContainer->mousey;

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
	else if(mx<(this->tabContainer->width-ScrollBar::SCROLLBAR_WIDTH))//right pane
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

void Resources::OnUpdate()
{
	if(tabContainer->mouseDown)
		this->OnLMouseDown();
}

void Resources::OnReparent()
{
	this->OnSize();
	this->OnRecreateTarget();
	this->DrawItems();
}

void Resources::DrawLeftItems()
{
	
}

void Resources::DrawRightItems()
{
	
}

void Resources::DrawItems()
{
	
}

void Resources::OnPaint()
{
	

	if(!tabContainer->isRender)
		tabContainer->renderer->BeginDraw();

	tabContainer->renderer->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)tabContainer->width,(float)tabContainer->height),this->tabContainer->SetColor(GuiTab::COLOR_MAIN_BACKGROUND));

	//dirs hierarchy

	tabContainer->renderer->PushAxisAlignedClip(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)this->leftFrameWidth-4,(float)tabContainer->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tabContainer->renderer->FillRectangle(D2D1::RectF(0,(float)TabContainer::CONTAINER_HEIGHT,(float)this->leftFrameWidth-4,(float)tabContainer->height),this->tabContainer->SetColor(GuiTab::COLOR_GUI_BACKGROUND));

	tabContainer->renderer->SetTransform(D2D1::Matrix3x2F::Translation(0,(float)TabContainer::CONTAINER_HEIGHT-leftScrollBar.GetScrollValue()));

	leftElements.drawdirlist(this);

	tabContainer->renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	tabContainer->renderer->PopAxisAlignedClip();

	leftScrollBar.OnPaint();

	//files hierarchy

	tabContainer->renderer->PushAxisAlignedClip(D2D1::RectF(this->leftFrameWidth,(float)TabContainer::CONTAINER_HEIGHT,(float)this->tabContainer->width,(float)tabContainer->height),D2D1_ANTIALIAS_MODE_ALIASED);

	tabContainer->renderer->FillRectangle(D2D1::RectF(this->leftFrameWidth,(float)TabContainer::CONTAINER_HEIGHT,(float)this->tabContainer->width,(float)tabContainer->height),this->tabContainer->SetColor(GuiTab::COLOR_GUI_BACKGROUND));

	tabContainer->renderer->SetTransform(D2D1::Matrix3x2F::Translation(this->leftFrameWidth,(float)TabContainer::CONTAINER_HEIGHT-rightScrollBar.GetScrollValue()));

	rightElements.drawfilelist(this);

	tabContainer->renderer->SetTransform(D2D1::Matrix3x2F::Identity());

	tabContainer->renderer->PopAxisAlignedClip();

	rightScrollBar.OnPaint();

	if(!tabContainer->isRender)
	{
		if(tabContainer->renderer->EndDraw() & D2DERR_RECREATE_TARGET)
			tabContainer->recreateTarget=true;
	}
}

