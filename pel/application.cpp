#include "application.hpp"
//
#include "camera.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "stl_loader.hpp"

using namespace std;

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

constexpr czstring vertex_shader_text =
    "#version 330 core\n"

    "uniform mat4 projection;"
    "uniform mat4 view;"

    "in vec3 p;"
    "in vec3 n;"

    "out vec3 normal;"

    "void main(){"
    "  gl_Position = projection * view * vec4(p, 1.0);"
    "  normal = n;"
    "}";

constexpr czstring fragment_shader_text =
    "#version 330 core\n"

    "uniform vec3 light_dir;"

    "in vec3 normal;"

    "void main(){"
    // "  float light = max(-dot(light_dir, normalize(normal)), 0.0);"
    "  float light = abs(dot(light_dir, normalize(normal)));"
    "  gl_FragColor = vec4(0.5 * vec3(light) + 0.5, 1.0);"
    "}";

shader_program shader{};
model mesh{};

}  // namespace

void init() {
  glfwSetFramebufferSizeCallback(
      window,
      [](GLFWwindow* window, int width, int height) { resize(width, height); });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) {
    zoom({x, y});
  });

  shader = shader_program({vertex_shader_text}, {fragment_shader_text});
}

void run() {
  setup();

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    process_events();
    update();
    render();

    glfwSwapBuffers(window);
  }

  cleanup();
}

void setup() {
  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  resize(width, height);

  glEnable(GL_DEPTH_TEST);
  glClearColor(0.0, 0.5, 0.8, 1.0);
  glPointSize(3.0f);
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

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) fit_view();
  if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) set_y_as_up();
  if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) set_z_as_up();
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
}

void render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shader.bind();
  mesh.render();
}

void cleanup() {}

void update_view() {
  // Computer camera position by using spherical coordinates.
  // This transformation is a variation of the standard
  // called horizontal coordinates often used in astronomy.
  auto p = cos(altitude) * sin(azimuth) * right -  //
           cos(altitude) * cos(azimuth) * front +  //
           sin(altitude) * up;
  p *= radius;
  p += origin;
  cam.move(p).look_at(origin, up);

  shader.bind();
  shader  //
      .set("projection", cam.projection_matrix())
      .set("view", cam.view_matrix())
      .set("light_dir", cam.direction());
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

void fit_view() {
  // AABB computation
  vec3 aabb_min = mesh.vertices[0].position;
  vec3 aabb_max = mesh.vertices[0].position;
  for (size_t i = 1; i < size(mesh.vertices); ++i) {
    aabb_min = min(aabb_min, mesh.vertices[i].position);
    aabb_max = max(aabb_max, mesh.vertices[i].position);
  }
  origin = 0.5f * (aabb_max + aabb_min);
  radius = 0.5f * length(aabb_max - aabb_min) *
           (1.0f / tan(0.5f * cam.vfov() * pi / 180.0f));
  cam.set_near_and_far(1e-4f * radius, 2 * radius);
  view_should_update = true;
}

void set_z_as_up() {
  right = {1, 0, 0};
  front = {0, -1, 0};
  up = {0, 0, 1};
  view_should_update = true;
}

void set_y_as_up() {
  right = {1, 0, 0};
  front = {0, 0, 1};
  up = {0, 1, 0};
  view_should_update = true;
}

void load_model(czstring file_path) {
  auto start = system_clock::now();
  stl_binary_format stl_data{file_path};
  auto end = system_clock::now();
  auto time = duration<float>(end - start).count();
  cout << "stl file:\n"
       << "load time = " << time << " s" << '\n'
       << "triangle count = " << stl_data.triangles.size() << '\n'
       << endl;

  start = system_clock::now();
  transform(stl_data, mesh);
  end = system_clock::now();
  time = duration<float>(end - start).count();
  cout << "mesh transform:\n"
       << "time = " << time << " s" << '\n'
       << "vertices = " << mesh.vertices.size() << '\n'
       << "faces = " << mesh.faces.size() << '\n'
       << endl;

  fit_view();
  mesh.setup(shader);
  mesh.update();
}

}  // namespace application
