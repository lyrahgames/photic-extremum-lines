#include "application.hpp"
//
#include "../camera.hpp"
#include "../shader.hpp"

namespace application {

glfw_context context{};
glfw_window window{800, 450, "Photic Extremum Lines"};

namespace {

// World Origin
vec3 origin;
// Basis Vectors of Right-Handed Coordinate System
vec3 up{0, 1, 0};
vec3 right{1, 0, 0};
vec3 front{0, 0, 1};
// Spherical/Horizontal Coordinates of Camera
float radius = 10;
float altitude = 0;
float azimuth = 0;

// Mouse Interaction
vec2 old_mouse_pos;
vec2 mouse_pos;
bool view_should_update = true;

camera cam{};

constexpr struct {
  float x, y;     // 2D Position
  float r, g, b;  // Color
} vertices[3] = {
    {-0.6f, -0.4f, /**/ 1.f, 0.0f, 0.0f},
    {0.6f, -0.4f, /**/ 0.0f, 1.f, 0.0f},
    {0.0f, 0.6f, /**/ 0.0f, 0.0f, 1.f},
};

constexpr czstring vertex_shader_text =
    "#version 330 core\n"
    "uniform mat4 MVP;"
    "attribute vec3 vCol;"
    "attribute vec2 vPos;"
    "out vec3 color;"
    "void main(){"
    "  gl_Position = MVP * vec4(vPos, 0.0, 1.0);"
    "  color = vCol;"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"
    "in vec3 color;"
    "void main(){"
    "  gl_FragColor = vec4(color, 1.0);"
    "}";

GLuint vertex_array;
GLuint vertex_buffer;
shader_program shader;
mat4 projection;

}  // namespace

void setup() {
  glfwSetFramebufferSizeCallback(
      window,
      [](GLFWwindow* window, int width, int height) { resize(width, height); });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) {
    zoom({x, y});
  });

  shader = shader_program({vertex_shader_text}, {fragment_shader_text});

  // Use a vertex array to be able to reference the vertex buffer and
  // the vertex attribute arrays of the triangle with one single variable.
  glGenVertexArrays(1, &vertex_array);
  glBindVertexArray(vertex_array);

  // Generate and bind the buffer which shall contain the triangle data.
  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  // The data is not changing rapidly. Therefore we use GL_STATIC_DRAW.
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Set the data layout of the position and colors
  // with vertex attribute pointers.
  const auto vpos_location = glGetAttribLocation(shader, "vPos");
  glEnableVertexAttribArray(vpos_location);
  glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*)0);
  const auto vcol_location = glGetAttribLocation(shader, "vCol");
  glEnableVertexAttribArray(vcol_location);
  glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                        sizeof(vertices[0]), (void*)(sizeof(float) * 2));

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  resize(width, height);

  glClearColor(0.0, 0.5, 0.8, 1.0);
}

void process_events() {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  // Compute the mouse move vector.
  old_mouse_pos = mouse_pos;
  double xpos, ypos;
  glfwGetCursorPos(window, &xpos, &ypos);
  mouse_pos = vec2{xpos, ypos};
  const auto mouse_move = mouse_pos - old_mouse_pos;

  // Left mouse button should rotate the camera by using spherical coordinates.
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    turn(mouse_move);

  // Right mouse button should translate the camera.
  if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    shift(mouse_move);
}

void resize(int width, int height) {
  glViewport(0, 0, width, height);
  cam.set_screen_resolution(width, height);
}

void update() {
  if (view_should_update) {
    update_view();
    view_should_update = false;
  }

  // Continuously rotate the triangle.
  auto model = glm::mat4{1.0f};
  const auto axis = glm::normalize(glm::vec3(1, 1, 1));
  model = rotate(model, float(glfwGetTime()), axis);

  const auto view = lookAt(vec3{0, 0, 5}, {0, 0, 0}, {0, 1, 0});

  // Compute the model-view-projection matrix (MVP).
  const auto mvp = cam.projection_matrix() * cam.view_matrix() * model;
  // Transfer the MVP to the GPU.
  glUniformMatrix4fv(glGetUniformLocation(shader, "MVP"), 1, GL_FALSE,
                     glm::value_ptr(mvp));
}

void render() {
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(shader);
  glBindVertexArray(vertex_array);
  glDrawArrays(GL_TRIANGLES, 0, 3);
}

void cleanup() {}

void update_view() {
  // Computer camera position by using spherical coordinates.
  // This transformation is a variation of the standard
  // called horizontal coordinates often used in astronomy.
  auto p = cos(altitude) * cos(azimuth) * right +  //
           cos(altitude) * sin(azimuth) * front +  //
           sin(altitude) * up;
  p *= radius;
  p += origin;
  cam.move(p).look_at(origin, up);
}

void turn(const vec2& mouse_move) {
  altitude += mouse_move.y * 0.01;
  azimuth += mouse_move.x * 0.01;
  constexpr float bound = pi / 2 - 1e-5f;
  altitude = std::clamp(altitude, -bound, bound);
  view_should_update = true;
}

void shift(const vec2& mouse_move) {
  const auto shift = mouse_move.x * cam.right() + mouse_move.y * cam.up();
  const auto scale = 1.3f * cam.pixel_size() * radius;
  origin += scale * shift;
  view_should_update = true;
}

void zoom(const vec2& mouse_scroll) {
  radius *= exp(-0.1f * float(mouse_scroll.y));
  view_should_update = true;
}

}  // namespace application
