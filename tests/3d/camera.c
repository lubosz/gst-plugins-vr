#include <glib.h>
#include <stdio.h>

/*
void setup_test_hdrlist(void);
void setup_test_mimetypes(void);
void setup_test_request(void);
*/

// #include <gst/check/gstcheck.h>

#define GST_USE_UNSTABLE_API 1
#include <gst/gl/gl.h>
#include <gst/gl/gstglfuncs.h>
#include <gst/gl/gstglcontext.h>
#include <gst/gl/gstglupload.h>

#define WIDTH 1280
#define HEIGHT 720

static GstGLDisplay *display = NULL;
static GstGLContext *context = NULL;
static GstGLWindow *window = NULL;
static GstGLShader *shader;
static GstGLUpload *upload;
static guint tex_id;
static GLint shader_attr_position_loc;
static GLint shader_attr_texture_loc;

static void
draw_render (gpointer data)
{
  GstGLContext *context = data;
  GstGLContextClass *context_class = GST_GL_CONTEXT_GET_CLASS (context);
  const GstGLFuncs *gl = context->gl_vtable;
  GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

  gl->Clear (GL_COLOR_BUFFER_BIT);

  gst_gl_shader_use (shader);

  gl->ActiveTexture (GL_TEXTURE0);
  gl->BindTexture (GL_TEXTURE_2D, tex_id);
  gst_gl_shader_set_uniform_1i (shader, "s_texture", 0);

  gl->DrawElements (GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

  context_class->swap_buffers (context);
}

static void
init (gpointer data)
{
  GError *error = NULL;

  shader = gst_gl_shader_new_default (context, &error);
  g_assert_false (shader == NULL);

  shader_attr_position_loc =
      gst_gl_shader_get_attribute_location (shader, "a_position");
  shader_attr_texture_loc =
      gst_gl_shader_get_attribute_location (shader, "a_texCoord");
}

static void
test_display ()
{
  GError *error = NULL;
  gint i = 0;
  gst_init (NULL, NULL);

  display = gst_gl_display_new ();
  context = gst_gl_context_new (display);

  gst_gl_context_create (context, 0, &error);
  g_assert (error == NULL);

  window = gst_gl_context_get_window (context);

  upload = gst_gl_upload_new (context);
  /*
     shader = gst_gl_shader_new_default (context, &error);
   */
  g_assert (error == NULL);
  g_assert (context != NULL);
  g_assert (context->gl_vtable != NULL);
  // g_assert (context->gl_vtable->GetError () != GL_NONE);


  gst_gl_window_set_preferred_size (window, WIDTH, HEIGHT);
  gst_gl_window_draw (window);

  gst_gl_window_send_message (window, GST_GL_WINDOW_CB (init), context);

  while (i < 2) {
    gst_gl_window_send_message (window, GST_GL_WINDOW_CB (draw_render),
        context);
    i++;
  }

  gst_object_unref (window);
  gst_object_unref (context);
  gst_object_unref (display);

}

int
main (int argc, char *argv[])
{
  g_test_init (&argc, &argv, NULL);
  /*
     g_test_set_nonfatal_assertions();
     setup_test_hdrlist();
     setup_test_mimetypes();
     setup_test_request();
   */

  g_test_add_func ("/gst3d/camera", test_display);

  return g_test_run ();
}
