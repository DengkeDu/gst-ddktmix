/* gstddktmix.c */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gstddktmix.h"

GST_DEBUG_CATEGORY_STATIC (gst_ddktmix_debug);
#define GST_CAT_DEFAULT gst_ddktmix_debug

enum {
  PROP_0
};

static GstStaticPadTemplate sink_template = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE("{ RGB, BGR, RGBx, xRGB, BGRx, xBGR, RGBA, ARGB, BGRA, ABGR }"))
);

static GstStaticPadTemplate src_template = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE("{ RGB, BGR, RGBx, xRGB, BGRx, xBGR, RGBA, ARGB, BGRA, ABGR }"))
);

#define gst_ddktmix_parent_class parent_class
G_DEFINE_TYPE (GstDdktmix, gst_ddktmix, GST_TYPE_VIDEO_FILTER);

static void gst_ddktmix_finalize (GObject * object);
static GstFlowReturn gst_ddktmix_transform_frame (GstVideoFilter * filter, GstVideoFrame * in_frame, GstVideoFrame * out_frame);
static gboolean gst_ddktmix_set_info (GstVideoFilter * filter, GstCaps * incaps, GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info);

static void
gst_ddktmix_class_init (GstDdktmixClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstVideoFilterClass *vfilter_class = GST_VIDEO_FILTER_CLASS (klass);

  gobject_class->finalize = gst_ddktmix_finalize;

  gst_element_class_set_static_metadata (element_class,
      "DDK TMix Filter",
      "Filter/Effect/Video",
      "Applies temporal mixing to video frames",
      "Dengke Du <pinganddu90@gmail.com>");

  gst_element_class_add_static_pad_template (element_class, &sink_template);
  gst_element_class_add_static_pad_template (element_class, &src_template);

  vfilter_class->set_info = gst_ddktmix_set_info;
  vfilter_class->transform_frame = gst_ddktmix_transform_frame;
}

static void
gst_ddktmix_init (GstDdktmix * filter)
{
  filter->buffer_index = 0;
  filter->buffers_filled = FALSE;
  
  for (int i = 0; i < 9; i++) {
    filter->frame_buffers[i] = NULL;
  }
}

static void
gst_ddktmix_finalize (GObject * object)
{
  GstDdktmix *filter = GST_DDKTMIX (object);

  for (int i = 0; i < 9; i++) {
    if (filter->frame_buffers[i])
      gst_buffer_unref (filter->frame_buffers[i]);
  }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
gst_ddktmix_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstDdktmix *ddktmix = GST_DDKTMIX (filter);
  ddktmix->info = *in_info;
  return TRUE;
}

static GstFlowReturn
gst_ddktmix_transform_frame (GstVideoFilter * filter, GstVideoFrame * in_frame,
    GstVideoFrame * out_frame)
{
  GstDdktmix *ddktmix = GST_DDKTMIX (filter);
  gint width, height, stride;
  gint pixel_stride;
  GstVideoFrame frame_array[9];
  gboolean map_success = TRUE;
  
  width = GST_VIDEO_FRAME_WIDTH (in_frame);
  height = GST_VIDEO_FRAME_HEIGHT (in_frame);
  stride = GST_VIDEO_FRAME_PLANE_STRIDE (in_frame, 0);
  pixel_stride = GST_VIDEO_FRAME_COMP_PSTRIDE (in_frame, 0);
  
  // Store current frame
  if (ddktmix->frame_buffers[ddktmix->buffer_index])
    gst_buffer_unref (ddktmix->frame_buffers[ddktmix->buffer_index]);
    
  ddktmix->frame_buffers[ddktmix->buffer_index] = gst_buffer_copy (in_frame->buffer);
  
  ddktmix->buffer_index = (ddktmix->buffer_index + 1) % 9;
  if (ddktmix->buffer_index == 0)
    ddktmix->buffers_filled = TRUE;
    
  // If we don't have enough frames yet, just copy the input
  if (!ddktmix->buffers_filled) {
    gst_video_frame_copy(out_frame, in_frame);
    return GST_FLOW_OK;
  }
  
  // Map all frames
  for (int i = 0; i < 9; i++) {
    if (!gst_video_frame_map (&frame_array[i], &ddktmix->info, ddktmix->frame_buffers[i], GST_MAP_READ)) {
      GST_ERROR_OBJECT (ddktmix, "Failed to map video frame %d", i);
      map_success = FALSE;
      break;
    }
  }
  
  if (!map_success) {
    // Unmap any successfully mapped frames
    for (int i = 0; i < 9; i++) {
      if (frame_array[i].buffer != NULL) {
        gst_video_frame_unmap (&frame_array[i]);
      }
    }
    return GST_FLOW_ERROR;
  }
  
  // Get pointers to the pixel data of each frame
  guint8 *out_pixels = GST_VIDEO_FRAME_PLANE_DATA (out_frame, 0);
  guint8 *frame_pixels[9];
  for (int i = 0; i < 9; i++) {
    frame_pixels[i] = GST_VIDEO_FRAME_PLANE_DATA (&frame_array[i], 0);
  }
  
  // Perform 1:1:1 weighted average
  for (gint y = 0; y < height; y++) {
    guint8 *out_line = out_pixels + y * stride;
    guint8 *frame_lines[9];
    for (int i = 0; i < 9; i++) {
      frame_lines[i] = frame_pixels[i] + y * stride;
    }
    
    for (gint x = 0; x < width * pixel_stride; x++) {
      gint sum = frame_lines[0][x] + frame_lines[1][x] + frame_lines[2][x] + frame_lines[3][x] + frame_lines[4][x] + frame_lines[5][x] + frame_lines[6][x] + frame_lines[7][x] + frame_lines[8][x];
      out_line[x] = sum / 9;
    }
  }
  
  // Unmap frames
  for (int i = 0; i < 9; i++) {
    gst_video_frame_unmap (&frame_array[i]);
  }
  
  return GST_FLOW_OK;
}

/* plugin.c */
static gboolean
plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "ddktmix",
      GST_RANK_NONE, GST_TYPE_DDKTMIX);
}

GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    ddktmix,
    "DDK TMix Plugin",
    plugin_init,
    "1.0",
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)
