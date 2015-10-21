// glfw_main.cpp

#include <GL/glew.h>

#if defined(_WIN32)
#  include <Windows.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <stdio.h>
#include <string.h>
#include <sstream>
#include <algorithm>

#include "Timer.h"
#include "FPSTimer.h"
#include "Logger.h"
#include "ShaderWithVariables.h"

Timer g_timer;
double g_lastFrameTime = 0.0;
FPSTimer g_fps;

// mouse motion internal state
int oldx, oldy, newx, newy;
int which_button = -1;
int modifier_mode = 0;

GLFWwindow* g_pWindow = NULL;

ShaderWithVariables m_basic;
ShaderWithVariables m_plane;
const glm::ivec2 vp(1000, 800);

static void ErrorCallback(int p_Error, const char* p_Description)
{
    (void)p_Error;
    (void)p_Description;
    LOG_INFO("ERROR: %d, %s", p_Error, p_Description);
}

///@brief While the basic VAO is bound, gen and bind all buffers and attribs.
void _InitCubeAttributes()
{
    const glm::vec3 minPt(0, 0, 0);
    const glm::vec3 maxPt(1, 1, 1);
    const glm::vec3 verts[] = {
        minPt,
        glm::vec3(maxPt.x, minPt.y, minPt.z),
        glm::vec3(maxPt.x, maxPt.y, minPt.z),
        glm::vec3(minPt.x, maxPt.y, minPt.z),
        glm::vec3(minPt.x, minPt.y, maxPt.z),
        glm::vec3(maxPt.x, minPt.y, maxPt.z),
        maxPt,
        glm::vec3(minPt.x, maxPt.y, maxPt.z)
    };

    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_basic.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_basic.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_basic.AddVbo("vColor", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, 8 * 3 * sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_basic.GetAttrLoc("vColor"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_basic.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_basic.GetAttrLoc("vColor"));

    const unsigned int quads[] = {
        0, 3, 2, 1, 0, 2, // ccw
        4, 5, 6, 7, 4, 6,
        1, 2, 6, 5, 1, 6,
        2, 3, 7, 6, 2, 7,
        3, 0, 4, 7, 3, 4,
        0, 1, 5, 4, 0, 5,
    };
    GLuint quadVbo = 0;
    glGenBuffers(1, &quadVbo);
    m_basic.AddVbo("elements", quadVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 12 * 3 * sizeof(GLuint), quads, GL_STATIC_DRAW);
}

///@brief While the basic VAO is bound, gen and bind all buffers and attribs.
void _InitPlaneAttributes()
{
    const glm::vec3 minPt(-10.0f, 0.0f, -10.0f);
    const glm::vec3 maxPt(10.0f, 0.0f, 10.0f);
    const float verts[] = {
        minPt.x, minPt.y, minPt.z,
        minPt.x, minPt.y, maxPt.z,
        maxPt.x, minPt.y, maxPt.z,
        maxPt.x, minPt.y, minPt.z,
    };
    GLuint vertVbo = 0;
    glGenBuffers(1, &vertVbo);
    m_plane.AddVbo("vPosition", vertVbo);
    glBindBuffer(GL_ARRAY_BUFFER, vertVbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(m_plane.GetAttrLoc("vPosition"), 3, GL_FLOAT, GL_FALSE, 0, NULL);

    const float texs[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };
    GLuint colVbo = 0;
    glGenBuffers(1, &colVbo);
    m_plane.AddVbo("vTexCoord", colVbo);
    glBindBuffer(GL_ARRAY_BUFFER, colVbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(GLfloat), texs, GL_STATIC_DRAW);
    glVertexAttribPointer(m_plane.GetAttrLoc("vTexCoord"), 2, GL_FLOAT, GL_FALSE, 0, NULL);

    glEnableVertexAttribArray(m_plane.GetAttrLoc("vPosition"));
    glEnableVertexAttribArray(m_plane.GetAttrLoc("vTexCoord"));

    const unsigned int tris[] = {
        0, 3, 2, 1, 0, 2, // ccw
    };
    GLuint triVbo = 0;
    glGenBuffers(1, &triVbo);
    m_plane.AddVbo("elements", triVbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triVbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * 3 * sizeof(GLuint), tris, GL_STATIC_DRAW);
}

void initGL()
{
    m_plane.initProgram("basicplane");
    m_plane.bindVAO();
    _InitPlaneAttributes();
    glBindVertexArray(0);
}

void keyboard(GLFWwindow* pWindow, int key, int codes, int action, int mods)
{
    (void)pWindow;
    (void)codes;

    if (action == GLFW_PRESS)
    {
    switch (key)
    {
        default:
            break;

        case GLFW_KEY_SPACE:
            //g_app.RecenterPose();
            break;

        case GLFW_KEY_ESCAPE:
            {
                glfwDestroyWindow(g_pWindow);
                glfwTerminate();
                exit(0);
            }
            break;
        }
    }

}

void mouseDown(GLFWwindow* pWindow, int button, int action, int mods)
{
    (void)mods;

    double xd, yd;
    glfwGetCursorPos(pWindow, &xd, &yd);
    const int x = static_cast<int>(xd);
    const int y = static_cast<int>(yd);

    which_button = button;
    oldx = newx = x;
    oldy = newy = y;
    if (action == GLFW_RELEASE)
    {
        which_button = -1;
    }
}

void mouseMove(GLFWwindow* pWindow, double xd, double yd)
{
    glfwGetCursorPos(pWindow, &xd, &yd);
    const int x = static_cast<int>(xd);
    const int y = static_cast<int>(yd);

    oldx = newx;
    oldy = newy;
    newx = x;
    newy = y;
    const int mmx = x-oldx;
    const int mmy = y-oldy;

    if (which_button == GLFW_MOUSE_BUTTON_1)
    {
    }
}

void mouseWheel(GLFWwindow* pWindow, double x, double y)
{
    (void)pWindow;
    (void)x;

    const int delta = static_cast<int>(y);
}

void resize(GLFWwindow* pWindow, int w, int h)
{
    (void)pWindow;
}

void display()
{
    const float lum = .6f;
    glClearColor(lum, lum, lum, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const glm::mat4 lookat = glm::lookAt(
        glm::vec3(1.f, 1.f, 1.f),
        glm::vec3(0.f),
        glm::vec3(0.f, 1.f, 0.f)
        );

    const glm::mat4 persp = glm::perspective(
        90.0f,
        static_cast<float>(vp.x) / static_cast<float>(vp.y),
        0.004f,
        500.0f);


    glUseProgram(m_plane.prog());
    {
        glUniformMatrix4fv(m_plane.GetUniLoc("mvmtx"), 1, false, glm::value_ptr(lookat));
        glUniformMatrix4fv(m_plane.GetUniLoc("prmtx"), 1, false, glm::value_ptr(persp));

        m_plane.bindVAO();
        {
            // floor
            glDrawElements(GL_TRIANGLES,
                3 * 2, // 2 triangle pairs
                GL_UNSIGNED_INT,
                0);
        }
        glBindVertexArray(0);

    }
    glUseProgram(0);
}

void timestep()
{
    const double absT = g_timer.seconds();
    const double dt = absT - g_lastFrameTime;
    g_lastFrameTime = absT;
}

// OpenGL debug callback
void GLAPIENTRY myCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar *msg,
    const void *data)
{
    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
    case GL_DEBUG_SEVERITY_MEDIUM:
    case GL_DEBUG_SEVERITY_LOW:
        LOG_INFO("[[GL Debug]] %x %x %x %x %s", source, type, id, severity, msg);
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        break;
    }
}

int main(int argc, char** argv)
{
    bool useOpenGLCoreContext = true;

    GLFWwindow* l_Window = NULL;
    glfwSetErrorCallback(ErrorCallback);
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }

    // Context setup - before window creation
    glfwWindowHint(GLFW_DEPTH_BITS, 16);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, useOpenGLCoreContext ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    // Create a normal, decorated application window
    const std::string windowTitle = PROJECT_NAME "-GLFW-NoVRSDK";

    l_Window = glfwCreateWindow(vp.x, vp.y, windowTitle.c_str(), NULL, NULL);

    if (!l_Window)
    {
        LOG_INFO("Glfw failed to create a window. Exiting.");
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(l_Window);
    glfwSetWindowSizeCallback(l_Window, resize);
    glfwSetMouseButtonCallback(l_Window, mouseDown);
    glfwSetCursorPosCallback(l_Window, mouseMove);
    glfwSetScrollCallback(l_Window, mouseWheel);
    glfwSetKeyCallback(l_Window, keyboard);

    glfwMakeContextCurrent(l_Window);
    g_pWindow = l_Window;


    // Don't forget to initialize Glew, turn glewExperimental on to
    // avoid problems fetching function pointers...
    glewExperimental = GL_TRUE;
    const GLenum l_Result = glewInit();
    if (l_Result != GLEW_OK)
    {
        LOG_INFO("glewInit() error.");
        exit(EXIT_FAILURE);
    }

#ifdef _DEBUG
    // Debug callback initialization
    // Must be done *after* glew initialization.
    glDebugMessageCallback(myCallback, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
        GL_DEBUG_SEVERITY_NOTIFICATION, -1 , "Start debugging");
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

    initGL();
    glfwSwapInterval(0);

    while (!glfwWindowShouldClose(l_Window))
    {
        glfwPollEvents();
        timestep();
        g_fps.OnFrame();

        display();
        glfwSwapBuffers(g_pWindow);

#ifndef _LINUX
        // Indicate FPS in window title
        // This is absolute death for performance in Ubuntu Linux 12.04
        {
            std::ostringstream oss;
            oss << windowTitle
                << " "
                << static_cast<int>(g_fps.GetFPS())
                << " fps";
            glfwSetWindowTitle(l_Window, oss.str().c_str());
        }
#endif
    }

    glfwDestroyWindow(l_Window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}
