#include "gui_interfaces.h"
#include "win32.h"


////////////////////////////
//////////////Editor////////
////////////////////////////


std::vector<SceneEntityNode*> Editor::selection;


////////////////////////////
//////////////GuiTab////////
////////////////////////////


void GuiInterface::BroadcastToGui(void (GuiInterface::*func)())
{
	for_each(pool.begin(),pool.end(),std::mem_fun(func));
}

GuiTab::GuiTab(TabContainer* tc):tabContainer(tc),guiTabRootElement(0,TabContainer::CONTAINER_HEIGHT,tc->width,tc->height-TabContainer::CONTAINER_HEIGHT)
{
	guiTabRootElement.name="GuiTabMainTab";
}

GuiTab::~GuiTab()
{

}

bool GuiTab::IsSelected(){return (tabContainer && tabContainer->GetSelected()==this);}


void GuiTab::OnGuiPaint(){this->guiTabRootElement.OnPaint(this);}
void GuiTab::OnEntitiesChange(){this->guiTabRootElement.OnEntitiesChange(this);}
void GuiTab::OnGuiSize()
{
	this->guiTabRootElement.OnSize(this);
}
void GuiTab::OnGuiLMouseDown(){this->guiTabRootElement.OnLMouseDown(this);}
void GuiTab::OnGuiLMouseUp(){this->guiTabRootElement.OnLMouseUp(this);};
void GuiTab::OnGuiMouseMove(){this->guiTabRootElement.OnMouseMove(this);}
void GuiTab::OnGuiUpdate(){/*this->guiTabRootElement.OnGuiUpdate(this);*/}
void GuiTab::OnGuiReparent(){this->guiTabRootElement.OnReparent(this);}
void GuiTab::OnGuiSelected(){this->guiTabRootElement.OnSelected(this);}
void GuiTab::OnGuiRender(){/*this->guiTabRootElement.OnGuiRender(this);*/}
void GuiTab::OnGuiMouseWheel(){this->guiTabRootElement.OnMouseWheel(this);}


////////////////////////////
////////GuiTabElement///////
////////////////////////////

GuiRect::GuiRect(GuiRect* iParent,float ix, float iy, float iw,float ih,vec2 _alignPos,vec2 _alignRect):
	rect(ix,iy,iw,ih),
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
	alignRect(_alignRect),
	animFrame(-1),
	container(-1)
{
	this->parent=iParent;

	if(iParent)
		iParent->childs.push_back(this);

	memset(this->sibling,0,16);
}

GuiRect::~GuiRect()
{
}

void GuiRect::Set(GuiRect* iParent,GuiRect* iSibling,int siblingIdx,int iContainer,float ix, float iy, float iw,float ih,float apx,float apy,float arx,float ary)
{
	this->parent=iParent;
	this->sibling[siblingIdx]=iSibling;
	this->container=iContainer;
	this->rect.make(ix,iy,iw,ih);
	this->alignPos.make(apx,apy);
	this->alignRect.make(arx,ary);

	if(iParent)
		iParent->childs.push_back(this);
}

bool GuiRect::_contains(vec4& quad,vec2 point)
{
	return (point.x>quad.x && point.x<(quad.x+quad.z) && point.y>quad.y && point.y<(quad.y+quad.w));
}
	
void GuiRect::BroadcastToChilds(void (GuiRect::*func)(GuiTab*),GuiTab*tab)
{
	for_each(this->childs.begin(),this->childs.end(),std::bind(func,std::placeholders::_1,tab));
}




void GuiRect::OnEntitiesChange(GuiTab* tab)
{
	this->BroadcastToChilds(&GuiRect::OnEntitiesChange,tab);
}

void GuiRect::OnPaint(GuiTab* tab)
{
	bool selfRender=!tab->tabContainer->isRender;

	if(selfRender)
		tab->tabContainer->BeginDraw();

	if(this->imageHovering || this->imagePressed || this->imageBackground)
	{
		this->hovering ? this->imageHovering.Draw(tab,this->rect) : (this->pressing ? this->imagePressed.Draw(tab,this->rect) : this->imageBackground.Draw(tab,this->rect));
	}
	else
		tab->tabContainer->renderer->FillRectangle(D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+this->rect.z,this->rect.y+this->rect.w),tab->tabContainer->SetColor(this->pressing ? this->colorPressing : (this->hovering ? this->colorHovering : this->colorBackground/*(this->checked ? this->colorChecked : this->colorBackground)*/)));

	tab->tabContainer->renderer->DrawRectangle(D2D1::RectF(this->rect.x + 0.5f,this->rect.y + 0.5f,this->rect.x+this->rect.z - 0.5f,this->rect.y+this->rect.w - 0.5f),tab->tabContainer->SetColor(this->colorBorder));

	if(this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnPaint,tab);

	if(selfRender)
		tab->tabContainer->EndDraw();
}


void GuiRect::OnSize(GuiTab* tab)
{
	if(parent)
	{
		vec4 &pRect=this->parent->rect;

		this->rect.z = this->alignRect.x>=0 ? this->alignRect.x * pRect.z : this->rect.z;
		this->rect.w = this->alignRect.y>=0 ? this->alignRect.y * pRect.w : this->rect.w;

		if(this->alignPos.x>=0)
			this->rect.x=pRect.x+this->alignPos.x*pRect.z;
		if(this->alignPos.y>=0)
			this->rect.y=pRect.y+this->alignPos.y*pRect.w;

		if(parent->container>=0)
		{
			this->rect.x+=20;
			this->rect.z-=20;
		}

		this->rect.x = pRect.x > this->rect.x ? pRect.x : (pRect.x+pRect.z < this->rect.x+this->rect.z ? this->rect.x - (this->rect.x+this->rect.z - (pRect.x+pRect.z)) : this->rect.x);
		this->rect.y = pRect.y > this->rect.y ? pRect.y : (pRect.y+pRect.w < this->rect.y+this->rect.w ? this->rect.y - (this->rect.y+this->rect.w - (pRect.y+pRect.w)) : this->rect.y);

		if(parent->container>=0)
		{
			this->rect.y+=20;
		}
	}
	else
		this->rect.make(0,TabContainer::CONTAINER_HEIGHT,tab->tabContainer->width,tab->tabContainer->height-TabContainer::CONTAINER_HEIGHT);

	if(sibling[0])
	{
		this->rect.x=this->sibling[0]->rect.x+this->sibling[0]->rect.z;
	}
	else if(this->sibling[1])
	{
		this->rect.y=this->sibling[1]->rect.y+this->sibling[1]->rect.w;
	}
	else if(sibling[2])
	{
		this->rect.x=this->sibling[2]->rect.x-this->rect.z;
	}
	else if(sibling[3])
	{
		this->rect.y=this->sibling[3]->rect.y-this->rect.w;
	}

	if(this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnSize,tab);



	if(container==0)
	{
		this->rect.w=20;
	}
	else if(container==1)
		this->rect.w=20;//calc on childs



	if(this->container==1 && !this->childs.empty())
	{
		GuiRect* te=this->childs.back();

		this->rect.w=te->rect.y+te->rect.w-this->rect.y;
	}
}

void GuiRect::OnLMouseDown(GuiTab* tab)
{
	bool wasPressing=this->pressing;
	bool bContainerButtonPressed=0;

	this->pressing=this->hovering;

	if(this->hovering)
	{
		this->checked=!this->checked;


		if(this->container>=0)
		{
			bContainerButtonPressed=(tab->tabContainer->mousex > this->rect.x && tab->tabContainer->mousex < this->rect.x+TabContainer::CONTAINER_ICON_WH && tab->tabContainer->mousey > this->rect.y && tab->tabContainer->mousey <this->rect.y+TabContainer::CONTAINER_ICON_WH);

			if(bContainerButtonPressed)
				this->container=!this->container;
		}

		if(bContainerButtonPressed)
		{
			tab->OnGuiSize();
			tab->OnGuiPaint();
		}

		if(wasPressing!=this->pressing && this->colorPressing!=this->colorBackground)
			this->OnPaint(tab);
		
		return;
	}
	if(this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnLMouseDown,tab);

	if(wasPressing!=this->pressing && this->colorPressing!=this->colorBackground)
		this->OnPaint(tab);
}
void GuiRect::OnLMouseUp(GuiTab* tab)
{
	bool wasPressing=this->pressing;

	if(this->hovering)
		this->pressing = false;

	if(this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnLMouseUp,tab);

	if(wasPressing!=this->pressing && this->colorPressing!=this->colorBackground)
		this->OnPaint(tab);
}


void GuiRect::OnMouseMove(GuiTab* tab)
{
	bool _oldHover=this->hovering;
	bool _curHover=_contains(this->rect,vec2(tab->tabContainer->mousex,tab->tabContainer->mousey));

	if(parent && _curHover)
		parent->hovering=false;
	
	this->hovering=_curHover;

	if(/*_curHover && */this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnMouseMove,tab);

	if(_oldHover!=this->hovering && this->colorBackground!=this->colorHovering)
		this->OnPaint(tab);
}

void GuiRect::OnUpdate(GuiTab* tab)
{

}

void GuiRect::OnReparent(GuiTab* tab)
{
	this->BroadcastToChilds(&GuiRect::OnReparent,tab);
}
void GuiRect::OnSelected(GuiTab* tab)
{
	this->BroadcastToChilds(&GuiRect::OnSelected,tab);
}
void GuiRect::OnRender(GuiTab* tab)
{
	this->BroadcastToChilds(&GuiRect::OnRender,tab);
}
void GuiRect::OnMouseWheel(GuiTab* tab)
{
	this->BroadcastToChilds(&GuiRect::OnMouseWheel,tab);
}


GuiRect* GuiRect::Rect(float ix, float iy, float iw,float ih)
{
	return new GuiRect(this,ix,iy,iw,ih);
}

GuiRect* GuiRect::Rect(vec2 _alignPos,vec2 _alignRect)
{
	return new GuiRect(this,0,0,0,0,_alignPos,_alignRect);
}

GuiString* GuiRect::Text(String str,float ix, float iy, float iw,float ih,vec2 _alignText)
{
	GuiString* label=new GuiString;
	label->parent=this;
	label->rect.make(ix,iy,iw,ih);
	label->alignText=_alignText;
	label->text=str;
	this->childs.push_back(label);
	return label;
}

GuiString* GuiRect::Text(String str,vec2 _alignPos,vec2 _alignRect,vec2 _alignText)
{
	GuiString* label=new GuiString;
	label->parent=this;
	label->alignPos=_alignPos;
	label->alignRect=_alignRect;
	label->alignText=_alignText;
	label->text=str;
	this->childs.push_back(label);
	return label;
}


GuiString* GuiRect::Container(const char* iText)
{
	GuiString* container=this->Text(iText,vec2(0,0),vec2(1,-1),vec2(1,-1));
	container->rect.make(0,0,0,20);
	container->container=0;
	container->sibling[1]= ((int)this->childs.size()>=2) ? this->childs.rbegin()[1] : 0;

	return container;
}

GuiPropertyString* GuiRect::Property(const char* iProp,const char* iVal)
{
	GuiPropertyString* p=new GuiPropertyString;
	p->parent=this;
	p->prp=iProp;
	p->val=iVal;
	p->rect.make(0,0,0,20);
	p->alignPos.make(0,0);
	p->alignRect.make(1,-1);
	p->sibling[1]= !this->childs.empty() ? this->childs.back() : 0;
	this->childs.push_back(p);
	

	return p;
}

GuiPropertyString* GuiRect::Property(const char* iProp,vec3 iRVal)
{
	char str[100];
	sprintf(str,"%g,%g,%g",iRVal.x,iRVal.y,iRVal.z);
	return this->Property(iProp,str);
}

GuiPropertySlider* GuiRect::Slider(const char* iLeft,float* ref)
{
	GuiPropertySlider* s=new GuiPropertySlider;
	s->Set(this,!this->childs.empty() ? this->childs.back() : 0,1,-1,0,0,0,25,0,0,1,-1);
	s->prp=iLeft;
	s->slider.Set(s,0,0,-1,0,0,0,0,0.5f,0,0.5f,1);
	s->slider.referenceValue=ref;
	return s;
}


GuiButton* GuiRect::Button(String str,float ix, float iy, float iw,float ih)
{
	GuiButton* b=new GuiButton;
	b->Set(this,0,0,-1,ix,iy,iw,ih);
	b->text=str;
	b->alignText.make(0.5f,0.5f);
	return b;
}
GuiButton* GuiRect::Button(String str,vec2 _alignPos,vec2 _alignRect,vec2 _alignText)
{
	GuiButton* b=new GuiButton;
	b->Set(this,0,0,-1,0,0,0,0,_alignPos.x,_alignPos.y,_alignRect.x,_alignRect.y);
	b->text=str;
	b->alignText=_alignText;
	return b;
}

GuiPropertyAnimation* GuiRect::PropertyAnimControl(AnimationController* ac)
{
	GuiPropertyAnimation* a=new GuiPropertyAnimation;
	a->Set(this,!this->childs.empty() ? this->childs.back() : 0,1,-1,0,0,0,41,0,0,1,-1);
	a->text.Set(a,0,0,-1,0,0,0,0,0,0.5f,1);
	a->text.text="Controller";

	a->animController=ac;

	a->slider.Set(a,0,0,-1,0,0,0,26,0.5f,0,0.5f,-1);
	a->slider.referenceValue=&a->animController->cursor;
	a->slider.minimum=a->animController->start;
	a->slider.maximum=a->animController->end;

	

	a->play.Set(a,0,0,-1,0,0,15,15,0.75f,0.75f,-1,-1);
	a->stop.Set(a,&a->play,2,-1,0,0,15,15,0,0.75f,-1,-1);
	
	a->stop.colorBackground=a->play.colorBackground=0x000000;
	a->stop.colorPressing=a->play.colorPressing=0xffffff;
	a->stop.colorHovering=a->play.colorHovering=0x88888888;

	a->play.referenceValue=&ac->play;
	a->play.updateMode=1;
	a->stop.referenceValue=&ac->play;
	a->stop.updateMode=0;//set 0 onlyif 1

	return a;
}


void GuiString::OnPaint(GuiTab* tab)
{
	bool selfRender=!tab->tabContainer->isRender;

	if(selfRender)
		tab->tabContainer->BeginDraw();

	GuiRect::OnPaint(tab);

	if(text.Buf())
	{
		if(this->container>=0)
		{
			tab->tabContainer->renderer->DrawBitmap(this->container==1 ? tab->tabContainer->iconDown : tab->tabContainer->iconRight,D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+TabContainer::CONTAINER_ICON_WH,this->rect.y+TabContainer::CONTAINER_ICON_WH));
			Direct2DGuiBase::DrawText(tab->tabContainer->renderer,tab->tabContainer->SetColor(0x000000),text,this->rect.x + TabContainer::CONTAINER_ICON_WH,this->rect.y,this->rect.x+this->textRect.z+100,this->rect.y+this->textRect.w);
		}
		else Direct2DGuiBase::DrawText(tab->tabContainer->renderer,tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT),text,this->textRect.x,this->textRect.y,this->textRect.x+this->textRect.z,this->textRect.y+this->textRect.w);
		//tab->tabContainer->renderer->DrawRectangle(D2D1::RectF(this->textRect.x,this->textRect.y,this->textRect.x+this->textRect.z,this->textRect.y+this->textRect.w),tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT));
	}

	if(selfRender)
		tab->tabContainer->EndDraw();
}

void GuiString::OnSize(GuiTab* tab)
{
	GuiRect::OnSize(tab);

	if(this->text.Buf())
	{
		int tLen=this->text.Count();
		SIZE resSize;
		if(!GetTextExtentPoint32(GetWindowDC(tab->tabContainer->hwnd),this->text,tLen,&resSize))
			__debugbreak();

		this->textRect.z=resSize.cx;
		this->textRect.w=resSize.cy;

		if(this->alignText.x>=0)
			this->textRect.x=this->rect.x+this->alignText.x*this->rect.z-this->textRect.z/2.0f;
		if(this->alignText.y>=0)
			this->textRect.y=this->rect.y+this->alignText.y*this->rect.w-this->textRect.w/2.0f;

		this->textRect.x = this->rect.x > this->textRect.x ? this->rect.x : (this->rect.x+this->rect.z < this->textRect.x+this->textRect.z ? this->textRect.x - (this->textRect.x+this->textRect.z - (this->rect.x+this->rect.z)) : this->textRect.x);
		this->textRect.y = this->rect.y > this->textRect.y ? this->rect.y : (this->rect.y+this->rect.w < this->textRect.y+this->textRect.w ? this->textRect.y - (this->textRect.y+this->textRect.w - (this->rect.y+this->rect.w)) : this->textRect.y);
	}
}

void GuiButton::OnLMouseUp(GuiTab* tab)
{
	GuiRect::OnLMouseUp(tab);

	if(this->hovering && this->referenceValue)
	{
		switch(this->updateMode)
		{
		case -1:
			(*referenceValue)=!(*referenceValue);
		break;
		case 0:
			(*referenceValue)=false;
		break;
		case 1:
			(*referenceValue)=true;
		break;
		}

		if(mouseUpFunc)
			(this->*mouseUpFunc)();
	}
}



void GuiPropertyString::OnPaint(GuiTab* tab)
{
	bool selfRender=!tab->tabContainer->isRender;

	if(selfRender)
		tab->tabContainer->BeginDraw();

	GuiRect::OnPaint(tab);

	if(prp.Buf())
		Direct2DGuiBase::DrawText(tab->tabContainer->renderer,tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT),prp,this->rect.x,this->rect.y,this->rect.x+this->rect.z/2.0f,this->rect.y+this->rect.w);
	if(val.Buf())	
		Direct2DGuiBase::DrawText(tab->tabContainer->renderer,tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT),val,this->rect.x+this->rect.z/2.0f,this->rect.y,this->rect.x+this->rect.z,this->rect.y+this->rect.w);

	if(selfRender)
		tab->tabContainer->EndDraw();
}

void GuiSlider::OnPaint(GuiTab* tab)
{
	bool selfRender=!tab->tabContainer->isRender;

	if(selfRender)
		tab->tabContainer->BeginDraw();

	GuiRect::OnPaint(tab);

	tab->tabContainer->renderer->FillRectangle(D2D1::RectF(this->rect.x+10,this->rect.y+this->rect.w/4.0f-2,this->rect.x+this->rect.z-10,this->rect.y+this->rect.w/4.0f+2),tab->tabContainer->SetColor(0x000000));

	String smin(this->minimum);
	String smax(this->maximum);
	String value(*this->referenceValue);

	Direct2DGuiBase::DrawText(tab->tabContainer->renderer,tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT),smin,this->rect.x+10,this->rect.y,this->rect.x+this->rect.z-10,this->rect.y+this->rect.w,0,0.75);
	Direct2DGuiBase::DrawText(tab->tabContainer->renderer,tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT),smax,this->rect.x+10,this->rect.y,this->rect.x+this->rect.z-10,this->rect.y+this->rect.w,1,0.75);
	Direct2DGuiBase::DrawText(tab->tabContainer->renderer,tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT),value,this->rect.x+10,this->rect.y,this->rect.x+this->rect.z-10,this->rect.y+this->rect.w,0.5f,0.75);



	float tip=(this->rect.x+10) + ((*referenceValue)/(maximum-minimum))*(this->rect.z-20);

	tab->tabContainer->renderer->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(tip-5,this->rect.y+this->rect.w/4.0f-5,tip+5,this->rect.y+this->rect.w/4.0f+5),2,2),tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT));


	if(selfRender)
		tab->tabContainer->EndDraw();
}

void GuiSlider::OnMouseMove(GuiTab* tab)
{
	GuiRect::OnMouseMove(tab);

	if(this->pressing)
	{
		if(tab->tabContainer->mousex > (this->rect.x+10) && tab->tabContainer->mousex < (this->rect.x+this->rect.z-10))
		{
			float f1=(tab->tabContainer->mousex-(this->rect.x+10))/(this->rect.z-20);
			float f2=maximum-minimum;
			(*referenceValue)=f1*f2;

			this->OnPaint(tab);
		}
	}
}

void GuiPropertySlider::OnPaint(GuiTab* tab)
{
	bool selfRender=!tab->tabContainer->isRender;

	if(selfRender)
		tab->tabContainer->BeginDraw();

	GuiRect::OnPaint(tab);

	if(prp.Buf())
	{
		String s=prp + " " + String(*this->slider.referenceValue);
		Direct2DGuiBase::DrawText(tab->tabContainer->renderer,tab->tabContainer->SetColor(GuiInterface::COLOR_TEXT),s,this->rect.x,this->rect.y,this->rect.x+rect.z/2.0f,this->rect.y+this->rect.w);
	}

	if(selfRender)
		tab->tabContainer->EndDraw();
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