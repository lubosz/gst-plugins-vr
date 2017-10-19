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
#include <gst/gl/gstglcontext.h>
#include <gst/gl/gstglupload.h>

#include "../../gst-libs/gst/3d/gst3dmesh.h"

#define WIDTH 1280
#define HEIGHT 720

static GstGLDisplay *display = NULL;
static GstGLContext *context = NULL;
static GstGLWindow *window = NULL;
static GstGLUpload *upload;
static guint tex_id;

static Gst3DShader *shader;
static Gst3DMesh *mesh;


static void
draw_render (gpointer data)
{
  GstGLContext *context = data;
  GstGLContextClass *context_class = GST_GL_CONTEXT_GET_CLASS (context);
  const GstGLFuncs *gl = context->gl_vtable;

  gl->Clear (GL_COLOR_BUFFER_BIT);

  gst_gl_shader_use (shader->shader);

  gl->ActiveTexture (GL_TEXTURE0);
  gl->BindTexture (GL_TEXTURE_2D, tex_id);
  gst_gl_shader_set_uniform_1i (shader->shader, "s_texture", 0);

  gst_3d_mesh_bind (mesh);
  gst_3d_mesh_draw (mesh);

  context_class->swap_buffers (context);
}

static void
init (gpointer data)
{
  shader =
      gst_3d_shader_new_vert_frag (context, "mvp_uv.vert", "debug_uv.frag");

  mesh = gst_3d_mesh_new_sphere (context, 0.5, 100, 100);
  gst_gl_shader_use (shader->shader);
  gst_3d_mesh_bind_shader (mesh, shader);

  g_assert_false (shader == NULL);
}

static void
test_mesh ()
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

  g_test_add_func ("/gst3d/mesh", test_mesh);

  return g_test_run ();
}
