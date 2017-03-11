#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <map>

#include <GL/glew.h>
#include <OpenGL/gl.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

struct VAO
{
   GLuint VertexArrayID;
   GLuint VertexBuffer;
   GLuint ColorBuffer;

   GLenum PrimitiveMode;
   GLenum FillMode;
   int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices
{
   glm::mat4 projection, projectionP;
   glm::mat4 model;
   glm::mat4 view;
   GLuint MatrixID;
} Matrices;
struct COLOR
{
   float r;
   float g;
   float b;
};
typedef struct COLOR COLOR;

struct Sprite
{
   string name;
   COLOR color;
   float x, y, z;
   VAO *object;
   int status;
   int type;
   float height, width, depth;
   float x_speed, y_speed;
   float angle; //Current Angle (Actual rotated angle of the object)
   int inAir;
   float radius;
   int fixed;
   float friction; //Value from 0 to 1
   int health;
   int isRotating;
   int direction;  //0 for clockwise and 1 for anticlockwise for animation
   float remAngle; //the remaining angle to finish animation
   int isMovingAnim;
   int dx;
   int dy;
   double last_refl_time;
   float weight;
};

typedef struct Sprite Sprite;
map<string, Sprite> tiles;
map<string, Sprite> blocks;
map<string, Sprite> chut;
map<string, Sprite> scores;
GLuint programID;
int proj_type;
glm::vec3 tri_pos, rect_pos, cube_pos;
int flag;
int cunt;
int fl = 0;
int score = 0;
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char *vertex_file_path, const char *fragment_file_path)
{

   // Create the shaders
   GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
   GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

   // Read the Vertex Shader code from the file
   std::string VertexShaderCode;
   std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
   if (VertexShaderStream.is_open())
   {
      std::string Line = "";
      while (getline(VertexShaderStream, Line))
         VertexShaderCode += "\n" + Line;
      VertexShaderStream.close();
   }

   // Read the Fragment Shader code from the file
   std::string FragmentShaderCode;
   std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
   if (FragmentShaderStream.is_open())
   {
      std::string Line = "";
      while (getline(FragmentShaderStream, Line))
         FragmentShaderCode += "\n" + Line;
      FragmentShaderStream.close();
   }

   GLint Result = GL_FALSE;
   int InfoLogLength;

   // Compile Vertex Shader
   //    printf("Compiling shader : %s\n", vertex_file_path);
   char const *VertexSourcePointer = VertexShaderCode.c_str();
   glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
   glCompileShader(VertexShaderID);

   // Check Vertex Shader
   glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
   glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
   std::vector<char> VertexShaderErrorMessage(InfoLogLength);
   glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
   //    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

   // Compile Fragment Shader
   //    printf("Compiling shader : %s\n", fragment_file_path);
   char const *FragmentSourcePointer = FragmentShaderCode.c_str();
   glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
   glCompileShader(FragmentShaderID);

   // Check Fragment Shader
   glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
   glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
   std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
   glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
   //    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

   // Link the program
   //    fprintf(stdout, "Linking program\n");
   GLuint ProgramID = glCreateProgram();
   glAttachShader(ProgramID, VertexShaderID);
   glAttachShader(ProgramID, FragmentShaderID);
   glLinkProgram(ProgramID);

   // Check the program
   glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
   glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
   std::vector<char> ProgramErrorMessage(max(InfoLogLength, int(1)));
   glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
   //    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

   glDeleteShader(VertexShaderID);
   glDeleteShader(FragmentShaderID);

   return ProgramID;
}

static void error_callback(int error, const char *description)
{
   fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
   glfwDestroyWindow(window);
   glfwTerminate();
   exit(EXIT_SUCCESS);
}

void initGLEW(void)
{
   glewExperimental = GL_TRUE;
   if (glewInit() != GLEW_OK)
   {
      fprintf(stderr, "Glew failed to initialize : %s\n", glewGetErrorString(glewInit()));
   }
   if (!GLEW_VERSION_3_3)
      fprintf(stderr, "3.3 version not available\n");
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO *create3DObject(GLenum primitive_mode, int numVertices, const GLfloat *vertex_buffer_data, const GLfloat *color_buffer_data, GLenum fill_mode = GL_FILL)
{
   struct VAO *vao = new struct VAO;
   vao->PrimitiveMode = primitive_mode;
   vao->NumVertices = numVertices;
   vao->FillMode = fill_mode;

   // Create Vertex Array Object
   // Should be done after CreateWindow and before any other GL calls
   glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
   glGenBuffers(1, &(vao->VertexBuffer));       // VBO - vertices
   glGenBuffers(1, &(vao->ColorBuffer));        // VBO - colors

   glBindVertexArray(vao->VertexArrayID);                                                                // Bind the VAO
   glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);                                                     // Bind the VBO vertices
   glBufferData(GL_ARRAY_BUFFER, 3 * numVertices * sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
   glVertexAttribPointer(
       0,        // attribute 0. Vertices
       3,        // size (x,y,z)
       GL_FLOAT, // type
       GL_FALSE, // normalized?
       0,        // stride
       (void *)0 // array buffer offset
       );

   glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);                                                     // Bind the VBO colors
   glBufferData(GL_ARRAY_BUFFER, 3 * numVertices * sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW); // Copy the vertex colors
   glVertexAttribPointer(
       1,        // attribute 1. Color
       3,        // size (r,g,b)
       GL_FLOAT, // type
       GL_FALSE, // normalized?
       0,        // stride
       (void *)0 // array buffer offset
       );

   return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO *create3DObject(GLenum primitive_mode, int numVertices, const GLfloat *vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode = GL_FILL)
{
   GLfloat *color_buffer_data = new GLfloat[3 * numVertices];
   for (int i = 0; i < numVertices; i++)
   {
      color_buffer_data[3 * i] = red;
      color_buffer_data[3 * i + 1] = green;
      color_buffer_data[3 * i + 2] = blue;
   }

   return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}
int clk=0;
/* Render the VBOs handled by VAO */
void draw3DObject(struct VAO *vao)
{
   // Change the Fill Mode for this object
   glPolygonMode(GL_FRONT_AND_BACK, vao->FillMode);

   // Bind the VAO to use
   glBindVertexArray(vao->VertexArrayID);

   // Enable Vertex Attribute 0 - 3d Vertices
   glEnableVertexAttribArray(0);
   // Bind the VBO to use
   glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

   // Enable Vertex Attribute 1 - Color
   glEnableVertexAttribArray(1);
   // Bind the VBO to use
   glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

   // Draw the geometry !
   glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
* Customizable functions *
**************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
int rectangle_rot_status = 0;
int j = 0;
int jump_status = 0;
int ve = 1;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard(GLFWwindow *window, int key, int scancode, int action, int mods)
{
   // Function is called first on GLFW_PRESS.

   if (action == GLFW_RELEASE)
   {
      switch (key)
      {
      case GLFW_KEY_W:
         cunt = 0;
         if (rectangle_rot_status == -1)
            rectangle_rot_status = 0;
         else if(rectangle_rot_status>0)
            break;
         else 
            rectangle_rot_status = 2;
         j = 0;
         break;
      case GLFW_KEY_S:
         if (rectangle_rot_status == -1)
            rectangle_rot_status = 0;
         else if(rectangle_rot_status>0)
            break;
         else
            rectangle_rot_status = 4;
         j = 0;
         cunt = 0;
         break;
      case GLFW_KEY_A:
         if (rectangle_rot_status == -1)
            rectangle_rot_status = 0;
         else if(rectangle_rot_status>0)
            break;
         else
            rectangle_rot_status = 1;
         j = 0;
         cunt = 0;
         // do something ..
         break;
      case GLFW_KEY_D:
         if (rectangle_rot_status == -1)
            rectangle_rot_status = 0;
         else if(rectangle_rot_status>0)
            break;
         else
            rectangle_rot_status = 3;
         cunt = 0;
         j = 0;
         break;
      case GLFW_KEY_L:
         ve = 2;
         break;
      case GLFW_KEY_T:
         ve = 1;
         break;
      case GLFW_KEY_P:
         ve = 3;
         break;
      case GLFW_KEY_B:
         ve = 4;
         break;
      case GLFW_KEY_F:
         ve = 5;
         break;
      case GLFW_KEY_H:
         ve = 6;
         break; 
      case GLFW_KEY_SPACE:
         if (j == 1)
            jump_status = 1;
         else if (j == 2)
            jump_status = 2;
         else if (j == 3)
            jump_status = 3;
         else if (j == 4)
            jump_status = 4;
         j = 0;
      default:
         break;
      }
   }
   else if (action == GLFW_PRESS)
   {
      switch (key)
      {
      case GLFW_KEY_ESCAPE:
         quit(window);
         break;
      case GLFW_KEY_W:
         j = 1;
         break;
      case GLFW_KEY_A:
         j = 3;
         break;
      case GLFW_KEY_S:
         j = 2;
         break;
      case GLFW_KEY_D:
         j = 4;
         break;
      default:
         break;
      }
   }
}

/* Executed for character input (like in text boxes) */
void keyboardChar(GLFWwindow *window, unsigned int key)
{
   switch (key)
   {
   case 'Q':
   case 'q':
      quit(window);
      break;
   case ' ':
      proj_type ^= 1;
      break;
   case 'a':
      tri_pos.x -= 0.2;
      break;
   case 'd':
      tri_pos.x += 0.2;
      break;
   case 'w':
      tri_pos.y += 0.2;
      break;
   case 's':
      tri_pos.y -= 0.2;
      break;
   case 'f':
      tri_pos.z += 0.2;
      break;
   case 'r':
      tri_pos.z -= 0.2;
      break;
   case 'j':
      rect_pos.x -= 0.2;
      break;
   case 'l':
      rect_pos.x += 0.2;
      break;
   case 'i':
      rect_pos.y += 0.2;
      break;
   case 'k':
      rect_pos.y -= 0.2;
      break;
   case 'y':
      rect_pos.z -= 0.2;
      break;
   case 'h':
      rect_pos.z += 0.2;
      break;
   default:
      break;
   }
}
int gand;
/* Executed when a mouse button is pressed/released */
void mouseButton(GLFWwindow *window, int button, int action, int mods)
{
   switch (button)
   {
   case GLFW_MOUSE_BUTTON_LEFT:
      if (action == GLFW_RELEASE)
         clk=0;
      //break;
      else
        clk=1;
        break;
   case GLFW_MOUSE_BUTTON_RIGHT:
      if (action == GLFW_RELEASE)
      {
         rectangle_rot_dir *= -1;
      }
      break;
    case GLFW_CURSOR_DISABLED:
        gand=1;
        break;
        
   default:
      break;
   }
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow(GLFWwindow *window, int width, int height)
{
   int fbwidth = width, fbheight = height;
   glfwGetFramebufferSize(window, &fbwidth, &fbheight);

   GLfloat fov = M_PI / 2;

   // sets the viewport of openGL renderer
   glViewport(0, 0, (GLsizei)fbwidth, (GLsizei)fbheight);

   // Store the projection matrix in a variable for future use
   // Perspective projection for 3D views
   Matrices.projection = glm::perspective(fov, (GLfloat)fbwidth / (GLfloat)fbheight, 0.1f, 500.0f);

   // Ortho projection for 2D views
   //Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle, *cube;

// Creates the triangle object used in this sample code
void createTriangle()

{
   /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

   /* Define vertex array as used in glBegin (GL_TRIANGLES) */
   static const GLfloat vertex_buffer_data[] = {
       0, 0, 0,  // vertex 0
       -1, 1, 0, // vertex 1
       -1, 0, 0, // vertex 2
   };

   static const GLfloat color_buffer_data[] = {
       1, 0, 0, // color 0
       1, 0, 0, // color 1
       1, 0, 0, // color 2
   };

   // create3DObject creates and returns a handle to a VAO that can be used later
   triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createCube1(float length, float breadth, float height, COLOR color, COLOR color1, COLOR color2, float x, float y, float z, string Component, string name, int status, int type)
{
   float l = length / 2;
   float w = breadth / 2;
   float h = height / 2;
   GLfloat vertex_buffer_data[] = {
       // GL3 accepts only Triangles. Quads are not supported
       -l, -w, 0, // vertex 1
       l, -w, 0,  // vertex 2
       l, w, 0,   // vertex 3

       l, w, 0,   // vertex 3
       -l, w, 0,  // vertex 4
       -l, -w, 0, // vertex 1

       -l, -w, height, // vertex 1
       l, -w, height,  // vertex 2
       l, w, height,   // vertex 3

       l, w, height,   // vertex 3
       -l, w, height,  // vertex 4
       -l, -w, height, // vertex 1

       l, -w, 0,      // vertex 1
       l, -w, height, // vertex 2
       l, w, height,  // vertex 3

       l, w, height, // vertex 3
       l, w, 0,      // vertex 4
       l, -w, 0,     // vertex 1

       -l, -w, 0,      // vertex 1
       -l, -w, height, // vertex 2
       -l, w, height,  // vertex 3

       -l, w, height, // vertex 3
       -l, w, 0,      // vertex 4
       -l, -w, 0,     // vertex 1

       -l, -w, 0,     // vertex 1
       l, -w, 0,      // vertex 2
       l, -w, height, // vertex 3

       l, -w, height,  // vertex 3
       -l, -w, height, // vertex 4
       -l, -w, 0,      // vertex 1

       -l, w, 0,     // vertex 1
       l, w, 0,      // vertex 2
       l, w, height, // vertex 3

       l, w, height,  // vertex 3
       -l, w, height, // vertex 4
       -l, w, 0,
   };
   //GLfloat color_buffer_data [];
   if (Component == "block")
   {
      GLfloat color_buffer_data[] = {
          color.r, color.g, color.b, // color 1
          color.r, color.g, color.b, // color 2
          color.r, color.g, color.b, // color 3
          color.r, color.g, color.b, // color 3
          color.r, color.g, color.b, // color 4
          color.r, color.g, color.b, // color 1
          color.r, color.g, color.b, // color 1
          color.r, color.g, color.b, // color 2
          color.r, color.g, color.b, // color 3
          color.r, color.g, color.b, // color 3
          color.r, color.g, color.b, // color 4
          color.r, color.g, color.b, // color 1

          color2.r, color2.g, color2.b, // color2 1
          color2.r, color2.g, color2.b, // color2 2
          color2.r, color2.g, color2.b, // color2 3
          color2.r, color2.g, color2.b, // color2 3
          color2.r, color2.g, color2.b, // color2 4
          color2.r, color2.g, color2.b, // color2 1
          color2.r, color2.g, color2.b, // color2 1
          color2.r, color2.g, color2.b, // color2 2
          color2.r, color2.g, color2.b, // color2 3
          color2.r, color2.g, color2.b, // color2 3
          color2.r, color2.g, color2.b, // color2 4
          color2.r, color2.g, color2.b, // color 1

          color1.r, color1.g, color1.b, // color2 1
          color1.r, color1.g, color1.b, // color1 2
          color1.r, color1.g, color1.b, // color1 3
          color1.r, color1.g, color1.b, // color1 3
          color1.r, color1.g, color1.b, // color1 4
          color1.r, color1.g, color1.b, // color1 1
          color1.r, color1.g, color1.b, // color1 1
          color1.r, color1.g, color1.b, // color1 2
          color1.r, color1.g, color1.b, // color1 3
          color1.r, color1.g, color1.b, // color1 3
          color1.r, color1.g, color1.b, // color1 4
          color1.r, color1.g, color1.b, // color 1
      };
      cube = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   }
   else
   {
      GLfloat color_buffer_data[] = {
          color.r, color.g, color.b,       // color 1
          color.r, color.g, color.b + 0.2, // color 2
          color.r, color.g, color.b + 0.4, // color 3
          color.r, color.g, color.b + 0.4, // color 3
          color.r, color.g, color.b + 0.6, // color 4
          color.r, color.g, color.b + 0.8, // color 1
          color.r, color.g, color.b,       // color 1
          color.r, color.g, color.b + 0.2, // color 2
          color.r, color.g, color.b + 0.4, // color 3
          color.r, color.g, color.b + 0.4, // color 3
          color.r, color.g, color.b + 0.6, // color 4
          color.r, color.g, color.b + 0.8, // color 1

          color1.r, color1.g, color1.b,       // color1 1
          color1.r, color1.g, color1.b + 0.2, // color1 2
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.6, // color1 4
          color1.r, color1.g, color1.b + 0.8, // color1 1
          color1.r, color1.g, color1.b,       // color1 1
          color1.r, color1.g, color1.b + 0.2, // color1 2
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.6, // color1 4
          color1.r, color1.g, color1.b + 0.8, // color 1

          color1.r, color1.g, color1.b,       // color1 1
          color1.r, color1.g, color1.b + 0.2, // color1 2
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.6, // color1 4
          color1.r, color1.g, color1.b + 0.8, // color1 1
          color1.r, color1.g, color1.b,       // color1 1
          color1.r, color1.g, color1.b + 0.2, // color1 2
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.6, // color1 4
          color1.r, color1.g, color1.b + 0.8, // color 1
      };
      cube = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   }
   // create3DObject creates and returns a handle to a VAO that can be used later

   Sprite suhansprite = {};
   suhansprite.color = color;
   suhansprite.name = name;
   suhansprite.object = cube;
   suhansprite.x = x;
   suhansprite.y = y;
   suhansprite.z = z;
   suhansprite.height = length;
   suhansprite.width = breadth;
   suhansprite.depth = height;
   if (Component == "tile")
   {
      suhansprite.status = status;
      suhansprite.type = type;
      tiles[name] = suhansprite;
   }
   else if (Component == "block")
   {
      suhansprite.status = status;
      suhansprite.remAngle = 2;
      blocks[name] = suhansprite;
   }
   else
   {
      chut[name] = suhansprite;
   }
}
// Creates the rectangle object used in this sample code
void createCube(float length, float breadth, float height, COLOR color, COLOR color1, COLOR color2, float x, float y, float z, string Component, string name, int status, int type)
{
   float l = length / 2;
   float w = breadth / 2;
   float h = height / 2;

   // GL3 accepts only Triangles. Quads are not supported
   GLfloat vertex_buffer_data[] = {
       -l, -w, -h, // vertex 1
       l, -w, -h,  // vertex 2
       l, w, -h,   // vertex 3

       l, w, -h,   // vertex 3
       -l, w, -h,  // vertex 4
       -l, -w, -h, // vertex 1

       -l, -w, h, // vertex 1
       l, -w, h,  // vertex 2
       l, w, h,   // vertex 3

       l, w, h,   // vertex 3
       -l, w, h,  // vertex 4
       -l, -w, h, // vertex 1

       l, -w, -h, // vertex 1
       l, -w, h,  // vertex 2
       l, w, h,   // vertex 3

       l, w, h,   // vertex 3
       l, w, -h,  // vertex 4
       l, -w, -h, // vertex 1

       -l, -w, -h, // vertex 1
       -l, -w, h,  // vertex 2
       -l, w, h,   // vertex 3

       -l, w, h,   // vertex 3
       -l, w, -h,  // vertex 4
       -l, -w, -h, // vertex 1

       -l, -w, -h, // vertex 1
       l, -w, -h,  // vertex 2
       l, -w, h,   // vertex 3

       l, -w, h,   // vertex 3
       -l, -w, h,  // vertex 4
       -l, -w, -h, // vertex 1

       -l, w, -h, // vertex 1
       l, w, -h,  // vertex 2
       l, w, h,   // vertex 3

       l, w, h,   // vertex 3
       -l, w, h,  // vertex 4
       -l, w, -h, // vertex 1
   };
   //GLfloat color_buffer_data [];
   if (Component == "block")
   {
      GLfloat color_buffer_data[] = {
          color.r, color.g, color.b, // color 1
          color.r, color.g, color.b, // color 2
          color.r, color.g, color.b, // color 3
          color.r, color.g, color.b, // color 3
          color.r, color.g, color.b, // color 4
          color.r, color.g, color.b, // color 1
          color.r, color.g, color.b, // color 1
          color.r, color.g, color.b, // color 2
          color.r, color.g, color.b, // color 3
          color.r, color.g, color.b, // color 3
          color.r, color.g, color.b, // color 4
          color.r, color.g, color.b, // color 1

          color2.r, color2.g, color2.b, // color2 1
          color2.r, color2.g, color2.b, // color2 2
          color2.r, color2.g, color2.b, // color2 3
          color2.r, color2.g, color2.b, // color2 3
          color2.r, color2.g, color2.b, // color2 4
          color2.r, color2.g, color2.b, // color2 1
          color2.r, color2.g, color2.b, // color2 1
          color2.r, color2.g, color2.b, // color2 2
          color2.r, color2.g, color2.b, // color2 3
          color2.r, color2.g, color2.b, // color2 3
          color2.r, color2.g, color2.b, // color2 4
          color2.r, color2.g, color2.b, // color 1

          color1.r, color1.g, color1.b, // color2 1
          color1.r, color1.g, color1.b, // color1 2
          color1.r, color1.g, color1.b, // color1 3
          color1.r, color1.g, color1.b, // color1 3
          color1.r, color1.g, color1.b, // color1 4
          color1.r, color1.g, color1.b, // color1 1
          color1.r, color1.g, color1.b, // color1 1
          color1.r, color1.g, color1.b, // color1 2
          color1.r, color1.g, color1.b, // color1 3
          color1.r, color1.g, color1.b, // color1 3
          color1.r, color1.g, color1.b, // color1 4
          color1.r, color1.g, color1.b, // color 1
      };
      cube = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   }
   else
   {
      GLfloat color_buffer_data[] = {
          color.r, color.g, color.b,       // color 1
          color.r, color.g, color.b + 0.2, // color 2
          color.r, color.g, color.b + 0.4, // color 3
          color.r, color.g, color.b + 0.4, // color 3
          color.r, color.g, color.b + 0.6, // color 4
          color.r, color.g, color.b + 0.8, // color 1
          color.r, color.g, color.b,       // color 1
          color.r, color.g, color.b + 0.2, // color 2
          color.r, color.g, color.b + 0.4, // color 3
          color.r, color.g, color.b + 0.4, // color 3
          color.r, color.g, color.b + 0.6, // color 4
          color.r, color.g, color.b + 0.8, // color 1

          color1.r, color1.g, color1.b,       // color1 1
          color1.r, color1.g, color1.b + 0.2, // color1 2
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.6, // color1 4
          color1.r, color1.g, color1.b + 0.8, // color1 1
          color1.r, color1.g, color1.b,       // color1 1
          color1.r, color1.g, color1.b + 0.2, // color1 2
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.6, // color1 4
          color1.r, color1.g, color1.b + 0.8, // color 1

          color1.r, color1.g, color1.b,       // color1 1
          color1.r, color1.g, color1.b + 0.2, // color1 2
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.6, // color1 4
          color1.r, color1.g, color1.b + 0.8, // color1 1
          color1.r, color1.g, color1.b,       // color1 1
          color1.r, color1.g, color1.b + 0.2, // color1 2
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.4, // color1 3
          color1.r, color1.g, color1.b + 0.6, // color1 4
          color1.r, color1.g, color1.b + 0.8, // color 1
      };
      cube = create3DObject(GL_TRIANGLES, 36, vertex_buffer_data, color_buffer_data, GL_FILL);
   }
   // create3DObject creates and returns a handle to a VAO that can be used later

   Sprite suhansprite = {};
   suhansprite.color = color;
   suhansprite.name = name;
   suhansprite.object = cube;
   suhansprite.x = x;
   suhansprite.y = y;
   suhansprite.z = z;
   suhansprite.height = length;
   suhansprite.width = breadth;
   suhansprite.depth = height;
   if (Component == "tile")
   {
      suhansprite.status = status;
      suhansprite.type = type;
      tiles[name] = suhansprite;
   }
   else if (Component == "block")
   {
      suhansprite.status = status;
      suhansprite.remAngle = 2;
      blocks[name] = suhansprite;
   }
   else
   {
      chut[name] = suhansprite;
   }
}
void createRectangles(float length, float breadth, COLOR color, float x, float y, string Component, string name)
{
   // GL3 accepts only Triangles. Quads are not supported
   //float
   float h = length / 2;
   float w = breadth / 2;
   GLfloat vertex_buffer_data[] = {
       -h, -w, 0, // vertex 1
       h, -w, 0,  // vertex 2
       h, w, 0,   // vertex 3

       h, w, 0,  // vertex 3
       -h, w, 0, // vertex 4
       -h, -w, 0 // vertex 1
   };

   GLfloat color_buffer_data[] = {
       color.r, color.g, color.b, // color 1
       color.r, color.g, color.b, // color 2
       color.r, color.g, color.b, // color 3

       color.r, color.g, color.b, // color 3
       color.r, color.g, color.b, // color 4
       color.r, color.g, color.b, // color 1
   };
   // create3DObject creates and returns a handle to a VAO that can be used later
   VAO *rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
   Sprite suhansprite = {};
   suhansprite.color = color;
   suhansprite.name = name;
   suhansprite.object = rectangle;
   suhansprite.x = x;
   suhansprite.y = y;
   suhansprite.height = length;
   suhansprite.width = breadth;
   if (Component == "score")
   {
      suhansprite.status = 0;
      scores[name] = suhansprite;
   }
}
float rad;
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

void createscore()
{
   //cout << score << "\n";
   int k, digits, r;
   for (map<string, Sprite>::iterator it = scores.begin(); it != scores.end(); it++)
   {
      string current = it->first; //The name of the current object
      scores[current].status = 0;
      //cout<<current<<" "<<scores[current].status<<"\n";

   }
   if (score / 10 == 0)
      digits = 1;
   else
   {
      digits = 2;
      r = score / 10;
   }
   k = score % 10;

   //cout<<k<<endl;
   if (digits == 1 || digits == 2)
   {
      //cout << "yo";
      if (k==2||k == 3 || k == 5 || k == 6 || k == 7 || k == 8 || k == 9 || k == 0)
         scores["box1"].status = 1;
      if (k == 1 || k == 4 || k == 5 || k == 6 || k == 8 || k == 9 || k == 0)
         scores["box2"].status = 1;
      if (k == 2 || k == 3 || k == 7 || k == 8 || k == 9 || k == 0 || k == 4)
         scores["box3"].status = 1;
      if (k == 2 || k == 3 || k == 5 || k == 6 || k == 8 || k == 9 || k == 4)
         scores["box4"].status = 1;
      if (k == 2 || k == 1 || k == 6 || k == 8 || k == 0)
         scores["box5"].status = 1;
      if (k == 3 || k == 4 || k == 5 || k == 6 || k == 7 || k == 8 || k == 9 || k == 0)
         scores["box6"].status = 1;
      if (k == 2 || k == 3 || k == 5 || k == 6 || k == 8 || k == 9 || k == 0)
         scores["box7"].status = 1;
   }
   //cout<<scores["box1"].status<<" "<<"box1; "<<"score: "<<score<<"\n";
   if (digits == 2)
   {
      if (r == 2 || r == 3 || r == 5 || r == 6 || r == 7 || r == 8 || r == 9 || r == 0)
         scores["box8"].status = 1;
      if (r == 1 || r == 4 || r == 5 || r == 6 || r == 8 || r == 9 || r == 0)
         scores["box9"].status = 1;
      if (r == 2 || r == 3 || r == 7 || r == 8 || r == 9 || r == 0 || r == 4)
         scores["box10"].status = 1;
      if (r == 2 || r == 3 || r == 5 || r == 6 || r == 8 || r == 9 || r == 4)
         scores["box11"].status = 1;
      if (r == 2 || r == 1 || r == 6 || r == 8 || r == 0)
         scores["box12"].status = 1;
      if (r == 3 || r == 4 || r == 5 || r == 6 || r == 7 || r == 8 || r == 9 || r == 0)
         scores["box13"].status = 1;
      if (r == 2 || r == 3 || r == 5 || r == 6 || r == 8 || r == 9 || r == 0)
         scores["box14"].status = 1;
   }
}
float camx=0,camy=0;
double xpospr=0,ypospr=0;
double mouseyoffset;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{   
        mouseyoffset=yoffset;

}
void mouseuse(GLFWwindow *window,int fbwidth, int fbheight)
{
    double xpos,ypos;
    glfwGetCursorPos(window,&xpos,&ypos);
    xpos=(xpos/fbwidth)*8-4;
    ypos=(ypos/600)*8-4;
    float ox=(float)xpos-xpospr;
    float oy=(float)ypos-ypospr;
    //cout<<clk;
    if(clk==1)
    {
        camx-=ox;
        camy-=oy;
        cout<<camx<<" "<<camy<<" "<<fbheight<<" "<<fbwidth<<"\n";
        //clk=0;
    }
    xpospr=xpos;
    ypospr=ypos;
     
}
float eyx=4;
//int cunt=0;
/* Render the scene with openGL */
/* Edit this function according to your assignment */
glm::vec3 eye, target;
glm::vec3 up;
double lut;
void draw(GLFWwindow *window, float x, float y, float w, float h, int i,int pk)
{
   if(!pk)
   {
      int fbwidth, fbheight;
      //float rad;
      glfwGetFramebufferSize(window, &fbwidth, &fbheight);
      glViewport((int)(x * fbwidth), (int)(y * fbheight), (int)(w * fbwidth), (int)(h * fbheight));

      // use the loaded shader program
      // Don't change unless you know what you are doing
      glUseProgram(programID);

      // Eye - Location of camera. Don't change unless you are sure!!

      if (ve == 2)
      {
         eye = glm::vec3(0, -4, 1);
         target = glm::vec3(0, 0, 0);
         up = glm::vec3(0, 1, 0);
      }
      else if (ve == 3)
      {
         eye = glm::vec3(0, 0, 5);
         target = glm::vec3(0, 0, 0);
         up = glm::vec3(0, 1, 0);
      }
      else if (ve == 5)
      {
         for (map<string, Sprite>::iterator it = blocks.begin(); it != blocks.end(); it++)
         {
            string current = it->first; //The name of the current object
            if (blocks[current].status == 1)
            {

               //eye=glm::vec3(blocks[current].x,blocks[current].y-1.5,2.0);
               float temp;
               if (rectangle_rot_status == 2)
               {
                  eye = glm::vec3(blocks[current].x, blocks[current].y - 1.5, 2);
                  temp = 5;
                  up = glm::vec3(0, 1, 0);
                  target = glm::vec3(blocks[current].x, temp, 0);
               }
               else if (rectangle_rot_status == 3)
               {
                  eye = glm::vec3(blocks[current].x - 1.5, blocks[current].y, 2);
                  temp = 5;
                  up = glm::vec3(1, 0, 0);
                  target = glm::vec3(temp, blocks[current].y, 0);
               }
               else if (rectangle_rot_status == 1)
               {
                  eye = glm::vec3(blocks[current].x + 1.5, blocks[current].y, 2);
                  temp = 5;
                  up = glm::vec3(-1, 0, 0);
                  target = glm::vec3(-temp, blocks[current].y, 0);
               }
               else if (rectangle_rot_status == 4)
               {
                  eye = glm::vec3(blocks[current].x, blocks[current].y + 1.5, 2);
                  temp = 5;
                  up = glm::vec3(0, -1, 0);
                  target = glm::vec3(blocks[current].x, -temp, 0);
               }
               
            }
         }
      }
      else if (ve == 4)
      {
         for (map<string, Sprite>::iterator it = blocks.begin(); it != blocks.end(); it++)
         {
            string current = it->first; //The name of the current object
            if (blocks[current].status == 1)
            {
               eye = glm::vec3(blocks[current].x, blocks[current].y + 0.5, 1);
               float temp;
               //if(blocks[current].y<1)
               //{
               eye = glm::vec3(blocks[current].x, blocks[current].y + 0.5, 1);
               temp = 5;
               up = glm::vec3(0, 1, 0);
               //}
               /*else if(blocks[current].y>1)
                     {
                           eye=glm::vec3(blocks[current].x,blocks[current].y-0.5,1);
                           temp=-5;
                           up=glm::vec3(0,-1,0);
                     }*/
               target = glm::vec3(blocks[current].x, temp, 0);
               //glm::vec3 up (0, -1, 0);
            }
         }
      }
      else if(ve==6)
      {

          //sif(gand==1)
            eyx=5-mouseyoffset;
          mouseuse(window,fbwidth,fbheight);
          eye = glm::vec3(0,0, eyx);

         // Target - Where is the camera looking at.  Don't change unless you are sure!!
         target = glm::vec3(camx, camy , 0);
         // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
         up = glm::vec3(0, 1, 0);
      }
      else
      {

         eye = glm::vec3(0, -3, 4);

         // Target - Where is the camera looking at.  Don't change unless you are sure!!
         target = glm::vec3(0, 0, 0);
         // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
         up = glm::vec3(0, 1, 0);
      }
      // Compute Camera matrix (view)
      // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
      //  Don't change unless you are sure!!
      Matrices.view = glm::lookAt(eye, target, up); // Fixed camera for 2D (ortho) in XY plane

      glm::mat4 VP = (Matrices.projection) * Matrices.view;
      glm::mat4 MVP; // MVP = Projection * View * Model

      // Load identity to model matrix

      // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
      // glPopMatrix ();
      Matrices.model = glm::mat4(1.0f);
      //glm::rect_pos=
      //glm::mat4 translateRectangle = glm::translate (glm::vec3(chut["one"].x,chut["one"].y,chut["one"].z));        // glTranslatef
      glm::mat4 rotateRectangle = glm::rotate((float)(cunt * M_PI / 1800.0f), glm::vec3(1, 0, 0)); // rotate about vector (-1,1,1)
      for (map<string, Sprite>::iterator it = tiles.begin(); it != tiles.end(); it++)
      {
         string current = it->first; //The name of the current object
         //if(mirrors[current].status==0)
         //    continue;
         glm::mat4 MVP; // MVP = Projection * View * Model
         Matrices.model = glm::mat4(1.0f);
         glm::mat4 ObjectTransform;
         glm::mat4 translateObject = glm::translate(glm::vec3(tiles[current].x, tiles[current].y, tiles[current].z)); // glTranslatef
         ObjectTransform = translateObject;                                                                           //*rotateTriangle;
         Matrices.model *= ObjectTransform;
         MVP = VP * Matrices.model; // MVP = p * V * M

         glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
         if (tiles[current].status)
            draw3DObject(tiles[current].object);
         //glPopMatrix ();
      }
      
      for (map<string, Sprite>::iterator it = blocks.begin(); it != blocks.end(); it++)
      {
         string current = it->first; //The name of the current object
         if (blocks[current].status == 1)
         {
            for (map<string, Sprite>::iterator it = tiles.begin(); it != tiles.end(); it++)
            {
               //cout<<current<<" ";
               string cur = it->first;
               if (tiles[cur].status == 0 && blocks[current].x == tiles[cur].x && blocks[current].y == tiles[cur].y && current == "three")
               {
                   if(cur=="02")
                    {
                        cout<<"You Win"<<endl;
                        exit(0);
                    }
                      blocks[current].status = 2;
                     // exit(0);
               }
               if (tiles[cur].status == 0 && blocks[current].x == tiles[cur].x && ((blocks[current].y == (tiles[cur].y + (tiles[cur].width / 2))) || (blocks[current].y == (tiles[cur].y - (tiles[cur].width / 2)))) && current == "one")
               {
                   if(cur!="02")
                        blocks[current].status = 2;
                  //cout<<"ll ";
               }
               if (tiles[cur].status == 0 && blocks[current].y == tiles[cur].y && ((blocks[current].x == (tiles[cur].x + (tiles[cur].height / 2))) || (blocks[current].x == (tiles[cur].x - (tiles[cur].height / 2)))) && current == "two")
               {
                  //cout<<"uu\n";
                  if(cur!="02")
                    blocks[current].status = 2;
               }
            }
            if (current == "three")
            {
               if (blocks[current].x == tiles["411"].x && blocks[current].y == tiles["411"].y)
               {
                  tiles["411"].status = 0;
                  blocks[current].status = 2;
                  break;
                  //cout<<"yoyo\n";
               }
               else if (blocks[current].x == tiles["55"].x && blocks[current].y == tiles["55"].y)
               {
                  tiles["55"].status = 0;
                  blocks[current].status = 2;
                  break;
                  //cout<<"yoyo\n";
               }
            }
            if (blocks[current].status)
            {
               if (current == "three" && (tiles["231"].x == blocks[current].x) && (tiles["231"].y == blocks[current].y))
               {
                  //cout<<tiles["231"].y<<" "<<blocks[current].y<<" "<<current<<"\n";
                  //fl=0;
                  if (fl == 0)
                  {
                     tiles["431"].status = (tiles["431"].status + 1) % 2;
                     fl = 1;
                  }
               }
               else if (current == "two" && ((tiles["231"].x == blocks[current].x - 0.25) || (tiles["231"].x == blocks[current].x + 0.25)) && tiles["231"].y == blocks[current].y)
               {
                  if (fl == 0)
                  {
                     tiles["431"].status = (tiles["431"].status + 1) % 2;
                     fl = 1;
                  }
               }
               else if (current == "one" && ((tiles["231"].y == blocks[current].y - 0.25) || (tiles["231"].y == blocks[current].y + 0.25)) && tiles["231"].x == blocks[current].x)
               {
                  // cout<<fl<<"\n";
                  if (fl == 0)
                  {
                     tiles["431"].status = (tiles["431"].status + 1) % 2;
                     fl = 1;
                  }
               }
               //else
               //    fl=0;

               else if (current == "three" && (tiles["6"].x == blocks[current].x) && (tiles["6"].y == blocks[current].y))
               {
                  //cout<<tiles["6"].y<<" "<<blocks[current].y<<" "<<current<<"\n";
                  //fl=0;
                  if (fl == 0)
                  {
                     tiles["73"].status = (tiles["73"].status + 1) % 2;
                     fl = 1;
                  }
               }
               else if (current == "two" && ((tiles["6"].x == blocks[current].x - 0.25) || (tiles["6"].x == blocks[current].x + 0.25)) && tiles["6"].y == blocks[current].y)
               {

                  if (fl == 0)
                  {
                     tiles["73"].status = (tiles["73"].status + 1) % 2;
                     fl = 1;
                  }
               }
               else if (current == "one" && ((tiles["6"].y == blocks[current].y - 0.25) || (tiles["6"].y == blocks[current].y + 0.25)) && tiles["6"].x == blocks[current].x)
               {
                  if (fl == 0)
                  {
                     tiles["73"].status = (tiles["73"].status + 1) % 2;
                     fl = 1;
                  }
               }
               else if (current == "three" && (tiles["73"].x == blocks[current].x) && (tiles["73"].y == blocks[current].y))
               {
                  //cout<<tiles["6"].y<<" "<<blocks[current].y<<" "<<current<<"\n";
                  //fl=0;
                  if (fl == 0)
                  {
                     tiles["25"].status = (tiles["25"].status + 1) % 2;
                     fl = 1;
                  }
               }
               else if (current == "two" && ((tiles["73"].x == blocks[current].x - 0.25) || (tiles["73"].x == blocks[current].x + 0.25)) && tiles["73"].y == blocks[current].y)
               {

                  if (fl == 0)
                  {
                     tiles["25"].status = (tiles["25"].status + 1) % 2;
                     fl = 1;
                  }
               }
               else if (current == "one" && ((tiles["73"].y == blocks[current].y - 0.25) || (tiles["73"].y == blocks[current].y + 0.25)) && tiles["73"].x == blocks[current].x)
               {
                  if (fl == 0)
                  {
                     tiles["25"].status = (tiles["25"].status + 1) % 2;
                     fl = 1;
                  }
               }
               else
                  fl = 0;
               //cout<<fl<<"\n";
            }
            if (blocks[current].x > 1.75 || blocks[current].y > 4.5 || blocks[current].x < -2.75 || blocks[current].y < -2.75)
               blocks[current].status = 2;
         }
      }
      for (map<string, Sprite>::iterator it = blocks.begin(); it != blocks.end(); it++)
      {
         string current = it->first; //The name of the current object
         //rad=M_PI/2;
         //int i;
         if (blocks[current].status)
         //for(i=1;i<90;i++)
         {
            if (blocks[current].status == 2)
            {
               if (lut == 0)
                  lut = glfwGetTime();
               if (glfwGetTime() - lut < 2)
                  blocks[current].z -= 0.2;
               else
               {
                  blocks["one"].status = blocks["two"].status = 0;
                  blocks["three"].status = 1;
                  blocks["three"].x = -0.75;
                  blocks["three"].y = -1.75;
                  blocks["three"].z = blocks["two"].z = blocks["one"].z = 0;
                  tiles["411"].status = tiles["55"].status = 1;
                  lut = 0;
                  score = 0;
               }
            }
            glm::mat4 MVP; // MVP = Projection * View * Model
            Matrices.model = glm::mat4(1.0f);
            glm::mat4 ObjectTransform, rotateRectangle, translateo, translateb;
            glm::mat4 translateObject = glm::translate(glm::vec3(blocks[current].x, blocks[current].y, blocks[current].z)); //blocks[current].z));
            //glm::mat4 translateo = glm::translate (glm::vec3(-0.25, 0, -0.5));//blocks[current].z));

            rad = (cunt * M_PI) / 180.0f;
            //if(i<=90&&i!=0)
            //cout<<current<<" "<<rad<<" "<<rectangle_rot_status<<"\n";
            system("aplay -q soundname.wav %");
            if (rectangle_rot_status == 1)
            {
               translateo = glm::translate(glm::vec3(+blocks[current].height / 2, 0, 0));
               translateb = glm::translate(glm::vec3(-blocks[current].height / 2, 0, 0));
               //cout<<"you\n";
               rotateRectangle = glm::rotate((float)(-1 * rad), glm::vec3(0, 1, 0));
               glm::mat4 translateObject = glm::translate(glm::vec3(blocks[current].x, blocks[current].y, blocks[current].z));
            }
            else if (rectangle_rot_status == 3)
            {
               translateo = glm::translate(glm::vec3(-blocks[current].height / 2, 0, 0));
               translateb = glm::translate(glm::vec3(+blocks[current].height / 2, 0, 0));
               //cout<<"you\n";
               rotateRectangle = glm::rotate((float)(rad), glm::vec3(0, 1, 0));
               glm::mat4 translateObject = glm::translate(glm::vec3(blocks[current].x, blocks[current].y, blocks[current].z));
            }
            else if (rectangle_rot_status == 2)
            {
               translateo = glm::translate(glm::vec3(0, -blocks[current].width / 2, 0));
               translateb = glm::translate(glm::vec3(0, +blocks[current].width / 2, 0));
               glm::mat4 translateObject = glm::translate(glm::vec3(blocks[current].x, blocks[current].y, blocks[current].z));
               rotateRectangle = glm::rotate((float)(-1 * rad), glm::vec3(1, 0, 0));
            }
            else if (rectangle_rot_status == 4)
            {
               translateo = glm::translate(glm::vec3(0, +blocks[current].width / 2, 0));
               translateb = glm::translate(glm::vec3(0, -blocks[current].width / 2, 0));
               glm::mat4 translateObject = glm::translate(glm::vec3(blocks[current].x, blocks[current].y, blocks[current].z));
               rotateRectangle = glm::rotate((float)(rad), glm::vec3(1, 0, 0));
            }
            else
            {
               glm::mat4 translateObject = glm::translate(glm::vec3(blocks[current].x, blocks[current].y, blocks[current].z));
               rotateRectangle = glm::rotate((float)(0), glm::vec3(0, 1, 0));
            }
            ObjectTransform = translateObject * translateb * rotateRectangle * translateo;
            Matrices.model *= ObjectTransform;
            MVP = VP * Matrices.model; // MVP = p * V * M
            glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
            if (blocks[current].x > 3.25 || blocks[current].y > 5.75 || blocks[current].y < -3.25 || blocks[current].x < -3.25)
            {
               //map.erase(it++);
               //continue;
            }
            draw3DObject(blocks[current].object);
         }
         if (jump_status == 1 && blocks[current].status == 1)
         {
            blocks[current].y += 1;
            jump_status = 0;
            rectangle_rot_status = -1;
         }
         else if (jump_status == 2 && blocks[current].status == 1)
         {
            blocks[current].y -= 1;
            jump_status = 0;
            rectangle_rot_status = -1;
         }
         else if (jump_status == 3 && blocks[current].status == 1)
         {
            blocks[current].x -= 1;
            jump_status = 0;
            rectangle_rot_status = -1;
         }
         else if (jump_status == 4 && blocks[current].status == 1)
         {
            blocks[current].x += 1;
            jump_status = 0;
            rectangle_rot_status = -1;
         }
         if ((blocks[current].status == 1) && flag == 1)
         {
            system("aplay -q soundname.wav %");

            //cout<<cunt<<"\n";
            if (current == "one" && rectangle_rot_status == 1)
            {
               blocks["one"].status = 1;
               blocks["one"].x = blocks[current].x - 0.5;
               blocks["one"].y = blocks[current].y;
               //blocks["two"].z=blocks[current].z;
            }
            else if (current == "one" && rectangle_rot_status == 3)
            {
               blocks["one"].status = 1;
               blocks["one"].x = blocks[current].x + 0.5;
               blocks["one"].y = blocks[current].y;

               //blocks["two"].z=blocks[current].z;
            }
            else if (current == "one" && rectangle_rot_status == 2)
            {
               blocks["three"].status = 1;
               blocks["three"].x = blocks[current].x;
               blocks["three"].y = blocks[current].y + 0.75;
               blocks[current].status = 0;
               //blocks["three"].z=blocks[current].z+0.25;
            }
            else if (current == "one" && rectangle_rot_status == 4)
            {
               blocks["three"].status = 1;
               blocks["three"].x = blocks[current].x;
               blocks["three"].y = blocks[current].y - 0.75;
               blocks[current].status = 0;
               //blocks["three"].z=blocks[current].z+0.25;
            }
            if (current == "two" && rectangle_rot_status == 4)
            {
               blocks["two"].status = 1;
               blocks["two"].x = blocks[current].x;
               blocks["two"].y = blocks[current].y - 0.5;
               //blocks["two"].z=blocks[current].z;
            }
            else if (current == "two" && rectangle_rot_status == 2)
            {
               blocks["two"].status = 1;
               blocks["two"].x = blocks[current].x;
               blocks["two"].y = blocks[current].y + 0.5;
               //blocks["one"].z=blocks[current].z;
            }
            else if (current == "two" && rectangle_rot_status == 1)
            {
               blocks["three"].status = 1;
               blocks["three"].x = blocks[current].x - 0.75;
               blocks["three"].y = blocks[current].y;
               blocks[current].status = 0;

               //blocks["two"].z=blocks[current].z+0.25;
            }
            else if (current == "two" && rectangle_rot_status == 3)
            {
               blocks["three"].status = 1;
               blocks["three"].x = blocks[current].x + 0.75;
               blocks["three"].y = blocks[current].y;
               blocks[current].status = 0;

               //blocks["two"].z=blocks[current].z+0.25;
            }

            if (current == "three" && rectangle_rot_status == 1)
            {
               blocks["two"].status = 1;
               blocks["two"].x = blocks[current].x - 0.75;
               blocks["two"].y = blocks[current].y;
               blocks[current].status = 0;
               //cout<<blocks["two"].x<<" "<<blocks["two"].y<<"\n";
               //blocks["two"].z=blocks[current].z;
            }
            else if (current == "three" && rectangle_rot_status == 3)
            {
               blocks["two"].status = 1;
               blocks["two"].x = blocks[current].x + 0.75;
               blocks["two"].y = blocks[current].y;
               blocks[current].status = 0;

               //blocks["one"].z=blocks[current].z;
            }
            else if (current == "three" && rectangle_rot_status == 2)
            {
               blocks["one"].status = 1;
               blocks["one"].x = blocks[current].x;
               blocks["one"].y = blocks[current].y + 0.75;
               blocks[current].status = 0;

               //blocks["one"].z=blocks[current].z+0.25;
            }
            else if (current == "three" && rectangle_rot_status == 4)
            {
               blocks["one"].status = 1;
               blocks["one"].x = blocks[current].x;
               blocks["one"].y = blocks[current].y - 0.75;
               blocks[current].status = 0;

               //blocks["two"].z=blocks[current].z+0.25;
            }
            rectangle_rot_status = 0;
            flag = 0;
            cunt = 0;
            score++;
            createscore();
         }
         //glPopMatrix ();
      }
      if (cunt < 90 && rectangle_rot_status > 0)
         cunt += 5;
      else if (cunt == 90)
      {
         flag = 1;

         //    cunt=0;
      }

      // Increment angles
      float increments = 1;
   }
   else if(pk==1)
   {
      //cout<<"ji";
      int fbwidth, fbheight;
      //float rad;
      glfwGetFramebufferSize(window, &fbwidth, &fbheight);
      glViewport((int)(x * fbwidth), (int)(y * fbheight), (int)(w * fbwidth), (int)(h * fbheight));
      glm::vec3 ey,targ,u;
      ey = glm::vec3(0, 0, 5);
      targ = glm::vec3(0, 0, 0);
      u = glm::vec3(0, 1, 0);   
      Matrices.view = glm::lookAt(ey, targ, u); // Fixed camera for 2D (ortho) in XY plane
      Matrices.projectionP = glm::ortho(3.0f, 4.0f, 3.0f, 4.0f, 0.1f, 500.0f);
      glm::mat4 VP = (Matrices.projectionP) * Matrices.view;
      glm::mat4 MVP; 
      // use the loaded shader program
      // Don't change unless you know what you are doing
      glUseProgram(programID);
      for (map<string, Sprite>::iterator it = scores.begin(); it != scores.end(); it++)
      {
         string current = it->first; //The name of the current object
         glm::mat4 MVP; // MVP = Projection * View * Model
         //cout<<"Brick:"<<current<<"\n";
         Matrices.model = glm::mat4(1.0f);

         /* Render your scene */
         glm::mat4 ObjectTransform;
         glm::mat4 translateObject = glm::translate(glm::vec3(scores[current].x, scores[current].y, 0.0f)); // glTranslatef
         ObjectTransform = translateObject;
         Matrices.model *= ObjectTransform;
         MVP = VP * Matrices.model; // MVP = p * V * M

         glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
         if (scores[current].status == 1)
         {
            //cout<<"score is "<<score<<endl;  
            //cout<<"Displaying bit for "<<current<<endl;
            draw3DObject(scores[current].object);
         //  cout<<current<<"\n";
         }
      }
   }
   //camera_rotation_angle++; // Simulating camera rotation
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow *initGLFW(int width, int height)
{
   GLFWwindow *window; // window desciptor/handle

   glfwSetErrorCallback(error_callback);
   if (!glfwInit())
   {
      exit(EXIT_FAILURE);
   }

   glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
   glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
   glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
   glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

   window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

   if (!window)
   {
      exit(EXIT_FAILURE);
      glfwTerminate();
   }

   glfwMakeContextCurrent(window);
   //    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
   glfwSwapInterval(1);
   glfwSetFramebufferSizeCallback(window, reshapeWindow);
   glfwSetWindowSizeCallback(window, reshapeWindow);
   glfwSetWindowCloseCallback(window, quit);
   glfwSetKeyCallback(window, keyboard);            // general keyboard input
   glfwSetCharCallback(window, keyboardChar);       // simpler specific character handling
   glfwSetMouseButtonCallback(window, mouseButton); // mouse button clicks

   return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
string createname(int j)
{
   char s[10];
   int i = 0, l;
   while (j != 0)
   {
      l = j % 10;
      s[i++] = (char)('0' + l);
      j /= 10;
   }
   s[i] = '\0';
   //cout<<s<<"\n";
   return s;
}
void initGL(GLFWwindow *window, int width, int height)
{
   /* Objects should be created before any other gl function and shaders */
   // Create the models
   string box1 = "box1";
   string box2 = "box2";
   string box3 = "box3";
   string box4 = "box4";
   string box5 = "box5";
   string box6 = "box6";
   string box7 = "box7";
   string box8 = "box8";
   string box9 = "box9";
   string box10 = "box10";
   string box11 = "box11";
   string box12 = "box12";
   string box13 = "box13";
   string box14 = "box14";
   createscore();
   COLOR col, col1, col2, color2;
   color2.r = 1;
   color2.b = 1;
   color2.g = 1;
   col.r = col.g = 1;
   col.b = 0;
   col1.r = col1.b = 1;
   col1.g = 0;
   col2.b = col2.g = 1;
   col2.r = 0;

   createRectangles(0.1, 0.015, color2, 3.75, 3.6-0.2, "score", box1);
   createRectangles(0.015, 0.15, color2, 3.69, 3.825-0.5, "score", box2);
   createRectangles(0.015, 0.15, color2, 3.79, 3.825-0.5, "score", box3);
   createRectangles(0.1, 0.015, color2, 3.75, 3.74-0.5, "score", box4);
   createRectangles(0.015, 0.15, color2, 3.69, 3.675-0.5, "score", box5);
   createRectangles(0.015, 0.15, color2, 3.79, 3.675-0.5, "score", box6);
   createRectangles(0.1, 0.015, color2, 3.75, 3.59-0.5, "score", box7);

   createRectangles(0.1, 0.015, color2, 3.60, 3.9-0.5, "score", box8);
   createRectangles(0.015, 0.15, color2, 3.54, 3.825-0.5, "score", box9);
   createRectangles(0.015, 0.15, color2, 3.64, 3.825-0.5, "score", box10);
   createRectangles(0.1, 0.015, color2, 3.60, 3.74-0.5, "score", box11);
   createRectangles(0.015, 0.15, color2, 3.54, 3.675-0.5, "score", box12);
   createRectangles(0.015, 0.15, color2, 3.64, 3.675-0.5, "score", box13);
   createRectangles(0.1, 0.015, color2, 3.60, 3.59-0.5, "score", box14);

   createCube1(0.5, 1, 0.5, col1, col, col1, -1.25, -1.75, 0, "block", "one", 0, 1);
   createCube1(1, 0.5, 0.5, col1, col1, col, -1.75, -1.25, 0, "block", "two", 0, 1);
   createCube1(0.5, 0.5, 1, col, col1, col1, 0.75, -1.75, 0, "block", "three", 1, 1);
   //createCube(0.5,1,0.5,col1,col,col1,-1.25,-1.5,0.25,"chut","one",1);
   float x = -2.75;

   int nm = 1;
   COLOR bor;

   int status;
   for (int i = 0; i < 10; i++)
   {
      float y = -2.75;

      for (int j = 0; j < 15; j++)
      {
         bor.r = bor.g = 0;
         bor.b = 0.2;
         int type = 0;
         if ((nm > 0 && nm < 4) || nm == 15 || (nm > 15 && nm < 19) || nm == 20 || (nm > 22 && nm < 28) || nm == 30 || (nm > 30 && nm < 34) || (nm > 36 && nm < 43 && nm != 37) || (nm > 45 && nm < 53) || (nm > 55 && nm < 58) || nm == 61 || nm == 62 || (nm > 63 && nm < 68) || (nm > 69 && nm < 75) || nm == 76 || (nm > 79 && nm < 83) || (nm > 84 && nm < 90) || (nm > 94 && nm < 98) || (nm > 100 && nm < 103) || nm == 116 || nm == 117 || nm == 120 || nm == 134 || nm == 135 || nm == 149 || nm == 150 || nm == 136 || nm == 142 || nm == 143)
         {
            status = 0;
            type = 0;
         }
         else if (nm == 114 || nm == 55)
         {
            type = 1;
            status = 1;
            bor.r = bor.b = 0;
            bor.g = 0.8;
         }
         else if (nm == 132 || nm == 6 || nm == 37)
         {
            if (nm != 37)
               status = 1;
            else
               status = 0;
            type = 2;
            bor.b = bor.g = 0;
            bor.r = 0.8;
         }
         else
            status = 1;
         createCube(0.5, 0.5, 0.2, bor, bor, bor, x, y, -0.1, "tile", createname(nm++), status, type);
         y += 0.5;
      }
      x += 0.5;
   }

   // Create and compile our GLSL program from the shaders
   programID = LoadShaders("Sample_GL.vert", "Sample_GL.frag");
   // Get a handle for our "MVP" uniform
   Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

   reshapeWindow(window, width, height);

   // Background color of the scene
   glClearColor(0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
   glClearDepth(1.0f);

   glEnable(GL_DEPTH_TEST);
   glDepthFunc(GL_LEQUAL);

   // cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
   // cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
   // cout << "VERSION: " << glGetString(GL_VERSION) << endl;
   // cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main(int argc, char **argv)
{
   int width = 600;
   int height = 600;
   proj_type = 0;
   tri_pos = glm::vec3(0, 0, 0);
   rect_pos = glm::vec3(0, 0, 0);
   cube_pos = glm::vec3(2, 3, 0);
   GLFWwindow *window = initGLFW(width, height);
   initGLEW();
   initGL(window, width, height);

   double last_update_time = glfwGetTime(), current_time;

   /* Draw in loop */
   while (!glfwWindowShouldClose(window))
   {

      // clear the color and depth in the frame buffer
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      //flag=0;
      // OpenGL Draw commands
      //if(rectangle_rot_status>0)
      //{
      //flag=0;
      //for(int i=0;i<=900;i++)
      //{
      //if(i==90)
      //      flag=1;
      //draw(window, 0, 0, 1, 1,i);
      //}

      //}
      //else
     glfwSetScrollCallback(window, scroll_callback);

      draw(window, 0, 0, 0.8, 0.8, 0,0);
      draw(window, 0.8,0.8,0.2,0.2,1,1);
      // proj_type ^= 1;
      // draw(window, 0.5, 0, 0.5, 1);
      // proj_type ^= 1;

      // Swap Frame Buffer in double buffering
      glfwSwapBuffers(window);

      // Poll for Keyboard and mouse events
      glfwPollEvents();

      // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
      current_time = glfwGetTime(); // Time in seconds
      if ((current_time - last_update_time) >= 0.5)
      { // atleast 0.5s elapsed since last frame
         // do something every 0.5 seconds ..
         last_update_time = current_time;
      }
   }

   glfwTerminate();
   //    exit(EXIT_SUCCESS);
}
