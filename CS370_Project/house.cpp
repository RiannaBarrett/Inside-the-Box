// CS370 Final Project
// Fall 2022
// Rianna Barrett
#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"   // Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#define DEG2RAD (M_PI/180.0)

using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {Cube,Table,Soda, Cup, Bowl,Chair,Mirror,Frame, Tvstand, NumVAOs};
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Color_Buffer_IDs {RedCube,PurpleCube, BlackCube, NumColorBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames { RedPlastic, WhitePlastic, Brass};
enum Textures { Carpet, Wall, Wood, Label, Picture, Wood1,Blank,Bowlcol,MirrorTex, NumTextures };

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];

// use for now

GLuint TextureIDs[NumTextures];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
vector<const char *> objFiles = {"../models/unitcube.obj", "../models/table.obj", "../models/soda.obj", "../models/cup.obj", "../models/bowl.obj", "../models/chair.obj","../models/plane.obj","../models/apple.obj" };

// Texture files
vector<const char *> texFiles = {"../textures/Carpetjpg.jpg","../textures/wall.jpg", "../textures/wood.jpg", "../textures/soda.png", "../textures/artwall.jpg","../textures/wood1.jpg", "../textures/white.jpg", "../textures/red.jpg"  };

// Camera
vec3 eye = {0.0f, 1.0f, 0.0f};
vec3 center = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};

vec3 mirror_eye = {0.0f, 1.0f, 3.8f};
vec3 mirror_center = {0.0f, 1.0f, 0.0f};
vec3 mirror_up = {0.0f, 1.0f, 0.0f};

GLfloat azimuth = 0.0f;
GLfloat daz = 2.0f;
GLfloat elevation = 90.0f;
GLfloat del = 2.0f;
GLfloat radius = 2.0f;
GLfloat dr = 0.1f;



// Debug mirror shader
GLuint debug_mirror_program;
const char *debug_mirror_vertex_shader = "../debugMirror.vert";
const char *debug_mirror_frag_shader = "../debugMirror.frag";

// Mirror flag
GLboolean mirror = false;

// Shader variables
// Default (color) shader program references
GLuint default_program;
GLuint default_vPos;
GLuint default_vCol;
GLuint default_proj_mat_loc;
GLuint default_cam_mat_loc;
GLuint default_model_mat_loc;
const char *default_vertex_shader = "../default.vert";
const char *default_frag_shader = "../default.frag";

// Lighting shader program reference
GLuint lighting_program;
GLuint lighting_vPos;
GLuint lighting_vNorm;
GLuint lighting_camera_mat_loc;
GLuint lighting_model_mat_loc;
GLuint lighting_proj_mat_loc;
GLuint lighting_norm_mat_loc;
GLuint lighting_lights_block_idx;
GLuint lighting_materials_block_idx;
GLuint lighting_material_loc;
GLuint lighting_num_lights_loc;
GLuint lighting_light_on_loc;
GLuint lighting_eye_loc;
const char *lighting_vertex_shader = "../lighting.vert";
const char *lighting_frag_shader = "../lighting.frag";

// Texture shader program reference
GLuint texture_program;
GLuint texture_vPos;
GLuint texture_vTex;
GLuint texture_proj_mat_loc;
GLuint texture_camera_mat_loc;
GLuint texture_model_mat_loc;
const char *texture_vertex_shader = "../texture.vert";
const char *texture_frag_shader = "../texture.frag";

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;

vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Global screen dimensions
GLint ww,hh;

void display();
void render_scene();
void build_geometry();
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer);
void build_materials( );
void build_lights( );
void create_mirror( );
void build_textures();
void build_mirror( );
void build_frame(GLuint obj);
void draw_frame(GLuint obj);
void load_object(GLuint obj);
void draw_color_obj(GLuint obj, GLuint color);
void draw_mat_object(GLuint obj, GLuint material);
void draw_tex_object(GLuint obj, GLuint texture);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);
void renderQuad(GLuint shader, GLuint tex);
int main(int argc, char**argv)
{
    // Create OpenGL window
    GLFWwindow* window = CreateWindow("Think Inside The Box");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size
    glfwGetFramebufferSize(window, &ww, &hh);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

    // Load shaders and associate variables
    ShaderInfo default_shaders[] = { {GL_VERTEX_SHADER, default_vertex_shader},{GL_FRAGMENT_SHADER, default_frag_shader},{GL_NONE, NULL} };
    default_program = LoadShaders(default_shaders);
    default_vPos = glGetAttribLocation(default_program, "vPosition");
    default_vCol = glGetAttribLocation(default_program, "vColor");
    default_proj_mat_loc = glGetUniformLocation(default_program, "proj_matrix");
    default_cam_mat_loc = glGetUniformLocation(default_program, "camera_matrix");
    default_model_mat_loc = glGetUniformLocation(default_program, "model_matrix");

    // Load shaders
    // Load light shader
    ShaderInfo lighting_shaders[] = { {GL_VERTEX_SHADER, lighting_vertex_shader},{GL_FRAGMENT_SHADER, lighting_frag_shader},{GL_NONE, NULL} };
    lighting_program = LoadShaders(lighting_shaders);
    lighting_vPos = glGetAttribLocation(lighting_program, "vPosition");
    lighting_vNorm = glGetAttribLocation(lighting_program, "vNormal");
    lighting_proj_mat_loc = glGetUniformLocation(lighting_program, "proj_matrix");
    lighting_camera_mat_loc = glGetUniformLocation(lighting_program, "camera_matrix");
    lighting_norm_mat_loc = glGetUniformLocation(lighting_program, "normal_matrix");
    lighting_model_mat_loc = glGetUniformLocation(lighting_program, "model_matrix");
    lighting_lights_block_idx = glGetUniformBlockIndex(lighting_program, "LightBuffer");
    lighting_materials_block_idx = glGetUniformBlockIndex(lighting_program, "MaterialBuffer");
    lighting_material_loc = glGetUniformLocation(lighting_program, "Material");
    lighting_num_lights_loc = glGetUniformLocation(lighting_program, "NumLights");
    lighting_light_on_loc = glGetUniformLocation(lighting_program, "LightOn");
    lighting_eye_loc = glGetUniformLocation(lighting_program, "EyePosition");

    // Load shaders
    ShaderInfo texture_shaders[] = { {GL_VERTEX_SHADER, texture_vertex_shader},{GL_FRAGMENT_SHADER, texture_frag_shader},{GL_NONE, NULL} };
    texture_program = LoadShaders(texture_shaders);
    texture_vPos = glGetAttribLocation(texture_program, "vPosition");
    texture_vTex = glGetAttribLocation(texture_program, "vTexCoord");
    texture_proj_mat_loc = glGetUniformLocation(texture_program, "proj_matrix");
    texture_camera_mat_loc = glGetUniformLocation(texture_program, "camera_matrix");
    texture_model_mat_loc = glGetUniformLocation(texture_program, "model_matrix");

    // Create geometry buffers
    build_geometry();
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();
    // Create textures
    build_textures();
    // Create mirror texture
    build_mirror();


    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set Initial camera position
    center[0] = eye[0] + cos(azimuth* DEG2RAD);
    center[1] = eye[1];
    center[2] = eye[2] + sin(azimuth* DEG2RAD);


    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
        create_mirror();
   // Uncomment instead of display() to view mirror map for debugging
        //renderQuad(debug_mirror_program, MirrorTex);
        // Draw graphics
        display();
        // Update other events like input handling
        glfwPollEvents();
        // Swap buffer onto screen
        glfwSwapBuffers( window );
    }

    // Close window
    glfwTerminate();
    return 0;

}

void display( )
{
    // Declare projection and camera matrices
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

    // Clear window and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute anisotropic scaling
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }

    // DEFAULT ORTHOGRAPHIC PROJECTION
    proj_matrix = frustum(-0.1f*xratio, 0.1f*xratio, -0.1f*yratio, 0.1f*yratio, 0.2f, 50.0f);

    // Set camera matrix
    camera_matrix = lookat(eye, center, up);

    // Render objects
    render_scene();

    // Flush pipelineYYYYYYY
    glFlush();
}

void render_scene( ) {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // TODO: creating carpet on floor.

    trans_matrix = translation(0.0f, 0.0f, 0.0f);
    // rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(8.0f, 0.0f, 8.0f);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    draw_tex_object(Cube, Carpet);

//    /// Set cube transformation matrix(Walls)
//
//    trans_matrix = translate(0.0f, 0.0f, 0.0f);
//    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
//    scale_matrix = scale(5.0f, 0.0f, 5.0f);
//    model_matrix = trans_matrix*rot_matrix*scale_matrix;
//    // Draw cube
//    draw_color_obj(Cube, PurpleCube);



    /// Set cube transformation matrix(Walls)
    trans_matrix = translate(0.0f, 3.5f,-3.5f);
    //rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(2.0f, 2.0f, 0.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Draw cube
    draw_tex_object(Cube, Picture);

    /// Set cube transformation matrix(Walls)
    trans_matrix = translate(3.8f, 1.2f,0.0f);
    //rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.0f, -2.0f, -1.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Draw cube
    draw_tex_object(Cube, Wood1);



    /// Set cube transformation matrix(Walls)
    trans_matrix = translate(4.0f, 4.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.0f, 8.0f, 8.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Draw cube
    draw_tex_object(Cube, Wall);



    /// Set cube transformation matrix(Walls)
    trans_matrix = translate(-4.0f, 4.0f, 0.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(0.0f, -8.0f, 8.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Draw cube
    draw_tex_object(Cube, Wall);

    /// Set cube transformation matrix(Walls)
    trans_matrix = translate(0.0f, 4.0f, 4.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(8.0f, 8.0f, 0.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Draw cube
    draw_tex_object(Cube, Wall);

    /// Set cube transformation matrix(Walls)
    trans_matrix = translate(0.0f, 4.0f, -4.0f);
    rot_matrix = rotate(0.0f, vec3(0.0f, 0.0f, 1.0f));
    scale_matrix = scale(8.0f, 8.0f, 0.0f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Draw cube
    draw_tex_object(Cube, Wall);

    /// Set cube placeholders transformation matrix(Table)
    //Transfer blender model to replace cube as table.
    trans_matrix = translation(0.0f, 0.2f, 0.0f);
    //rot_matrix = rotation(cube_angle, normalize(axis));
    scale_matrix = scale(0.2f, 0.2f, 0.2f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // TABLE
    draw_tex_object(Table, Wood);

    /// Set cube placeholders transformation matrix(SODA)
    //Transfer blender model to replace cube as table.
    trans_matrix = translation(0.0f, -0.01f, 0.0f);
    //rot_matrix = rotation(cube_angle, normalize(axis));
    scale_matrix = scale(0.2f, 0.2f, 0.2f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // TABLE
    draw_tex_object(Soda, Label);



    /// Set cube placeholders transformation matrix(SODA)
    //Transfer blender model to replace cup holder
    trans_matrix = translation(0.0f, 0.6f, 0.0f);
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();

    //Cup
    draw_tex_object(Cup, Blank);

    /// Set cube placeholders transformation matrix(SODA)
    //Transfer blender model to replace cup holder
    trans_matrix = translation(1.0f, 0.6f, 0.0f);
    scale_matrix = scale(0.1f, 0.1f, 0.1f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    //bowl
    draw_tex_object(Bowl, Bowlcol);

    /// Set cube placeholders transformation matrix(SODA)
    //Transfer blender model to replace cup holder
    trans_matrix = translation(-0.5f, 0.1f, 0.0f);
    scale_matrix = scale(0.2f, 0.2f, 0.2f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    //Chair
    draw_tex_object(Chair, Wood);


    // Draw mirror with wireframe
    if (!mirror) {
        draw_frame(Frame);
        // Render mirror in scene
        // TODO: Set mirror translation
        trans_matrix = translate(mirror_eye);
        rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
        scale_matrix = scale(1.0f, 1.0f, 1.0f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        // TODO: Draw mirror
        draw_tex_object(Mirror, MirrorTex);
    }

    /// Set cube placeholders transformation matrix(SODA)
    //Transfer blender model to replace cup holder
    trans_matrix = translation(4.0f, 1.5f, 0.0f);
    scale_matrix = scale(0.3f, 0.3f, 0.3f);
    model_matrix = trans_matrix*rot_matrix*scale_matrix;

    //tv stand
    draw_tex_object(Tvstand, Wood);


}



void create_mirror( ) {
    // Clear framebuffer for mirror rendering pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Set mirror projection matrix
    proj_matrix = frustum(-0.2f, 0.2f, -0.2f, 0.2f, 0.2f, 10.0f);

    // TODO: Set mirror camera matrix
    camera_matrix = lookat(mirror_eye, mirror_center, mirror_up);

    // Render mirror scene (without mirror)
    mirror = true;
    render_scene();
    glFlush();
    mirror = false;

    // TODO: Activate texture unit 0
    glActiveTexture(GL_TEXTURE0);
    // TODO: Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    // TODO: Copy framebuffer into mirror texture
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, ww, hh, 0);
}
void build_geometry( )
{
    // Generate vertex arrays and buffers
    glGenVertexArrays(NumVAOs, VAOs);

    // Load models
    load_object(Cube);
    load_object(Table);
    load_object(Soda);
    load_object(Cup);
    load_object(Bowl);
    load_object(Chair);

    load_object(Mirror);

    // Build mirror frame
    build_frame(Frame);

   // load_object(Tvstand);

    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // Build color buffers
    // Define cube vertex colors (red)
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 0.0f, 0.0f, 1.0f), RedCube);
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 0.0f, 1.0f, 1.0f), PurpleCube);
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 2.0f, 0.0f, 1.0f), BlackCube);
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 2.0f, 0.0f, 1.0f), BlackCube);
}



void build_materials( ) {
    // Add materials to Materials vector

//    // Create brass material
//    MaterialProperties brass = {
//            vec4(0.33f, 0.22f, 0.03f, 1.0f), //ambient
//            vec4(0.78f, 0.57f, 0.11f, 1.0f), //diffuse
//            vec4(0.99f, 0.91f, 0.81f, 1.0f), //specular
//            27.8f, //shininess
//            {0.0f, 0.0f, 0.0f}  //pad
//    };
//
//    // Create red plastic material
//    MaterialProperties redPlastic = {
//            vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
//            vec4(0.6f, 0.0f, 0.0f, 1.0f), //diffuse
//            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
//            32.0f, //shininess
//            {0.0f, 0.0f, 0.0f}  //pad
//    };
//
//    // Add materials to Materials vector
//    Materials.push_back(brass);
//    Materials.push_back(redPlastic);

    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);

}

void build_lights( ) {
    // Add lights to Lights vector
    LightProperties whitePointLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(3.0f, 0.0f, 3.0f, 1.0f),  //position
            vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };
    // Set numLights
    numLights = Lights.size();

    // Turn all lights on
    for (int i = 0; i < numLights; i++) {
        lightOn[i] = 1;
    }

    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}


void build_mirror( ) {
    // Generate mirror texture
    glGenTextures(1, &TextureIDs[MirrorTex]);
    // Bind mirror texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[MirrorTex]);
    // TODO: Create empty mirror texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ww, hh, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void build_frame(GLuint obj) {
    vector<vec4> vertices;
    vector<vec3> normals;

    // Create wireframe for mirror
    vertices = {
            vec4(1.0f, 0.0f, -1.0f, 1.0f),
            vec4(1.0f, 0.0f, 1.0f, 1.0f),
            vec4(-1.0f, 0.0f, 1.0f, 1.0f),
            vec4(-1.0f, 0.0f, -1.0f, 1.0f)
    };

    normals = {
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f),
            vec3(0.0f, 1.0f, 0.0f)
    };

    numVertices[obj] = vertices.size();

    // Create and load object buffers
    glGenBuffers(NumObjBuffers, ObjBuffers[obj]);
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*posCoords*numVertices[obj], vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*normCoords*numVertices[obj], normals.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_tex_object(GLuint obj, GLuint texture) {
    // Select shader program
    glUseProgram(texture_program);

    // Pass projection matrix to shader
    glUniformMatrix4fv(texture_proj_mat_loc, 1, GL_FALSE, proj_matrix);

    // Pass camera matrix to shader
    glUniformMatrix4fv(texture_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Pass model matrix to shader
    glUniformMatrix4fv(texture_model_mat_loc, 1, GL_FALSE, model_matrix);

    // Bind texture
    glBindTexture(GL_TEXTURE_2D, TextureIDs[texture]);

    // Bind vertex array
    glBindVertexArray(VAOs[obj]);

    // Bind position object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(texture_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(texture_vPos);

    // Bind texture object buffer and set attributes
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][TexBuffer]);
    glVertexAttribPointer(texture_vTex, texCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(texture_vTex);

    // Draw object
    glDrawArrays(GL_TRIANGLES, 0, numVertices[obj]);
}

void draw_frame(GLuint obj){
    // Draw frame using lines at mirror location
    glUseProgram(lighting_program);
    // Pass projection and camera matrices to shader
    glUniformMatrix4fv(lighting_proj_mat_loc, 1, GL_FALSE, proj_matrix);
    glUniformMatrix4fv(lighting_camera_mat_loc, 1, GL_FALSE, camera_matrix);

    // Bind lights
    glUniformBlockBinding(lighting_program, lighting_lights_block_idx, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, LightBuffers[LightBuffer], 0, Lights.size()*sizeof(LightProperties));
    // Bind materials
    glUniformBlockBinding(lighting_program, lighting_materials_block_idx, 1);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, MaterialBuffers[MaterialBuffer], 0, Materials.size()*sizeof(MaterialProperties));
    // Set camera position
    glUniform3fv(lighting_eye_loc, 1, eye);
    // Set num lights and lightOn
    glUniform1i(lighting_num_lights_loc, numLights);
    glUniform1iv(lighting_light_on_loc, numLights, lightOn);

    // Set frame transformation matrix
    mat4 trans_matrix = translate(mirror_eye);
    mat4 rot_matrix = rotate(-90.0f, vec3(1.0f, 0.0f, 0.0f));
    mat4 scale_matrix = scale(2.0f, 1.0f, 2.0f);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    // Compute normal matrix from model matrix
    normal_matrix = model_matrix.inverse().transpose();
    // Pass model matrix and normal matrix to shader
    glUniformMatrix4fv(lighting_model_mat_loc, 1, GL_FALSE, model_matrix);
    glUniformMatrix4fv(lighting_norm_mat_loc, 1, GL_FALSE, normal_matrix);
    glUniform1i(lighting_material_loc, RedPlastic);

    // Draw object using line loop
    glBindVertexArray(VAOs[obj]);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][PosBuffer]);
    glVertexAttribPointer(lighting_vPos, posCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vPos);
    glBindBuffer(GL_ARRAY_BUFFER, ObjBuffers[obj][NormBuffer]);
    glVertexAttribPointer(lighting_vNorm, normCoords, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(lighting_vNorm);
    glDrawArrays(GL_LINE_LOOP, 0, numVertices[obj]);

}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC to quit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    // Adjust azimuth
    if (key == GLFW_KEY_A) {
        azimuth += daz;
        if (azimuth > 360.0) {
            azimuth -= 360.0;
        }
    } else if (key == GLFW_KEY_D) {
        azimuth -= daz;
        if (azimuth < 0.0)
        {
            azimuth += 360.0;
        }
    }

    // Adjust elevation angle
    if (key == GLFW_KEY_W)
    {
        vec3 dir= center- eye;
        eye= eye + dir*0.1;
    }
    else if (key == GLFW_KEY_S)
    {
        vec3 dir= center- eye;
        eye= eye - dir*0.1;
    }


    // Compute updated camera position
    center[0] = eye[0] + cos(azimuth* DEG2RAD);
    center[1] = eye[1];
    center[2] = eye[2] + sin(azimuth* DEG2RAD);


}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}
// Debug mirror renderer
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad(GLuint shader, GLuint tex)
{
    // reset viewport
    glViewport(0, 0, ww, hh);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render Depth map to quad for visual debugging
    // ---------------------------------------------
    glUseProgram(shader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureIDs[tex]);
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

#include "utilfuncs.cpp"

