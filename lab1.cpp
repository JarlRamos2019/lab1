//modified by: Jarl Ramos
//date: 23 August 2022
//
//author: Gordon Griesel
//date: Spring 2022
//purpose: get openGL working on your personal computer
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

//some structures

class Global {
    public:
    int xres, yres;
    float w;
    float dir;
    float pos[2];
    Global();

} g;

class X11_wrapper {
private:
    Display *dpy;
    Window win;
    GLXContext glc;
public:
    ~X11_wrapper();
    X11_wrapper();
    void set_title();
    bool getXPending();
    XEvent getXNextEvent();
    void swapBuffers();
    void reshape_window(int width, int height);
    void check_resize(XEvent *e);
    void check_mouse(XEvent *e);
    int check_keys(XEvent *e);
} x11;

//Function prototypes

void init_opengl(void);
void physics(void);
void render(int xResPrev, GLfloat & red, GLfloat & green, GLfloat & blue, GLfloat & pRed);

//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
    init_opengl();
    
    //Set initial colors of the box.
    GLfloat redVal   = 0.3f;
    GLfloat greenVal = 0.2f;
    GLfloat blueVal  = 0.8f;
    //Needed to reset the red value after box touches the window edge.
    GLfloat prevRed;
    //Main loop
    int done = 0;
    //Set previous resolution to current one before starting loop.
    int prevXRes = g.xres;
    while (!done) {
        //Process external events.
        while (x11.getXPending()) {
            XEvent e = x11.getXNextEvent();
            x11.check_resize(&e);
            x11.check_mouse(&e);
            done = x11.check_keys(&e);
        }
        physics();
        //render() passes the references of the color values and takes in
        //the previous resolution for comparison with the current one.
        render(prevXRes, redVal, greenVal, blueVal, prevRed);
        //The previous resolution is always set to the current one each time
        //the loop runs.
        prevXRes = g.xres;
        x11.swapBuffers();
        usleep(200);
    }
    return 0;
}

Global::Global()
{
    xres = 400;
    yres = 200;
    w = 20.0f;
    dir = 25.0f;
    pos[0] = 0.0f + w;
    pos[1] = yres / 2.0f;
}

X11_wrapper::~X11_wrapper()
{
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w = g.xres, h = g.yres;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        cout << "\n\tcannot connect to X server\n" << endl;
        exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if (vi == NULL) {
        cout << "\n\tno appropriate visual found\n" << endl;
        exit(EXIT_FAILURE);
    }
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask =
        ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPress | ButtonReleaseMask |
        PointerMotionMask |
        StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
        InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "3350 Lab1");
}

bool X11_wrapper::getXPending()
{
    //See if there are pending events.
    return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
    //Get a pending event.
    XEvent e;
    XNextEvent(dpy, &e);
    return e;
}

void X11_wrapper::swapBuffers()
{
    glXSwapBuffers(dpy, win);
}

void X11_wrapper::reshape_window(int width, int height)
{
    //window has been resized.
    g.xres = width;
    g.yres = height;
    //
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
}

void X11_wrapper::check_resize(XEvent *e)
{
    //The ConfigureNotify is sent by the
    //server if the window is resized.
    if (e->type != ConfigureNotify)
        return;
    XConfigureEvent xce = e->xconfigure;
    if (xce.width != g.xres || xce.height != g.yres) {
        //Window size did change.
        reshape_window(xce.width, xce.height);
    }
}
//-----------------------------------------------------------------------------

void X11_wrapper::check_mouse(XEvent *e)
{
    static int savex = 0;
    static int savey = 0;

    //Weed out non-mouse events
    if (e->type != ButtonRelease &&
        e->type != ButtonPress &&
        e->type != MotionNotify) {
        //This is not a mouse event that we care about.
        return;
    }
    //
    if (e->type == ButtonRelease) {
        return;
    }
    if (e->type == ButtonPress) {
        if (e->xbutton.button==1) {
            //Left button was pressed.
            //int y = g.yres - e->xbutton.y;
            return;
        }
        if (e->xbutton.button==3) {
            //Right button was pressed.
            return;
        }
    }
    if (e->type == MotionNotify) {
        //The mouse moved!
        if (savex != e->xbutton.x || savey != e->xbutton.y) {
            savex = e->xbutton.x;
            savey = e->xbutton.y;
            //Code placed here will execute whenever the mouse moves.


        }
    }
}

int X11_wrapper::check_keys(XEvent *e)
{
    if (e->type != KeyPress && e->type != KeyRelease)
        return 0;
    int key = XLookupKeysym(&e->xkey, 0);
    if (e->type == KeyPress) {
        switch (key) {
            case XK_1:
                //Key 1 was pressed
                break;
            case XK_Escape:
                //Escape key was pressed
                return 1;
        }
    }
    return 0;
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, g.xres, g.yres);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, g.xres, 0, g.yres, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
}

void physics()
{
    //Following function controls the box's motion.
    //Increments g.dir to the position every time this function is called.
    g.pos[0] += g.dir;
    //If the box reaches the rightmost boundary of the window
    if (g.pos[0] >= (g.xres-g.w)) {
        g.pos[0] = (g.xres-g.w);
        //Direction is reversed
        g.dir = -g.dir;
    }
    //If the box reaches the leftmost boundary of the window
    if (g.pos[0] <= g.w) {
        g.pos[0] = g.w;
        //Direction is reversed
        g.dir = -g.dir;
    }
}

void render(int xResPrev, GLfloat & red, GLfloat & green, GLfloat & blue, GLfloat & pRed)
{
    //This will reset itself to the original red value everytime the
    //function runs.
    if (g.pos[0] == (g.xres / 2)) {
        pRed = red;
    }
    glClear(GL_COLOR_BUFFER_BIT);
    //If the resolution at the x- and y-axis is smaller than the box width,
    //the function will automatically end and the box will not render, making
    //it disappear.
    if (g.xres < 2 * g.w || g.yres < 2 * g.w) {
        return;
    }
    //Draw box.
    glPushMatrix();
    //The first time render() is called, the initial values for color declared
    //in main() will be passed as parameters of glColor3f().
    glColor3f(red, green, blue);
    glTranslatef(g.pos[0], g.pos[1], 0.0f);
    glBegin(GL_QUADS);
        glVertex2f(-g.w, -g.w);
        glVertex2f(-g.w,  g.w);
        glVertex2f( g.w,  g.w);
        glVertex2f( g.w, -g.w);
    glEnd();
    glPopMatrix();
    //The box gets slower as the resolution increases; if the current resolution
    //is higher than the previous one, it will turn bluer.
    if (xResPrev < g.xres) {
        blue += 0.1f;
        red  -= 0.1f;
        //Both colors are restricted to the range of 0.0f-1.0f.
        if (red < 0.0f) {
            red = 0.0f;
        }
        if (blue > 1.0f) {
            blue = 1.0f;
        }
    }
    //The box gets faster as the resolution decreases; if the current resolution
    //is lower than the previous one, it will turn redder.
    if (xResPrev > g.xres) {
        red  += 0.1f;
        blue -= 0.1f;
        //Both colors are restricted to the range of 0.0f-1.0f.
        if (red > 1.0f) {
            red = 1.0f;
        }
        if (blue < 0.0f) {
            blue = 0.0f;
        }
    }
    //This will make the box redder if it touches the window edge.
    if ((g.pos[0] + g.w) == g.xres) {
        red = 1.0f;
    }
    if ((g.pos[0] - g.w) == 0) {
        red = 1.0f;
    }
    if (((g.pos[0] + g.w) < (g.xres - 30)) && ((g.pos[0] - g.w) > 30)) {
        red = pRed;
    }
}
