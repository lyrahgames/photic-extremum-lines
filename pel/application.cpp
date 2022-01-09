#include "application.hpp"
//
#include "camera.hpp"
#include "contours_shader.hpp"
#include "flat_shader.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "silhouette_shader.hpp"
#include "stl_loader.hpp"
#include "toon_shader.hpp"
#include "viewer_shader.hpp"
#include "white_shader.hpp"
#include "wireframe_shader.hpp"

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

shader_program shader{};
shader_program line_shader{};
bool feature_lines_enabled = false;
model mesh{};

}  // namespace

void init() {
  glfwSetFramebufferSizeCallback(
      window,
      [](GLFWwindow* window, int width, int height) { resize(width, height); });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) {
    zoom({x, y});
  });

  // shader = shader_program({vertex_shader_text},    //
  //                         {geometry_shader_text},  //
  //                         {fragment_shader_text});
  // shader = wireframe_shader();
  shader = viewer_shader();
  line_shader = contours_shader();
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
  glLineWidth(3.0f);
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

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    shader = wireframe_shader();
    view_should_update = true;
  }
  if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
    shader = viewer_shader();
    view_should_update = true;
  }
  if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
    shader = toon_shader();
    view_should_update = true;
  }
  if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
    shader = white_shader();
    view_should_update = true;
  }
  if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
    shader = flat_shader();
    view_should_update = true;
  }
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    shader = silhouette_shader();
    view_should_update = true;
  }

  if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
    feature_lines_enabled = !feature_lines_enabled;
    view_should_update = true;
  }
  if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
    line_shader = contours_shader();
    feature_lines_enabled = true;
    view_should_update = true;
  }
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
  if (feature_lines_enabled) {
    line_shader.bind();
    mesh.render();
  }
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

  // auto t = vec3(cam.view_matrix() * vec4(cam.direction(), 0.0f));
  // cout << t.x << ", " << t.y << ", " << t.z << endl;

  shader.bind();
  shader  //
      .set("projection", cam.projection_matrix())
      .set("view", cam.view_matrix())
      // .set("light_dir", vec3(cam.view_matrix() * vec4(cam.direction(),
      // 0.0f)))
      .set("viewport", scale(mat4{1.0f}, {cam.screen_width() / 2.0f,
                                          cam.screen_height() / 2.0f, 1.0f}));

  line_shader.bind();
  line_shader  //
      .set("projection", cam.projection_matrix())
      .set("view", cam.view_matrix());
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
