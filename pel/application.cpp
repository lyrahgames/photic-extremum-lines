#include "application.hpp"
//
#include "camera.hpp"
#include "contours_shader.hpp"
#include "flat_shader.hpp"
#include "model.hpp"
#include "photic_extremum_lines.hpp"
#include "photic_extremum_lines_shader.hpp"
#include "shader.hpp"
#include "silhouette_shader.hpp"
#include "stl_loader.hpp"
#include "toon_shader.hpp"
#include "vertex_light_shader.hpp"
#include "vertex_light_variation_shader.hpp"
#include "vertex_light_variation_slope_shader.hpp"
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
shader_program contour_shader{};
bool surface_shading_enabled = true;
bool pels_enabled = true;
bool contours_enabled = true;
model mesh{};

vec3 aabb_min{};
vec3 aabb_max{};
float bounding_radius;

vector<illumination_info> illumination_data{};
vector<gradient_info> gradient_data{};
vertex_buffer illumination_buffer;

float threshold = 0.01;
float threshold_shift = -1 / log(threshold);
float line_shift = 0.001;

bool illumination_should_update = true;

bool control_key_pressed = false;

}  // namespace

void init() {
  glfwSetFramebufferSizeCallback(
      window,
      [](GLFWwindow* window, int width, int height) { resize(width, height); });

  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode,
                                int action, int mods) {
    control_key_pressed = mods & GLFW_MOD_CONTROL;
    if ((key == GLFW_KEY_L) && (action == GLFW_PRESS))
      pels_enabled = !pels_enabled;
    if ((key == GLFW_KEY_C) && (action == GLFW_PRESS))
      contours_enabled = !contours_enabled;
    if ((key == GLFW_KEY_S) && (action == GLFW_PRESS))
      surface_shading_enabled = !surface_shading_enabled;
    if ((key == GLFW_KEY_U) && (action == GLFW_PRESS))
      illumination_should_update = !illumination_should_update;

    view_should_update = true;
  });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double x, double y) {
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
      adjust_threshold(y);
    else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
      adjust_shift(y);
    else
      zoom({x, y});
  });

  shader = viewer_shader();
  line_shader = photic_extremum_lines_shader();
  contour_shader = contours_shader();
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

  glEnable(GL_MULTISAMPLE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  // glClearColor(0.0, 0.5, 0.8, 1.0);
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glPointSize(3.0f);
  glLineWidth(2.5f);
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
  if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
    shader = vertex_light_shader();
    view_should_update = true;
  }
  if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
    shader = vertex_light_variation_shader();
    view_should_update = true;
  }
  if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
    shader = vertex_light_variation_slope_shader();
    view_should_update = true;
  }
}

void resize(int width, int height) {
  glViewport(0, 0, width, height);
  cam.set_screen_resolution(width, height);
  view_should_update = true;
}

void update() {
  if (view_should_update) {
    update_view();
    view_should_update = false;
  }
}

void render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (surface_shading_enabled) {
    shader.bind();
    mesh.render();
  }
  if (pels_enabled) {
    line_shader.bind();
    mesh.render();
  }
  if (contours_enabled) {
    contour_shader.bind();
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

  cam.set_near_and_far(std::max(1e-3f * radius, radius - bounding_radius),
                       radius + bounding_radius);

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

  contour_shader.bind();
  contour_shader  //
      .set("projection", cam.projection_matrix())
      .set("view", cam.view_matrix())
      .set("threshold", threshold)
      .set("shift", line_shift);

  line_shader.bind();
  line_shader  //
      .set("projection", cam.projection_matrix())
      .set("view", cam.view_matrix())
      .set("threshold", threshold)
      .set("shift", line_shift);

  if (illumination_should_update) update_illumination_data();
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

void adjust_threshold(float x) {
  threshold_shift *= exp(-0.01f * x);
  threshold = exp(-1.0f / threshold_shift);
  view_should_update = true;
}

void adjust_shift(float x) {
  line_shift *= exp(-0.01f * x);
  view_should_update = true;
}

void fit_view() {
  // AABB computation
  aabb_min = mesh.vertices[0].position;
  aabb_max = mesh.vertices[0].position;
  for (size_t i = 1; i < size(mesh.vertices); ++i) {
    aabb_min = min(aabb_min, mesh.vertices[i].position);
    aabb_max = max(aabb_max, mesh.vertices[i].position);
  }
  origin = 0.5f * (aabb_max + aabb_min);
  bounding_radius = 0.5f * length(aabb_max - aabb_min);
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

  illumination_data.resize(mesh.vertices.size());
  gradient_data.resize(mesh.faces.size());
  compute_voronoi_weights(mesh, gradient_data);
  compute_vertex_voronoi_area(mesh, gradient_data, illumination_data);
  compute_vertex_tangent_system(mesh, gradient_data, illumination_data);
}

void setup_illumination_locations(const shader_program& shader) {
  illumination_buffer.bind();
  {
    const auto location = glGetAttribLocation(shader, "l");
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 1, GL_FLOAT, GL_FALSE,
                          sizeof(illumination_info),
                          (void*)offsetof(illumination_info, light));
  }
  {
    const auto location = glGetAttribLocation(shader, "lg");
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(illumination_info),
                          (void*)offsetof(illumination_info, light_gradient));
  }
  {
    const auto location = glGetAttribLocation(shader, "lv");
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 1, GL_FLOAT, GL_FALSE,
                          sizeof(illumination_info),
                          (void*)offsetof(illumination_info, light_variation));
  }
  {
    const auto location = glGetAttribLocation(shader, "lvs");
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
        location, 1, GL_FLOAT, GL_FALSE, sizeof(illumination_info),
        (void*)offsetof(illumination_info, light_variation_slope));
  }
  {
    const auto location = glGetAttribLocation(shader, "lvc");
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
        location, 1, GL_FLOAT, GL_FALSE, sizeof(illumination_info),
        (void*)offsetof(illumination_info, light_variation_curve));
  }
}

void update_illumination_data() {
  compute_vertex_light(cam.direction(), mesh, illumination_data);
  compute_vertex_light_gradient(mesh, gradient_data, illumination_data);
  compute_vertex_light_variation_slope(mesh, gradient_data, illumination_data);
  compute_vertex_light_variation_curve(mesh, gradient_data, illumination_data);

  setup_illumination_locations(shader);
  setup_illumination_locations(line_shader);

  illumination_buffer.bind();
  glBufferData(GL_ARRAY_BUFFER,
               illumination_data.size() * sizeof(illumination_data[0]),
               illumination_data.data(), GL_DYNAMIC_DRAW);
}

}  // namespace application
