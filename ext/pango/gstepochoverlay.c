/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) <2005> Tim-Philipp Müller <tim@centricular.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/**
 * SECTION:element-clockoverlay
 * @see_also: #GstBaseTextOverlay, #GstTimeOverlay
 *
 * This element overlays the current clock time on top of a video
 * stream. You can position the text and configure the font details
 * using the properties of the #GstBaseTextOverlay class. By default, the
 * time is displayed in the top left corner of the picture, with some
 * padding to the left and to the top.
 *
 * <refsect2>
 * <title>Example launch lines</title>
 * |[
 * gst-launch -v videotestsrc ! clockoverlay ! xvimagesink
 * ]| Display the current time in the top left corner of the video picture
 * |[
 * gst-launch -v videotestsrc ! clockoverlay halign=right valign=bottom text="Edge City" shaded-background=true ! videoconvert ! ximagesink
 * ]| Another pipeline that displays the current time with some leading
 * text in the bottom right corner of the video picture, with the background
 * of the text being shaded in order to make it more legible on top of a
 * bright video background.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstepochoverlay.h"
#include <gst/video/video.h>
#include <sys/time.h>
#include <inttypes.h>

// enum
// {
//   PROP_0,
//   PROP_LAST
// };

#define gst_epoch_overlay_parent_class parent_class
G_DEFINE_TYPE (GstEpochOverlay, gst_epoch_overlay, GST_TYPE_BASE_TEXT_OVERLAY);

static void gst_epoch_overlay_finalize (GObject * object);
static void gst_epoch_overlay_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_epoch_overlay_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

/* Called with lock held */
static gchar* gst_epoch_overlay_get_text (GstBaseTextOverlay * overlay, GstBuffer * video_frame)
{
  gchar buf[256];

  struct timeval tp;

  int ret = gettimeofday(&tp, NULL);
  if (ret == 0) {
    unsigned long long micro = tp.tv_sec;
    micro = micro * 1000000L + tp.tv_usec;
    sprintf(buf, "%" PRIu64, micro);
  } else {
    sprintf(buf, "gettimeofday failed");
  }

  overlay->need_render = TRUE;
  //overlay->text = g_strdup(buf);

  return g_strdup (buf);
}

static void gst_epoch_overlay_class_init (GstEpochOverlayClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseTextOverlayClass *gsttextoverlay_class;
  PangoContext *context;
  PangoFontDescription *font_description;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gsttextoverlay_class = (GstBaseTextOverlayClass *) klass;

  gobject_class->finalize = gst_epoch_overlay_finalize;
  gobject_class->set_property = gst_epoch_overlay_set_property;
  gobject_class->get_property = gst_epoch_overlay_get_property;

  gst_element_class_set_static_metadata (gstelement_class, "Epoch overlay",
      "Filter/Editor/Video",
      "Overlays the current time in microseconds from the unix epoch on a video stream",
      "Tim-Philipp Müller <tim@centricular.net> with modifications from Konstantinos Sofokleous <kostas@epoch.com>");

  gsttextoverlay_class->get_text = gst_epoch_overlay_get_text;

  g_mutex_lock (gsttextoverlay_class->pango_lock);
  context = gsttextoverlay_class->pango_context;

  pango_context_set_language (context, pango_language_from_string ("en_US"));
  pango_context_set_base_dir (context, PANGO_DIRECTION_LTR);

  font_description = pango_font_description_new ();
  pango_font_description_set_family_static (font_description, "Courier");
  pango_font_description_set_style (font_description, PANGO_STYLE_NORMAL);
  pango_font_description_set_variant (font_description, PANGO_VARIANT_NORMAL);
  pango_font_description_set_weight (font_description, PANGO_WEIGHT_NORMAL);
  pango_font_description_set_stretch (font_description, PANGO_STRETCH_NORMAL);
  pango_font_description_set_size (font_description, 50 * PANGO_SCALE);
  pango_context_set_font_description (context, font_description);
  pango_font_description_free (font_description);
  g_mutex_unlock (gsttextoverlay_class->pango_lock);
}


static void gst_epoch_overlay_finalize (GObject * object)
{
  //GstEpochOverlay *overlay = GST_EPOCH_OVERLAY (object);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}


static void gst_epoch_overlay_init (GstEpochOverlay * overlay)
{
  GstBaseTextOverlay *textoverlay;

  textoverlay = GST_BASE_TEXT_OVERLAY (overlay);

  textoverlay->valign = GST_BASE_TEXT_OVERLAY_VALIGN_TOP;
  textoverlay->halign = GST_BASE_TEXT_OVERLAY_HALIGN_LEFT;
}


static void gst_epoch_overlay_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstEpochOverlay *overlay = GST_EPOCH_OVERLAY (object);

  GST_OBJECT_LOCK (overlay);
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (overlay);
}


static void gst_epoch_overlay_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstEpochOverlay *overlay = GST_EPOCH_OVERLAY (object);

  GST_OBJECT_LOCK (overlay);
  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
  GST_OBJECT_UNLOCK (overlay);
}
