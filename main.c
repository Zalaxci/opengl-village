// 3Η ΕΡΓΑΣΙΑ ΓΡΑΦΙΚΩΝ
// ΚΑΛΙ ΓΚΑΝΑΤΣΑ (ΟΝΟΜΑ ΤΑΥΤΟΤΗΤΑΣ ΠΑΝΤΕΛΗΣ ΓΚΑΝΑΤΣΑΣ), ΑΕΜ 4636
// ΠΛΗΡΟΦΟΡΙΚΗ ΑΠΘ

#include <math.h>

#include <GL/gl.h>
#include <GL/freeglut.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define VEC3(arr, x, y, z)    arr[0] = x; arr[1] = y; arr[2] = z;
#define VEC3CPY(dest, src)    dest[0] = src[0]; dest[1] = src[1]; dest[2] = src[2];
#define VEC3ADD(arr, addend)  arr[0] += addend[0]; arr[1] += addend[1]; arr[2] += addend[2];
#define VEC3SUB(arr, sub)     arr[0] -= sub[0]; arr[1] -= sub[1]; arr[2] -= sub[2];
#define VEC3DIV(arr, divisor) arr[0] /= divisor; arr[1] /= divisor; arr[2] /= divisor;
#define VEC3INVERT(arr)       arr[0] = -arr[0]; arr[1] = -arr[1]; arr[2] = -arr[2];
#define VEC3DOT(a, b)         a[0]*b[0] + a[1]*b[1] + a[2]*b[2]
#define VEC3CROSS(dest, a, b) dest[0] = a[1]*b[2] - a[2]*b[1];\
                              dest[1] = a[2]*b[0] - a[0]*b[2];\
                              dest[2] = a[0]*b[1] - a[1]*b[0];
#define VEC3TRANSLATE(arr)    glTranslatef(arr[0], arr[1], arr[2]);
#define VEC3UNTRANSLATE(arr)  glTranslatef(-arr[0], -arr[1], -arr[2]);
#define VEC3PRINT(arr)        printf("%f %f %f\n", arr[0], arr[1], arr[2])
#define VEC4(arr, x, y, z, w) arr[0] = x; arr[1] = y; arr[2] = z; arr[3] = w;

// ---- GEOMETRY ----
// (1) SOME BASIC, SIMPLE SHAPES (and our not-so-simple sphere)
// Draw a parallelogram given a vector (length-3 array or pointer) to a corner position, one for each side & normal
// Side b has to come after side a when going counterclockwise from how the user's facing, so e.g. side a would be the bottom & side b the left one if x is the bottom left corner
void draw_parallelogram(GLfloat *x, GLfloat *side_a, GLfloat *side_b, GLfloat *normal) {
    glBegin(GL_QUADS);
    glNormal3f(normal[0], normal[1], normal[2]);
    glVertex3f(x[0], x[1], x[2]);
    glVertex3f(x[0] + side_a[0], x[1] + side_a[1], x[2] + side_a[2]);
    glVertex3f(x[0] + side_a[0] + side_b[0],
               x[1] + side_a[1] + side_b[1],
               x[2] + side_a[2] + side_b[2]
    );
    glVertex3f(x[0] + side_b[0], x[1] + side_b[1], x[2] + side_b[2]);
    glEnd();
}
// Draw a triangle
void draw_triangle(GLfloat *x, GLfloat *side_a, GLfloat *side_b, GLfloat *normal) {
    glBegin(GL_TRIANGLES);
    glNormal3f(normal[0], normal[1], normal[2]);
    glVertex3f(x[0], x[1], x[2]);
    glVertex3f(x[0] + side_a[0], x[1] + side_a[1], x[2] + side_a[2]);
    glVertex3f(x[0] + side_b[0], x[1] + side_b[1], x[2] + side_b[2]);
    glEnd();
}
// Represent a "sphere face", this is just a triangle described by 1 vertice/corner & 2 sides starting at "corner" in counterclockwise order
typedef struct sphere_face_str {
    GLfloat corner[3];
    GLfloat side_a[3];
    GLfloat side_b[3];
} sphere_face;
// Necessary for spheres, but can be used for other stuff too - divide a triangle into 4 repeatedly until subdivision_reps is 0
void draw_triangle_tesselation(
    sphere_face *face, // Information about the sphere face (triangle)
    bool should_free_memory, // Whether to free face pointer at the end
    unsigned short subdivision_reps, // How many times to further subdivide the triangle (call this function recursively)
    void (*preprocess)(sphere_face *, void *), // Function to "preprocess" face to be passed to "draw_triangle_tesselation" again after each subdivision
    void *preprocess_parameter // void pointer parameter passed to "preprocess_params_per_subdivision"
) {
    if (subdivision_reps == 0) {
        draw_triangle(face->corner, face->side_a, face->side_b, face->corner); // The normal is the corner (1st vertice); lazy but good enough cause it's normalized
        if (should_free_memory) free(face); // Free memory
        return;
    }
    sphere_face *segment_face = malloc(sizeof(sphere_face)); // The face information for each triangle segment
    // Draw segments 1 to 3
    for (unsigned short current_segment = 0; current_segment < 3; current_segment++) {
        // (Mem)copy triangle face parameters for the subface
        memcpy(segment_face->corner, face->corner, 3 * sizeof(GLfloat));
        memcpy(segment_face->side_a, face->side_a, 3 * sizeof(GLfloat));
        memcpy(segment_face->side_b, face->side_b, 3 * sizeof(GLfloat));
        VEC3DIV(segment_face->side_a, 2.); // Divide "side_a" by 2
        VEC3DIV(segment_face->side_b, 2.); // Divide "side_b" by 2
        // The 2nd segment is offset by half of side_a (if side_a is the bottom, then it is the bottom right triangle)
        if (current_segment == 1) { VEC3ADD(segment_face->corner, segment_face->side_a); }
        // The 3rd segment is offset by half of side_b (if side_b is the left, then it is the top triangle)
        else if (current_segment == 2) { VEC3ADD(segment_face->corner, segment_face->side_b); }
        // The 1st one start at the original corner & has vertices the midpoints of the (original) side_a & side_b, so no modifications need to be made
        preprocess(segment_face, preprocess_parameter); // "Preprocess"
        draw_triangle_tesselation(segment_face, false, subdivision_reps - 1, preprocess, preprocess_parameter); // At last, draw our small lil' triangle (or subdivide further)
    }
    // The final, "middle" triangle is more annoying to draw
    // Start by (mem)copying triangle face parameters for the subface, and dividing side_a & side_b by 2
    memcpy(segment_face->corner, face->corner, 3 * sizeof(GLfloat));
    memcpy(segment_face->side_a, face->side_a, 3 * sizeof(GLfloat));
    memcpy(segment_face->side_b, face->side_b, 3 * sizeof(GLfloat));
    VEC3DIV(segment_face->side_a, 2.);
    VEC3DIV(segment_face->side_b, 2.);
    // Then, add an offset of half of side_a + half of side_b to our corner; this equates to moving to the midpoint of the side opposite to the corner
    VEC3ADD(segment_face->corner, segment_face->side_a);
    VEC3ADD(segment_face->corner, segment_face->side_b);
    // Now side_a & side_b point outside the triangle; to draw the "middle" triangle we need to invert them; since 180° is added to both, counterclockwise order is preserved
    VEC3INVERT(segment_face->side_a);
    VEC3INVERT(segment_face->side_b);
    if (should_free_memory) free(face); // Free memory
    preprocess(segment_face, preprocess_parameter); // "Preprocess"
    draw_triangle_tesselation(segment_face, true, subdivision_reps - 1, preprocess, preprocess_parameter); // At last, draw our small lil' triangle (or subdivide further)
}
void fix_triangle_to_sphere(sphere_face *face, void *parameter) {
    GLfloat radius = *(GLfloat *)parameter;
    // If one were to normalize the corner vector, side_a & side_b point to different points; normalizing the corner followed by side_a & side_b
    // might result in compounding error due to side effects of the corner's normalization to the other points. It's best to calculate all 3 triangle points & normalize.
    VEC3ADD(face->side_a, face->corner); // This gives the 2nd triangle vertice (1st counterclockwise from corner)
    VEC3ADD(face->side_b, face->corner); // This gives the 3rd triangle vertice (2nd counterclockwise from corner)
    // Normalize each point
    VEC3DIV(face->corner, sqrt(VEC3DOT(face->corner, face->corner)) / radius);
    VEC3DIV(face->side_a, sqrt(VEC3DOT(face->side_a, face->side_a)) / radius);
    VEC3DIV(face->side_b, sqrt(VEC3DOT(face->side_b, face->side_b)) / radius);
    // Subtract the corner to get side_a & side_b again
    VEC3SUB(face->side_a, face->corner);
    VEC3SUB(face->side_b, face->corner);
}
// Draw sphere
void draw_sphere(GLfloat r, unsigned short subdivision_reps) {
    GLfloat initial_vertices[4][3] = {
        { 0.,          0.,          r },
        { 0.,          0.942809*r, -0.333333*r },
        {-0.816497*r, -0.471405*r, -0.333333*r },
        { 0.816497*r, -0.471405*r, -0.333333*r }
    };
    GLuint initial_faces[4][3] = {
        { 2, 0, 1 }, // Left
        { 0, 3, 1 }, // Right
        { 0, 2, 3 }, // Bottom
        { 2, 1, 3 }  // Back
    };
    // Store face parameters for triangle to be drawn
    sphere_face face;
    // Iterate over each face
    for (unsigned short i = 0; i < 4; i++) {
        memcpy(face.corner, initial_vertices[initial_faces[i][0]], 3 * sizeof(GLfloat)); // Vertex index is given by the corresponding face item
        // side_a is the 2nd vertice minus the corner; similarly side_b is the 3rd vertice minus the corner (1st vertice)
        memcpy(face.side_a, initial_vertices[initial_faces[i][1]], 3 * sizeof(GLfloat));
        memcpy(face.side_b, initial_vertices[initial_faces[i][2]], 3 * sizeof(GLfloat));
        VEC3SUB(face.side_a, face.corner);
        VEC3SUB(face.side_b, face.corner);
        // draw!
        draw_triangle_tesselation(&face, false, subdivision_reps, fix_triangle_to_sphere, (void *)&r);
    }
}
// (2) HOUSE & GRASS
// Draw a house given a length-3 array or pointer to the front bottom left corner position & size, the house color (vec4) & roof height
// The roof always has a shiny orange color as all pretty house roofs ought to have
void draw_house(GLfloat *pos_vec, GLfloat *size_vec, GLfloat *house_color, GLfloat roof_height) {
    GLfloat curr_pos[3];
    GLfloat curr_side_a[3];
    GLfloat curr_side_b[3];
    GLfloat curr_normal[3];
    GLfloat curr_color[4]; // color is RGBA & hence needs to be a 4-vector

    // ----- HOUSE BODY -----
    // Set parameters for house body
    VEC4(curr_color, 0., 0., 0., 1.);
    glMaterialfv(GL_FRONT, GL_SPECULAR, curr_color);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, house_color);
    VEC3DIV(house_color, 4.);
    glMaterialfv(GL_FRONT, GL_AMBIENT, house_color);
    // Draw front face (sides parallel to x'x & y'y)
    VEC3CPY(curr_pos, pos_vec); // Start out at bottom front left
    VEC3(curr_side_a, size_vec[0], 0., 0.); // bottom side
    VEC3(curr_side_b, 0., size_vec[1], 0.); // left side
    VEC3(curr_normal,  0., 0., 1.);
    draw_parallelogram(curr_pos, curr_side_a, curr_side_b, curr_normal);
    // Draw right face (sides parallel to z'z & y'y)
    VEC3ADD(curr_pos, curr_side_a); // Move to bottom front right corner
    VEC3(curr_side_a, 0., 0., -size_vec[2]); // side a is the bottom of right face
    VEC3(curr_normal, 1., 0., 0.);
    draw_parallelogram(curr_pos, curr_side_a, curr_side_b, curr_normal);
    // Draw back face (sides parallel to x'x & y'y)
    VEC3ADD(curr_pos, curr_side_a); // Move to bottom back right corner
    VEC3(curr_side_a, -size_vec[0], 0., 0.); // side a is the bottom of back face
    VEC3(curr_normal, 0., 0., -1.);
    draw_parallelogram(curr_pos, curr_side_a, curr_side_b, curr_normal);
    // Draw left face (sides parallel to z'z & y'y)
    VEC3ADD(curr_pos, curr_side_a); // Move to bottom back left corner
    VEC3(curr_side_a, 0., 0., size_vec[2]); // side a is the bottom of left face
    VEC3(curr_normal, -1., 0., 0.);
    draw_parallelogram(curr_pos, curr_side_a, curr_side_b, curr_normal);

    // ----- HOUSE ROOF -----
    // Set parameters for house roof
    glMaterialfv(GL_FRONT, GL_AMBIENT, curr_color);
    VEC4(curr_color, 1.5, 0.3, 0., 1.); // red-orange diffuse color
    glMaterialfv(GL_FRONT, GL_DIFFUSE, curr_color);
    VEC4(curr_color, 20., 8., 0., 1.); // orange-yellow diffuse color
    glMaterialfv(GL_FRONT, GL_SPECULAR, curr_color);
    VEC4(curr_color, 100., 0., 0., 0.); // it needs a pointer to the shininess
    glMaterialfv(GL_FRONT, GL_SHININESS, curr_color); // shiny shiny!
    // Draw left roof face
    VEC3ADD(curr_pos, curr_side_b); // Move to top back left corner
    VEC3(curr_side_b, size_vec[0] / 2., roof_height, 0.); // left roof side (goes up to the back of the roof's ridge)
    VEC3CROSS(curr_normal, curr_side_a, curr_side_b); // cross product cause the roof is inclined
    draw_parallelogram(curr_pos, curr_side_a, curr_side_b, curr_normal);
    // Draw right roof face
    VEC3ADD(curr_pos, curr_side_b); // Move up to the back of the roof's ridge
    VEC3(curr_side_b, size_vec[0] / 2., -roof_height, 0.); // right roof side (goes down to the top back right corner of house)
    VEC3CROSS(curr_normal, curr_side_a, curr_side_b);
    draw_parallelogram(curr_pos, curr_side_a, curr_side_b, curr_normal);
    // Draw back roof face
    VEC3(curr_side_a, -size_vec[0] / 2., -roof_height, 0.) // left roof edge goes back down to top left corner
    VEC3(curr_normal, 0., 0., -1.);
    draw_triangle(curr_pos, curr_side_b, curr_side_a, curr_normal); // reverse sides cause a's left & b's right, from the back they're in order clockwise
    // Draw front roof face
    curr_pos[2] += size_vec[2]; // Move to the front of the roof's ridge
    VEC3(curr_normal, 0., 0., 1.);
    draw_triangle(curr_pos, curr_side_a, curr_side_b, curr_normal); // a's left & b's right, from the front they're counterclockwise
}
// Draw grass
bool grassier_grass_enabled = false;
void draw_grass(bool multiple_tiles /*TODO*/) {
    // Green flat color
    GLfloat curr_color[4] = { 0., 0., 0., 1. };
    glMaterialfv(GL_FRONT, GL_SPECULAR, curr_color);
    VEC4(curr_color, 0., 0.5, 0., 1.);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, curr_color);
    glMaterialfv(GL_FRONT, GL_AMBIENT, curr_color);
    // 10x10 size, centered at the origin
    if (!grassier_grass_enabled) { // If we don't enable fancy grass then just draw big nice parallelogram
        GLfloat front_left_corner[] = { -60., 0., 50. };
        GLfloat Δx[] = { 120., 0., 0. };
        GLfloat Δz[] = { 0., 0., -100. };
        GLfloat normal[] = { 0., 1., 0. };
        draw_parallelogram(front_left_corner, Δx, Δz, normal);
        return;
    }
    // If grassier grass enabled, draw grass in 10x10 tiles
    GLfloat Δx[] = { 10., 0., 0. };
    GLfloat Δz[] = { 0., 0., -10. };
    GLfloat normal[] = { 0., 1., 0. };
    for (unsigned i = 0; i < 12; i += 1) // 12 x axis tiles
        for (unsigned j = 0; j < 10; j += 1) { // 10 y axis tiles
            GLfloat front_left_corner[] = { -60. + 10.*i, 0., 50. - 10.*j };
            draw_parallelogram(front_left_corner, Δx, Δz, normal);
        }
}

// ---- TIME KEEPING & SUN/MOON CYCLE MODELING SHENANIGANS ----
// (or how to make the sun move "accurately" in a semicircle)
#define ORBIT_RADIUS 100
unsigned time = 0.; // Time in minutes (e.g. 5 -> 5 minutes past midnight, 125 -> 2:05AM)
bool night_mode_enabled = false;
GLUquadric *the_moon;
// The basis for our sun cycle is the sigmoid - while obviously not corresponding to real world sun movement, and technically not matching
// the instruction to move from precisely x=-100 to x=+100, it allows for a smooth, real-world-like, fade in & out of the sun at sunrise & sunset
float sigmoid(float x) {
    return 1. / (1. + (exp(-x)));
}
float sigmoid_derivative(float sig) {
    return sig * (1. - sig);
}
// Transform sigmoid function to max the sun's orbit & use that to update a pointer to sun's position according to current time
void update_sun_or_moon_pos(float *horizontal_pos, float *vertical_pos) {
    float sig = sigmoid((time / 144.) - 5.); // Sun is at the top of the sky at 12:00pm = 144 * 5 minutes past midnight, goes from s(-5) to s(5) from sunrise to sunset
    float sig_prime = sigmoid_derivative(sig);
    *horizontal_pos = -ORBIT_RADIUS + 2*ORBIT_RADIUS*sig; // Shift sigmoid (0 <= sig(x) <= 1) to go from -R to R
    *vertical_pos = 4*ORBIT_RADIUS*sig_prime; // Shift sigmoid's derivative (0 <= sig'(x) <= 0.25!!!) to go from 0 to R
}
float get_luminescence() {
    if (night_mode_enabled) return 0.5;
    return 0.4 + 0.6*sin(time * M_PI / 1440.); // 0.4 at dawn & dusk, 1.0 at noon
}
// Draw the sun
void draw_sun() {
    // Calculate position
    GLfloat sun_position[] = { 0., 0., 0. };
    update_sun_or_moon_pos(sun_position, sun_position + 1);
    // Translate
    VEC3TRANSLATE(sun_position);
    // Create light source
    glEnable(GL_LIGHT0);
    GLfloat vec4[] = { 0., 0., 0., 1. }; // Set position to zero, since we need to transform for the sphere there's no need to set ligh position to non-zero
    glLightfv(GL_LIGHT0, GL_POSITION, vec4);
    VEC3(vec4, 0.2, 0.2, 0.2); // I also added some abient light
    glLightfv(GL_LIGHT0, GL_AMBIENT, vec4);
    // Set luminescence
    float lum = get_luminescence();
    VEC3(vec4, lum, lum, lum);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, vec4);
    glLightfv(GL_LIGHT0, GL_SPECULAR, vec4);
    // Set sphere color parameters
    VEC3(vec4, 0., 0., 0.);
    glMaterialfv(GL_FRONT, GL_SPECULAR, vec4);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, vec4);
    glMaterialfv(GL_FRONT, GL_AMBIENT, vec4);
    if (night_mode_enabled) {
        VEC3(vec4, 2., 2., 2.);
    } else {
        VEC3(vec4, 3., 4., 0.);
    }
    glMaterialfv(GL_FRONT, GL_EMISSION, vec4);
    // Simulate "fade-in" / "fade-out"
    float μ = 1.; // Moon has half the sun's radius
    if (time <= 480) μ *= sigmoid(time/24. - 7.);
    else if (time >= 960) μ *= sigmoid(53. - time/24.);
    // Draw sun // moon
    if (night_mode_enabled) {
        if (the_moon != NULL) gluDeleteQuadric(the_moon); // Delete if necessary
        the_moon = gluNewQuadric();
        gluSphere(the_moon, μ*4., 32., 32.);
    } else {
        draw_sphere(μ*8., 4.);
    }
    // Undo translate & colors
    VEC3UNTRANSLATE(sun_position);
    VEC4(vec4, 0., 0., 0., 1.);
    glMaterialfv(GL_FRONT, GL_EMISSION, vec4);
}
// Increment time
void inc_time() {
    time = time >= 1440.? 3. : time + 3.; // 1 days has 1440 = 60 * 24 = 12^2 * 10 minutes
    glutPostRedisplay(); // Rerender the scene
}

// ---- CAMERA MOVEMENT ----
float camera_angle = 0.;
float camera_height = 60.;
void move_camera_with_key(int key, int x, int y) {
    // Make it spin
    if(key == GLUT_KEY_LEFT)
        camera_angle -= 0.01 * M_PI;
    else if(key == GLUT_KEY_RIGHT)
        camera_angle += 0.01 * M_PI;
    // Make it go up 'n down
    else if(key == GLUT_KEY_DOWN)
        camera_height -= 3;
    else if(key == GLUT_KEY_UP)
        camera_height += 3;
    else
        return; // Don't rerender for other keys
    // Prevent overflow
    if (camera_angle <= -2*M_PI) camera_angle += 2*M_PI;
    if (camera_angle >= 2*M_PI) camera_angle -= 2*M_PI;
}

// ---- SPOTLIGHT ----
bool spotlight_is_enabled = true;
GLUquadric *the_light_pole;
GLUquadric *the_lightbulb;
void draw_spotlight() {
    // Set position (front left)
    GLfloat spotlight_position[] = { 10., 30., 30., 1. };
    // Create light source (spotlight is GL_LIGHT1) & move it to position
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT1, GL_POSITION, spotlight_position);
    // Diffuse & specular white light
    GLfloat vec4[] = { 1., 1., 1., 1. };
    glLightfv(GL_LIGHT1, GL_DIFFUSE, vec4);
    glLightfv(GL_LIGHT1, GL_SPECULAR, vec4);
    // Make it a spot light!
    float epxponent = 20.;
    glLightfv(GL_LIGHT1, GL_SPOT_EXPONENT, &epxponent);
    float cutoff = 20.;
    glLightfv(GL_LIGHT1, GL_SPOT_CUTOFF, &cutoff);
    // Make it point to the origin!
    VEC3(vec4, -1., -3., -3.); // Reuse vec4 (only 3 number array necessary)
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, vec4);
    // Create cylinder
    if (the_light_pole != NULL) gluDeleteQuadric(the_light_pole); // Delete if necessary
    the_light_pole = gluNewQuadric();
    // Translate
    glPushMatrix();
    spotlight_position[1] = 0; // Don't move it up
    VEC3TRANSLATE(spotlight_position);
    // Rotate
    glPushMatrix();
    glRotatef(-90., 1., 0., 0.);
    // Make it matt gray with a hue of blue
    VEC3(vec4, 0.3, 0.3, 0.35);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, vec4);
    VEC3DIV(vec4, 2.); // Ambient light refraction's the half of diffuse
    glMaterialfv(GL_FRONT, GL_AMBIENT, vec4);
    gluCylinder(the_light_pole, 2., 2., 30., 32., 32.); // Draw
    // Pop matrix
    glPopMatrix();
    // Create lamp
    if (the_lightbulb != NULL) gluDeleteQuadric(the_lightbulb); // Delete if necessary
    the_lightbulb = gluNewQuadric();
    VEC3(spotlight_position, 0, 30., 0.); // Move the lamp up
    VEC3TRANSLATE(spotlight_position);
    // Make it emit light
    VEC3(vec4, 1., 1., 1.);
    glMaterialfv(GL_FRONT, GL_EMISSION, vec4);
    gluSphere(the_lightbulb, 4., 32., 32.);
    glPopMatrix();
    // No side effects no no
    VEC3(vec4, 0., 0., 0.);
    glMaterialfv(GL_FRONT, GL_EMISSION, vec4);
}
void turn_off_spotlight() {
    glDisable(GL_LIGHT1);
    gluDeleteQuadric(the_light_pole);
    gluDeleteQuadric(the_lightbulb);
    the_light_pole = NULL; the_lightbulb = NULL;
}
void toggle_spotlight() {
    spotlight_is_enabled = !spotlight_is_enabled;
    if (!spotlight_is_enabled) turn_off_spotlight();
}

// ---- MENU ----
bool lighting_is_smooth = true;
void handle_menu_action(int id) {
    switch (id) {
    case 0:
        toggle_spotlight();
        break;
    case 1:
        lighting_is_smooth = !lighting_is_smooth;
        if (lighting_is_smooth) glShadeModel(GL_SMOOTH);
        else glShadeModel(GL_FLAT);
        break;
    case 2:
        night_mode_enabled = !night_mode_enabled;
        break;
    case 3:
        grassier_grass_enabled = !grassier_grass_enabled;
        break;
    // when user requests to exit, use freeglut to do so gracefully
    case 4:
        glutLeaveMainLoop();
        break;
    }
}
void create_menu() {
    glutCreateMenu(handle_menu_action);
    // Εδώ χρησιμοποιώ αγγλικά μιας και -στο linux τουλάχιστον- τα ελληνικά σε freeglut εμφανίζονται ως minecraft enchanting table language (αλαμπουρνέζικα)
    glutAddMenuEntry("Toggle the spotlight", 0);
    glutAddMenuEntry("Smooth/Flat lighting toggle", 1);
    glutAddMenuEntry("Day/Night toggle", 2);
    glutAddMenuEntry("Grassier grass toggle", 3); // Turn on grassier grass to rerender the grass in 120 10x10 tiles, making the spotlight appear ✨ smoother ✨
    glutAddMenuEntry("Exit", 4);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// ---- DRAW THE SCENE ----
void draw_scene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the window

    glLoadIdentity(); // Reset model view matrix

    // Camera moves at camera_angle on circle of radius 120. & at camera_height
    gluLookAt(120.*sin(camera_angle), camera_height, 120.*cos(camera_angle), /* Cam. Position */ 0., 0., 0., /* Point to origin */ 0., 1., 0. /* y's up */);

    draw_sun();
    if (spotlight_is_enabled) draw_spotlight();
    draw_grass(false);
    // !!! The z axis is pointing FORWARD (!), i.e. towards the observer/camera
    // SEE: The right hand rule https://www.techmechacademy.com/consult/general-tools/right-hand-rule
    // Here we need to give z > 0 for the bottom *front* left corner (draw_box is written accordingly)
    GLfloat house_bottom_left_pos[] = { -50., 0., 30. };
    GLfloat house_body_size[] = { 30., 25., 40. };
    GLfloat roof_height = 12.;
    GLfloat house_color[] = { 4., 0.5, 1., 1. }; // Pinkish
    draw_house(house_bottom_left_pos, house_body_size, house_color, roof_height); // Draw our beautiful house

    VEC3(house_bottom_left_pos, -12., 0., -5.);
    VEC3(house_body_size, 20., 30., 35.);
    roof_height = 20.;
    VEC4(house_color, 0.3, 0.1, 1., 1.); // Blueish
    draw_house(house_bottom_left_pos, house_body_size, house_color, roof_height); // Draw another one!

    // Rotate the last one to spice things up a little bit (modifying "draw_house" to account for rotation is way too hard)
    glRotatef(-45., 0., 1., 0.);
    VEC3(house_bottom_left_pos, 12., 0., 0.);
    VEC3(house_body_size, 30., 30., 40.);
    roof_height = 15.;
    VEC4(house_color, 0.6, 0.4, 0.2, 1.); // Brown
    draw_house(house_bottom_left_pos, house_body_size, house_color, roof_height); // Draw the final one!

    glutSwapBuffers(); // Swap buffers
}

// Update viewport & projection matrix
void update_window_dimensions(int w, int h) {
    // Update viewport
    glViewport(0, 0, w, h);
    // Calculate aspect ratio (consider height to be 1 if window is resized so that it has no heigh but the title bar)
    unsigned nonZeroHeight = h == 0? 1 : h;
    float aspectRatio = (float)w / (float)h;
    // Request from OpenGL that we work with projection matrix; set working matrix to model view afterwards
    glMatrixMode(GL_PROJECTION);
    // Reset matrix, then update it with current aspect_ratio
    glLoadIdentity();
    gluPerspective(50., aspectRatio, 1, 1000);
    glMatrixMode(GL_MODELVIEW);
}


int main(int argc, char** argv) {
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1000, 600);
    glutCreateWindow("beautiful little village");

    update_window_dimensions(1000, 600); // Set viewport & projection matrix for initial window size
    glEnable(GL_BLEND);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);

    glutDisplayFunc(draw_scene);
    glutIdleFunc(inc_time);
    glutReshapeFunc(update_window_dimensions); // On resize update viewport & projection matrix
    glutSpecialFunc(move_camera_with_key);
    create_menu();

    glutMainLoop();
    if (the_moon != NULL) gluDeleteQuadric(the_moon);
    if (spotlight_is_enabled) {
        gluDeleteQuadric(the_light_pole);
        gluDeleteQuadric(the_lightbulb);
    }
}
