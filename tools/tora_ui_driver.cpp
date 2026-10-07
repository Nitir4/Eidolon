// Reference UI automation only. No binary inspection or product dependency.
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {
BOOL CALLBACK findTora(HWND window,LPARAM pointer);
std::vector<WCHAR> wide(const char* text) {
    // Winelib's narrow argv can use the Windows ANSI code page even though
    // the Linux launcher received UTF-8. Prefer valid UTF-8; recover ACP argv.
    UINT encoding=CP_UTF8;
    int size=MultiByteToWideChar(encoding,MB_ERR_INVALID_CHARS,text,-1,nullptr,0);
    if (!size) {encoding=CP_ACP;size=MultiByteToWideChar(encoding,0,text,-1,nullptr,0);}
    if (!size) return {};
    std::vector<WCHAR> value(static_cast<std::size_t>(size),0);
    MultiByteToWideChar(encoding,0,text,-1,value.data(),size);
    return value;
}
bool ownedByTora(HWND window) {
    HWND root=nullptr;EnumWindows(findTora,reinterpret_cast<LPARAM>(&root));
    DWORD rootPid=0,windowPid=0;
    if (root) GetWindowThreadProcessId(root,&rootPid);
    GetWindowThreadProcessId(window,&windowPid);
    if (rootPid && rootPid==windowPid) return true;
    for (int i=0;window && i<16;++i) {
        char title[1024]{};GetWindowTextA(window,title,1024);
        if (std::strncmp(title,"TORA",4)==0) return true;
        HWND parent=GetParent(window);window=parent?parent:GetWindow(window,GW_OWNER);
    }
    return false;
}
struct DialogSearch {DWORD pid;HWND found=nullptr;};
BOOL CALLBACK findDialog(HWND window,LPARAM pointer) {
    auto& search=*reinterpret_cast<DialogSearch*>(pointer);
    char klass[128]{};GetClassNameA(window,klass,128);DWORD pid=0;GetWindowThreadProcessId(window,&pid);
    if (pid==search.pid && IsWindowVisible(window) && IsWindowEnabled(window) && std::strcmp(klass,"#32770")==0) {search.found=window;return FALSE;}
    return TRUE;
}
HWND activeDialog(HWND root) {
    DWORD pid=0;GetWindowThreadProcessId(root,&pid);DialogSearch search{pid};
    EnumWindows(findDialog,reinterpret_cast<LPARAM>(&search));
    return search.found?search.found:GetLastActivePopup(root);
}
BOOL CALLBACK findTora(HWND window,LPARAM pointer) {
        char title[1024]{};GetWindowTextA(window,title,1024);
        if (IsWindowVisible(window) && std::strncmp(title,"TORA",4)==0) {
            *reinterpret_cast<HWND*>(pointer)=window;return FALSE;
        }
        return TRUE;
}
BOOL CALLBACK dumpWindows(HWND window,LPARAM) {
    char title[1024]{},klass[128]{};GetWindowTextA(window,title,1024);GetClassNameA(window,klass,128);
    {
        RECT r{};GetWindowRect(window,&r);
        std::printf("TORA form: %s class=%s visible=%d enabled=%d rect=%d,%d,%d,%d\n",title,klass,IsWindowVisible(window),IsWindowEnabled(window),r.left,r.top,r.right,r.bottom);
    }
    return TRUE;
}
BOOL CALLBACK findReadOnly(HWND window,LPARAM pointer) {
    char title[256]{};GetWindowTextA(window,title,256);
    if (std::strstr(title,"read-only")) {*reinterpret_cast<HWND*>(pointer)=window;return FALSE;}
    return TRUE;
}
std::string caption(HWND window) {
    char text[1024]{};GetWindowTextA(window,text,sizeof(text));
    std::string result;
    for (const char* at=text;*at && *at!='\t';++at) if (*at!='&') result+=*at;
    return result;
}
struct ButtonSearch {const char* text;HWND found=nullptr;};
BOOL CALLBACK findButton(HWND window,LPARAM pointer) {
    auto& search=*reinterpret_cast<ButtonSearch*>(pointer);
    if (IsWindowEnabled(window) && caption(window)==search.text) {search.found=window;return FALSE;}
    return TRUE;
}
UINT findMenu(HMENU menu,const char* requested) {
    for (int i=0;menu && i<GetMenuItemCount(menu);++i) {
        char label[1024]{};GetMenuStringA(menu,i,label,sizeof(label),MF_BYPOSITION);
        std::string text;for (const char* at=label;*at && *at!='\t';++at) if (*at!='&') text+=*at;
        if (text==requested && !(GetMenuState(menu,i,MF_BYPOSITION)&(MF_DISABLED|MF_GRAYED))) return GetMenuItemID(menu,i);
        if (const auto command=findMenu(GetSubMenu(menu,i),requested)) return command;
    }
    return 0;
}
void dumpMenu(HMENU menu) {
    for (int i=0;menu && i<GetMenuItemCount(menu);++i) {
        char label[1024]{};GetMenuStringA(menu,i,label,sizeof(label),MF_BYPOSITION);
        std::printf("Menu: %s state=%u id=%u\n",label,GetMenuState(menu,i,MF_BYPOSITION),GetMenuItemID(menu,i));
        dumpMenu(GetSubMenu(menu,i));
    }
}
HWND focusedControl() {
    HWND root=nullptr;
    EnumWindows(findTora,reinterpret_cast<LPARAM>(&root));
    if (!root) return nullptr;
    const HWND foreground=activeDialog(root);
    if (!ownedByTora(foreground)) return nullptr;
    GUITHREADINFO info{};info.cbSize=sizeof(info);
    if (!GetGUIThreadInfo(GetWindowThreadProcessId(foreground,nullptr),&info)) return nullptr;
    return info.hwndFocus;
}
}
int main(int argc,char** argv) {
    if (argc<2) return 2;
    const HDESK desktop=OpenDesktopW(L"ToraOracle800",0,FALSE,
        DESKTOP_READOBJECTS|DESKTOP_WRITEOBJECTS|DESKTOP_ENUMERATE);
    if (!desktop || !SetThreadDesktop(desktop)) {
        std::fprintf(stderr,"Cannot attach to TORA's named reference desktop: %lu\n",static_cast<unsigned long>(GetLastError()));return 7;
    }
    if (std::strcmp(argv[1],"windows")==0) {EnumWindows(dumpWindows,0);return 0;}
    HWND current=nullptr;EnumWindows(findTora,reinterpret_cast<LPARAM>(&current));
    if (!current) {
        const HDESK standard=OpenDesktopW(L"Default",0,FALSE,DESKTOP_READOBJECTS|DESKTOP_WRITEOBJECTS|DESKTOP_ENUMERATE);
        if (standard) SetThreadDesktop(standard);
    }
    if (std::strcmp(argv[1],"present")==0) {
        HWND root=nullptr;EnumWindows(findTora,reinterpret_cast<LPARAM>(&root));return root?0:3;
    }
    if (std::strcmp(argv[1],"dialog")==0) {
        HWND root=nullptr;EnumWindows(findTora,reinterpret_cast<LPARAM>(&root));if (!root) return 3;
        const auto popup=activeDialog(root);char klass[128]{};GetClassNameA(popup,klass,128);
        if (std::strcmp(klass,"#32770")!=0 || !IsWindowVisible(popup)) return 3;
        SetForegroundWindow(popup);return 0;
    }
    if (std::strcmp(argv[1],"click")==0 && argc==5) {
        POINT p{std::atoi(argv[2]),std::atoi(argv[3])};
        if (p.x<0 || p.x>800 || p.y<0 || p.y>600) return 17;
        HWND target=WindowFromPoint(p);
        if (!target || !ownedByTora(target) || !IsWindowEnabled(target) || caption(target)=="Exit TORA") return 18;
        if (!ownedByTora(GetForegroundWindow())) {
            HWND root=nullptr;EnumWindows(findTora,reinterpret_cast<LPARAM>(&root));
            SetForegroundWindow(activeDialog(root));
        }
        if (!SetCursorPos(p.x,p.y)) return 19;
        Sleep(80);
        INPUT clicks[2]{};clicks[0].type=INPUT_MOUSE;clicks[0].mi.dwFlags=MOUSEEVENTF_LEFTDOWN;
        clicks[1].type=INPUT_MOUSE;clicks[1].mi.dwFlags=MOUSEEVENTF_LEFTUP;
        if (SendInput(2,clicks,sizeof(INPUT))!=2) return 20;
        if (std::atoi(argv[4])==2) {Sleep(60);if (SendInput(2,clicks,sizeof(INPUT))!=2) return 20;}
        return 0;
    }
    if (std::strcmp(argv[1],"key")==0 && argc==3) {
        HWND root=nullptr;EnumWindows(findTora,reinterpret_cast<LPARAM>(&root));if (!root) return 3;
        if (!ownedByTora(GetForegroundWindow())) SetForegroundWindow(activeDialog(root));
        WORD code=0,modifier=0;
        const std::string key=argv[2];
        if (key=="Return") code=VK_RETURN;else if(key=="Escape")code=VK_ESCAPE;
        else if(key=="Tab")code=VK_TAB;else if(key=="Home")code=VK_HOME;
        else if(key=="BackSpace")code=VK_BACK;else if(key=="shift+End"){code=VK_END;modifier=VK_SHIFT;}
        else if(key=="ctrl+a"){code='A';modifier=VK_CONTROL;}else if(key=="n")code='N';else return 21;
        const auto send=[](WORD value,bool up){INPUT event{};event.type=INPUT_KEYBOARD;event.ki.wVk=value;event.ki.dwFlags=up?KEYEVENTF_KEYUP:0;return SendInput(1,&event,sizeof(INPUT))==1;};
        if(modifier && !send(modifier,false))return 20;
        const bool success=send(code,false) && send(code,true);
        if(modifier)send(modifier,true);
        return success?0:20;
    }
    if (std::strcmp(argv[1],"menus")==0) {
        EnumWindows(dumpWindows,0);
        HWND root=nullptr;EnumWindows(findTora,reinterpret_cast<LPARAM>(&root));
        if (!root) return 3;
        dumpMenu(GetMenu(root));std::printf("ACP: %u\n",GetACP());return 0;
    }
    if (std::strcmp(argv[1],"arrange")==0) {
        HWND root=nullptr;EnumWindows(findTora,reinterpret_cast<LPARAM>(&root));
        if (!root) return 3;
        ShowWindow(root,SW_RESTORE);
        if (!SetWindowPos(root,HWND_TOP,-4,-4,808,608,SWP_SHOWWINDOW)) return 12;
        if (IsWindowEnabled(root)) SetForegroundWindow(root);
        return 0;
    }
    if ((std::strcmp(argv[1],"button")==0 || std::strcmp(argv[1],"menu")==0) && argc==3) {
        HWND root=nullptr;EnumWindows(findTora,reinterpret_cast<LPARAM>(&root));
        if (!root) return 3;
        if (std::strcmp(argv[1],"menu")==0) {
            if (!IsWindowEnabled(root) || std::strcmp(argv[2],"Exit TORA")==0) return 13;
            const auto command=findMenu(GetMenu(root),argv[2]);if (!command) return 14;
            EndMenu();
            PostMessageW(root,WM_COMMAND,command,0);return 0;
        }
        HWND dialog=activeDialog(root);if (!dialog) dialog=root;
        ButtonSearch search{argv[2]};EnumChildWindows(dialog,findButton,reinterpret_cast<LPARAM>(&search));
        if (!search.found || !ownedByTora(search.found)) return 15;
        // A VB button handler can open a modal confirmation. Queue the normal
        // click so the observer can capture and answer that next dialog.
        return PostMessageW(search.found,BM_CLICK,0,0)?0:16;
    }
    const HWND control=focusedControl();
    if (!control) {std::fprintf(stderr,"No focused control owned by TORA\n");return 3;}
    if (std::strcmp(argv[1],"readonly")==0 && argc==3) {
        HWND checkbox=nullptr;EnumChildWindows(GetAncestor(control,GA_ROOT),findReadOnly,reinterpret_cast<LPARAM>(&checkbox));
        if (!checkbox || !ownedByTora(checkbox)) return 8;
        const auto desired=std::strcmp(argv[2],"true")==0?BST_CHECKED:BST_UNCHECKED;
        if (SendMessageW(checkbox,BM_GETCHECK,0,0)!=desired) SendMessageW(checkbox,BM_CLICK,0,0);
        return SendMessageW(checkbox,BM_GETCHECK,0,0)==desired?0:9;
    }
    if (std::strcmp(argv[1],"text")==0 && argc==3) {
        char klass[128]{};GetClassNameA(control,klass,128);
        if (std::strcmp(klass,"Edit")!=0 && !(std::strstr(klass,"ThunderRT")==klass && std::strstr(klass,"TextBox"))) {
            std::fprintf(stderr,"Focused TORA control is not a text editor\n");return 4;
        }
        const auto value=wide(argv[2]);
        if (value.empty()) return 22;
        if (!IsWindowEnabled(control) || (GetWindowLongPtrW(control,GWL_STYLE)&ES_READONLY)) {
            std::fprintf(stderr,"TORA editor is disabled or read-only\n");return 10;
        }
        const auto limit=SendMessageW(control,EM_GETLIMITTEXT,0,0);
        if (limit>0 && value.size()-1>static_cast<std::size_t>(limit)) return 11;
        // Character messages follow the editor's normal change notifications,
        // including VB's cell editor, rather than setting its backing text.
        SendMessageW(control,EM_SETSEL,0,-1);
        SendMessageW(control,WM_CLEAR,0,0);
        for (std::size_t i=0;i+1<value.size();++i) SendMessageW(control,WM_CHAR,value[i],1);
        SendMessageW(control,EM_SETSEL,value.size()-1,value.size()-1);
        std::vector<WCHAR> actual(value.size(),0);SendMessageW(control,WM_GETTEXT,actual.size(),reinterpret_cast<LPARAM>(actual.data()));
        if (actual!=value) {std::fprintf(stderr,"TORA editor did not retain the requested text\n");return 6;}
        return 0;
    }
    if (std::strcmp(argv[1],"dump")==0) {
        WCHAR klass[128]{},text[2048]{};GetClassNameW(control,klass,128);SendMessageW(control,WM_GETTEXT,2048,reinterpret_cast<LPARAM>(text));
        char utf8Class[512]{},utf8Text[8192]{};
        WideCharToMultiByte(CP_UTF8,0,klass,-1,utf8Class,sizeof(utf8Class),nullptr,nullptr);
        WideCharToMultiByte(CP_UTF8,0,text,-1,utf8Text,sizeof(utf8Text),nullptr,nullptr);
        std::printf("Focused class: %s\nText: %s\nEdit limit: %lld\n",utf8Class,utf8Text,static_cast<long long>(SendMessageW(control,EM_GETLIMITTEXT,0,0)));
        return 0;
    }
    return 2;
}
