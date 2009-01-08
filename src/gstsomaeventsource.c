/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2008 Eric Jonas <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-somaeventsource
 *
 * FIXME:Describe somaeventsource here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m somaeventsource ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>
#include <gst/audio/gstaudioclock.h>
#include <somanetevent/netevent.h> 
#include "gstsomaeventsource.h"

GST_DEBUG_CATEGORY_STATIC (gst_soma_event_source_debug);
#define GST_CAT_DEFAULT gst_soma_event_source_debug

/* Filter signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SILENT, 
  PROP_SOMAIP
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */

/* static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src", */
/*     GST_PAD_SRC, */
/*     GST_PAD_ALWAYS, */
/*     GST_STATIC_CAPS ("soma/eventset") */
/*     ); */

GST_BOILERPLATE (GstSomaEventSource, gst_soma_event_source, GstPushSrc, 
		 GST_TYPE_PUSH_SRC);

static void gst_soma_event_source_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_soma_event_source_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static GstFlowReturn gst_soma_event_source_create (GstPushSrc * psrc, GstBuffer ** buffer);

static gboolean gst_soma_event_source_setcaps (GstBaseSrc * bsrc, GstCaps * caps);
static GstCaps *  gst_soma_event_source_getcaps (GstBaseSrc * bsrc);

static void gst_soma_event_source_fixate(GstPad * pad, GstCaps * caps); 
static GstCaps * gst_soma_event_source_get_all_caps(); 

static GstStateChangeReturn
gst_soma_event_source_change_state (GstElement * element, GstStateChange transition); 

// clock
static GstClock*
gst_soma_event_source_provide_clock(GstElement * element);

static GstClockTime
gst_soma_event_source_get_clock_time(GstClock * clock, gpointer element);

//static GstFlowReturn gst_soma_event_source_chain (GstPad * pad, GstBuffer * buf);

/* GObject vmethod implementations */

static void
gst_soma_event_source_base_init (gpointer gclass)
{
  //GstPushSrcClass *pushelement_class = GST_PUSH_SRC_CLASS (gclass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

  gst_element_class_set_details_simple(element_class,
    "Soma Event Source",
    "Source/Soma/Event",
    "A clock-synchronous element providing Events off the network from Soma", 
    "Eric Jonas <jonas@mit.edu>");

  gst_element_class_add_pad_template (element_class,
				      gst_pad_template_new("src", GST_PAD_SRC, 
							   GST_PAD_ALWAYS, 
							   gst_soma_event_source_get_all_caps())); 
  
 
}

/* initialize the somaeventsource's class */
static void
gst_soma_event_source_class_init (GstSomaEventSourceClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSrcClass *gstbasesrc_class;
  GstPushSrcClass *gstpushsrc_class;


  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstpushsrc_class = (GstPushSrcClass *) klass; 
  gstbasesrc_class = (GstBaseSrcClass * ) klass; 

  gobject_class->set_property = gst_soma_event_source_set_property;
  gobject_class->get_property = gst_soma_event_source_get_property;
  
  
  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verb/ose output ?",
          FALSE, G_PARAM_READWRITE));
  
  g_object_class_install_property(gobject_class, PROP_SOMAIP, 
				  g_param_spec_string("soma-ip",
						      "SomaIP", 
						      "The IP address of the Soma HW", 
						      "10.0.0.2", 
						      G_PARAM_READWRITE)); 

/*   g_object_class_install_property (gobject_class, PROP_PROVIDE_CLOCK, */
/* 				   g_param_spec_boolean ("provide-clock", "Provide Clock", */
/* 							 "Provide a clock to be used as the global pipeline clock", */
/* 							 TRUE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)); */
  
  //gstelement_class-> = gst_soma_event_source_get_clock;  FIXME
  gstelement_class->provide_clock = 
    GST_DEBUG_FUNCPTR(gst_soma_event_source_provide_clock); 
  gstbasesrc_class->get_caps = gst_soma_event_source_getcaps; 
  gstbasesrc_class->set_caps = gst_soma_event_source_setcaps; 

  gstpushsrc_class->create = gst_soma_event_source_create;

  gstelement_class->change_state = GST_DEBUG_FUNCPTR (gst_soma_event_source_change_state);
  
  
}

/* initialize the new element
 * initialize instance structure
 */
static void
gst_soma_event_source_init (GstSomaEventSource * src,
    GstSomaEventSourceClass * gclass)
{

  src->silent = FALSE;
  src->somaip = "10.0.0.2"; 
  src->caps = gst_soma_event_source_get_all_caps();
  src->eventsrc = 0; 
  //src->debugfile = fopen("/tmp/gstsomaeventsrc.debug.dat", "w"); 

  gst_pad_set_fixatecaps_function (GST_BASE_SRC_PAD (src),
      gst_soma_event_source_fixate);

  gst_base_src_set_live (GST_BASE_SRC (src), TRUE);
  
}

static void
gst_soma_event_source_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstSomaEventSource *filter = GST_SOMAEVENTSOURCE (object);

  switch (prop_id) {
  case PROP_SILENT:
    filter->silent = g_value_get_boolean (value);
    break;
  case PROP_SOMAIP:
    filter->somaip = (char*) g_value_get_string(value); 
    break; 
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

static void
gst_soma_event_source_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstSomaEventSource *filter = GST_SOMAEVENTSOURCE (object);

  switch (prop_id) {
  case PROP_SILENT:
    g_value_set_boolean (value, filter->silent);
    break;
  case PROP_SOMAIP:
    g_value_set_string(value, filter->somaip); 
    break; 
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}

/* GstElement vmethod implementations */

/* this function handles the link with other elements */
/* static gboolean */
/* gst_soma_event_source_setcaps (GstPad * pad, GstCaps * caps) */
/* { */
/*   GstSomaEventSource *filter; */

/*   filter = GST_SOMAEVENTSOURCE (gst_pad_get_parent (pad)); */
/*   gst_object_unref (filter); */

/*   return gst_pad_set_caps (filter->srcpad, caps); */
/* } */


static gboolean
gst_soma_event_source_setcaps (GstBaseSrc * bsrc, GstCaps * caps)
{

  gboolean res = TRUE;
  GstSomaEventSource * sesrc;
  const GstStructure *structure; 
  sesrc = GST_SOMAEVENTSOURCE (bsrc);

  if (sesrc->caps) {
    gst_caps_unref (sesrc->caps);
  }

  sesrc->caps = gst_caps_copy (caps);

  if(gst_caps_get_size (caps) < 1) {
    return FALSE; 
  }

  structure = gst_caps_get_structure(caps, 0); 
  res &= gst_structure_get_int(structure, "src", &sesrc->eventsrc); 

  
  return res;
}



static GstCaps *
gst_soma_event_source_getcaps (GstBaseSrc * bsrc)
{
  GstSomaEventSource *sesrc;

  sesrc = GST_SOMAEVENTSOURCE (bsrc);

  g_return_val_if_fail (sesrc->caps, NULL);

  return gst_caps_copy (sesrc->caps);
}



/* chain function
 * this function does the actual processing
 */
/* static GstFlowReturn */
/* gst_soma_event_source_chain (GstPad * pad, GstBuffer * buf) */
/* { */
/*   GstSomaEventSource *filter; */

/*   filter = GST_SOMAEVENTSOURCE (GST_OBJECT_PARENT (pad)); */

/*   if (filter->silent == FALSE) */
/*     g_print ("I'm plugged, therefore I'm in.\n"); */

/*   /\* just push out the incoming buffer without touching it *\/ */
/*   return gst_pad_push (filter->srcpad, buf); */
/* } */


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
//somaeventsource_init (GstSomaEventSource * somaeventsource)
somaeventsource_init (GstPlugin * gstsource)
{
  /* debug category for fltering log messages
   *
   * exchange the string 'Template somaeventsource' with your description
   */

  //GstSomaEventSource* somaeventsource = GST_SOMAEVENTSOURCE(gstsource); 
  GST_DEBUG_CATEGORY_INIT (gst_soma_event_source_debug, "somaeventsource",
      0, "Template somaeventsource");

  return gst_element_register (gstsource, "somaeventsource", GST_RANK_NONE,
			       GST_TYPE_SOMAEVENTSOURCE);
}


static GstFlowReturn
gst_soma_event_source_create (GstPushSrc * psrc, GstBuffer ** buffer)
{
  //GstDc1394 *src;
  GstBuffer *outbuf; 
  GstCaps *caps; 
  //dc1394video_frame_t *frame[1];
  GstFlowReturn res = GST_FLOW_OK;
  //dc1394error_t err;
  int i = 0; 
  GstSomaEventSource * sesrc;
  sesrc = GST_SOMAEVENTSOURCE (psrc);
  
  int MAXEVENT = 100; 
  int rxcnt = 0; 
  int srcrxcnt = 0; // how many of the events are for our filtered src; 
  // create a buffer
  sesrc->clocktime++; 
  struct event_t * eventbuffer = malloc(MAXEVENT * sizeof(struct event_t)); 
  struct event_t * srconlyeventbuffer = malloc(MAXEVENT * sizeof(struct event_t)); 
  struct event_t * cursrcevt = srconlyeventbuffer; 
  
  while ((rxcnt == 0) || (rxcnt == NETEVENT_EGETEVENTS)) {
    rxcnt = NetEvent_getEvents(sesrc->pnh, eventbuffer, MAXEVENT); 
  } 
  
  if(rxcnt < 0) {
    free(eventbuffer); 
    free(srconlyeventbuffer); 
  } 

  // now update the time information
  for(i = 0; i < rxcnt; i++) {
    if(eventbuffer[i].src == 0 && eventbuffer[i].cmd == 0x10) {
      // extract out the time
      uint64_t somatime = 0; 
      somatime = eventbuffer[i].data[0]; 
      somatime = somatime << 16; 
      somatime |= eventbuffer[i].data[1]; 
      somatime = somatime << 16; 
      somatime |= eventbuffer[i].data[2]; 

      sesrc->clocktime = somatime * 20000; 
      // FIXME: what is the time of the first buffer? That is,  is
      // the buffer time the time-at-which-the-buffer was created? 
    } 
    if (eventbuffer[i].src == sesrc->eventsrc) {
      memcpy(cursrcevt, &eventbuffer[i], sizeof(struct event_t));   
      // debug FIXME: 
      //fwrite(&eventbuffer[i], sizeof(struct event_t ), 1,  sesrc->debugfile); 
      cursrcevt++; 
      srcrxcnt++; 
    }
  }
  outbuf = gst_buffer_new_and_alloc (srcrxcnt * sizeof(struct event_t)); 
  
  memcpy (GST_BUFFER_MALLOCDATA (outbuf), (guchar *) srconlyeventbuffer,
	  srcrxcnt * sizeof(struct event_t)); 

  GST_BUFFER_DATA (outbuf) = GST_BUFFER_MALLOCDATA (outbuf); 

  caps = gst_pad_get_caps (GST_BASE_SRC_PAD (psrc)); 
  gst_buffer_set_caps (outbuf, caps);
  gst_caps_unref (caps); 

  free(eventbuffer); 
  free(srconlyeventbuffer); 

  GST_BUFFER_TIMESTAMP (outbuf) = sesrc->clocktime; 

  *buffer = outbuf; 

  return res; 

/* error: */
/*   { */
/*     return GST_FLOW_ERROR; */
/*   } */

}


GstCaps *
gst_soma_event_source_get_all_caps ()
{
  /* 
     generate all possible caps

   */
  //GST_LOG_OBJECT ( "get_all_caps"); 

  GstCaps *gcaps;
  //gint i = 0;

  gcaps = gst_caps_new_simple ("soma/event", 
			       "src", GST_TYPE_INT_RANGE, 0, 79, NULL);
  

  return gcaps;

}


static GstStateChangeReturn
gst_soma_event_source_change_state (GstElement * element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstSomaEventSource * src = GST_SOMAEVENTSOURCE (element);
  int i; 
  
  switch (transition) {
  case GST_STATE_CHANGE_NULL_TO_READY: 
    GST_LOG_OBJECT (src, "State change null to ready"); 
    GST_LOG_OBJECT (src, "IP is %s", src->somaip); 
    src->pnh = NetEvent_new(src->somaip); 
    char * clockname = "SomaNetEventClock"; 
    src->clocktime = 0; 
    src->peventclock = gst_audio_clock_new(clockname, 
					   gst_soma_event_source_get_clock_time, 
					   src);  
    break; 
    
  case GST_STATE_CHANGE_READY_TO_PAUSED:
    GST_LOG_OBJECT (src, "State ready to paused"); 
    break; 
     
  case GST_STATE_CHANGE_PAUSED_TO_PLAYING: 
    GST_LOG_OBJECT (src, "State change paused to playing"); 
    for (i = 0; i < 256; i++) {
      NetEvent_setMask(src->pnh, src->eventsrc, i); 
    }
    GST_LOG_OBJECT (src, "Listening to event source %d", src->eventsrc); 
    NetEvent_setMask(src->pnh, 0, 0x10); // always get the clock; 
    NetEvent_startEventRX(src->pnh); 
    break; 
    
  default:
    break; 
  } 
  
  if (ret == GST_STATE_CHANGE_FAILURE) 
    return ret; 
  
   // now the other direction


   ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition); 

   switch (transition) { 
   case GST_STATE_CHANGE_PLAYING_TO_PAUSED: 
     GST_LOG_OBJECT (src, "State change playing to paused"); 
     NetEvent_stopEventRX(src->pnh); 

     break; 

   case GST_STATE_CHANGE_PAUSED_TO_READY: 
     GST_LOG_OBJECT (src, "State change paused to ready"); 

/*       if (src->camera && !gst_dc1394_change_camera_transmission (src, FALSE)) { */

/*         if (src->camera) { */
/*           dc1394_camera_free (src->camera); */
/*         } */
/*         src->camera = NULL; */

/*         if (src->caps) { */
/*           gst_caps_unref (src->caps); */
/*           src->caps = NULL; */
/*         } */

/*         ret = GST_STATE_CHANGE_FAILURE; */
/*       } */

     break;
     case GST_STATE_CHANGE_READY_TO_NULL:
       GST_LOG_OBJECT (src, "State change ready to null"); 
       NetEvent_free(src->pnh); ; 
       //free(src->peventclock); 
       GST_LOG_OBJECT (src, "State change ready to null: done with freeing"); 
       
/*       if (src->caps) { */
/*         gst_caps_unref (src->caps); */
/*         src->caps = NULL; */
/*       } */
       break; 

     default: 
       break; 
   } 

  return ret;
}

static void
gst_soma_event_source_fixate (GstPad * pad, GstCaps * caps)
{

  GstSomaEventSource *src = GST_SOMAEVENTSOURCE (gst_pad_get_parent (pad));
/*   GstStructure *structure; */
/*   int i; */

/*   GST_LOG_OBJECT (src, " fixating caps to closest to 320x240 , 30 fps"); */

  gst_object_unref (GST_OBJECT (src));
}




static GstClock*
gst_soma_event_source_provide_clock(GstElement * element)
{
  
  GstClock * clock; 
  GstSomaEventSource * src = GST_SOMAEVENTSOURCE (element);
  if (src->peventclock == 0) {
    GST_ERROR_OBJECT(src, "clock not available to provide"); 
    return NULL; 
  }
  clock = GST_CLOCK_CAST(gst_object_ref(src->peventclock)); 

  return src->peventclock;
}

static GstClockTime
gst_soma_event_source_get_clock_time(GstClock * clock, gpointer element)
{
  GstSomaEventSource * src = GST_SOMAEVENTSOURCE (element);

  //GST_LOG_OBJECT (src, "creating clock");
  return src->clocktime;

}



GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "somaeventsource",
    "Soma Network Event Source",
    somaeventsource_init,
    VERSION,
    "GPL",
    "Soma",
    "http://soma.mit.edu/"
)

/* gstreamer looks for this structure to register somaeventsources
 *
 * exchange the string 'Template somaeventsource' with your somaeventsource description
 */
