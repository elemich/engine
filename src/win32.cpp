#include "win32.h"

GLOBALGETTERFUNC(GlobalIdeInstance,IdeWin32*);
GLOBALGETTERFUNC(GlobalTimerInstance,TimerWin32*);

static const wchar_t* gWM_MESSAGES[] = 
{
	L"WM_NULL",
	L"WM_CREATE",
	L"WM_DESTROY",
	L"WM_MOVE",
	L"WM_SIZEWAIT",
	L"WM_SIZE",
	L"WM_ACTIVATE",
	L"WM_SETFOCUS",
	L"WM_KILLFOCUS",
	L"WM_SETVISIBLE",
	L"WM_ENABLE",
	L"WM_SETREDRAW",
	L"WM_SETTEXT",
	L"WM_GETTEXT",
	L"WM_GETTEXTLENGTH",
	L"WM_PAINT",
	L"WM_CLOSE",
	L"WM_QUERYENDSESSION",
	L"WM_QUIT",
	L"WM_QUERYOPEN",
	L"WM_ERASEBKGND",
	L"WM_SYSCOLORCHANGE",
	L"WM_ENDSESSION",
	L"WM_SYSTEMERROR",
	L"WM_SHOWWINDOW",
	L"WM_CTLCOLOR",
	L"WM_WININICHANGE",
	L"WM_DEVMODECHANGE",
	L"WM_ACTIVATEAPP",
	L"WM_FONTCHANGE",
	L"WM_TIMECHANGE",
	L"WM_CANCELMODE",
	L"WM_SETCURSOR",
	L"WM_MOUSEACTIVATE",
	L"WM_CHILDACTIVATE",
	L"WM_QUEUESYNC",
	L"WM_GETMINMAXINFO",
	L"WM_LOGOFF",
	L"WM_PAINTICON",
	L"WM_ICONERASEBKGND",
	L"WM_NEXTDLGCTL",
	L"WM_ALTTABACTIVE",
	L"WM_SPOOLERSTATUS",
	L"WM_DRAWITEM",
	L"WM_MEASUREITEM",
	L"WM_DELETEITEM",
	L"WM_VKEYTOITEM",
	L"WM_CHARTOITEM",
	L"WM_SETFONT",
	L"WM_GETFONT",
	L"WM_SETHOTKEY",
	L"WM_GETHOTKEY",
	L"WM_SHELLNOTIFY",
	L"WM_ISACTIVEICON",
	L"WM_QUERYPARKICON",
	L"WM_QUERYDRAGICON",
	L"WM_WINHELP",
	L"WM_COMPAREITEM",
	L"WM_FULLSCREEN",
	L"WM_CLIENTSHUTDOWN",
	L"WM_DDEMLEVENT",
	L"WM_GETOBJECT",
	L"<undefined>",
	L"<undefined>",
	L"WM_TESTING",
	L"WM_COMPACTING",
	L"WM_OTHERWINDOWCREATED",
	L"WM_OTHERWINDOWDESTROYED",
	L"WM_COMMNOTIFY",
	L"<undefined>",
	L"WM_WINDOWPOSCHANGING",
	L"WM_WINDOWPOSCHANGED",
	L"WM_POWER",
	L"WM_COPYGLOBALDATA",
	L"WM_COPYDATA",
	L"WM_CANCELJOURNAL",
	L"<undefined>",
	L"WM_KEYF1",
	L"WM_NOTIFY",
	L"WM_ACCESS_WINDOW",
	L"WM_INPUTLANGCHANGEREQUEST",
	L"WM_INPUTLANGCHANGE",
	L"WM_TCARD",
	L"WM_HELP",
	L"WM_USERCHANGED",
	L"WM_NOTIFYFORMAT",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_FINALDESTROY",
	L"WM_MEASUREITEM_CLIENTDATA",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_CONTEXTMENU",
	L"WM_STYLECHANGING",
	L"WM_STYLECHANGED",
	L"WM_DISPLAYCHANGE",
	L"WM_GETICON",
	L"WM_SETICON",
	L"WM_NCCREATE",
	L"WM_NCDESTROY",
	L"WM_NCCALCSIZE",
	L"WM_NCHITTEST",
	L"WM_NCPAINT",
	L"WM_NCACTIVATE",
	L"WM_GETDLGCODE",
	L"WM_SYNCPAINT",
	L"WM_SYNCTASK",
	L"<undefined>",
	L"WM_KLUDGEMINRECT",
	L"WM_LPKDRAWSWITCHWND",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_UAHDESTROYWINDOW",
	L"WM_UAHDRAWMENU",
	L"WM_UAHDRAWMENUITEM",
	L"WM_UAHINITMENU",
	L"WM_UAHMEASUREMENUITEM",
	L"WM_UAHNCPAINTMENUPOPUP",
	L"WM_UAHUPDATE",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_NCMOUSEMOVE",
	L"WM_NCLBUTTONDOWN",
	L"WM_NCLBUTTONUP",
	L"WM_NCLBUTTONDBLCLK",
	L"WM_NCRBUTTONDOWN",
	L"WM_NCRBUTTONUP",
	L"WM_NCRBUTTONDBLCLK",
	L"WM_NCMBUTTONDOWN",
	L"WM_NCMBUTTONUP",
	L"WM_NCMBUTTONDBLCLK",
	L"<undefined>",
	L"WM_NCXBUTTONDOWN",
	L"WM_NCXBUTTONUP",
	L"WM_NCXBUTTONDBLCLK",
	L"WM_NCUAHDRAWCAPTION",
	L"WM_NCUAHDRAWFRAME",
	L"EM_GETSEL",
	L"EM_SETSEL",
	L"EM_GETRECT",
	L"EM_SETRECT",
	L"EM_SETRECTNP",
	L"EM_SCROLL",
	L"EM_LINESCROLL",
	L"EM_SCROLLCARET",
	L"EM_GETMODIFY",
	L"EM_SETMODIFY",
	L"EM_GETLINECOUNT",
	L"EM_LINEINDEX",
	L"EM_SETHANDLE",
	L"EM_GETHANDLE",
	L"EM_GETTHUMB",
	L"<undefined>",
	L"<undefined>",
	L"EM_LINELENGTH",
	L"EM_REPLACESEL",
	L"EM_SETFONT",
	L"EM_GETLINE",
	L"EM_LIMITTEXT",
	L"EM_CANUNDO",
	L"EM_UNDO",
	L"EM_FMTLINES",
	L"EM_LINEFROMCHAR",
	L"EM_SETWORDBREAK",
	L"EM_SETTABSTOPS",
	L"EM_SETPASSWORDCHAR",
	L"EM_EMPTYUNDOBUFFER",
	L"EM_GETFIRSTVISIBLELINE",
	L"EM_SETREADONLY",
	L"EM_SETWORDBREAKPROC",
	L"EM_GETWORDBREAKPROC",
	L"EM_GETPASSWORDCHAR",
	L"EM_SETMARGINS",
	L"EM_GETMARGINS",
	L"EM_GETLIMITTEXT",
	L"EM_POSFROMCHAR",
	L"EM_CHARFROMPOS",
	L"EM_SETIMESTATUS",
	L"EM_GETIMESTATUS",
	L"EM_MSGMAX",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_INPUT_DEVICE_CHANGE",
	L"WM_INPUT",
	L"WM_KEYDOWN",
	L"WM_KEYUP",
	L"WM_CHAR",
	L"WM_DEADCHAR",
	L"WM_SYSKEYDOWN",
	L"WM_SYSKEYUP",
	L"WM_SYSCHAR",
	L"WM_SYSDEADCHAR",
	L"WM_YOMICHAR",
	L"WM_UNICHAR",
	L"WM_CONVERTREQUEST",
	L"WM_CONVERTRESULT",
	L"WM_INTERIM",
	L"WM_IME_STARTCOMPOSITION",
	L"WM_IME_ENDCOMPOSITION",
	L"WM_IME_COMPOSITION",
	L"WM_INITDIALOG",
	L"WM_COMMAND",
	L"WM_SYSCOMMAND",
	L"WM_TIMER",
	L"WM_HSCROLL",
	L"WM_VSCROLL",
	L"WM_INITMENU",
	L"WM_INITMENUPOPUP",
	L"WM_SYSTIMER",
	L"WM_GESTURE",
	L"WM_GESTURENOTIFY",
	L"WM_GESTUREINPUT",
	L"WM_GESTURENOTIFIED",
	L"<undefined>",
	L"<undefined>",
	L"WM_MENUSELECT",
	L"WM_MENUCHAR",
	L"WM_ENTERIDLE",
	L"WM_MENURBUTTONUP",
	L"WM_MENUDRAG",
	L"WM_MENUGETOBJECT",
	L"WM_UNINITMENUPOPUP",
	L"WM_MENUCOMMAND",
	L"WM_CHANGEUISTATE",
	L"WM_UPDATEUISTATE",
	L"WM_QUERYUISTATE",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_LBTRACKPOINT",
	L"WM_CTLCOLORMSGBOX",
	L"WM_CTLCOLOREDIT",
	L"WM_CTLCOLORLISTBOX",
	L"WM_CTLCOLORBTN",
	L"WM_CTLCOLORDLG",
	L"WM_CTLCOLORSCROLLBAR",
	L"WM_CTLCOLORSTATIC",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"CB_GETEDITSEL",
	L"CB_LIMITTEXT",
	L"CB_SETEDITSEL",
	L"CB_ADDSTRING",
	L"CB_DELETESTRING",
	L"CB_DIR",
	L"CB_GETCOUNT",
	L"CB_GETCURSEL",
	L"CB_GETLBTEXT",
	L"CB_GETLBTEXTLEN",
	L"CB_INSERTSTRING",
	L"CB_RESETCONTENT",
	L"CB_FINDSTRING",
	L"CB_SELECTSTRING",
	L"CB_SETCURSEL",
	L"CB_SHOWDROPDOWN",
	L"CB_GETITEMDATA",
	L"CB_SETITEMDATA",
	L"CB_GETDROPPEDCONTROLRECT",
	L"CB_SETITEMHEIGHT",
	L"CB_GETITEMHEIGHT",
	L"CB_SETEXTENDEDUI",
	L"CB_GETEXTENDEDUI",
	L"CB_GETDROPPEDSTATE",
	L"CB_FINDSTRINGEXACT",
	L"CB_SETLOCALE",
	L"CB_GETLOCALE",
	L"CB_GETTOPINDEX",
	L"CB_SETTOPINDEX",
	L"CB_GETHORIZONTALEXTENT",
	L"CB_SETHORIZONTALEXTENT",
	L"CB_GETDROPPEDWIDTH",
	L"CB_SETDROPPEDWIDTH",
	L"CB_INITSTORAGE",
	L"CB_MSGMAX_OLD",
	L"CB_MULTIPLEADDSTRING",
	L"CB_GETCOMBOBOXINFO",
	L"CB_MSGMAX",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"LB_ADDSTRING",
	L"LB_INSERTSTRING",
	L"LB_DELETESTRING",
	L"LB_SELITEMRANGEEX",
	L"LB_RESETCONTENT",
	L"LB_SETSEL",
	L"LB_SETCURSEL",
	L"LB_GETSEL",
	L"LB_GETCURSEL",
	L"LB_GETTEXT",
	L"LB_GETTEXTLEN",
	L"LB_GETCOUNT",
	L"LB_SELECTSTRING",
	L"LB_DIR",
	L"LB_GETTOPINDEX",
	L"LB_FINDSTRING",
	L"LB_GETSELCOUNT",
	L"LB_GETSELITEMS",
	L"LB_SETTABSTOPS",
	L"LB_GETHORIZONTALEXTENT",
	L"LB_SETHORIZONTALEXTENT",
	L"LB_SETCOLUMNWIDTH",
	L"LB_ADDFILE",
	L"LB_SETTOPINDEX",
	L"LB_GETITEMRECT",
	L"LB_GETITEMDATA",
	L"LB_SETITEMDATA",
	L"LB_SELITEMRANGE",
	L"LB_SETANCHORINDEX",
	L"LB_GETANCHORINDEX",
	L"LB_SETCARETINDEX",
	L"LB_GETCARETINDEX",
	L"LB_SETITEMHEIGHT",
	L"LB_GETITEMHEIGHT",
	L"LB_FINDSTRINGEXACT",
	L"LBCB_CARETON",
	L"LBCB_CARETOFF",
	L"LB_SETLOCALE",
	L"LB_GETLOCALE",
	L"LB_SETCOUNT",
	L"LB_INITSTORAGE",
	L"LB_ITEMFROMPOINT",
	L"LB_INSERTSTRINGUPPER",
	L"LB_INSERTSTRINGLOWER",
	L"LB_ADDSTRINGUPPER",
	L"LB_ADDSTRINGLOWER",
	L"LBCB_STARTTRACK",
	L"LBCB_ENDTRACK",
	L"LB_MSGMAX_OLD",
	L"LB_MULTIPLEADDSTRING",
	L"LB_GETLISTBOXINFO",
	L"LB_MSGMAX",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"MN_FIRST",
	L"MN_GETHMENU",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_MOUSEMOVE",
	L"WM_LBUTTONDOWN",
	L"WM_LBUTTONUP",
	L"WM_LBUTTONDBLCLK",
	L"WM_RBUTTONDOWN",
	L"WM_RBUTTONUP",
	L"WM_RBUTTONDBLCLK",
	L"WM_MBUTTONDOWN",
	L"WM_MBUTTONUP",
	L"WM_MBUTTONDBLCLK",
	L"WM_MOUSEWHEEL",
	L"WM_XBUTTONDOWN",
	L"WM_XBUTTONUP",
	L"WM_XBUTTONDBLCLK",
	L"WM_MOUSEHWHEEL",
	L"<undefined>",
	L"WM_PARENTNOTIFY",
	L"WM_ENTERMENULOOP",
	L"WM_EXITMENULOOP",
	L"WM_NEXTMENU",
	L"WM_SIZING",
	L"WM_CAPTURECHANGED",
	L"WM_MOVING",
	L"<undefined>",
	L"WM_POWERBROADCAST",
	L"WM_DEVICECHANGE",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_MDICREATE",
	L"WM_MDIDESTROY",
	L"WM_MDIACTIVATE",
	L"WM_MDIRESTORE",
	L"WM_MDINEXT",
	L"WM_MDIMAXIMIZE",
	L"WM_MDITILE",
	L"WM_MDICASCADE",
	L"WM_MDIICONARRANGE",
	L"WM_MDIGETACTIVE",
	L"WM_DROPOBJECT",
	L"WM_QUERYDROPOBJECT",
	L"WM_BEGINDRAG",
	L"WM_DRAGLOOP",
	L"WM_DRAGSELECT",
	L"WM_DRAGMOVE",
	L"WM_MDISETMENU",
	L"WM_ENTERSIZEMOVE",
	L"WM_EXITSIZEMOVE",
	L"WM_DROPFILES",
	L"WM_MDIREFRESHMENU",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_POINTERDEVICECHANGE",
	L"WM_POINTERDEVICEINRANGE",
	L"WM_POINTERDEVICEOUTOFRANGE",
	L"WM_STOPINERTIA",
	L"WM_ENDINERTIA",
	L"WM_EDGYINERTIA",
	L"<undefined>",
	L"<undefined>",
	L"WM_TOUCH",
	L"WM_NCPOINTERUPDATE",
	L"WM_NCPOINTERDOWN",
	L"WM_NCPOINTERUP",
	L"WM_NCPOINTERLAST",
	L"WM_POINTERUPDATE",
	L"WM_POINTERDOWN",
	L"WM_POINTERUP",
	L"<WM_POINTER_reserved_248>",
	L"WM_POINTERENTER",
	L"WM_POINTERLEAVE",
	L"WM_POINTERACTIVATE",
	L"WM_POINTERCAPTURECHANGED",
	L"WM_TOUCHHITTESTING",
	L"WM_POINTERWHEEL",
	L"WM_POINTERHWHEEL",
	L"<WM_POINTER_reserved_250>",
	L"<WM_POINTER_reserved_251>",
	L"<WM_POINTER_reserved_252>",
	L"<WM_POINTER_reserved_253>",
	L"<WM_POINTER_reserved_254>",
	L"<WM_POINTER_reserved_255>",
	L"<WM_POINTER_reserved_256>",
	L"WM_POINTERLAST",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_VISIBILITYCHANGED",
	L"WM_VIEWSTATECHANGED",
	L"WM_UNREGISTER_WINDOW_SERVICES",
	L"WM_CONSOLIDATED",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_IME_REPORT",
	L"WM_IME_SETCONTEXT",
	L"WM_IME_NOTIFY",
	L"WM_IME_CONTROL",
	L"WM_IME_COMPOSITIONFULL",
	L"WM_IME_SELECT",
	L"WM_IME_CHAR",
	L"WM_IME_SYSTEM",
	L"WM_IME_REQUEST",
	L"<WM_KANJI_reserved_289>",
	L"<WM_KANJI_reserved_28a>",
	L"<WM_KANJI_reserved_28b>",
	L"<WM_KANJI_reserved_28c>",
	L"<WM_KANJI_reserved_28d>",
	L"<WM_KANJI_reserved_28e>",
	L"<WM_KANJI_reserved_28f>",
	L"WM_IME_KEYDOWN",
	L"WM_IME_KEYUP",
	L"<WM_KANJI_reserved_292>",
	L"<WM_KANJI_reserved_293>",
	L"<WM_KANJI_reserved_294>",
	L"<WM_KANJI_reserved_295>",
	L"<WM_KANJI_reserved_296>",
	L"<WM_KANJI_reserved_297>",
	L"<WM_KANJI_reserved_298>",
	L"<WM_KANJI_reserved_299>",
	L"<WM_KANJI_reserved_29a>",
	L"<WM_KANJI_reserved_29b>",
	L"<WM_KANJI_reserved_29c>",
	L"<WM_KANJI_reserved_29d>",
	L"<WM_KANJI_reserved_29e>",
	L"WM_KANJILAST",
	L"WM_NCMOUSEHOVER",
	L"WM_MOUSEHOVER",
	L"WM_NCMOUSELEAVE",
	L"WM_MOUSELEAVE",
	L"<WM_TRACKMOUSEEVENT__reserved_2a4>",
	L"<WM_TRACKMOUSEEVENT__reserved_2a5>",
	L"<WM_TRACKMOUSEEVENT__reserved_2a6>",
	L"<WM_TRACKMOUSEEVENT__reserved_2a7>",
	L"<WM_TRACKMOUSEEVENT__reserved_2a8>",
	L"<WM_TRACKMOUSEEVENT__reserved_2a9>",
	L"<WM_TRACKMOUSEEVENT__reserved_2aa>",
	L"<WM_TRACKMOUSEEVENT__reserved_2ab>",
	L"<WM_TRACKMOUSEEVENT__reserved_2ac>",
	L"<WM_TRACKMOUSEEVENT__reserved_2ad>",
	L"<WM_TRACKMOUSEEVENT__reserved_2ae>",
	L"WM_TRACKMOUSEEVENT_LAST",
	L"<undefined>",
	L"WM_WTSSESSION_CHANGE",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_TABLET_FIRST",
	L"<WM_TABLET__reserved_2c1>",
	L"<WM_TABLET__reserved_2c2>",
	L"<WM_TABLET__reserved_2c3>",
	L"<WM_TABLET__reserved_2c4>",
	L"<WM_TABLET__reserved_2c5>",
	L"<WM_TABLET__reserved_2c6>",
	L"<WM_TABLET__reserved_2c7>",
	L"WM_POINTERDEVICEADDED",
	L"WM_POINTERDEVICEDELETED",
	L"<WM_TABLET__reserved_2ca>",
	L"WM_FLICK",
	L"<WM_TABLET__reserved_2cc>",
	L"WM_FLICKINTERNAL",
	L"WM_BRIGHTNESSCHANGED",
	L"<WM_TABLET__reserved_2cf>",
	L"<WM_TABLET__reserved_2d0>",
	L"<WM_TABLET__reserved_2d1>",
	L"<WM_TABLET__reserved_2d2>",
	L"<WM_TABLET__reserved_2d3>",
	L"<WM_TABLET__reserved_2d4>",
	L"<WM_TABLET__reserved_2d5>",
	L"<WM_TABLET__reserved_2d6>",
	L"<WM_TABLET__reserved_2d7>",
	L"<WM_TABLET__reserved_2d8>",
	L"<WM_TABLET__reserved_2d9>",
	L"<WM_TABLET__reserved_2da>",
	L"<WM_TABLET__reserved_2db>",
	L"<WM_TABLET__reserved_2dc>",
	L"<WM_TABLET__reserved_2dd>",
	L"<WM_TABLET__reserved_2de>",
	L"WM_TABLET_LAST",
	L"WM_DPICHANGED",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_CUT",
	L"WM_COPY",
	L"WM_PASTE",
	L"WM_CLEAR",
	L"WM_UNDO",
	L"WM_RENDERFORMAT",
	L"WM_RENDERALLFORMATS",
	L"WM_DESTROYCLIPBOARD",
	L"WM_DRAWCLIPBOARD",
	L"WM_PAINTCLIPBOARD",
	L"WM_VSCROLLCLIPBOARD",
	L"WM_SIZECLIPBOARD",
	L"WM_ASKCBFORMATNAME",
	L"WM_CHANGECBCHAIN",
	L"WM_HSCROLLCLIPBOARD",
	L"WM_QUERYNEWPALETTE",
	L"WM_PALETTEISCHANGING",
	L"WM_PALETTECHANGED",
	L"WM_HOTKEY",
	L"WM_SYSMENU",
	L"WM_HOOKMSG",
	L"WM_EXITPROCESS",
	L"WM_WAKETHREAD",
	L"WM_PRINT",
	L"WM_PRINTCLIENT",
	L"WM_APPCOMMAND",
	L"WM_THEMECHANGED",
	L"WM_UAHINIT",
	L"WM_DESKTOPNOTIFY",
	L"WM_CLIPBOARDUPDATE",
	L"WM_DWMCOMPOSITIONCHANGED",
	L"WM_DWMNCRENDERINGCHANGED",
	L"WM_DWMCOLORIZATIONCOLORCHANGED",
	L"WM_DWMWINDOWMAXIMIZEDCHANGE",
	L"WM_DWMEXILEFRAME",
	L"WM_DWMSENDICONICTHUMBNAIL",
	L"WM_MAGNIFICATION_STARTED",
	L"WM_MAGNIFICATION_ENDED",
	L"WM_DWMSENDICONICLIVEPREVIEWBITMAP",
	L"WM_DWMTHUMBNAILSIZECHANGED",
	L"WM_MAGNIFICATION_OUTPUT",
	L"WM_BSDRDATA",
	L"WM_DWMTRANSITIONSTATECHANGED",
	L"<undefined>",
	L"WM_KEYBOARDCORRECTIONCALLOUT",
	L"WM_KEYBOARDCORRECTIONACTION",
	L"WM_UIACTION",
	L"WM_ROUTED_UI_EVENT",
	L"WM_MEASURECONTROL",
	L"WM_GETACTIONTEXT",
	L"<WM_CE_ONLY__reserved_332>",
	L"WM_FORWARDKEYDOWN",
	L"WM_FORWARDKEYUP",
	L"<WM_CE_ONLY__reserved_335>",
	L"<WM_CE_ONLY__reserved_336>",
	L"<WM_CE_ONLY__reserved_337>",
	L"<WM_CE_ONLY__reserved_338>",
	L"<WM_CE_ONLY__reserved_339>",
	L"<WM_CE_ONLY__reserved_33a>",
	L"<WM_CE_ONLY__reserved_33b>",
	L"<WM_CE_ONLY__reserved_33c>",
	L"<WM_CE_ONLY__reserved_33d>",
	L"WM_CE_ONLY_LAST",
	L"WM_GETTITLEBARINFOEX",
	L"WM_NOTIFYWOW",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"<undefined>",
	L"WM_HANDHELDFIRST",
	L"<WM_HANDHELD_reserved_359>",
	L"<WM_HANDHELD_reserved_35a>",
	L"<WM_HANDHELD_reserved_35b>",
	L"<WM_HANDHELD_reserved_35c>",
	L"<WM_HANDHELD_reserved_35d>",
	L"<WM_HANDHELD_reserved_35e>",
	L"WM_HANDHELDLAST",
	L"WM_AFXFIRST",
	L"<WM_AFX_reserved_361>",
	L"<WM_AFX_reserved_362>",
	L"<WM_AFX_reserved_363>",
	L"<WM_AFX_reserved_364>",
	L"<WM_AFX_reserved_365>",
	L"<WM_AFX_reserved_366>",
	L"<WM_AFX_reserved_367>",
	L"<WM_AFX_reserved_368>",
	L"<WM_AFX_reserved_369>",
	L"<WM_AFX_reserved_36a>",
	L"<WM_AFX_reserved_36b>",
	L"<WM_AFX_reserved_36c>",
	L"<WM_AFX_reserved_36d>",
	L"<WM_AFX_reserved_36e>",
	L"<WM_AFX_reserved_36f>",
	L"<WM_AFX_reserved_370>",
	L"<WM_AFX_reserved_371>",
	L"<WM_AFX_reserved_372>",
	L"<WM_AFX_reserved_373>",
	L"<WM_AFX_reserved_374>",
	L"<WM_AFX_reserved_375>",
	L"<WM_AFX_reserved_376>",
	L"<WM_AFX_reserved_377>",
	L"<WM_AFX_reserved_378>",
	L"<WM_AFX_reserved_379>",
	L"<WM_AFX_reserved_37a>",
	L"<WM_AFX_reserved_37b>",
	L"<WM_AFX_reserved_37c>",
	L"<WM_AFX_reserved_37d>",
	L"<WM_AFX_reserved_37e>",
	L"WM_AFXLAST",
	L"WM_PENWINFIRST",
	L"<WM_PENWIN_reserved_381>",
	L"<WM_PENWIN_reserved_382>",
	L"<WM_PENWIN_reserved_383>",
	L"<WM_PENWIN_reserved_384>",
	L"<WM_PENWIN_reserved_385>",
	L"<WM_PENWIN_reserved_386>",
	L"<WM_PENWIN_reserved_387>",
	L"<WM_PENWIN_reserved_388>",
	L"<WM_PENWIN_reserved_389>",
	L"<WM_PENWIN_reserved_38a>",
	L"<WM_PENWIN_reserved_38b>",
	L"<WM_PENWIN_reserved_38c>",
	L"<WM_PENWIN_reserved_38d>",
	L"<WM_PENWIN_reserved_38e>",
	L"WM_PENWINLAST",
	L"WM_COALESCE_FIRST",
	L"<WM_COALESCE__reserved_391>",
	L"<WM_COALESCE__reserved_392>",
	L"<WM_COALESCE__reserved_393>",
	L"<WM_COALESCE__reserved_394>",
	L"<WM_COALESCE__reserved_395>",
	L"<WM_COALESCE__reserved_396>",
	L"<WM_COALESCE__reserved_397>",
	L"<WM_COALESCE__reserved_398>",
	L"<WM_COALESCE__reserved_399>",
	L"<WM_COALESCE__reserved_39a>",
	L"<WM_COALESCE__reserved_39b>",
	L"<WM_COALESCE__reserved_39c>",
	L"<WM_COALESCE__reserved_39d>",
	L"<WM_COALESCE__reserved_39e>",
	L"WM_COALESCE_LAST",
	L"WM_MM_RESERVED_FIRST",
	L"<WM_MM_RESERVED__reserved_3a1>",
	L"<WM_MM_RESERVED__reserved_3a2>",
	L"<WM_MM_RESERVED__reserved_3a3>",
	L"<WM_MM_RESERVED__reserved_3a4>",
	L"<WM_MM_RESERVED__reserved_3a5>",
	L"<WM_MM_RESERVED__reserved_3a6>",
	L"<WM_MM_RESERVED__reserved_3a7>",
	L"<WM_MM_RESERVED__reserved_3a8>",
	L"<WM_MM_RESERVED__reserved_3a9>",
	L"<WM_MM_RESERVED__reserved_3aa>",
	L"<WM_MM_RESERVED__reserved_3ab>",
	L"<WM_MM_RESERVED__reserved_3ac>",
	L"<WM_MM_RESERVED__reserved_3ad>",
	L"<WM_MM_RESERVED__reserved_3ae>",
	L"<WM_MM_RESERVED__reserved_3af>",
	L"<WM_MM_RESERVED__reserved_3b0>",
	L"<WM_MM_RESERVED__reserved_3b1>",
	L"<WM_MM_RESERVED__reserved_3b2>",
	L"<WM_MM_RESERVED__reserved_3b3>",
	L"<WM_MM_RESERVED__reserved_3b4>",
	L"<WM_MM_RESERVED__reserved_3b5>",
	L"<WM_MM_RESERVED__reserved_3b6>",
	L"<WM_MM_RESERVED__reserved_3b7>",
	L"<WM_MM_RESERVED__reserved_3b8>",
	L"<WM_MM_RESERVED__reserved_3b9>",
	L"<WM_MM_RESERVED__reserved_3ba>",
	L"<WM_MM_RESERVED__reserved_3bb>",
	L"<WM_MM_RESERVED__reserved_3bc>",
	L"<WM_MM_RESERVED__reserved_3bd>",
	L"<WM_MM_RESERVED__reserved_3be>",
	L"<WM_MM_RESERVED__reserved_3bf>",
	L"<WM_MM_RESERVED__reserved_3c0>",
	L"<WM_MM_RESERVED__reserved_3c1>",
	L"<WM_MM_RESERVED__reserved_3c2>",
	L"<WM_MM_RESERVED__reserved_3c3>",
	L"<WM_MM_RESERVED__reserved_3c4>",
	L"<WM_MM_RESERVED__reserved_3c5>",
	L"<WM_MM_RESERVED__reserved_3c6>",
	L"<WM_MM_RESERVED__reserved_3c7>",
	L"<WM_MM_RESERVED__reserved_3c8>",
	L"<WM_MM_RESERVED__reserved_3c9>",
	L"<WM_MM_RESERVED__reserved_3ca>",
	L"<WM_MM_RESERVED__reserved_3cb>",
	L"<WM_MM_RESERVED__reserved_3cc>",
	L"<WM_MM_RESERVED__reserved_3cd>",
	L"<WM_MM_RESERVED__reserved_3ce>",
	L"<WM_MM_RESERVED__reserved_3cf>",
	L"<WM_MM_RESERVED__reserved_3d0>",
	L"<WM_MM_RESERVED__reserved_3d1>",
	L"<WM_MM_RESERVED__reserved_3d2>",
	L"<WM_MM_RESERVED__reserved_3d3>",
	L"<WM_MM_RESERVED__reserved_3d4>",
	L"<WM_MM_RESERVED__reserved_3d5>",
	L"<WM_MM_RESERVED__reserved_3d6>",
	L"<WM_MM_RESERVED__reserved_3d7>",
	L"<WM_MM_RESERVED__reserved_3d8>",
	L"<WM_MM_RESERVED__reserved_3d9>",
	L"<WM_MM_RESERVED__reserved_3da>",
	L"<WM_MM_RESERVED__reserved_3db>",
	L"<WM_MM_RESERVED__reserved_3dc>",
	L"<WM_MM_RESERVED__reserved_3dd>",
	L"<WM_MM_RESERVED__reserved_3de>",
	L"WM_MM_RESERVED_LAST",
	L"WM_INTERNAL_DDE_FIRST",
	L"<WM_INTERNAL_DDE__reserved_3e1>",
	L"<WM_INTERNAL_DDE__reserved_3e2>",
	L"<WM_INTERNAL_DDE__reserved_3e3>",
	L"<WM_INTERNAL_DDE__reserved_3e4>",
	L"<WM_INTERNAL_DDE__reserved_3e5>",
	L"<WM_INTERNAL_DDE__reserved_3e6>",
	L"<WM_INTERNAL_DDE__reserved_3e7>",
	L"<WM_INTERNAL_DDE__reserved_3e8>",
	L"<WM_INTERNAL_DDE__reserved_3e9>",
	L"<WM_INTERNAL_DDE__reserved_3ea>",
	L"<WM_INTERNAL_DDE__reserved_3eb>",
	L"<WM_INTERNAL_DDE__reserved_3ec>",
	L"<WM_INTERNAL_DDE__reserved_3ed>",
	L"<WM_INTERNAL_DDE__reserved_3ee>",
	L"WM_INTERNAL_DDE_LAST",
	L"WM_CBT_RESERVED_FIRST",
	L"<WM_CBT_RESERVED__reserved_3f1>",
	L"<WM_CBT_RESERVED__reserved_3f2>",
	L"<WM_CBT_RESERVED__reserved_3f3>",
	L"<WM_CBT_RESERVED__reserved_3f4>",
	L"<WM_CBT_RESERVED__reserved_3f5>",
	L"<WM_CBT_RESERVED__reserved_3f6>",
	L"<WM_CBT_RESERVED__reserved_3f7>",
	L"<WM_CBT_RESERVED__reserved_3f8>",
	L"<WM_CBT_RESERVED__reserved_3f9>",
	L"<WM_CBT_RESERVED__reserved_3fa>",
	L"<WM_CBT_RESERVED__reserved_3fb>",
	L"<WM_CBT_RESERVED__reserved_3fc>",
	L"<WM_CBT_RESERVED__reserved_3fd>",
	L"<WM_CBT_RESERVED__reserved_3fe>",
	L"WM_CBT_RESERVED_LAST"
};


#define PRINTPROCEDUREMESSAGSCALLS 0

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

bool Keyboard::IsPressed(unsigned int iCharCode)
{
	return ((::GetKeyState(iCharCode) >> 8) & 0xff)!=0;
}

//////////////////ThreadWin32//////////////////

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

	GuiFont::GetFontPool().push_back(tGuiFont);

	if(!GuiFont::GetDefaultFont())
		GuiFont::SetDefaultFont(tGuiFont);

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
	/*int p1x=p1.x;
	int p1y=p1.y;
	int p2x=p2.x;
	int p2y=p2.y;*/

	brush->SetOpacity(iOpacity);
	//renderer->DrawLine(D2D1::Point2F(p1x,p1y),D2D1::Point2F(p2x,p2y),brush,iWidth);
	renderer->DrawLine(D2D1::Point2F(p1.x,p1.y),D2D1::Point2F(p2.x,p2.y),brush,iWidth);
}

void Direct2D::DrawRectangle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y, float w,float h,float iStroke,float iOpacity)
{
	brush->SetOpacity(iOpacity);
	!iStroke ? renderer->FillRectangle(D2D1::RectF(x,y,w,h),brush) : renderer->DrawRectangle(D2D1::RectF(x,y,w,h),brush,iStroke);
}

void Direct2D::DrawRoundRectangle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y, float w,float h,float iRadiusA,float iRadiusB,float iStroke,float iOpacity)
{
	brush->SetOpacity(iOpacity);
	!iStroke ? renderer->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(x,y,w,h),iRadiusA,iRadiusB),brush) : renderer->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(x,y,w,h),iRadiusA,iRadiusB),brush,iStroke);
}
void Direct2D::DrawCircle(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y,float iRadius,float iStroke,float iOpacity)
{
	brush->SetOpacity(iOpacity);
	!iStroke ? renderer->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x,y),iRadius,iRadius),brush) : renderer->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x,y),iRadius,iRadius),brush,iStroke);
}
void Direct2D::DrawEllipse(ID2D1RenderTarget*renderer,ID2D1Brush* brush,float x,float y,float iRadiusA,float iRadiusB,float iStroke,float iOpacity)
{
	brush->SetOpacity(iOpacity);
	!iStroke ? renderer->FillEllipse(D2D1::Ellipse(D2D1::Point2F(x,y),iRadiusA,iRadiusB),brush) : renderer->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(x,y),iRadiusA,iRadiusB),brush,iStroke);
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

void Direct2D::SetAntialiasing(ID2D1RenderTarget* renderer,bool iAntialiasing)
{
	renderer->SetAntialiasMode(iAntialiasing ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE : D2D1_ANTIALIAS_MODE_ALIASED);
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

Renderer2DWin32::Renderer2DWin32(Frame* iFrame,HWND handle):Renderer2D(iFrame),framewin32((FrameWin32*)iFrame),brush(0),renderer(0)
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

	result=this->renderer->CreateSolidColorBrush(D2D1::ColorF(Frame::COLOR_BACK),&brush);

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

	this->renderer->PushAxisAlignedClip(D2D1::RectF(left,top,right,bottom),D2D1_ANTIALIAS_MODE_ALIASED);

	Direct2D::DrawText(this,iFont,iColor,iText,tLeft,tTop,tLeft + tTextSize.x,tTop + tTextSize.y);

	this->renderer->PopAxisAlignedClip();
}

void Renderer2DWin32::DrawLine(vec2 p1,vec2 p2,unsigned int iColor,float iWidth,float iOpacity)
{
	Direct2D::DrawLine(this->renderer,this->SetColorWin32(iColor),p1,p2,iWidth,iOpacity);
}

void Renderer2DWin32::DrawRectangle(float x,float y, float z,float w,unsigned int iColor,float iStroke,float op)
{
	int ix=x;
	int iy=y;
	int iz=z;
	int iw=w;

	Direct2D::DrawRectangle(this->renderer,this->SetColorWin32(iColor),ix,iy,iz,iw,iStroke,op);
}

void Renderer2DWin32::DrawRoundRectangle(float x,float y, float w,float h,float iRadiusA,float iRadiusB,unsigned iColor,float iStroke,float iOpacity)
{
	Direct2D::DrawRoundRectangle(this->renderer,this->SetColorWin32(iColor),x,y,w,h,iRadiusA,iRadiusB,iStroke,iOpacity);
}
void Renderer2DWin32::DrawCircle(float x,float y,float iRadius,unsigned iColor,float iStroke,float iOpacity)
{
	Direct2D::DrawCircle(this->renderer,this->SetColorWin32(iColor),x,y,iRadius,iStroke,iOpacity);
}
void Renderer2DWin32::DrawEllipse(float x,float y,float iRadiusA,float iRadiusB,unsigned iColor,float iStroke,float iOpacity)
{
	Direct2D::DrawEllipse(this->renderer,this->SetColorWin32(iColor),x,y,iRadiusA,iRadiusB,iStroke,iOpacity);
}

void Renderer2DWin32::DrawBitmap(Picture* iBitmap,float iX,float iY, float iW,float iH)
{
	Direct2D::DrawBitmap(this->renderer,(ID2D1Bitmap*)iBitmap->handle,iX,iY,iW,iH);
}

unsigned int Renderer2DWin32::ReadPixel(float x,float y)
{
	COLORREF		tcolorref=::GetPixel(this->framewin32->windowDataWin32->hdc,x,y);
	unsigned int	trgb=0;
	unsigned char*	t=(unsigned char*)&trgb;

	t[0]=GetRValue(tcolorref);
	t[1]=GetGValue(tcolorref);
	t[2]=GetBValue(tcolorref);

	return trgb;
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

void Renderer2DWin32::PushScissor(float x,float y, float z,float w)
{
	int ix=x;
	int iy=y;
	int iz=z;
	int iw=w;

	Direct2D::PushScissor(this->renderer,ix,iy,iz,iw);
}

void Renderer2DWin32::PopScissor()
{
	Direct2D::PopScissor(this->renderer);
}

void Renderer2DWin32::Translate(float x,float y)
{
	int ix=x;
	int iy=y;

	Direct2D::Translate(this->renderer,ix,iy);
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

void Renderer2DWin32::SetAntialiasing(bool iAntialiasing)
{
	Direct2D::SetAntialiasing(this->renderer,iAntialiasing);
}

/////////////////MenuInterface/////////////////

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


				MainFrame* tMainFrame=MainFrame::Instance();
				HWND hwnd=(HWND)tMainFrame->frame->windowData->GetWindowHandle();

				::InsertMenuItem(tMenu,tFirstLevelQuestionMark ? menuIdx-1 : menuIdx,true,&mitem);
				::DrawMenuBar(hwnd);
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

MainFrame::MainFrame():frame(new FrameWin32(0,0,1024,768,0,false))
{
	this->frames.push_back(frame);
}

void MainFrame::Initialize()
{
	HWND hwnd=(HWND)this->frame->windowData->GetWindowHandle();

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

	::SetMenu(hwnd,____MainMenu);

	this->MenuFile=this->Menu(L"File",true);
	this->MenuActionExit=this->Menu(L"File\\Exit",false);
	this->MenuBuild=this->Menu(L"Build",true);
	this->MenuActionBuildPC=this->Menu(L"Build\\PC",false);
	this->MenuPlugins=this->Menu(L"Plugins",true);
	this->MenuActionConfigurePlugin=this->Menu(L"Plugins\\Configure",false);
	this->MenuInfo=this->Menu(L"?",true);
	this->MenuActionProgramInfo=this->Menu(L"?\\Info",false);

	vec2 tdim=this->frame->Size();

	float tx1=tdim.x/3.0f;
	float ty1=tdim.y/3.0f;

	GuiViewer* tViewer=this->GetFrame()->GetViewers().front();

	tViewer->edges.make(0,0,tx1,ty1);
	tViewer->AddTab(GuiViewport::Instance(),L"Renderer");

	GuiCaret::Instance();

	this->GetFrame()->AddViewer(new GuiViewer(0,ty1+4,tx1,ty1*2))->AddTab(GuiLogger::GetPool().front(),L"Logger");
	this->GetFrame()->AddViewer(new GuiViewer(0,ty1*2+4,tx1,tdim.y))->AddTab(GuiProject::Instance(),L"Project");
	this->GetFrame()->AddViewer(new GuiViewer(tx1+4,0,tdim.x,ty1*2))->AddTab(GuiScene::Instance(),L"Scene");
	this->GetFrame()->AddViewer(new GuiViewer(tx1+4,ty1*2+4,tdim.x,tdim.y))->AddTab(GuiEntity::Instance(),L"Properties");

	ShowWindow(hwnd,true);
}

void MainFrame::Deintialize()
{

}

Frame* MainFrame::CreateFrame(float x,float y,float z,float w,Frame* iParentFrame,bool iModal)
{
	FrameWin32* tFrameWin32=new FrameWin32(x,y,z,w,(FrameWin32*)iParentFrame,iModal);

	this->AddFrame(tFrameWin32);

	return tFrameWin32; 
}

//////////////////TimerWin32///////////////////

TimerWin32::TimerWin32()
{
	GlobalTimerInstance()=this;
}

Timer* Timer::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalTimerInstance,TimerWin32);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
/////////////WindowDataWin32///////////////////
///////////////////////////////////////////////
///////////////////////////////////////////////
WindowDataWin32::WindowDataWin32():
	hwnd(0),
	hdc(0)
{}

//#pragma message(LOCATION " LNK1123: Failure during conversion to COFF: file invalid or corrupt\" was resolved renaming cvtres.exe to cvtres1.exe.")

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
	
	::GetClientRect(this->hwnd,&tRect);

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

int WindowDataWin32::GetWindowHandle()
{
	return (int)this->hwnd;
}

void WindowDataWin32::SendMessage(unsigned iCode,unsigned data1,unsigned data2)
{
	::SendMessage(this->hwnd,iCode,data1,data2);
}

void WindowDataWin32::PostMessage(unsigned iCode,unsigned data1,unsigned data2)
{
	::PostMessage(this->hwnd,iCode,data1,data2);
}

///////////////////IdeWin32////////////////////

Ide* Ide::Instance()
{
	GLOBALGETTERFUNCASSINGLETON(GlobalIdeInstance,IdeWin32);
}


IdeWin32::IdeWin32():
processThreadHandle(0),
processHandle(0),
projectDirHasChanged(false)
{
	GlobalIdeInstance()=this;

	HRESULT result;

	result=CoInitialize(0);

	if(S_OK!=result)
		DEBUG_BREAK();

	Direct2D::Init();

	GuiFont::CreateFont(L"Verdana",10);
	GuiFont::CreateFont(L"Verdana",12);
	GuiFont::CreateFont(L"Verdana",14);
	GuiFont::CreateFont(L"Verdana",16);

	GuiLogger::Instance();

	GuiLogger::Log(L"Direct2D Initialized\n");


	if(GuiRectMessages::MAXMESSAGES>=31)
		GuiLogger::Log(L"GuiRectMessages are too much\n");
	
	//process stuff
	{
		this->processHandle=::GetCurrentProcess();
		this->processId=(unsigned int)::GetProcessId(this->processHandle);
		this->processThreadHandle=::GetCurrentThread();
		this->processThreadId=(unsigned int)::GetCurrentThreadId();

		GuiLogger::Log(StringUtils::Format(L"Engine: process id is %d\n",this->processId));
		GuiLogger::Log(StringUtils::Format(L"Engine: process main thread id is %d\n",this->processThreadId));
	}

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
				GuiLogger::Log(StringUtils::Format(L"User project folder: %s\n",this->folderProject.c_str()));
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

		if(!::SetDllDirectory(pathExecutablePath.c_str()))
			GuiLogger::Log(L"failed to add dll search path\n");

		GuiLogger::Log(StringUtils::Format(L"Application folder: %s\n",pathExecutablePath.c_str()));
	}

	{//applicationDataFolder
		wchar_t ch[5000];
		if(S_OK!=SHGetFolderPath(0,CSIDL_APPDATA,0,SHGFP_TYPE_CURRENT,ch))
			DEBUG_BREAK();

		this->folderAppData=String(ch) + L"\\EngineAppData";

		if(!::PathFileExists(this->folderAppData.c_str()))
		{
			SECURITY_ATTRIBUTES sa;
			::CreateDirectory(this->folderAppData.c_str(),&sa);
		}
		else
		{
			Subsystem::Execute(this->folderAppData,L"del /F /Q *.*");
			Subsystem::Execute(this->folderAppData,L"FOR /D %p IN (*.*) DO rmdir \"%p\" /s /q");
		}

		GuiLogger::Log(StringUtils::Format(L"Application data folder: %s\n",this->folderAppData.c_str()));
	}

	int error=ERROR_SUCCESS;


	{
		this->timerThread=new ThreadWin32;
		this->timerThread->NewTask(L"MainTimerTask",std::function<void()>(std::bind(&Timer::update,Timer::Instance())),false);
	}

	

	MainFrame::Instance()->Initialize();


	this->projectDirChangedThread=new ThreadWin32;
	this->projectDirChangedThread->NewTask(L"ProjectDirChangedTask",std::function<void()>(std::bind(&Ide::ScanProjectDirectoryForFileChanges,this)),false);
}

IdeWin32::~IdeWin32()
{
	FindCloseChangeNotification(this->projectDirChangeHandle);

	SetDllDirectory(0);
	Direct2D::Release();
	CoUninitialize();

	GlobalIdeInstance()=0;
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

	int tEngineExtensionCharSize=Ide::Instance()->GetEntityExtension().size();

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

		if(wcsstr(tData.cFileName,Ide::Instance()->GetEntityExtension().c_str()))
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
			String engineFile = iDirectory + L"\\" + String(tData.cFileName) + Ide::Instance()->GetEntityExtension();

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
				dirNode->isDir=tIsDir;

				ResourceNodeDir* dirNodeParent=(ResourceNodeDir*)dirNode->parent;

				dirNode->dirLabel.SetLabel(dirNode->fileName);
				if(dirNodeParent)
					dirNodeParent->dirLabel.Insert(dirNode->dirLabel);

				dirNode->fileLabel.SetLabel(dirNode->fileName);
				if(dirNodeParent)
					dirNodeParent->fileLabels.push_back(&dirNode->fileLabel);


				this->ScanDir(iDirectory + L"\\"+ tCreateNodeFilename,dirNode);
			}
			else
			{
				ResourceNode* fileNode=new ResourceNode;

				iParent->files.push_back(fileNode);
				fileNode->parent=iParent;
				fileNode->fileName=tCreateNodeFilename;
				fileNode->isDir=tIsDir;

				ResourceNodeDir* dirNodeParent=(ResourceNodeDir*)fileNode->parent;

				fileNode->fileLabel.SetLabel(fileNode->fileName);

				if(dirNodeParent)
					dirNodeParent->fileLabels.push_back(&fileNode->fileLabel);
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
		//if(PeekMessage(&msg,0,0,0,PM_REMOVE)!=WM_QUIT)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if(this->projectDirHasChanged)
		{
			this->projectDirHasChanged=false;

			Frame* tFrame=GuiProject::Instance()->GetRoot()->GetFrame();

			tFrame->DrawBlock(true);

			tFrame->Message(GuiProject::Instance(),ONDEACTIVATE);
			tFrame->Message(GuiProject::Instance(),ONACTIVATE);

			tFrame->DrawBlock(false);

			tFrame->SetDraw();
		}
	}
}

void IdeWin32::Sleep(int iMilliseconds)
{
	::Sleep(iMilliseconds);
}

void IdeWin32::ScanProjectDirectoryForFileChanges()
{
	this->projectDirChangeHandle=FindFirstChangeNotification(this->folderProject.c_str(),true,
		FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_DIR_NAME|FILE_NOTIFY_CHANGE_ATTRIBUTES|FILE_NOTIFY_CHANGE_SIZE|FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_SECURITY);
	DWORD tProjectDirectoryIsChanged=WaitForSingleObject(this->projectDirChangeHandle,INFINITE); 

	this->projectDirHasChanged = WAIT_OBJECT_0==tProjectDirectoryIsChanged ? true : false;

	FindCloseChangeNotification(this->projectDirChangeHandle);
}

///////////////////TimerWin32//////////////////

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




DirectXRenderer::DirectXRenderer(Frame* iFrame):
	Renderer3D(iFrame)
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

		GuiLogger::Log(StringUtils::Format(L"OPENGL ERROR %d, HGLRC: %p, HDC: %p\n",err,currentContext,currentContextDC));

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
		GuiLogger::Log(StringUtils::Format(L"glCreateShader error for %s,%s\n",shader_type,shader_src));glCheckError();
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
		Subsystem::SystemMessage(StringUtils::ToWide(message),L"Engine",MB_OK|MB_ICONEXCLAMATION);
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
		GuiLogger::Log(StringUtils::Format(L"glCreateProgram error for %s,%s\n",vertexsh,fragmentsh));
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
		GuiLogger::Log(StringUtils::Format(L"glLinkProgram error for %s\n",tMessage));
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
		GuiLogger::Log(StringUtils::Format(L"error creating shader %s\n",name));

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


Renderer3DOpenGL::Renderer3DOpenGL(FrameWin32* iFrame):
	Renderer3D(iFrame),
	frameWin32((FrameWin32*)iFrame),
	hglrc(0),
	hdc(0)
{
	
}

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
	ReleaseDC(this->hwnd,this->hdc);
}

void Renderer3DOpenGL::Initialize()
{
#if RENDERER_FRAMED
	this->hwnd=this->frameWin32->windowDataWin32->hwnd;
	this->hdc=this->frameWin32->windowDataWin32->hdc;
#else
	vec2 tFrameSize=this->frame->Size();
	this->hwnd=CreateWindow(WC_DIALOG,L"Renderer3DOpenGL",WS_VISIBLE,0,0,tFrameSize.x,tFrameSize.y,0,0,0,0);

	if(!this->hwnd)
		Subsystem::SystemMessage(L"Creating Renderer window",L"Renderer3DOpenGL::CreateWindow",MB_OK|MB_ICONEXCLAMATION);

	this->hdc=GetDC(this->hwnd);
#endif

	DWORD error=0;

	if(!this->hdc)
		Subsystem::SystemMessage(L"Renderer3DOpenGL::Initialize",L"Creating Device Context",MB_OK|MB_ICONEXCLAMATION);

	PIXELFORMATDESCRIPTOR pfd={0};
	pfd.nVersion=1;
	pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
	pfd.dwFlags=PFD_DOUBLEBUFFER|PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL;
	pfd.iPixelType=PFD_TYPE_RGBA;
	pfd.cColorBits=32;
	pfd.cDepthBits=32;
	pfd.cStencilBits=32;

	pixelFormat = ChoosePixelFormat(this->hdc,&pfd);

	error=GetLastError();

	//ERROR_PROC_NOT_FOUND	127 (0x7F)

	if(error!=NO_ERROR && error!=ERROR_OLD_WIN_VERSION)
		DEBUG_BREAK();

	if(pixelFormat==0)
		DEBUG_BREAK();

	if(!SetPixelFormat(this->hdc,pixelFormat,&pfd))
		DEBUG_BREAK();

	if(!(this->hglrc = wglCreateContext(this->hdc)))
		DEBUG_BREAK();

	if(!wglMakeCurrent(this->hdc,this->hglrc))
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
	wglDeleteContext(this->hglrc);

	ReleaseDC(this->hwnd,this->hdc);
	
	const int pixelFormatAttribList[] =
	{
		WGL_DRAW_TO_BITMAP_ARB, GL_TRUE,
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

	this->hdc=GetDC(this->hwnd);

	if(!this->hdc)
		Subsystem::SystemMessage(L"Renderer3DOpenGL::Initialize",L"Recreating Device Context",MB_OK|MB_ICONEXCLAMATION);

	UINT numFormats;

	if(!wglChoosePixelFormatARB(this->hdc, pixelFormatAttribList, NULL, 1, &pixelFormat, &numFormats))
		Subsystem::SystemMessage(L"wglChoosePixelFormatARB fails",L"Engine",MB_OK|MB_ICONEXCLAMATION);


	const int versionAttribList[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB,1,
		WGL_CONTEXT_MINOR_VERSION_ARB,0,
		0,        //End
	};

	if(!(this->hglrc = wglCreateContextAttribsARB(this->hdc, 0, versionAttribList)))
		Subsystem::SystemMessage(L"wglCreateContextAttribsARB fails",L"Engine",MB_OK|MB_ICONEXCLAMATION);

	if(this->hglrc)
		GuiLogger::Log(StringUtils::Format(L"HWND: %p, HGLRC: %p, HDC: %p\n",this->hwnd,this->hglrc,this->hdc));

	if(!wglMakeCurrent(this->hdc,this->hglrc))
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

	GuiLogger::Log(StringUtils::Format(L"Status: Using GL %s\n", StringUtils::ToWide((const char*)glGetString(GL_VERSION)).c_str()));
	GuiLogger::Log(StringUtils::Format(L"Status: GLSL ver %s\n",StringUtils::ToWide((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)).c_str()));

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
	this->DrawLine(vec3(0,0,0),vec3(10,0,0),vec3(1,0,0));
	this->DrawLine(vec3(0,0,0),vec3(0,10,0),vec3(0,1,0));
	this->DrawLine(vec3(0,0,0),vec3(0,0,10),vec3(0,0,1));
}



void Renderer3DOpenGL::DrawPoint(vec3 point,float psize,vec3 col)
{
	Shader* shader=this->shader_unlit_color;

	if(!shader)
		return;

	shader->Use();

	////shader->SetMatrices(MatrixStack::GetProjectionMatrix(),MatrixStack::GetModelviewMatrix());

	shader->SetSelectionColor(this->picking,0,this->frame->mouse,this->frame->Size());

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

	shader->SetSelectionColor(false,0,this->frame->mouse,this->frame->Size());

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

void Renderer3DOpenGL::DrawLine(vec3 a,vec3 b,vec3 color)
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

	shader->SetSelectionColor(false,0,this->frame->mouse,this->frame->Size());

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

void Renderer3DOpenGL::DrawText(char* text,float x,float y,float width,float height,float sizex,float sizey,float* color4)
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
		this->DrawPoint(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(mesh->isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CCW);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),mesh->Entity()->world);


	shader->SetSelectionColor(this->picking,mesh->Entity(),this->frame->mouse,this->frame->Size());

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
		this->DrawPoint(lightpos,5);

	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glFrontFace(skin->isCCW ? GL_CCW : GL_CW);
	//glFrontFace(GL_CW);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),skin->Entity()->local);


	shader->SetSelectionColor(this->picking,skin->Entity(),this->frame->mouse,this->frame->Size());

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

	vec3 a=bone->Entity()->Parent()->world.position();
	vec3 b=bone->Entity()->world.position();

	float line[]=
	{
		a[0],a[1],a[2],
		b[0],b[1],b[2],
	};

	shader->Use();

	shader->SetMatrices(MatrixStack::GetViewMatrix()*MatrixStack::GetProjectionMatrix(),mat4());

	shader->SetSelectionColor(this->picking,bone->Entity(),this->frame->mouse,this->frame->Size());

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
	iEntity->render(this);
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


LRESULT CALLBACK FrameWin32::FrameWin32Procedure(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
#if PRINTPROCEDUREMESSAGESCALL
	wprintf(L"Begin %i:%s",msg,msg<sizeof(gWM_MESSAGES) ? gWM_MESSAGES[msg] : L"unmapped");
	wprintf(L"%s\n",(msg==WM_PAINT || msg==WM_NULL) ? L"11111111111111111111111111111111111111" : L"");
#endif
	FrameWin32* frame=(FrameWin32*)GetWindowLongPtr(hwnd,GWLP_USERDATA);

	LPARAM result=0;

	switch(msg)
	{
		case 0:
		{
			if(lparam)
				GuiCaret::Instance()->Draw(frame,wparam);
			else if(!wparam)
				frame->Draw();
			else if(frame->BeginDraw((GuiViewport*)wparam))
			{
				frame->BroadcastPaintTo((GuiViewport*)wparam);
				frame->EndDraw();
			}	

			result=0;
		}
		break;
		case WM_ERASEBKGND:
			/*An application should return nonzero in response to WM_ERASEBKGND if it processes the message and erases the background;
			this indicates that no further erasing is required. If the application returns zero, the window will remain marked for erasing.*/ 
			//wprintf(L"erasing background\n");
			result=1;
		break;
		case WM_NCLBUTTONDOWN:
		{
			unsigned tSizeboxCode=0;

			switch(wparam)
			{
				case HTBOTTOM:		tSizeboxCode=1;break;
				case HTBOTTOMLEFT:	tSizeboxCode=2;break;
				case HTBOTTOMRIGHT:	tSizeboxCode=3;break;
				case HTTOP:			tSizeboxCode=4;break;
				case HTTOPLEFT:		tSizeboxCode=5;break;
				case HTTOPRIGHT:	tSizeboxCode=6;break;
				case HTLEFT:		tSizeboxCode=7;break;
				case HTRIGHT:		tSizeboxCode=8;break;
				default:
					tSizeboxCode=0;
			}

			if(tSizeboxCode)
				frame->OnSizeboxDown(tSizeboxCode);

			result=DefWindowProc(hwnd,msg,wparam,lparam);
		}
		break;
		case WM_NCLBUTTONUP:
		{
			frame->OnSizeboxUp();
			result=DefWindowProc(hwnd,msg,wparam,lparam);
		}
		break;
		case WM_SIZE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			float tWidth=LOWORD(lparam);
			float tHeight=HIWORD(lparam);

			if(frame->wasdrawing)
				DEBUG_BREAK();

			HRESULT result=frame->renderer2DWin32->renderer->Resize(D2D1::SizeU(tWidth,tHeight));

			if(S_OK!=result)
				DEBUG_BREAK();
			
			frame->OnSize(tWidth,tHeight);
		}
		break;
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			if(!frame->skipFrameMouseExit)
			{
				unsigned int tIndex=msg==WM_LBUTTONDOWN ? 0 : (msg==WM_RBUTTONDOWN ? 2 : 1);

				frame->hittest.locked=true;

				::SetCapture(frame->windowDataWin32->hwnd);

				Mouse& tMi=*Mouse::Instance();

				tMi.RawButtons()[tIndex]=true;

				if(!tIndex && frame->windowDataWin32->hwnd!=::GetFocus())
					::SetFocus(frame->windowDataWin32->hwnd);

				unsigned int tOldTime=tMi.RawTiming()[tIndex];
				tMi.RawTiming()[tIndex]=Timer::Instance()->GetCurrent();

				unsigned int tFinalButtonIndex=tIndex+1;

				if(tMi.RawTiming()[tIndex]-tOldTime<1000/6.0f)
					frame->OnMouseClick(tFinalButtonIndex);
				else
					frame->OnMouseDown(tFinalButtonIndex);
			}
		}
		break;
		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
		case WM_RBUTTONUP:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			if(!frame->skipFrameMouseExit)
			{
				unsigned int tIndex=msg==WM_LBUTTONUP ? 0 : (msg==WM_RBUTTONUP ? 2 : 1);
				Mouse& tMi=*Mouse::Instance();
				tMi.RawButtons()[tIndex]=false;

				frame->hittest.locked=false;

				::SetCapture(0);

				frame->OnMouseUp(tIndex+1);
			}
		}
		break;
		case WM_MOUSEWHEEL:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			float wheelValue=GET_WHEEL_DELTA_WPARAM(wparam)>0 ? 1.0f : (GET_WHEEL_DELTA_WPARAM(wparam)<0 ? -1.0f : 0);
			frame->OnMouseWheel(wheelValue);
		}
		break;
		case WM_MOUSELEAVE:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			if(!frame->skipFrameMouseExit)
				frame->OnMouseMove(-100,-100);

			frame->trackmouseleave=true;
			result=0;
		}
		break;
		case WM_MOUSEMOVE:
			result=DefWindowProc(hwnd,msg,wparam,lparam);

			if(frame->trackmouseleave)
			{
				TRACKMOUSEEVENT tTrackMouseEvent={sizeof(TRACKMOUSEEVENT),TME_LEAVE,hwnd,HOVER_DEFAULT};

				if(!TrackMouseEvent(&tTrackMouseEvent))
					DEBUG_BREAK();

				frame->trackmouseleave=false;
			}
			
			frame->OnMouseMove((float)LOWORD(lparam),(float)HIWORD(lparam));
		break;
		case WM_KEYDOWN:
		{
			result=DefWindowProc(hwnd,msg,wparam,lparam);
			MSG _msg;
			PeekMessage(&_msg, NULL, 0, 0, PM_NOREMOVE);

			if(_msg.message==WM_CHAR)
			{
				TranslateMessage(&_msg);
				DefWindowProc(_msg.hwnd,_msg.message,_msg.wParam,_msg.lParam);
				frame->OnKeyDown(_msg.wParam);
			}
			else
				frame->OnKeyDown(0);
		}
		break;
		case WM_PAINT:
		{
			frame->SetDraw();
			PAINTSTRUCT ps;
			BeginPaint(hwnd,&ps);
			EndPaint(hwnd,&ps);
			result=0;
		}
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
					tMenuInterface->OnMenuPressed(frame,mii.wID);
			}
		}
		break;
		default:
			result=DefWindowProc(hwnd,msg,wparam,lparam);
	}

	/*if(frame)
	{
		//tabWin32->evtQueue.push_back(std::function<void()>(std::bind(&WindowDataWin32::CopyProcedureData,tabWin32->windowDataWin32,hwnd,msg,wparam,lparam)));
		frame->windowDataWin32->msg=0;
		frame->windowDataWin32->wparam;
		frame->windowDataWin32->lparam=0;
	}

	if(frame && frame->concurrentInstances.size())
	{
		for(std::list< std::function<void()> >::iterator iterConcurrentInstances=frame->concurrentInstances.begin();iterConcurrentInstances!=frame->concurrentInstances.end();)
		{
			(*iterConcurrentInstances)();
			iterConcurrentInstances=frame->concurrentInstances.erase(iterConcurrentInstances);
		}
	}

	if(Timer::Instance()->GetLastDelta()>16)
		PostMessage(hwnd,WM_NULL,0,0);*/
#if PRINTPROCEDUREMESSAGESCALL
	wprintf(L"End   %i:%s",msg,msg<sizeof(gWM_MESSAGES) ? gWM_MESSAGES[msg] : L"unmapped");
	wprintf(L"%s\n",(msg==WM_PAINT || msg==WM_NULL) ? L"00000000000000000000000000000000000000" : L"");
#endif

	return result;
}

FrameWin32::FrameWin32(float iX,float iY,float iW,float iH,FrameWin32* iParentFrame,bool iModal):
	Frame(iX,iY,iW,iH),
	windowDataWin32((WindowDataWin32*&)windowData),
	renderer2DWin32((Renderer2DWin32*&)renderer2D),
	renderer3DOpenGL((Renderer3DOpenGL*&)renderer3D),
	threadRenderWin32((ThreadWin32*&)thread)
{
	this->isModal=iModal;

	windowDataWin32=new WindowDataWin32;

	DWORD tStyle=WS_CLIPCHILDREN|WS_CLIPSIBLINGS;

	if(!iModal)
		tStyle = iParentFrame ? tStyle|WS_CHILD : tStyle|WS_OVERLAPPEDWINDOW;
	else
		tStyle |= WS_SIZEBOX;

	HWND tParent=iParentFrame ? iParentFrame->windowDataWin32->hwnd : 0;

	this->windowDataWin32->hwnd=::CreateWindow(WC_DIALOG,0,tStyle,(int)iX,(int)iY,(int)iW,(int)iH,tParent,0,0,0);

	if(!this->windowDataWin32->hwnd)
	{
		DWORD tError=::GetLastError();
		DEBUG_BREAK();
	}

	this->windowDataWin32->hdc=::GetDC(this->windowDataWin32->hwnd);

	this->iconDown=new PictureRef(Frame::rawDownArrow,Frame::ICON_WH,Frame::ICON_WH);
	this->iconUp=new PictureRef(Frame::rawUpArrow,Frame::ICON_WH,Frame::ICON_WH);
	this->iconLeft=new PictureRef(Frame::rawLeftArrow,Frame::ICON_WH,Frame::ICON_WH);
	this->iconRight=new PictureRef(Frame::rawRightArrow,Frame::ICON_WH,Frame::ICON_WH);
	this->iconFolder=new PictureRef(Frame::rawFolder,Frame::ICON_WH,Frame::ICON_WH);
	this->iconFile=new PictureRef(Frame::rawFile,Frame::ICON_WH,Frame::ICON_WH);

	this->renderer2D=this->renderer2DWin32=new Renderer2DWin32(this,this->windowDataWin32->hwnd);

	this->thread=new ThreadWin32;

	if(!MainFrame::IsInstanced())
	{
		GuiLogger::Log(StringUtils::Format(L"MainFrame: %p , HWND: %p",this,this->windowDataWin32->hwnd));

#if RENDERER_ENABLED
		this->renderer3D=this->renderer3DOpenGL=new Renderer3DOpenGL(this);
		if(!this->renderer3DOpenGL)
			DEBUG_BREAK();
#if RENDERER_THREADED
		Task* tCreateOpenGLContext=this->thread->NewTask(L"CreateOpenglContextTask",std::function<void()>(std::bind(&Renderer3DOpenGL::Initialize,this->renderer3DOpenGL)));
		while(tCreateOpenGLContext->func);
		SAFEDELETE(tCreateOpenGLContext);
#else
		this->renderer3DOpenGL->Initialize();
#endif //RENDERER_THREADED
#endif //ENABLE_RENDERER
	}

	this->renderingTask=this->thread->NewTask(L"TabDrawTask",std::function<void()>(std::bind(&Frame::Render,this)),false);

	this->AddViewer(GuiViewer::Instance());

	if(iModal) /*removes the caption style*/
		::SetWindowLong(this->windowDataWin32->hwnd,GWL_STYLE,::GetWindowLong(this->windowDataWin32->hwnd, GWL_STYLE) & ~(WS_CAPTION));

	::SetWindowLongPtr(this->windowDataWin32->hwnd,GWLP_USERDATA,(LONG_PTR)this);
	::SetWindowLongPtr(this->windowDataWin32->hwnd,GWLP_WNDPROC,(LONG_PTR)FrameWin32::FrameWin32Procedure);

	this->OnRecreateTarget();

	if(iModal)/*visually remove the caption*/
		::SetWindowPos(this->windowDataWin32->hwnd,0,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_FRAMECHANGED|SWP_DRAWFRAME);

	ShowWindow(this->windowDataWin32->hwnd,true);
}

FrameWin32::~FrameWin32()
{
	DestroyWindow(this->windowDataWin32->hwnd);

	SetWindowLongPtr(this->windowDataWin32->hwnd,GWLP_USERDATA,(LONG_PTR)0);

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

#if RENDERER_ENABLED
	if(this->renderer3D)
	{

#if RENDERER_THREADED
		Task* tDeleteOpenGLContext=this->thread->NewTask(L"DestroyOpenglContextTask",std::function<void()>(std::bind(&Renderer3DOpenGL::Deinitialize,this->renderer3DOpenGL)));
		while(tDeleteOpenGLContext->func);
		SAFEDELETE(tDeleteOpenGLContext);
#endif

		SAFEDELETE(this->renderer3D);

		this->threadRenderWin32->DestroyTask(this->renderingTask);
	}
#endif

	SAFEDELETE(this->threadRenderWin32);
	SAFEDELETE(this->renderer2DWin32);
	SAFEDELETE(this->windowDataWin32);
}

void FrameWin32::Destroy()
{

}

int FrameWin32::TrackTabMenuPopup()
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

	POINT tCursorPoint;
	::GetCursorPos(&tCursorPoint);

	int menuResult=Subsystem::TrackMenu((int)root,this,tCursorPoint.x,tCursorPoint.y);

	DestroyMenu(create);
	DestroyMenu(root);

	return menuResult;
}

int FrameWin32::TrackGuiSceneViewerPopup(bool iSelected)
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

	POINT tCursorPoint;
	::GetCursorPos(&tCursorPoint);

	int menuResult=Subsystem::TrackMenu((int)menu,this,tCursorPoint.x,tCursorPoint.y);

	DestroyMenu(menu);
	DestroyMenu(createEntity);
	DestroyMenu(createComponent);
	DestroyMenu(createMesh);
	DestroyMenu(createScript);

	return menuResult;
}

namespace FileViewerActions
{
	const unsigned char Delete=1;
	const unsigned char Create=2;
	const unsigned char Load=3;
}

int FrameWin32::TrackProjectFileViewerPopup(ResourceNode* iResourceNode)
{
	HMENU menu=CreatePopupMenu();

	if(iResourceNode)
	{
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,FileViewerActions::Delete,L"Delete");

		if(iResourceNode->fileName.PointedExtension() == Ide::Instance()->GetSceneExtension())
			InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,FileViewerActions::Load,L"Load");
	}
	else
	{
		InsertMenu(menu,0,MF_BYPOSITION|MF_STRING,FileViewerActions::Create,L"Create");
	}

	POINT tCursorPoint;
	::GetCursorPos(&tCursorPoint);

	int menuResult=Subsystem::TrackMenu((int)menu,this,tCursorPoint.x,tCursorPoint.y);

	DestroyMenu(menu);

	return menuResult;
}

bool FrameWin32::BeginDraw(void* iPtr)
{
	if(!this->wasdrawing)
	{
		this->wasdrawing=iPtr;

		if(this->retarget)
		{
			this->OnRecreateTarget();
			this->retarget=0;
		}

		this->renderer2DWin32->renderer->BeginDraw();
		
		return true;
	}
	else
		DEBUG_BREAK();

	return false;

}
void FrameWin32::EndDraw()
{
	if(this->wasdrawing)
	{
		HRESULT result=renderer2DWin32->renderer->EndDraw();

		this->retarget=(result==D2DERR_RECREATE_TARGET);

		if(result!=0)
		{
			DEBUG_BREAK();

			wprintf(L"D2D1HwndRenderTarget::EndDraw error: %x\n",result);

			result=renderer2DWin32->renderer->Flush();

			if(result!=0)
				wprintf(L"D2D1HwndRenderTarget::Flush error: %x\n",result);
		}

		this->wasdrawing=0;
	}
	else
		DEBUG_BREAK();

	//this->clips.clear();
}



void FrameWin32::OnRecreateTarget()
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

	this->Frame::OnRecreateTarget();

}


void FrameWin32::SetCursor(int iCursorCode)
{
	HCURSOR tCursor=0;

	switch(iCursorCode)
	{
		case 0: tCursor=LoadCursor(0,IDC_ARROW); break;
		case 1: tCursor=LoadCursor(0,IDC_SIZEWE); break;
		case 2: tCursor=LoadCursor(0,IDC_SIZENS); break;
		case 3: tCursor=LoadCursor(0,IDC_SIZENESW); break;
		case 4: tCursor=LoadCursor(0,IDC_SIZENWSE); break;
		case 5: tCursor=LoadCursor(0,IDC_SIZEALL); break;
	}

	if(tCursor)
		::SetCursor(tCursor);
}

///////////////////////////////////////////////
///////////////////////////////////////////////
////////////////GuiViewportWin32///////////////
///////////////////////////////////////////////
///////////////////////////////////////////////

GuiViewport::~GuiViewport()
{
	GuiViewport::GetPool().remove(this);

	SAFERELEASE((ID2D1Bitmap*&)this->bitmap);
}

void GuiViewport::SwapBuffer(Frame* iFrame)
{

}

Frame*	GuiViewport::GetFrame(){return this->frame;}

void GuiViewport::DrawBuffer(Frame* iFrame)
{
	if(this->buffer)
	{
		ID2D1Bitmap*&		tBitmap=(ID2D1Bitmap*&)this->bitmap;
		Renderer2DWin32*	tRenderer=(Renderer2DWin32*)iFrame->renderer2D;

		int					tx=this->edges.x;
		int					ty=this->edges.y;
		int					tz=this->edges.z;
		int					tw=this->edges.w;

		tRenderer->renderer->DrawBitmap(tBitmap,D2D1::RectF(tx,ty,tz,tw));
		//tRenderer->DrawRectangle(this->edges.x,this->edges.y,this->edges.z,this->edges.w,0xff0000,1);
	}
}

void GuiViewport::ResizeBuffers(Frame* iFrame)
{
	ID2D1Bitmap*&		tBitmap=(ID2D1Bitmap*&)this->bitmap;
	Renderer2DWin32*	tRenderer=(Renderer2DWin32*)iFrame->renderer2D;
	D2D1_SIZE_U			tBitmapSize;
	HRESULT				tDirect2DResult=S_OK;

	int					tx=this->edges.x;
	int					ty=this->edges.y;
	int					tz=this->edges.z;
	int					tw=this->edges.w;
	int					tWidth=tz-tx;
	int					tHeight=tw-ty;

	if(tBitmap)
		AGGREGATECALL(tBitmap->GetPixelSize,tBitmapSize);

	if(!tBitmap || tBitmapSize.width!=tWidth || tBitmapSize.height!=tHeight || !this->buffer)
	{
		int tBufferSize=tWidth*tHeight*4;

		SAFERELEASE(tBitmap);
		SAFEDELETEARRAY(this->buffer);

		D2D1_BITMAP_PROPERTIES bp=D2D1::BitmapProperties();
		AGGREGATECALL(tRenderer->renderer->GetPixelFormat,bp.pixelFormat);

		tDirect2DResult=tRenderer->renderer->CreateBitmap(D2D1::SizeU(tWidth,tHeight),bp,&tBitmap);

		if(tDirect2DResult!=S_OK)
			DEBUG_BREAK();

		this->buffer=new unsigned char[tBufferSize];
	}
}



int GuiViewport::Render(Frame* iFrame)
{
#if RENDERER_THREADED
	if((Timer::GetInstance()->GetCurrent()-this->lastFrameTime)>(1000.0f/this->renderFps))
		this->lastFrameTime=Timer::Instance()->GetCurrent();
	else 
		return 0;
#endif

	Renderer2DWin32*	tRenderer=(Renderer2DWin32*)iFrame->renderer2D;
	ID2D1Bitmap*&		tBitmap=(ID2D1Bitmap*&)this->bitmap;
	
	int					tx=this->edges.x;
	int					ty=this->edges.y;
	int					tz=this->edges.z;
	int					tw=this->edges.w;
	int					tWidth=tz-tx;
	int					tHeight=tw-ty;

	vec2				tMouse(iFrame->mouse);
	D2D1_RECT_U			tBitmapRect={0,0,tWidth,tHeight};
	HRESULT				tDirect2DResult=S_OK;
	EditorEntity*		tEntity=this->GetEntity();
	
	iFrame->renderer3D->ChangeContext();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_SCISSOR_TEST);

	glViewport(0,0,tWidth,tHeight);glCheckError();
	glScissor(0,0,tWidth,tHeight);glCheckError();

	{
		int tGuiRectColorBack=GuiRect::COLOR_BACK;
		char* pGuiRectColorBack=(char*)&tGuiRectColorBack;

		glClearColor(pGuiRectColorBack[2]/255.0f,pGuiRectColorBack[1]/255.0f,pGuiRectColorBack[0]/255.0f,0.0f);glCheckError();
		//glClearColor(1,0,0,0);glCheckError();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

		MatrixStack::Push(MatrixStack::PROJECTION,this->projection);
		MatrixStack::Push(MatrixStack::VIEW,this->view);
		MatrixStack::Push(MatrixStack::MODEL,this->model);

		iFrame->renderer3D->DrawLine(vec3(0,0,0),vec3(1000,0,0),vec3(1,0,0));
		iFrame->renderer3D->DrawLine(vec3(0,0,0),vec3(0,1000,0),vec3(0,1,0));
		iFrame->renderer3D->DrawLine(vec3(0,0,0),vec3(0,0,1000),vec3(0,0,1));

		if(tEntity)
			iFrame->renderer3D->RenderEntities(tEntity);

		MatrixStack::Pop(MatrixStack::MODEL);
		MatrixStack::Pop(MatrixStack::VIEW);
		MatrixStack::Pop(MatrixStack::PROJECTION);

		glReadBuffer(GL_BACK);glCheckError();

		glReadPixels(0,0,tWidth,tHeight,GL_BGRA,GL_UNSIGNED_BYTE,this->buffer);glCheckError();//@mic should implement pbo for performance

		tDirect2DResult=tBitmap->CopyFromMemory(&tBitmapRect,this->buffer,tWidth/**tHeight*/*4);

		if(tDirect2DResult!=S_OK)
			DEBUG_BREAK();

		tDirect2DResult=tRenderer->renderer->Flush();

		if(tDirect2DResult!=S_OK)
			DEBUG_BREAK();

#if RENDERER_THREADED
		iFrame->SetDraw(this);
#endif

		return 1;
	}

	/*if(this->needsPicking && tMouse.y-tCanvas.y>=0)
	{
		glDisable(GL_DITHER);

		iFrame->renderer3D->picking=true;

		glClearColor(0.0f,0.0f,0.0f,0.0f);glCheckError();
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);glCheckError();

		MatrixStack::Push(MatrixStack::PROJECTION,this->projection);
		MatrixStack::Push(MatrixStack::VIEW,this->view);
		MatrixStack::Push(MatrixStack::MODEL,this->model);

		if(this->Entity())
			iFrame->renderer3D->draw(this->Entity());

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

		iFrame->renderer3D->picking=false;

		glEnable(GL_DITHER);

		this->needsPicking=false;
	}*/
}


///////////////Subsystem//////////////////

bool Subsystem::Execute(String iPath,String iCmdLine,String iOutputFile,bool iInput,bool iError,bool iOutput,bool iNewConsole)
{
	if(iPath==L"none")
		iPath=Ide::Instance()->folderProject;

	STARTUPINFO si={0};
	PROCESS_INFORMATION pi={0};

	HANDLE tFileOutput=0;

	si.cb = sizeof(STARTUPINFO);
	si.wShowWindow = true;

	const int tOldCurrentPathSize=1024;
	wchar_t tOldCurrentPath[tOldCurrentPathSize];

	::GetCurrentDirectory(tOldCurrentPathSize,tOldCurrentPath);
	::SetCurrentDirectory(iPath.c_str());

	String tCommandLine=L"cmd.exe /V /C " + iCmdLine;

	int tOutputFileSize=iOutputFile.size();

	if(tOutputFileSize && (iInput || iError || iOutput))
	{
		SECURITY_ATTRIBUTES sa={0};

		sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		sa.lpSecurityDescriptor = 0;
		sa.bInheritHandle = !iNewConsole;

		tFileOutput = ::CreateFile(iOutputFile.c_str(),FILE_APPEND_DATA,FILE_SHARE_WRITE|FILE_SHARE_READ|FILE_SHARE_DELETE,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL/*|FILE_FLAG_DELETE_ON_CLOSE*/,0);

		if(!tFileOutput)
			DEBUG_BREAK();

		si.dwFlags |= STARTF_USESTDHANDLES;
		si.hStdInput = iInput ? tFileOutput : ::GetStdHandle(STD_INPUT_HANDLE);
		si.hStdError = iError ? tFileOutput : ::GetStdHandle(STD_ERROR_HANDLE);
		si.hStdOutput = iOutput ? tFileOutput : ::GetStdHandle(STD_OUTPUT_HANDLE);
	}

	if(!::CreateProcess(0,(wchar_t*)tCommandLine.c_str(),0,0,!iNewConsole,0,0,0,&si,&pi))
		return false;

	::WaitForSingleObject( pi.hProcess, INFINITE );

	if(!::CloseHandle( pi.hProcess ))
		DEBUG_BREAK();
	if(!::CloseHandle( pi.hThread ))
		DEBUG_BREAK();
	if(tFileOutput && !::CloseHandle( tFileOutput ))
		DEBUG_BREAK();

	::SetCurrentDirectory(tOldCurrentPath);

	return true;
}


unsigned int Subsystem::FindProcessId(String iProcessName)
{
	PROCESSENTRY32 tProcess;
	tProcess.dwSize = sizeof(PROCESSENTRY32);

	HANDLE tSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if(::Process32First(tSnapshot, &tProcess)==TRUE)
	{
		while(::Process32Next(tSnapshot, &tProcess) == TRUE)
		{
			if(iProcessName==tProcess.szExeFile)
				return tProcess.th32ProcessID;
		}
	}

	return 0;
}

unsigned int Subsystem::FindThreadId(unsigned int iProcessId,String iThreadName)
{
	return 0;
}

String Subsystem::DirectoryChooser(String iDescription,String iExtension)
{
	wchar_t _pszDisplayName[MAX_PATH]=L"";

	BROWSEINFO bi={0};
	bi.hwndOwner=(HWND)MainFrame::Instance()->GetFrame()->windowData->GetWindowHandle();
	bi.pszDisplayName=_pszDisplayName;
	bi.lpszTitle=L"Select Directory";

	PIDLIST_ABSOLUTE tmpFolder=::SHBrowseForFolder(&bi);

	DWORD err=::GetLastError();

	if(tmpFolder)
	{
		wchar_t path[MAX_PATH];

		if(::SHGetPathFromIDList(tmpFolder,path))
		{
			return path;
		}
	}

	return L"";
}

String Subsystem::FileChooser(wchar_t* iFilter,unsigned int iFilterIndex)
{
	wchar_t tOutputBuffer[5000]={0};
	
	OPENFILENAME openfilename={0};
	openfilename.lStructSize=sizeof(OPENFILENAME);
	openfilename.hwndOwner=(HWND)MainFrame::Instance()->frame->windowData->GetWindowHandle();
	openfilename.lpstrFilter=iFilter;
	openfilename.nFilterIndex=iFilterIndex;
	openfilename.lpstrFile=tOutputBuffer;
	openfilename.nMaxFile=5000;

	GetOpenFileName(&openfilename);

	DWORD tError=::GetLastError();

	return openfilename.lpstrFile;
}

std::vector<String> Subsystem::ListDirectories(String iDir)
{
	std::vector<String> tResult;

	HANDLE			tHandle;
	WIN32_FIND_DATA tData;

	String tScanDir=iDir + L"\\*";

	tHandle=::FindFirstFile(tScanDir.c_str(),&tData); //. dir

	if(!tHandle || INVALID_HANDLE_VALUE == tHandle)
		tResult;
	else
		::FindNextFile(tHandle,&tData);

	while(::FindNextFile(tHandle,&tData))
	{
		if(!(tData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		tResult.push_back(tData.cFileName);
	}

	::FindClose(tHandle);
	tHandle=0;

	return tResult;
}

bool Subsystem::CreateDir(String iDir)
{
	SECURITY_ATTRIBUTES sa={sizeof(SECURITY_ATTRIBUTES),0,false};

	return ::CreateDirectory(iDir.c_str(),&sa);
}

bool Subsystem::DirectoryExist(String iDir)
{
	DWORD tFileAttributes=GetFileAttributes(iDir.c_str());

	return (tFileAttributes!=INVALID_FILE_ATTRIBUTES && (tFileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

void* Subsystem::LoadLib(String iLibName)
{
	void* tModule=::LoadLibrary(iLibName.c_str());

	if(!tModule)
		GuiLogger::Log(StringUtils::Format(L"SubsystemWin32::LoadLibrary: error %d loading module %s\n",GetLastError(),iLibName.c_str()));

	return tModule;
}

bool Subsystem::FreeLib(void* iModule)
{
	return ::FreeLibrary((HMODULE)iModule);
}

void* Subsystem::GetProcAddress(void* iModule,String iAddress)
{
	return (void*)::GetProcAddress((HMODULE)iModule,StringUtils::ToChar(iAddress).c_str());
}

void Subsystem::SystemMessage(String iTitle,String iMessage,unsigned iFlags)
{
	::MessageBox(0,iMessage.c_str(),iTitle.c_str(),iFlags);
}

int Subsystem::TrackMenu(int iPopupMenu,Frame* iFrame,float x,float y)
{
	//set WM_MOUSELEAVE not to call ONMOUSEEXIT
	iFrame->skipFrameMouseExit=true;

	int tResult=TrackPopupMenu((HMENU)iPopupMenu,TPM_RETURNCMD |TPM_LEFTALIGN|TPM_TOPALIGN,x,y,0,(HWND)iFrame->windowData->GetWindowHandle(),0);

	//set WM_MOUSELEAVE not to call ONMOUSEEXIT
	iFrame->skipFrameMouseExit=false;

	//for WM_MOUSEMOVE
	TRACKMOUSEEVENT tTrackMouseEvent={sizeof(TRACKMOUSEEVENT),TME_LEAVE,(HWND)iFrame->windowData->GetWindowHandle(),HOVER_DEFAULT};

	if(!TrackMouseEvent(&tTrackMouseEvent))
		DEBUG_BREAK();

	return tResult;
}

/////////////////DebuggerWin32/////////////////

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

DWORD WINAPI DebuggerWin32::debuggeeThreadFunc(LPVOID iDebuggerWin32)
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
	DebuggerWin32* debuggerWin32=(DebuggerWin32*)Debugger::Instance();

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

	tEditorScript->scriptViewer->GetRoot()->GetFrame()->SetDraw(tEditorScript->scriptViewer);

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

	this->debuggeeThread=CreateThread(0,0,DebuggerWin32::debuggeeThreadFunc,this,/*CREATE_SUSPENDED*/0,(DWORD*)(int*)&this->debuggeeThreadId);

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

static bool STLRemovePlugins(PluginSystem::Plugin* iPlugin)
{
	SAFEDELETE(iPlugin);
	return true;
}

static bool STLRemoveHandles(void* iHandle)
{
	HMODULE tModule=(HMODULE)iHandle;

	if(iHandle)
		FreeLibrary(tModule);
	return true;
}

void PluginSystem::ScanPluginsDirectory()
{
	this->plugins.remove_if(STLRemovePlugins);

	Ide::Instance()->projectDirChangedThread->Block(true);

	File tPluginsFile=Ide::Instance()->folderPlugins + L"\\plugins.cfg";

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

	String tTargetDir=FilePath(Ide::Instance()->pathExecutable).Path() + L"\\plugins\\*";

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
					this->plugins.push_back(tPlugin);

					tPlugin->handle=tPluginDll;

					for(size_t i=0;i<tPluginStates.size();i++)
					{
						if(tPlugin->name==tPluginStates[i])
							tPlugin->Load();

						tPlugin->listBoxItem.SetLabel(tPlugin->name);
					}

					wprintf(L"%s plugin loaded\n",tPlugin->name.c_str());
				}
			}

			if(!ptfGetPluginPrototypeFunction || !tPlugin)
				FreeLibrary(tPluginDll);
		}
	}

	Ide::Instance()->projectDirChangedThread->Block(false);
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

