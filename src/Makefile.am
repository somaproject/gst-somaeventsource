# plugindir is set in configure

##############################################################################
# change libgstplugin.la to something more suitable, e.g. libmysomething.la  #
##############################################################################
plugin_LTLIBRARIES = libsomaeventsource.la

##############################################################################
# for the next set of variables, rename the prefix if you renamed the .la,   #
#  e.g. libgstplugin_la_SOURCES => libmysomething_la_SOURCES                 #
#       libgstplugin_la_CFLAGS  => libmysomething_la_CFLAGS                  #
#       libgstplugin_la_LIBADD  => libmysomething_la_LIBADD                  #
#       libgstplugin_la_LDFLAGS => libmysomething_la_LDFLAGS                 #
##############################################################################

# sources used to compile this plug-in
libsomaeventsource_la_SOURCES = gstsomaeventsource.c 

# flags used to compile this plugin
# add other _CFLAGS and _LIBS as needed
libsomaeventsource_la_CFLAGS = $(GST_CFLAGS) $(GST_PLUGINS_BASE_CFLAGS) $(SOMANETEVENT_CFLAGS)
libsomaeventsource_la_LIBADD = $(GST_BASE_LIBS) $(GST_LIBS) $(GST_PLUGINS_BASE_LIBS) $(SOMANETEVENT_LIBS)
libsomaeventsource_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) $(GST_BASE_LDFLAGS) -lgstaudio-0.10


# headers we need but don't want installed
noinst_HEADERS = gstsomaeventsource.h
