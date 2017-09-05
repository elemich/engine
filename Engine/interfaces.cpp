#include "gui_interfaces.h"
#include "win32.h"



std::vector<SceneEntityNode*> Editor::selection;

void GuiInterface::Broadcast(void (GuiInterface::*func)())
{
	for_each(pool.begin(),pool.end(),std::mem_fun(func));
}


GuiTab::GuiTab(TabContainer* tc):tabContainer(tc),guiTabRootElement(0,TabContainer::CONTAINER_HEIGHT,tc->width,tc->height-TabContainer::CONTAINER_HEIGHT)
{
	guiTabRootElement.name="GuiTabMainTab";
}

bool GuiTab::IsSelected(){return (tabContainer && tabContainer->GetSelected()==this);}


void GuiTab::OnPaint(){this->guiTabRootElement.OnPaint(this);}
void GuiTab::OnEntitiesChange(){this->guiTabRootElement.OnEntitiesChange(this);}
void GuiTab::OnSize()
{
	this->guiTabRootElement.x=0;
	this->guiTabRootElement.y=TabContainer::CONTAINER_HEIGHT;
	this->guiTabRootElement.width=this->tabContainer->width;
	this->guiTabRootElement.height=this->tabContainer->height-TabContainer::CONTAINER_HEIGHT;

	this->guiTabRootElement.OnSize(this);
}
void GuiTab::OnLMouseDown(){this->guiTabRootElement.OnLMouseDown(this);}
void GuiTab::OnLMouseUp(){this->guiTabRootElement.OnLMouseUp(this);};
void GuiTab::OnMouseMove(){this->guiTabRootElement.OnMouseMove(this);}
void GuiTab::OnUpdate(){this->guiTabRootElement.OnUpdate(this);}
void GuiTab::OnReparent(){this->guiTabRootElement.OnReparent(this);}
void GuiTab::OnSelected(){this->guiTabRootElement.OnSelected(this);}
void GuiTab::OnRender(){this->guiTabRootElement.OnRender(this);}
void GuiTab::OnMouseWheel(){this->guiTabRootElement.OnMouseWheel(this);}


GuiTabElement::GuiTabElement(GuiTabElement* iParent,float ix, float iy, float iw,float ih,vec2 _alignPos,vec2 _alignRect,vec2 _alignText):
	rect(ix + (iParent ? iParent->x : 0),iy + (iParent ? iParent->y : 0),iw,ih),
	x(rect.x),								  
	y(rect.y),
	width(rect.z),
	height(rect.w),
	colorBackground(GuiInterface::COLOR_GUI_BACKGROUND),
	colorForeground(GuiInterface::COLOR_TEXT),
	colorHovering(colorBackground),
	colorPressing(colorBackground),
	colorBorder(colorBackground),
	colorChecked(colorBackground),
	pressing(false),
	hovering(false),
	checked(false),
	alignPos(_alignPos),
	alignText(_alignText),
	alignRect(_alignRect),
	animFrame(-1),
	container(-1)
{
	this->parent=iParent;

	if(iParent)
		iParent->childs.push_back(this);

	memset(this->sibling,0,16);
}

GuiTabElement::~GuiTabElement()
{
}

void GuiTabElement::_reset()
{
	colorBackground=GuiInterface::COLOR_GUI_BACKGROUND;
	colorForeground=GuiInterface::COLOR_TEXT;
	colorHovering=colorBackground;
	colorPressing=colorBackground;
	colorBorder=colorBackground;
	colorChecked=colorBackground;
	pressing=false;
	hovering=false;
	checked=false;
	alignPos=vec2(-1,-1);
	alignText=vec2(-1,-1);
	alignRect=vec2(-1,-1);
	animFrame=-1;

	this->childs.clear();
}

bool GuiTabElement::_contains(vec4& quad,vec2 point)
{
	return (point.x>quad.x && point.x<(quad.x+quad.z) && point.y>quad.y && point.y<(quad.y+quad.w));
}

void GuiTabElement::BroadcastToChilds(void (GuiTabElement::*func)(GuiTab*),GuiTab*tab)
{
	for_each(this->childs.begin(),this->childs.end(),std::bind(func,std::placeholders::_1,tab));
}


vec4 GuiTabElement::_getrect()
{
	vec4 retVec(rect);

	if(!parent)
		return retVec;

	vec4 pRect=this->parent->rect;//_getrect();

	retVec.z = this->alignRect.x>=0 ? this->alignRect.x * pRect.z : retVec.z;
	retVec.w = this->alignRect.y>=0 ? this->alignRect.y * pRect.w : retVec.w;

	if(this->alignPos.x>=0)
		retVec.x=pRect.x+this->alignPos.x*pRect.z;
	if(this->alignPos.y>=0)
		retVec.y=pRect.y+this->alignPos.y*pRect.w;

	retVec.x = pRect.x > retVec.x ? pRect.x : (pRect.x+pRect.z < retVec.x+retVec.z ? retVec.x - (retVec.x+retVec.z - (pRect.x+pRect.z)) : retVec.x);
	retVec.y = pRect.y > retVec.y ? pRect.y : (pRect.y+pRect.w < retVec.y+retVec.w ? retVec.y - (retVec.y+retVec.w - (pRect.y+pRect.w)) : retVec.y);

	if(this->sibling[1])
	{
		vec4 v=this->sibling[1]->_getrect();
		retVec.y=v.y+v.w;
	}

	/*if(this->container==1 && this->childs.size())
	{
		GuiTabElement* gte=this->childs.back();
		if(gte)
		{
			vec4 v=gte->_getrect();
			retVec.w=v.w;
		}
	}*/

	return retVec;
}

vec4 GuiTabElement::_gettextrect(GuiTab* tab,vec4& alignedRect)
{
	vec4 retVec(alignedRect.x,alignedRect.y,alignedRect.z,alignedRect.w);

	if(!this->text.Buf() || this->alignText.x<0 || this->alignText.y<0)
		return retVec;

	int tLen=this->text.Count();
	SIZE resSize;
	if(!GetTextExtentPoint32(GetWindowDC(tab->tabContainer->hwnd),this->text,tLen,&resSize))
		__debugbreak();

	retVec.z=resSize.cx;
	retVec.w=resSize.cy;

	if(this->alignText.x>=0)
		retVec.x=alignedRect.x+this->alignText.x*alignedRect.z-retVec.z/2.0f;
	if(this->alignText.y>=0)
		retVec.y=alignedRect.y+this->alignText.y*alignedRect.w-retVec.w/2.0f;

	retVec.x = alignedRect.x > retVec.x ? alignedRect.x : (alignedRect.x+alignedRect.z < retVec.x+retVec.z ? retVec.x - (retVec.x+retVec.z - (alignedRect.x+alignedRect.z)) : retVec.x);
	retVec.y = alignedRect.y > retVec.y ? alignedRect.y : (alignedRect.y+alignedRect.w < retVec.y+retVec.w ? retVec.y - (retVec.y+retVec.w - (alignedRect.y+alignedRect.w)) : retVec.y);

	return retVec;
}

bool GuiTabElement::_mousemove(GuiTab* tab)
{
	vec4 rectAlignPos=this->_getrect();

	bool inside1=_contains(rectAlignPos,vec2(tab->tabContainer->mousex,tab->tabContainer->mousey));
	bool inside2=false;

	for(int i=0;i<(int)this->childs.size();i++)
	{
			if(this->childs[i]->_mousemove(tab))
				break;
	}

	this->hovering = inside1 && !inside2; 

	return inside1 || inside2;
}

void GuiTabElement::_draw(GuiTab*)
{
	
}

void GuiTabElement::OnPaint(GuiTab* tab)
{
	bool selfRender=!tab->tabContainer->isRender;

	if(selfRender)
		tab->tabContainer->BeginDraw();


	vec4 rectAlignPos=this->_getrect();

	if(this->imageHovering || this->imagePressed || this->imageBackground)
	{
		this->hovering ? this->imageHovering.Draw(tab,rectAlignPos) : (this->pressing ? this->imagePressed.Draw(tab,rectAlignPos) : this->imageBackground.Draw(tab,rectAlignPos));
	}
	else
		tab->tabContainer->renderer->FillRectangle(D2D1::RectF(rectAlignPos.x,rectAlignPos.y,rectAlignPos.x+rectAlignPos.z,rectAlignPos.y+rectAlignPos.w),tab->tabContainer->SetColor(this->pressing ? this->colorPressing : (this->hovering ? this->colorHovering : (this->checked ? this->colorChecked : this->colorBackground))));

	tab->tabContainer->renderer->DrawRectangle(D2D1::RectF(rectAlignPos.x + 0.5f,rectAlignPos.y + 0.5f,rectAlignPos.x+rectAlignPos.z - 0.5f,rectAlignPos.y+rectAlignPos.w - 0.5f),tab->tabContainer->SetColor(this->colorBorder));

	if(text.Buf())
	{
		vec4 textAlignPos=this->_gettextrect(tab,rectAlignPos);
			
		if(this->container>=0)
		{
			tab->tabContainer->renderer->DrawBitmap(this->container==1 ? tab->tabContainer->iconDown : tab->tabContainer->iconRight,D2D1::RectF(rectAlignPos.x,rectAlignPos.y,rectAlignPos.x+TabContainer::CONTAINER_ICON_WH,rectAlignPos.y+TabContainer::CONTAINER_ICON_WH));
			Direct2DGuiBase::DrawText(tab->tabContainer->renderer,tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT),text,textAlignPos.x + TabContainer::CONTAINER_ICON_WH,textAlignPos.y,textAlignPos.x+textAlignPos.z,textAlignPos.y+textAlignPos.w);
		}
		else Direct2DGuiBase::DrawText(tab->tabContainer->renderer,tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT),text,textAlignPos.x,textAlignPos.y,textAlignPos.x+textAlignPos.z,textAlignPos.y+textAlignPos.w);
		//tab->tabContainer->renderer->DrawRectangle(D2D1::RectF(textAlignPos.x,textAlignPos.y,textAlignPos.x+textAlignPos.z,textAlignPos.y+textAlignPos.w),tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT));
	}

	if(this->container!=0)
		this->BroadcastToChilds(&GuiTabElement::OnPaint,tab);

	if(selfRender)
		tab->tabContainer->EndDraw();
}


void GuiTabElement::OnMouseMove(GuiTab* tab)
{
	bool isInside=_contains(this->_getrect(),vec2(tab->tabContainer->mousex,tab->tabContainer->mousey));

	bool updateDraw=isInside!=this->hovering;

	this->hovering=isInside;

	if(updateDraw)
		this->OnPaint(tab);

	if(isInside && this->container!=0)
		this->BroadcastToChilds(&GuiTabElement::OnMouseMove,tab);
}


void GuiTabElement::OnEntitiesChange(GuiTab* tab){this->BroadcastToChilds(&GuiTabElement::OnEntitiesChange,tab);}
void GuiTabElement::OnSize(GuiTab* tab){this->BroadcastToChilds(&GuiTabElement::OnSize,tab);}
void GuiTabElement::OnLMouseDown(GuiTab* tab)
{
	vec4 v=this->_getrect();

	bool isInside=_contains(v,vec2(tab->tabContainer->mousex,tab->tabContainer->mousey));

	bool paintConditions = isInside!=this->pressing && isInside==true;

	this->pressing=isInside;

	if(isInside)
	{
		this->checked=!this->checked;

		if(this->container>=0)
			this->container=(tab->tabContainer->mousex > v.x && tab->tabContainer->mousex < v.x+TabContainer::CONTAINER_ICON_WH && tab->tabContainer->mousey > v.y && tab->tabContainer->mousey <v.y+TabContainer::CONTAINER_ICON_WH) ? !this->container : false;
	}

	if(paintConditions || this->container==1)
		this->OnPaint(tab);

	if(isInside && this->container!=0)
		this->BroadcastToChilds(&GuiTabElement::OnLMouseDown,tab);
}
void GuiTabElement::OnLMouseUp(GuiTab* tab)
{
	bool isInside=_contains(this->_getrect(),vec2(tab->tabContainer->mousex,tab->tabContainer->mousey));

	this->pressing = isInside ? false : this->pressing;

	this->OnPaint(tab);

	if(isInside && this->container!=0)
		this->BroadcastToChilds(&GuiTabElement::OnLMouseUp,tab);
}

void GuiTabElement::OnUpdate(GuiTab* tab){this->BroadcastToChilds(&GuiTabElement::OnUpdate,tab);}
void GuiTabElement::OnReparent(GuiTab* tab){this->BroadcastToChilds(&GuiTabElement::OnReparent,tab);}
void GuiTabElement::OnSelected(GuiTab* tab){this->BroadcastToChilds(&GuiTabElement::OnSelected,tab);}
void GuiTabElement::OnRender(GuiTab* tab){this->BroadcastToChilds(&GuiTabElement::OnRender,tab);}
void GuiTabElement::OnMouseWheel(GuiTab* tab){this->BroadcastToChilds(&GuiTabElement::OnMouseWheel,tab);}


GuiTabElement* GuiTabElement::CreateTabElement(float ix, float iy, float iw,float ih,vec2 _alignPos,vec2 _alignRect,vec2 _alignText)
{
	return new GuiTabElement(this,ix,iy,iw,ih,_alignPos,_alignRect,_alignText);
}

GuiTabElement* GuiTabElement::CreateTabElementLabel(float ix, float iy, float iw,float ih,vec2 _alignPos,vec2 _alignRect,vec2 _alignText)
{
	GuiTabElement* label=new GuiTabElement(this,ix,iy,iw,ih,_alignPos,_alignRect,_alignText);
	return label;
}


GuiTabElement* GuiTabElement::CreateTabElementContainer(GuiTabElement* iSibling,const char* iText)
{
	GuiTabElement* row=new GuiTabElement(this,0,0,0,20,vec2(0,0),vec2(1,-1),vec2(1,-1));
	row->container=0;
	row->sibling[1]=iSibling;
	row->text=iText;
	return row;
}

GuiTabElementRow* GuiTabElement::CreateTabElementRow(GuiTabElement* iSibling,const char* iLeft,const char* iRight)
{
	GuiTabElementRow* row=new GuiTabElementRowString(this,iLeft,iRight);
	row->sibling[1]=iSibling;
	return row;
}

GuiTabElementRow* GuiTabElement::CreateTabElementRow(GuiTabElement* iSibling,const char* iLeft,vec3 iRight)
{
	char str[100];
	sprintf(str,"%g,%g,%g",iRight.x,iRight.y,iRight.z);
	GuiTabElementRow* row=new GuiTabElementRowString(this,iLeft,str);
	row->sibling[1]=iSibling;
	return row;
}

GuiTabElementRow::GuiTabElementRow(GuiTabElement* iParent,const char* iLeft):
	GuiTabElement(iParent,0,0,0,20,vec2(0,0),vec2(1,-1),vec2(1,-1)),
	left(this,0,0,0,0,vec2(0,0.5f),vec2(0.5f,1),vec2(0,0.5f))
	
{
	this->left.text=iLeft;
}

GuiTabElementRowString::GuiTabElementRowString(GuiTabElement* iParent,const char* iLeft,const char* iRight):
	GuiTabElementRow(iParent,iLeft),
	right(this,0,0,0,0,vec2(0.5f,0.5f),vec2(0.5f,1),vec2(0,0.5f))
{
	this->right.text=iRight;
}


GuiTabImage::GuiTabImage():image(0),data(0),width(0),height(0){}



bool GuiTabImage::Load(const char* fName)
{
	SAFEDELETEARRAY(this->image);

	if(!fName)
		return false;

	size_t resLen=0;
	wchar_t resText[CHAR_MAX];
	mbstowcs_s(&resLen,resText,CHAR_MAX,fName,strlen(fName));

	Direct2DGuiBase::CreateRawBitmap(resText,this->image,this->width,this->height);

	return true;
}

void GuiTabImage::Draw(GuiTab* tab,vec4& rect)
{
	if(this->image)
	{
		ID2D1Bitmap* bitmap=(ID2D1Bitmap*)this->data;

		if(!bitmap)
		{
			D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
			bp.pixelFormat=tab->tabContainer->renderer->GetPixelFormat();

			HRESULT result=tab->tabContainer->renderer->CreateBitmap(D2D1::SizeU((int)this->width,(int)this->height),this->image,(int)(4*this->width),bp,&bitmap);

			if(!bitmap || result!=S_OK)
				__debugbreak();
		}

		tab->tabContainer->renderer->DrawBitmap(bitmap,D2D1::RectF(rect.x,rect.y,rect.x+rect.z,rect.y+rect.w));
	}
}

/*
void GuiCheckBox::OnLMouseDown()
{
	GuiTabElement::OnLMouseDown();

	if(this->pressing)
	{
		this->checked=!this->checked;
		this->OnPaint();
	}
}


void GuiButton::OnMouseMove()
{
	GuiTabElement::OnMouseMove();

	vec2 point(tab->tabContainer->mousex,tab->tabContainer->mousey);

	if(this->label.contains(point) || this->image.contains(point))
		__debugbreak();
}*/