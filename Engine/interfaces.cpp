#include "gui_interfaces.h"
#include "win32.h"



////////////////////////////
////////GuiRect///////
////////////////////////////

GuiRect::GuiRect(GuiRect* iParent,float ix, float iy, float iw,float ih,vec2 _alignPos,vec2 _alignRect):
	colorBackground(TabContainer::COLOR_GUI_BACKGROUND),
	colorForeground(TabContainer::COLOR_TEXT),
	colorHovering(colorBackground),
	colorPressing(colorBackground),
	colorBorder(colorBackground),
	colorChecked(colorBackground),
	pressing(false),
	hovering(false),
	checked(false),
	active(false),
	clip(0)
{
	this->Set(iParent,0,0,-1,ix,iy,iw,ih,_alignPos.x,_alignPos.y,_alignRect.x,_alignRect.y);

	memset(this->sibling,0,16);
}

GuiRect::~GuiRect()
{
}

void GuiRect::SetParent(GuiRect* iParent)
{
	GuiRect* oldParent=this->parent;
	this->parent=iParent;

	if(oldParent)
		oldParent->childs.erase(std::find(oldParent->childs.begin(),oldParent->childs.end(),this));

	if(this->parent)
		this->parent->childs.push_back(this);

	this->active=iParent ? iParent->active : 0;
}

void GuiRect::Set(GuiRect* iParent,GuiRect* iSibling,int siblingIdx,int iContainer,float ix, float iy, float iw,float ih,float apx,float apy,float arx,float ary)
{
	this->sibling[siblingIdx]=iSibling;
	this->container=iContainer;
	this->rect.make(ix,iy,iw,ih);
	this->alignPos.make(apx,apy);
	this->alignRect.make(arx,ary);
	this->SetParent(iParent);
}

bool GuiRect::_contains(vec4& quad,vec2 point)
{
	return (point.x>quad.x && point.x<(quad.x+quad.z) && point.y>quad.y && point.y<(quad.y+quad.w));
}
	
void GuiRect::BroadcastToChilds(void (GuiRect::*func)(TabContainer*,void*),TabContainer* tabContainer,void* data)
{
	for_each(this->childs.begin(),this->childs.end(),std::bind(func,std::placeholders::_1,tabContainer,data));
}

void GuiRect::BroadcastToRoot(void (GuiRect::*func)(TabContainer*,void*),void* data)
{
	GuiRootRect* root=dynamic_cast<GuiRootRect*>(this->GetRoot());

	if(root)
		(root->*func)(root->tabContainer,data);
}




void GuiRect::OnEntitiesChange(TabContainer* tabContainer,void* data)
{
	this->BroadcastToChilds(&GuiRect::OnEntitiesChange,tabContainer,data);
}

void GuiRect::OnPaint(TabContainer* tabContainer,void* data)
{
	if(!this->active)
		return;

	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	if(this->imageHovering || this->imagePressed || this->imageBackground)
	{
		this->hovering ? this->imageHovering.Draw(tabContainer,this->rect) : (this->pressing ? this->imagePressed.Draw(tabContainer,this->rect) : this->imageBackground.Draw(tabContainer,this->rect));
	}
	else
		tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+this->rect.z,this->rect.y+this->rect.w),tabContainer->SetColor(this->pressing ? this->colorPressing : (this->hovering ? this->colorHovering : this->colorBackground/*(this->checked ? this->colorChecked : this->colorBackground)*/)));

	tabContainer->renderTarget->DrawRectangle(D2D1::RectF(this->rect.x + 0.5f,this->rect.y + 0.5f,this->rect.x+this->rect.z - 0.5f,this->rect.y+this->rect.w - 0.5f),tabContainer->SetColor(this->colorBorder));

	if(this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnPaint,tabContainer,data);

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}


void GuiRect::OnSize(TabContainer* tabContainer,void* data)
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
		this->BroadcastToChilds(&GuiRect::OnSize,tabContainer,data);



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

void GuiRect::OnLMouseDown(TabContainer* tabContainer,void* data)
{
	vec2& mpos=*(vec2*)data;

	bool wasPressing=this->pressing;
	bool bContainerButtonPressed=0;

	this->pressing=this->hovering;

	if(this->hovering)
	{
		this->checked=!this->checked;


		if(this->container>=0)
		{
			bContainerButtonPressed=(mpos.x > this->rect.x && mpos.x < this->rect.x+TabContainer::CONTAINER_ICON_WH && mpos.y > this->rect.y && mpos.y <this->rect.y+TabContainer::CONTAINER_ICON_WH);

			if(bContainerButtonPressed)
				this->container=!this->container;
		}

		if(bContainerButtonPressed)
		{
			tabContainer->BroadcastToSelected(&GuiRect::OnSize);
			this->OnExpandos(tabContainer,this);
			tabContainer->BroadcastToSelected(&GuiRect::OnPaint);
		}

		if(wasPressing!=this->pressing && this->colorPressing!=this->colorBackground)
			this->OnPaint(tabContainer);
		
		return;
	}
	if(this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnLMouseDown,tabContainer,data);

	if(wasPressing!=this->pressing && this->colorPressing!=this->colorBackground)
		this->OnPaint(tabContainer);
}
void GuiRect::OnLMouseUp(TabContainer* tabContainer,void* data)
{
	bool wasPressing=this->pressing;

	this->pressing = false;

	if(this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnLMouseUp,tabContainer,data);

	if(wasPressing!=this->pressing && this->colorPressing!=this->colorBackground)
		this->OnPaint(tabContainer,data);
}

void GuiRect::OnRMouseUp(TabContainer* tabContainer,void* data)
{
	if(this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnRMouseUp,tabContainer,data);
}


void GuiRect::OnMouseMove(TabContainer* tabContainer,void* data)
{
	vec2& mpos=*(vec2*)data;

	bool _oldHover=this->hovering;
	bool _curHover=_contains(this->rect,vec2(mpos.x,mpos.y));

	if(parent && _curHover)
		parent->hovering=false;
	
	this->hovering=_curHover;

	if(/*_curHover && */this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnMouseMove,tabContainer,data);

	if(_oldHover!=this->hovering && this->colorBackground!=this->colorHovering)
		this->OnPaint(tabContainer);
}

void GuiRect::OnUpdate(TabContainer* tabContainer,void* data)
{
	this->BroadcastToChilds(&GuiRect::OnUpdate,tabContainer,data);
}

void GuiRect::OnReparent(TabContainer* tabContainer,void* data)
{
	this->BroadcastToChilds(&GuiRect::OnReparent,tabContainer,data);
}
void GuiRect::OnSelected(TabContainer* tabContainer,void* data)
{
	this->BroadcastToChilds(&GuiRect::OnSelected,tabContainer,data);
}
void GuiRect::OnRender(TabContainer* tabContainer,void* data)
{
	this->BroadcastToChilds(&GuiRect::OnRender,tabContainer);
}
void GuiRect::OnMouseWheel(TabContainer* tabContainer,void* data)
{
	this->BroadcastToChilds(&GuiRect::OnMouseWheel,tabContainer,data);
}
void GuiRect::OnActivate(TabContainer* tabContainer,void* data)
{
	/*if(this->active)
		__debugbreak();*/
	this->active=true;
	this->BroadcastToChilds(&GuiRect::OnActivate,tabContainer);
}
void GuiRect::OnDeactivate(TabContainer* tabContainer,void* data)
{
	/*if(!active)
		__debugbreak();*/
	this->active=false;
	this->BroadcastToChilds(&GuiRect::OnDeactivate,tabContainer,data);
}
void GuiRect::OnEntitySelected(TabContainer* tabContainer,void* data)
{
	this->BroadcastToChilds(&GuiRect::OnEntitySelected,tabContainer,data);
}

void GuiRect::OnExpandos(TabContainer* tabContainer,void* data)
{
	if(this->parent)
		this->parent->OnExpandos(tabContainer,data);
}

void GuiRect::SetClip(GuiScrollRect* scrollRect)
{
	this->clip=scrollRect;

	for_each(this->childs.begin(),this->childs.end(),std::bind(&GuiRect::SetClip,std::placeholders::_1,scrollRect));
}


bool GuiRect::SelfRender(TabContainer* tabContainer)
{
	bool selfRender=!tabContainer->isRender;

	if(selfRender)
		tabContainer->BeginDraw();

	return selfRender;
}

void GuiRect::SelfRenderEnd(TabContainer* tabContainer,bool& isSelfRender)
{
	if(isSelfRender)
		tabContainer->EndDraw();
}

bool GuiRect::SelfClip(TabContainer* tabContainer)
{
	bool clipped=false;

	if(this->clip && !this->clip->isClipped)
	{
		tabContainer->renderTarget->PushAxisAlignedClip(D2D1::RectF(this->clip->rect.x,this->clip->rect.y,this->clip->rect.x+this->clip->width,this->clip->rect.y+this->clip->rect.w),D2D1_ANTIALIAS_MODE_ALIASED);
		tabContainer->renderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0,-this->clip->scrollBar->scrollerPosition*this->clip->contentHeight));

		clipped=this->clip->isClipped=true;
	}

	return clipped;
}

void GuiRect::SelfClipEnd(TabContainer* tabContainer,bool& isSelfClip)
{
	if(isSelfClip)
	{
		tabContainer->renderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		tabContainer->renderTarget->PopAxisAlignedClip();
		this->clip->isClipped=false;
	}
}



GuiRect* GuiRect::GetRoot()
{
	return this->parent ? this->parent->GetRoot() : this;
}


GuiRect* GuiRect::Rect(float ix, float iy, float iw,float ih,float apx, float apy, float arx,float ary)
{
	return new GuiRect(this,ix,iy,iw,ih,vec2(apx,apy),vec2(arx,ary));
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
	GuiString* s=new GuiString;
	s->Set(this,((int)this->childs.size()) ? this->childs.back() : 0,1,0,0,0,0,20,0,0,1,-1);
	s->text=iText;
	s->alignText.make(1,-1);

	return s;
}

GuiPropertyString* GuiRect::Property(const char* iProp,const char* iVal)
{
	GuiPropertyString* p=new GuiPropertyString;
	p->Set(this,!this->childs.empty() ? this->childs.back() : 0,1,-1,0,0,0,20,0,0,1,-1);
	p->prp=iProp;
	p->val=iVal;
	
	return p;
}

GuiPropertyString* GuiRect::Property(const char* iProp,vec3 iRVal)
{
	char str[100];
	sprintf(str,"%3.2f , %3.2f , %3.2f",iRVal.x,iRVal.y,iRVal.z);
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

GuiViewport* GuiRect::Viewport(vec3 pos,vec3 target,vec3 up,bool perspective)
{
	GuiViewport* v=new GuiViewport();
	v->Set(this,0,0,-1,0,0,0,0,0,0,1,1);
	v->projection= !perspective ? v->projection : v->projection.perspective(90,16/9,1,1000);
	v->view.move(pos);
	v->view.lookat(target,up);
	return v;
}

GuiSceneViewer* GuiRect::SceneViewer()
{
	GuiSceneViewer* sv=new GuiSceneViewer();
	sv->Set(this,0,0,-1,0,0,0,0,0,0,1,1);
	return sv;
}

GuiEntityViewer* GuiRect::EntityViewer()
{
	GuiEntityViewer* sv=new GuiEntityViewer();
	sv->Set(this,0,0,-1,0,0,0,0,0,0,1,1);
	return sv;
}
GuiProjectViewer* GuiRect::ProjectViewer()
{
	GuiProjectViewer* sv=new GuiProjectViewer();
	sv->Set(this,0,0,-1,0,0,0,0,0,0,1,1);
	return sv;
}


////////////////////////////
////////GuiRect///////
////////////////////////////

void GuiRootRect::OnSize(TabContainer* tab)
{
	this->rect.make(0,TabContainer::CONTAINER_HEIGHT,tabContainer->width,tabContainer->height-TabContainer::CONTAINER_HEIGHT);

	if(this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnSize,tab);
}

////////////////////////////
////////GuiRect///////
////////////////////////////

void GuiString::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	GuiRect::OnPaint(tabContainer);

	if(text.Buf())
	{
		if(this->container>=0)
			tabContainer->renderTarget->DrawBitmap(this->container==1 ? tabContainer->iconDown : tabContainer->iconRight,D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+TabContainer::CONTAINER_ICON_WH,this->rect.y+TabContainer::CONTAINER_ICON_WH));
			
		Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),text,this->container>=0 ? this->rect.x+TREEVIEW_ROW_ADVANCE : this->rect.x,this->rect.y,this->rect.z,this->rect.y+TabContainer::CONTAINER_ICON_WH,-1,0.5);
	}

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}


////////////////////////////
////////GuiRect///////
////////////////////////////

void GuiButton::OnLMouseUp(TabContainer* tab,void* data)
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

GuiScrollRect::GuiScrollRect():
contentHeight(0),
width(0),
isClipped(false)
{
	this->scrollBar=new GuiScrollBar;
	this->scrollBar->guiRect=this;
	this->scrollBar->Set(this,0,0,-1,0,0,20,0,1,0,-1,1);
}

GuiScrollRect::~GuiScrollRect()
{
	SAFEDELETE(this->scrollBar);
}

void GuiScrollRect::OnMouseWheel(TabContainer* tabContainer,void* data)
{
	GuiRect::OnMouseWheel(tabContainer,data);

	if(!this->hovering)
		return;

	float scrollValue=*(float*)data;
	this->scrollBar->Scroll(scrollValue);

	this->OnPaint(tabContainer);
}

void GuiScrollRect::OnSize(TabContainer* tabContainer,void* data)
{
	GuiRect::OnSize(tabContainer,data);

	this->scrollBar->SetScrollerRatio(this->contentHeight,this->rect.w);
	this->width=this->scrollBar->IsVisible() ? this->rect.z-GuiScrollBar::SCROLLBAR_WIDTH : this->rect.z;
}


////////////////////////////
////////GuiRect///////
////////////////////////////

void GuiPropertyString::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	GuiRect::OnPaint(tabContainer);

	if(prp.Buf())
		Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),prp,this->rect.x,this->rect.y,this->rect.x+this->rect.z/2.0f,this->rect.y+this->rect.w);
	if(val.Buf())	
		Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),val,this->rect.x+this->rect.z/2.0f,this->rect.y,this->rect.x+this->rect.z,this->rect.y+this->rect.w);

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}

////////////////////////////
////////GuiRect///////
////////////////////////////

void GuiSlider::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	GuiRect::OnPaint(tabContainer);

	tabContainer->renderTarget->FillRectangle(D2D1::RectF(this->rect.x+10,this->rect.y+this->rect.w/4.0f-2,this->rect.x+this->rect.z-10,this->rect.y+this->rect.w/4.0f+2),tabContainer->SetColor(0x000000));

	String smin(this->minimum);
	String smax(this->maximum);
	String value(*this->referenceValue);

	Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),smin,this->rect.x+10,this->rect.y,this->rect.x+this->rect.z-10,this->rect.y+this->rect.w,0,0.75);
	Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),smax,this->rect.x+10,this->rect.y,this->rect.x+this->rect.z-10,this->rect.y+this->rect.w,1,0.75);
	Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),value,this->rect.x+10,this->rect.y,this->rect.x+this->rect.z-10,this->rect.y+this->rect.w,0.5f,0.75);

	float tip=(this->rect.x+10) + ((*referenceValue)/(maximum-minimum))*(this->rect.z-20);

	tabContainer->renderTarget->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(tip-5,this->rect.y+this->rect.w/4.0f-5,tip+5,this->rect.y+this->rect.w/4.0f+5),2,2),tabContainer->SetColor(TabContainer::COLOR_TEXT));

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}

void GuiSlider::OnMouseMove(TabContainer* tabContainer,void* data)
{
	GuiRect::OnMouseMove(tabContainer,data);

	if(this->pressing)
	{
		vec2& mpos=*(vec2*)data;

		if(mpos.x > this->rect.x && mpos.x < this->rect.x+this->rect.z)
		{
			float f1=(mpos.x-(this->rect.x+10))/(this->rect.z-20);
			float f2=maximum-minimum;
			float cursor=f1*f2;

			cursor = cursor<minimum ? minimum : (cursor>maximum ? maximum : cursor);
			
			if((*referenceValue)!=cursor)
			{
				(*referenceValue)=cursor;
				this->OnPaint(tabContainer);
			}
		}
	}
}

void GuiSlider::OnSize(TabContainer* tabContainer,void* data)
{
	/*float oldRatio=maximum/minimum;
	float value=this->referenceValue ? *this->referenceValue : 0;*/

	GuiRect::OnSize(tabContainer);



}

////////////////////////////
////////GuiRect///////
////////////////////////////

void GuiPropertySlider::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	GuiRect::OnPaint(tabContainer);

	if(prp.Buf())
	{
		String s=prp + " " + String(*this->slider.referenceValue);
		Direct2DGuiBase::DrawText(tabContainer->renderTarget,tabContainer->SetColor(TabContainer::COLOR_TEXT),s,this->rect.x,this->rect.y,this->rect.x+rect.z/2.0f,this->rect.y+this->rect.w);
	}

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}

////////////////////////////
////////GuiRect///////
////////////////////////////

void GuiPropertyAnimation::OnMouseMove(TabContainer* tab,void* data)
{
	float value=*this->slider.referenceValue;

	GuiRect::OnMouseMove(tab,data);

	if(value!=*this->slider.referenceValue && this->slider.pressing)
	{
		bool old=animController->play;
		animController->play=true;
		animController->update();
		animController->play=old;
	}
}

////////////////////////////
////////GuiRect///////
////////////////////////////

void GuiViewport::Register()
{
	GuiRootRect* root=dynamic_cast<GuiRootRect*>(this->GetRoot());

	if(root)
	{
		if(root->tabContainer->renderer->viewports.end()==std::find(root->tabContainer->renderer->viewports.begin(),root->tabContainer->renderer->viewports.end(),this))
			root->tabContainer->renderer->viewports.push_back(this);

		this->surface->tab=root->tabContainer;
	}
	else
		__debugbreak();
}

void GuiViewport::Unregister()
{
	GuiRootRect* root=dynamic_cast<GuiRootRect*>(this->GetRoot());

	if(root)
		root->tabContainer->renderer->viewports.remove(this);
	else
		__debugbreak();
}

bool GuiViewport::Registered()
{
	GuiRootRect* root=dynamic_cast<GuiRootRect*>(this->GetRoot());

	if(root)
		return root->tabContainer->renderer->viewports.end()!=std::find(root->tabContainer->renderer->viewports.begin(),root->tabContainer->renderer->viewports.end(),this);
	else
		__debugbreak();

	return 0;
}

GuiViewport::GuiViewport():surface(new RenderSurface),rootEntity(0)
{
	this->name="Viewport";
}
GuiViewport::~GuiViewport()
{
	Unregister();

	SAFEDELETE(surface);
}

void GuiViewport::OnSize(TabContainer* tabContainer,void* data)
{
	int w=this->rect.z;
	int h=this->rect.w;

	GuiRect::OnSize(tabContainer);

	if(this->surface && (w!=this->rect.z || h!=this->rect.w))
	{
		SAFERELEASE(this->surface->renderBitmap);
		SAFEDELETEARRAY(this->surface->renderBuffer);
		
		D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
		bp.pixelFormat=tabContainer->renderTarget->GetPixelFormat();

		tabContainer->renderTarget->CreateBitmap(D2D1::SizeU((int)this->rect.z,(int)this->rect.w),bp,&this->surface->renderBitmap);

		if(!this->surface->renderBitmap)
			__debugbreak();

		this->surface->renderBuffer=new unsigned char[(int)(this->rect.z*this->rect.w*4)];
	}
}

void GuiViewport::OnPaint(TabContainer* tabContainer,void* data)
{
	bool selfRender=this->SelfRender(tabContainer);
	bool selfClip=this->SelfClip(tabContainer);

	if(this->surface->renderBitmap)
	{
		if(!selfRender)
		{
			tabContainer->renderer->ChangeContext();
			tabContainer->renderer->Render(this,false);
		}
		
		tabContainer->renderTarget->DrawBitmap(this->surface->renderBitmap,D2D1::RectF(this->rect.x,this->rect.y,this->rect.x+this->rect.z,this->rect.y+this->rect.w));
	}

	if(this->container!=0)
		this->BroadcastToChilds(&GuiRect::OnPaint,tabContainer);

	this->SelfClipEnd(tabContainer,selfClip);
	this->SelfRenderEnd(tabContainer,selfRender);
}

void GuiViewport::OnMouseWheel(TabContainer* tabContainer,void* data)
{
	GuiRect::OnMouseWheel(tabContainer);

	float factor=*(float*)data;

	this->view*=mat4().translate(0,0,factor*10);
}

void GuiViewport::OnMouseMove(TabContainer* tabContainer,void* data)
{
	GuiRect::OnMouseMove(tabContainer,data);

	vec2& mpos=*(vec2*)data;
	vec2 &pos=InputManager::mouseInput.mouse_pos;
	vec2 &oldpos=InputManager::mouseInput.mouse_posold;

	oldpos=pos;
	pos.x=mpos.x;
	pos.y=mpos.y;

	if(tabContainer->buttonLeftMouseDown && GetFocus()==tabContainer->hwnd)
	{
		float dX=(pos.x-oldpos.x);
		float dY=(pos.y-oldpos.y);

		if(tabContainer->buttonControlDown)
		{

			mat4 mview;
			vec3 vx,vy,vz;
			vec3 pos;
			mat4 rot;

			mview=this->view;

			mview.traspose();
			mview.inverse();

			mview.axes(vx,vy,vz);

			pos=this->model.position();

			this->model.move(vec3());

			if(dY)
				rot.rotate(dY,vx);
			this->model.rotate(dX,0,0,1);

			this->model*=rot;

			this->model.move(pos);
		}
		else
		{
			this->view*=mat4().translate(dX,dY,0);
		}
	}
}


void GuiViewport::OnActivate(TabContainer* tabContainer,void* data)
{
	GuiRect::OnActivate(tabContainer);
	Register();
}
void GuiViewport::OnDeactivate(TabContainer* tabContainer,void* data)
{
	GuiRect::OnDeactivate(tabContainer);
	Unregister();
}

void GuiViewport::OnReparent(TabContainer* tabContainer,void* data)
{
	GuiRect::OnReparent(tabContainer);
}





GuiImage::GuiImage():image(0),data(0),width(0),height(0){}



bool GuiImage::Load(const char* fName)
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

void GuiImage::Draw(TabContainer* tabContainer,vec4& rect)
{
	if(this->image)
	{
		ID2D1Bitmap* bitmap=(ID2D1Bitmap*)this->data;

		if(!bitmap)
		{
			D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
			bp.pixelFormat=tabContainer->renderTarget->GetPixelFormat();

			HRESULT result=tabContainer->renderTarget->CreateBitmap(D2D1::SizeU((int)this->width,(int)this->height),this->image,(int)(4*this->width),bp,&bitmap);

			if(!bitmap || result!=S_OK)
				__debugbreak();
		}

		tabContainer->renderTarget->DrawBitmap(bitmap,D2D1::RectF(rect.x,rect.y,rect.x+rect.z,rect.y+rect.w));
	}
}