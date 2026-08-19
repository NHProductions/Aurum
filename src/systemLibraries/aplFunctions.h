#ifndef APLFUNC_H
#define APLFUNC_H
#include "../processing/bytecoder.h"
#include "sysFunctions.h"
static const char* aplDefinitions = "\n\
uint WM_OVERLAPPED = 0ui;\n\
uint WM_MAXIMIZEBOX = 0x00010000ui;\n\
uint WM_MINIMIZEBOX = 0x00020000ui;\n\
uint WM_THICKFRAME = 0x00040000ui;\n\
uint WM_SYSMENU = 0x00080000ui;\n\
uint WM_CAPTION = 0x00C00000ui;\n\
uint WM_VISIBLE = 0x10000000ui;\n\
uint WM_CHILD = 0x40000000ui;\n\
uint WM_POPUP = 0x80000000ui;\n\
uint WM_OVERLAPPEDWINDOW = 0x00CF0000ui;\n\
uint WMX_TOPMOST = 0x00000008ui;\n\
uint WMX_ACCEPTFILES = 0x00000010ui;\n\
uint WMX_TOOLWINDOW = 0x00000080ui;\n\
uint WMX_CLIENTEDGE = 0x00000200ui;\n\
uint WMX_LAYERED = 0x00080000ui;\n\
\n\
\n\
uint MSG_CLOSE = 16ui;\n\
uint MSG_DESTROY = 2ui;\n\
uint MSG_QUIT = 0x12ui;\n\
uint MSG_NCDESTROY = 0x82ui;\n\
uint MSG_QENDSESSION = 0x11ui;\n\
uint MSG_ENDSESSION = 0x16ui;\n\
struct window {\n\
    long ptr;\n\
    fnptr msgFunc;\n\
    bool allowShutdown;\n\
    bool allowMaximization;\n\
    bool wmPaintOnly;\n\
    bool isShown;\n\
    window[] children;\n\
    window parent;\n\
    bool isActive;\n\
    function init() -> window {\n\
        this.ptr = this.ptr;\n\
        this.msgFunc = 0;\n\
        this.allowShutdown = true;\n\
        this.allowMaximization = true;\n\
        this.children = [];\n\
        return this;\n\
    }\n\
}\n\
struct wmsg {\n\
    uint code;\n\
    char existCode;\n\
    window windowSent;\n\
    long time;\n\
    short px;\n\
    short py;\n\
    long wparam;\n\
    long lparam;\n\
    bool handled;\n\
}\n\
\n\
int SIZE_RESTORED = 0i;\n\
int SIZE_MINIMIZED = 1i;\n\
int SIZE_MAXIMIZED = 2i;\n\
int SIZE_MAXSHOW = 3i;\n\
int SIZE_MAXHIDE = 4i;\n\
\n\
int ENDSESSION_CLOSEAPP = 1i;\n\
int ENDSESSION_CRITICAL = 2i;\n\
int ENDSESSION_LOGOFF = 3i;\n\
int ENDSESSION_UNKNOWN = 0i;\n\
";
/*

struct window {
long ptr;
window[] children;
window parent;

bool pSetForeground;
... window info attributes ...

}
createWindow(class, title, style, extStyle, [x, y, xsize, ysize], parentWindow) -> window - Creates a window

window.setTitle(string) -> void Changes the title text
window.show() -> void Shows the window
window.switchTo() -> void Switches focus to the window.
window.changePermission(string, int) -> void Changes a permission associated with the window.
    Acceptable Params:
    "setForeground", bool - Changes if it's allowed to set the foreground of the window.

window.animate(ms, type) -> void Animates the window using one of several types.
window.hasPopup() -> bool - If a pop-up window exists, return true. Otherwise, false.
window.bring() -> void - Brings the window to the top.
window.cascade() -> void - Cascades window
window.childHasPt(x, y) -> window - Returns the first window that contains (x,y).
window.close() -> void Closes the window & any children window.
window.isMinimized() -> bool - Returns if the window is minimized or not
window.isVisible() -> bool - Returns if the window is visible
window.move(x, y) -> void - Moves the window
window.maximize() -> void - Maximizes the window
window.setParent() -> void - Sets the parent window

getMessage() - Gets the oldest message in the message queue.
handleMessage() - Handles the message using default windows handling.
*/
static const bcFunction aplFunctions[] = {
    {.name = "createWindow", .argc = 6, .returnStruct = "window"},
    {.name = "window_setTitle", .argc = 1, .returnStruct = NULL},
    {.name = "getMessage", .argc = 0, .returnStruct = "wmsg"},
    {.name = "window_setProperty", .argc = 2, .returnStruct = NULL},
    {.name = "window_show", .argc = 0, .returnStruct = NULL},
    {.name = "wmsg_handle", .argc = 1, .returnStruct = NULL},
    {.name = "wmsg_parse", .argc = 1, .returnStruct = NULL},
    
    {.name = "window_setText", .argc = 2, .returnStruct = NULL},
    {.name = "window_getText", .argc = 1, .returnStruct = "string"},

    {.name = "window_setRedraw", .argc = 2, .returnStruct = NULL},
    {.name = "window_getRedraw", .argc = 1, .returnStruct = NULL},
    {.name = "window_redraw", .argc = 1, .returnStruct = "string"},

    {.name = "window_eraseBackground", .argc = 1, .returnStruct = NULL},

    {.name = "window_isShown", .argc = 1, .returnStruct = NULL},
    {.name = "window_hide", .argc = 1, .returnStruct = NULL},


    {.name = "!endFunc", .argc = 0},
    {.name = "", .argc = -1}
};
void aWindow(auFunc);
/*


*/

#endif 