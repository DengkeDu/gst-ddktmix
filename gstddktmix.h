/* gstddktmix.h */
#ifndef __GST_DDKTMIX_H__
#define __GST_DDKTMIX_H__

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>

G_BEGIN_DECLS

#define GST_TYPE_DDKTMIX (gst_ddktmix_get_type())
G_DECLARE_FINAL_TYPE (GstDdktmix, gst_ddktmix, GST, DDKTMIX, GstVideoFilter)

struct _GstDdktmix {
  GstVideoFilter parent;
  
  GstVideoInfo info;
  GstBuffer *frame_buffers[9];
  guint buffer_index;
  gboolean buffers_filled;
};

G_END_DECLS

#endif /* __GST_DDKTMIX_H__ */
