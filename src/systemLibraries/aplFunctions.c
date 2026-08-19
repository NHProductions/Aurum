
#include "aplFunctions.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <pthread.h>
#include "../processing/vm.h"
int getIntArg(typedValue* tv) {
    if (tv->valueType != TYPE_NUM) fatalError(0x30, "", -1);
    return convertNum(tv->value.numberValue, NUM_INT).value.iVal;
}
typedValue* hwndToVar(HWND hwnd, virtualMachineState* vms, array* loc) {
    char winName[256];
    winName[0] = '\0';
    int hasWinName = GetClassName(hwnd, winName, 255);
    if (hasWinName) { 
    bool windowFound = false;
    if (loc != NULL) {
    for (int i = 0; i < loc->length; i++) {
        stackVariable* stv = getArray(loc, i);
        if (stv->value == NULL) continue;
        if (stv->value->valueType != TYPE_STRUCT) continue;
        if (strcmp(stv->value->value.so.def->name, "window") != 0) continue;
        int64_t ptr = stv->value->value.so.fields[0]->value.numberValue.value.lVal;
        HWND* wptr = (HWND*)ptr;
        char n[256];
        GetClassName(*wptr, n, 255);
        if (strcmp(n, winName) == 0) {
            return stv->value;
            break;
        }
     }
    }
    if (!windowFound) {
    for (int i = 0; i < vms->globals->length; i++) {
        stackVariable* stv = getArray(vms->globals, i);
        if (stv->value == NULL) continue;
        if (stv->value->valueType != TYPE_STRUCT) continue;
        if (strcmp(stv->value->value.so.def->name, "window") != 0) continue;
        int64_t ptr = stv->value->value.so.fields[0]->value.numberValue.value.lVal;
        HWND* wptr = (HWND*)ptr;
        char n[256];
        GetClassName(*wptr, n, 255);
        if (strcmp(n, winName) == 0) {
            return stv->value;
            break;
        }
    }}
    return 0;
}}
#ifdef _MSC_VER
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
typedef struct {
    MSG m;
    bool handled;
} queueMsg;
array* classes = NULL;
array* classNames = NULL;
virtualMachineState* windowVMS = NULL;
array* currentLocals = NULL;
bool isUserMsg = false;
char currentCreatingClass[256];
queueMsg currentWinMsg = {0};
array* msgQueue = NULL;
int parseMsg(queueMsg qmsg) {
    unsigned int uMsg = qmsg.m.message;
    HWND hwnd = qmsg.m.hwnd;
    WPARAM wParam = qmsg.m.wParam;
    LPARAM lParam = qmsg.m.lParam;
    typedValue* toReturn = poolAlloc(globalPool);
    *toReturn = (typedValue){
        .ptr = NULL,
        .valueType = TYPE_STRUCT,
        .value.so = (struct structObject){
            .def = getStructDefViaName("wmsg", *windowVMS->bc),
        }
    };
    toReturn->value.so.fields = malloc(sizeof(typedValue*)*(toReturn->value.so.def->fields->length));
    memset(toReturn->value.so.fields, 0, sizeof(typedValue*)*(toReturn->value.so.def->fields->length));
    toReturn->value.so.fields[0] = numToTV((num){.value.uiVal = uMsg, .type = NUM_UINT});
    toReturn->value.so.fields[1] = numToTV((num){.value.cVal = 2, .type = NUM_CHAR});
    toReturn->value.so.fields[3] = numToTV((num){.value.lVal = qmsg.m.time, .type = NUM_LONG});
    toReturn->value.so.fields[4] = numToTV((num){.value.sVal = (short)qmsg.m.pt.x, .type = NUM_SHORT});
    toReturn->value.so.fields[5] = numToTV((num){.value.sVal = (short)qmsg.m.pt.y, .type = NUM_SHORT});
    toReturn->value.so.fields[6] = numToTV((num){.value.lVal = wParam, .type = NUM_LONG});
    toReturn->value.so.fields[7] = numToTV((num){.value.lVal = lParam, .type = NUM_LONG});
    toReturn->value.so.fields[8] = numToTV((num){.value.bVal = qmsg.handled, .type = NUM_BOOL});
    char winName[256];
    int hasWinName = GetClassName(hwnd, winName, 255);
    if (hasWinName) { 
    bool windowFound = false;
    if (currentLocals != NULL) {
    for (int i = 0; i < currentLocals->length; i++) {
        stackVariable* stv = getArray(currentLocals, i);
        if (stv->value == NULL) continue;
        if (stv->value->valueType != TYPE_STRUCT) continue;
        if (strcmp(stv->value->value.so.def->name, "window") != 0) continue;
        int64_t ptr = stv->value->value.so.fields[0]->value.numberValue.value.lVal;
        HWND* wptr = (HWND*)ptr;
        char n[256];
        GetClassName(*wptr, n, 255);
        if (strcmp(n, winName) == 0) {
            toReturn->value.so.fields[2] = deepcopyTypedValue(stv->value);
            windowFound = true;
            break;
        }
     }
    }
    if (!windowFound) {
    for (int i = 0; i < windowVMS->globals->length; i++) {
        stackVariable* stv = getArray(windowVMS->globals, i);
        if (stv->value == NULL) continue;
        if (stv->value->valueType != TYPE_STRUCT) continue;
        if (strcmp(stv->value->value.so.def->name, "window") != 0) continue;
        int64_t ptr = stv->value->value.so.fields[0]->value.numberValue.value.lVal;
        HWND* wptr = (HWND*)ptr;
        char n[256];
        GetClassName(*wptr, n, 255);
        if (strcmp(n, winName) == 0) {
            windowFound = true;
            toReturn->value.so.fields[2] = deepcopyTypedValue(stv->value);
            break;
        }
    }}
    if (!windowFound) {
        toReturn->value.so.fields[2] = newTVArray(strlen(winName)+1, AT_CHARARR);
        for (int i = 0; i < strlen(winName); i++) {
            toReturn->value.so.fields[2]->value.av.data[i] = numToTV((num){.type = NUM_CHAR, .value.cVal = winName[i]});
        }   
        toReturn->value.so.fields[2]->value.av.data[strlen(winName)] = numToTV((num){.type = NUM_CHAR, .value.cVal = 0});
        toReturn->value.so.fields[1]->value.numberValue = (num){.type = NUM_CHAR, .value.cVal = 1};
    }
    } else {
        toReturn->value.so.fields[1]->value.numberValue = (num){.type = NUM_CHAR, .value.cVal = 3};
        toReturn->value.so.fields[2] = numToTV((num){.type = NUM_CHAR, .value.cVal = 0});
    }
    pushArray(windowVMS->stack, toReturn);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (!isUserMsg) {
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    int returnVal = 0;
    bool handled = false;
    switch (uMsg) {
        case WM_CREATE:
        case WM_DESTROY:
        case WM_SIZE:
        case WM_MOVE:
        case WM_SIZING:
        case WM_MOVING:
        case WM_ENTERSIZEMOVE:
        case WM_EXITSIZEMOVE:
        case WM_GETMINMAXINFO:
        case WM_NCCALCSIZE:
        case WM_NCPAINT:
        case WM_NCACTIVATE:
        case WM_NCHITTEST:
        case WM_SETCURSOR:
        case WM_PAINT:
        case WM_GETTEXT:
        case WM_SETTEXT:
        case WM_GETTEXTLENGTH:
        case WM_SYSCOLORCHANGE:
        {
            returnVal = DefWindowProc(hwnd, uMsg, wParam, lParam);
            handled = true;
            break;
        }    
        case WM_CLOSE:
        case WM_QUERYENDSESSION: {
            // Uses window.allowEndSession to return a value.
            typedValue* toCheck = hwndToVar(hwnd, windowVMS, currentLocals);
            if (toCheck == NULL) {returnVal = true; handled = true; break;};
            handled = true;
            returnVal = toCheck->value.so.fields[2]->value.numberValue.value.bVal; // If it's just WM_QUERYENDSESSION, return window.allowShutdown.
            if (returnVal && uMsg == WM_CLOSE) {
                returnVal = DefWindowProc(hwnd, uMsg, wParam, lParam); // If window.allowShutdown, then do the default windows handling.
            }
            break;
        }
        // If window.allowMaximization, then return 1, else return 0.
        case WM_QUERYOPEN: {
            typedValue* toCheck = hwndToVar(hwnd, windowVMS, currentLocals);
            if (toCheck == NULL) {returnVal = true; handled = true; break;};
            handled = true;
            returnVal = toCheck->value.so.fields[3]->value.numberValue.value.bVal;
            break;
        }
        case WM_ERASEBKGND: {
            typedValue* toCheck = hwndToVar(hwnd, windowVMS, currentLocals);
            if (toCheck == NULL) {returnVal = false; handled = true; break;}
            handled = true;
            returnVal = toCheck->value.so.fields[4]->value.numberValue.value.bVal;
            break;
        }
        default:
            break;
    }
    queueMsg* toAppend = malloc(sizeof(queueMsg));
    toAppend->m.hwnd = hwnd;
    toAppend->m.lParam = lParam;
    toAppend->m.wParam = wParam;
    toAppend->m.message = uMsg;
    toAppend->handled = handled;
    if (!GetCursorPos(&toAppend->m.pt)) {
        toAppend->m.pt = (POINT){.x = -1, .y = -1};
    }
    toAppend->m.time = time(NULL);
    insertArray(msgQueue, 0, toAppend);
    
    return returnVal;
}
typedValue* toPointTV(MSG m) {
    int x = GET_X_LPARAM(m.lParam);
    int y = GET_Y_LPARAM(m.lParam);
    typedValue* toReturn = newTVArray(0, AT_NUM);
    appendTypedValue(toReturn, numToTV((num){.type = NUM_INT, .value.iVal = x}));
    appendTypedValue(toReturn, numToTV((num){.type = NUM_INT, .value.iVal = y}));
    return toReturn;
}
typedValue* charToArr(char* n) {
    int len = strlen(n);
    typedValue* tr = newTVArray(0, AT_CHARARR);
    for (int i = 0; i <= len; i++) {
        appendTypedValue(tr, numToTV((num){.type = NUM_CHAR, .value.cVal = n[i]}));
    }
    return tr;
}
void aWindow(auFunc) {
    if (classes == NULL) {
        classes = mallocArray(0);
        classNames = mallocArray(0);
        windowVMS = vms;
        msgQueue = mallocArray(0);
        currentCreatingClass[0] = '\0';
    }
    currentLocals = locals;
    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (strcmp(identifier, "createWindow") == 0) {
        typedValue* arg0 = getArray(args, 0);
        if (arg0->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
        if (arg0->value.av.arrayType != AT_CHARARR) fatalError(0x30, "", -1);
        char* className = strArrToChar(arg0);
        WNDCLASS* wcl = NULL;
        for (int i = 0; i < classNames->length; i++) {
            char* n = getArray(classNames, i);
            if (strcmp(n, className) == 0) {
                wcl = getArray(classes, i);
                break;
            }
        }
        if (wcl != NULL) {
            fatalError(0x30, "Class names have to be unique.", -1);
        }
        WNDCLASS wc = {0};
        wc.lpfnWndProc   = WindowProc;
        wc.hInstance     = hInstance;
        wc.lpszClassName = className;
        wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        strncpy(currentCreatingClass, className, 255);
        WNDCLASS* toApp = malloc(sizeof(WNDCLASS));
        RegisterClass(&wc);
        appendArray(classes, toApp);
        *toApp = wc;
        appendArray(classNames, className);
        wcl = toApp;
        

        typedValue* arg1 = getArray(args, 1);
        if (arg1->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
        if (arg1->value.av.arrayType != AT_CHARARR) fatalError(0x30, "", -1);
        char* name = strArrToChar(arg1);

        typedValue* arg2 = getArray(args, 2);
        if (arg2->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        uint32_t style = convertNum(arg2->value.numberValue, NUM_UINT).value.uiVal;

        typedValue* arg3 = getArray(args, 3);
        if (arg3->valueType != TYPE_NUM) fatalError(0x30, "", -1);
        uint32_t extstyle = convertNum(arg3->value.numberValue, NUM_UINT).value.uiVal;

        typedValue* arg4 = getArray(args, 4);
        if (arg4->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
        if (arg4->value.av.len != 4) {fatalError(0x30, "", -1);}
        int xpos = getIntArg(arg4->value.av.data[0]);
        int ypos = getIntArg(arg4->value.av.data[1]);
        int xsize = getIntArg(arg4->value.av.data[2]);
        int ysize = getIntArg(arg4->value.av.data[3]);

        typedValue* arg5 = getArray(args, 5);
        if (arg5->valueType != TYPE_STRUCT && arg5->valueType != TYPE_NUM) {fatalError(0x30, "", -1);}
        if (arg5->valueType == TYPE_STRUCT) {
            if (strcmp(arg5->value.so.def->name, "window") != 0) fatalError(0x30, "", -1);
        }
        HWND* parentWindow = arg5->valueType == TYPE_STRUCT ? (HWND*)arg5->value.so.fields[0]->value.numberValue.value.lVal : NULL;
        
        HWND* newWindow = malloc(sizeof(HWND));
        isUserMsg = false;
        *newWindow = CreateWindowEx(
            extstyle,
            className,
            name,
            style,
            xpos, ypos,
            xsize, ysize,
            parentWindow != NULL ? *parentWindow : NULL,
            NULL,
            hInstance,
            NULL
        );
        isUserMsg = true;
        if (*newWindow == NULL) {
            fatalError(0x30, "Failed to create window!", -1);
        }
        typedValue* toReturn = poolAlloc(globalPool);
        toReturn->ptr = NULL; toReturn->valueType = TYPE_STRUCT;
        toReturn->value.so = (struct structObject){
            .def = getStructDefViaName("window", *vms->bc),
        };
        toReturn->value.so.fields = malloc(sizeof(typedValue*)*toReturn->value.so.def->fields->length);
        toReturn->value.so.fields[0] = numToTV((num){.type = NUM_LONG, .value.lVal = (int64_t)newWindow});
        for (int i = 1; i < toReturn->value.so.def->fields->length; i++) {
            toReturn->value.so.fields[i] = numToTV((num){.type = NUM_INT, .value.iVal = 0});
        }
        pushArray(vms->stack, toReturn);
        free(name);
        freeTypedValue(arg0);
        freeTypedValue(arg1);
        freeTypedValue(arg2);
        freeTypedValue(arg3);
        freeTypedValue(arg4);
        freeTypedValue(arg5);
        currentCreatingClass[0] = '\0';
        return;
    }
    else if (strcmp(identifier, "window_show") == 0) {
        typedValue* arg0 = getArray(args, 0);
        if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
        if (strcmp("window", arg0->value.so.def->name) != 0) fatalError(0x30, "", -1);
        isUserMsg = false;
        HWND* toShow = (HWND*)arg0->value.so.fields[0]->value.numberValue.value.lVal;
        ShowWindow(*toShow, SW_SHOW);
        windowsOpen++;
        freeTypedValue(arg0);
        isUserMsg = true;
        return;
    }
    /*
    struct wmsg {\n\
    uint code;\n\
    bool exists;\n\
    window windowSent;\n\
    long time;\n\
    short px;\n\
    short py;\n\
    }\n\
    */
    else if (strcmp(identifier, "getMessage") == 0) {

        // if there's no messages in the queue, do peakMessage() and get one.
        // if there is, parse it into a msg struct and push it to the stack.
        queueMsg* toPush = NULL;
        bool msgMalloc = false;
        if (msgQueue->length == 0) {
            MSG m = {0};        
            bool exists = PeekMessage(&m, NULL, 0, 0, PM_REMOVE) != 0;
            if (!exists) {pushArray(vms->stack, numToTV((num){.value.bVal = false, .type = NUM_BOOL})); return;}
            toPush = malloc(sizeof(queueMsg));
            *toPush = (queueMsg){.m = m, .handled = false};
            TranslateMessage(&toPush->m);
        }
        else {
            toPush = popArray(msgQueue);
            TranslateMessage(&toPush->m);
        }
        currentWinMsg = (queueMsg){.m = toPush->m, .handled = toPush->handled};
        int prevStackLen = vms->stack->length;
        parseMsg(*toPush);
        free(toPush);
        if (vms->stack->length == prevStackLen) {pushArray(vms->stack, numToTV((num){.type = NUM_BOOL, .value.bVal = false})); return;}
        return;
    }
    else if (strcmp(identifier, "wmsg_handle") == 0) {
        typedValue* arg0 = getArray(args, 0);
        if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
        if (strcmp(arg0->value.so.def->name, "wmsg") != 0) fatalError(0x30, "", -1);
        if (arg0->value.so.fields[8]->value.numberValue.value.bVal == true) {
            freeTypedValue(arg0);
            return;
        }
        MSG m = {0};
        if (arg0->value.so.fields[1]->value.numberValue.value.cVal == 2) {
            m.hwnd = *(HWND*)arg0->value.so.fields[2]->value.so.fields[0]->value.numberValue.value.lVal;
        }
        else if (arg0->value.so.fields[1]->value.numberValue.value.cVal == 1) {
            char* toFind = strArrToChar(arg0->value.so.fields[2]);
            m.hwnd = FindWindow(toFind, NULL);
            free(toFind);
        }
        else if (arg0->value.so.fields[1]->value.numberValue.value.cVal == 3) {
            m.hwnd = NULL;
        }
        m.message = arg0->value.so.fields[0]->value.numberValue.value.uiVal;
        m.time = arg0->value.so.fields[3]->value.numberValue.value.lVal;
        m.pt = (struct tagPOINT){
            .x = (long)arg0->value.so.fields[4]->value.numberValue.value.sVal,
            .y = (long)arg0->value.so.fields[5]->value.numberValue.value.sVal
        };
        m.wParam = arg0->value.so.fields[6]->value.numberValue.value.lVal;
        m.lParam = arg0->value.so.fields[7]->value.numberValue.value.lVal;
        DefWindowProc(m.hwnd, m.message, m.wParam, m.lParam);
        freeTypedValue(arg0);
        return;
    }
    else if (strcmp(identifier, "window_setText") == 0 || strcmp(identifier, "window_getText") == 0) {
        typedValue* arg0 = getArray(args, 0);
        if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
        if (strcmp("window", arg0->value.so.def->name) != 0) fatalError(0x30, "", -1);
        HWND* wnd = (HWND*)arg0->value.so.fields[0]->value.numberValue.value.lVal;
        if (strcmp(identifier, "window_setText") == 0) {
            typedValue* arg1 = getArray(args, 1);
            if (arg1->valueType != TYPE_ARRAY) fatalError(0x30, "", -1);
            if (arg1->value.av.arrayType != AT_CHARARR) fatalError(0x30, "", -1);
            char* toSet = strArrToChar(arg1);
            SendMessage(*wnd, WM_SETTEXT, 0, (LPARAM)toSet);
            free(toSet);
            freeTypedValue(arg1);
        }
        else {
            LRESULT textLength = SendMessage(*wnd, WM_GETTEXTLENGTH, 0, 0);
            char* buffer = malloc(textLength+1);
            SendMessage(*wnd, WM_GETTEXT, (WPARAM)(textLength+1), (LPARAM)(buffer));
            pushArray(vms->stack, charToArr(buffer));
            free(buffer);
        }
        freeTypedValue(arg0);
    }
    else if (strcmp(identifier, "window_setRedraw") == 0 || strcmp(identifier, "window_getRedraw") == 0 || strcmp(identifier, "window_redraw") == 0) {
        typedValue* arg0 = getArray(args, 0);
        if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
        if (strcmp("window", arg0->value.so.def->name) != 0) fatalError(0x30, "", -1);
        HWND wnd = *(HWND*)arg0->value.so.fields[0]->value.numberValue.value.lVal;
        bool canRedraw = GetProp(wnd, TEXT("SysSetRedraw")) == 0;
        if (strcmp(identifier, "window_getRedraw") == 0) {
            pushArray(vms->stack, numToTV((num){.type = NUM_BOOL, .value.bVal = canRedraw}));
            freeTypedValue(arg0);
        }
        else if (strcmp(identifier, "window_setRedraw") == 0) {
            typedValue* arg1 = getArray(args, 1);
            if (arg1->valueType != TYPE_NUM) fatalError(0x30, "", -1);
            bool toSet = arg1->value.numberValue.value.bVal;
            int n = SendMessage(wnd, WM_SETREDRAW, toSet, 0);
            freeTypedValue(arg0);
            freeTypedValue(arg1);
        }
        else {
            RedrawWindow(wnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
            freeTypedValue(arg0);
        }
    }
    else if (strcmp(identifier, "window_eraseBackground") == 0) {
        typedValue* arg0 = getArray(args, 0);
        if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
        if (strcmp("window", arg0->value.so.def->name) != 0) fatalError(0x30, "", -1);
        HWND wnd = *(HWND*)arg0->value.so.fields[0]->value.numberValue.value.lVal;
        int n = SendMessage(wnd, WM_ERASEBKGND, 0, 0);
        freeTypedValue(arg0);
    }
    else if (strcmp(identifier, "window_isShown") == 0 || strcmp(identifier, "window_hide") == 0) {
        typedValue* arg0 = getArray(args, 0);
        if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
        if (strcmp("window", arg0->value.so.def->name) != 0) fatalError(0x30, "", -1);
        HWND wnd = *(HWND*)arg0->value.so.fields[0]->value.numberValue.value.lVal;
        if (strcmp(identifier, "window_isShown") == 0) {
            pushArray(vms->stack, numToTV((num){.type = NUM_BOOL, .value.bVal = IsWindowVisible(wnd)}));
            freeTypedValue(arg0);
            return;
        }
        else if (strcmp(identifier, "window_hide") == 0) {
            ShowWindow(wnd, SW_HIDE);
            freeTypedValue(arg0);
            return;
        }
    }
    else if (strcmp(identifier, "wmsg_parse") == 0) {
        typedValue* arg0 = getArray(args, 0);
        if (arg0->valueType != TYPE_STRUCT) fatalError(0x30, "", -1);
        if (strcmp(arg0->value.so.def->name, "wmsg") != 0) fatalError(0x30, "", -1);
        MSG m = {0};
        if (arg0->value.so.fields[1]->value.numberValue.value.cVal == 2) {
            m.hwnd = *(HWND*)arg0->value.so.fields[2]->value.so.fields[0]->value.numberValue.value.lVal;
        }
        else if (arg0->value.so.fields[1]->value.numberValue.value.cVal == 1) {
            char* toFind = strArrToChar(arg0->value.so.fields[2]);
            m.hwnd = FindWindow(toFind, NULL);
            free(toFind);
        }
        else if (arg0->value.so.fields[1]->value.numberValue.value.cVal == 3) {
            m.hwnd = NULL;
        }
        m.message = arg0->value.so.fields[0]->value.numberValue.value.uiVal;
        m.time = arg0->value.so.fields[3]->value.numberValue.value.lVal;
        m.pt = (struct tagPOINT){
            .x = (long)arg0->value.so.fields[4]->value.numberValue.value.sVal,
            .y = (long)arg0->value.so.fields[5]->value.numberValue.value.sVal
        };
        m.wParam = arg0->value.so.fields[6]->value.numberValue.value.lVal;
        m.lParam = arg0->value.so.fields[7]->value.numberValue.value.lVal;
        freeTypedValue(arg0);
        switch (m.message) {
            // No params:
            case WM_NULL:
            case WM_CREATE:
            case WM_DESTROY:
            case WM_CLOSE:
            case WM_QUERYOPEN:
            case WM_SYSCOLORCHANGE:
            case WM_FONTCHANGE:
            case WM_TIMECHANGE:
            case WM_CANCELMODE:
            case WM_CHILDACTIVATE:
            case WM_QUEUESYNC:
            case WM_PAINTICON:
            case WM_GETFONT:
            case WM_GETHOTKEY:
            case WM_QUERYDRAGICON:
            case WM_NCDESTROY:
            case WM_IME_STARTCOMPOSITION:
            case WM_IME_ENDCOMPOSITION:
            case WM_QUERYUISTATE:
            case MN_GETHMENU:
            case WM_MDIICONARRANGE:
            case WM_ENTERSIZEMOVE:
            case WM_EXITSIZEMOVE:
            case WM_MDIREFRESHMENU:
            case WM_IME_COMPOSITIONFULL: 
            case WM_NCMOUSELEAVE:
            case WM_MOUSELEAVE:
            case WM_CUT:
            case WM_COPY:
            case WM_PASTE:
            case WM_CLEAR:
            case WM_UNDO:
            case WM_RENDERALLFORMATS:
            case WM_DRAWCLIPBOARD:
            case WM_QUERYNEWPALETTE:
            case WM_THEMECHANGED:
            case WM_CLIPBOARDUPDATE:
            case WM_DWMCOMPOSITIONCHANGED:
            case WM_DWMNCRENDERINGCHANGED:
            case WM_DWMWINDOWMAXIMIZEDCHANGE:
            case WM_PAINT:
            // Overridden by other functions
            case WM_SETTEXT: // window.setText
            case WM_GETTEXT: // window.getText
            case WM_GETTEXTLENGTH: // #window.getText
            {
                fatalError(0x30, "Message has no parameters.", -1);
                return;
            }
            // wmsg.parse() -> int[] = [x, y]
            case WM_NCHITTEST:
            case WM_MOVE: {
                pushArray(vms->stack, toPointTV(m));
                return;
            }
            // wmsg.parse() -> int[] = [x, y, SIZE enum]
            case WM_SIZE: {
                typedValue* arr = toPointTV(m);
                appendTypedValue(arr, numToTV((num){.type = NUM_INT, .value.iVal = m.wParam}));
                pushArray(vms->stack, arr);
                return;
            }
            // wmsg.parse() -> auto[] = [activation, thread ID]
            case WM_ACTIVATEAPP: {
                typedValue* arr = newTVArray(0, AT_UNKNOWN);
                appendTypedValue(arr, numToTV((num){.type = NUM_BOOL, .value.bVal = m.wParam}));
                appendTypedValue(arr, numToTV((num){.type = NUM_INT, .value.iVal = (int)m.lParam}));
                pushArray(vms->stack, arr);
                break;
            }
            // wmsg.parse() -> auto[] = [activation, minimized state, string]
            case WM_ACTIVATE: {
                typedValue* arr = newTVArray(0, AT_UNKNOWN);
                appendTypedValue(arr, numToTV((num){.type = NUM_CHAR, .value.cVal = LOWORD(m.wParam)}));
                appendTypedValue(arr, numToTV((num){.type = NUM_BOOL, .value.bVal = HIWORD(m.wParam) == 0}));
                HWND wnd = (HWND)m.lParam;
                char buffer[256];
                int exists = GetClassName(wnd, buffer, 255);
                if (exists) {
                    appendTypedValue(arr, charToArr(buffer));
                }
                else {
                    appendTypedValue(arr, numToTV((num){.type = NUM_CHAR, .value.cVal = 0}));
                }
                pushArray(vms->stack, arr);
                return;
            }
            // wmsg.parse() -> string = className of losing or gaining focus
            case WM_KILLFOCUS:
            case WM_SETFOCUS: {
                HWND wnd = (HWND)m.lParam;
                typedValue* arr = newTVArray(0, AT_CHARARR);
                char buffer[256];
                int exists = GetClassName(wnd, buffer, 255);
                if (exists) {
                    appendTypedValue(arr, charToArr(buffer));
                }
                else {
                    appendTypedValue(arr, numToTV((num){.type = NUM_CHAR, .value.cVal = 0}));
                }
                pushArray(vms->stack, arr);
            }
            // wmsg.parse() -> bool = if enabled or not (or if it does redraw after updates)
            case WM_SETREDRAW:
            case WM_ENABLE: {
                pushArray(vms->stack, numToTV((num){.type = NUM_BOOL, .value.bVal = m.wParam == 0}));
                return;
            }
            // wmsg.parse() -> string;
            // WM_WININICHANGE / WM_DEVMODECHANGE -> array = [identifying device, wparam]
            // WM_SETTEXT -> string = new text
            case WM_WININICHANGE: {
                typedValue* arr = newTVArray(0, AT_UNKNOWN);
                appendTypedValue(arr, charToArr((char*)m.lParam));
                appendTypedValue(arr, numToTV((num){.type = NUM_INT, .value.iVal = m.wParam == NULL ? 0 : m.wParam}));
                pushArray(vms->stack, arr);
                break;
            }
            case WM_DEVMODECHANGE: {
                pushArray(vms->stack, charToArr((char*)m.lParam));
                return;
            }
            // wmsg.parse() -> bool
            // if 0, then it's not shown, else it's shown.
            case WM_SHOWWINDOW: {
                pushArray(vms->stack, numToTV((num){.type = NUM_BOOL, .value = m.wParam != 0}));
                return;
            }
            // wmsg.parse() -> [window, hitarea test code, mouse msg identifier]
            case WM_MOUSEACTIVATE: {
                typedValue* arr = newTVArray(0, AT_UNKNOWN);
                appendTypedValue(arr, hwndToVar(m.wParam, vms, currentLocals));
                appendTypedValue(arr, numToTV((num){.type = NUM_INT, .value.iVal = LOWORD(m.lParam)}));
                appendTypedValue(arr, numToTV((num){.type = NUM_INT, .value.iVal = HIWORD(m.lParam)}));
                pushArray(vms->stack, arr);
                return;
            }
        }
    }
}

