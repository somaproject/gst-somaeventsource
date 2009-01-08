import pygst
pygst.require('0.10')
import gst
import gobject
gobject.threads_init ()



class MySink(gst.Element):

    _sinkpadtemplate = gst.PadTemplate ("sinkpadtemplate",
                                        gst.PAD_SINK,
                                        gst.PAD_ALWAYS,
                                        gst.caps_new_any())

    _srcpadtemplate = gst.PadTemplate ("srcpadtemplate",
                                        gst.PAD_SRC,
                                        gst.PAD_ALWAYS,
                                        gst.caps_new_any())

    def __init__(self):
        gst.Element.__init__(self)

        gst.info('creating srcpadpad')
        self.srcpad = gst.Pad(self._srcpadtemplate, "src")
        gst.info('adding srcpad to self')
        self.add_pad(self.srcpad)
        self.srcpad.set_getcaps_function(self.src_getcaps_function)
        self.srcpad.set_setcaps_function(self.src_setcaps_function)


        gst.info('creating sinkpad')
        self.sinkpad = gst.Pad(self._sinkpadtemplate, "sink")
        gst.info('adding sinkpad to self')
        self.add_pad(self.sinkpad)
        self.sinkpad.set_getcaps_function(self.sink_getcaps_function)
        self.sinkpad.set_setcaps_function(self.sink_setcaps_function)

        gst.info('setting chain/event functions')
        self.sinkpad.set_chain_function(self.chainfunc)
        self.sinkpad.set_event_function(self.eventfunc)

    def src_getcaps_function(self, *foo):
        print "src getcaps", foo
        caps = gst.caps_from_string('audio/x-raw-float, rate=32000, channels=1, endianness=1234, width=16')
        gst.info('src getcaps')
        return caps

    def src_setcaps_function(self, *foo):
        print "src setcaps"

        gst.info('src setcaps')

    def sink_getcaps_function(self, *foo):

        gst.info('sink getcaps')
        caps = gst.caps_from_string("soma/events, src=1")
        return caps
    
    def sink_setcaps_function(self, *foo):
        print "setcaps"
        gst.info('sink setcaps')

        
    def chainfunc(self, pad, buffer):
        self.info("%s timestamp(buffer):%d" % (pad, buffer.timestamp))
        # create different type of buffer
        # stick the data into it
        # go on your merry way
        
        return self.srcpad.push(buffer)

    def eventfunc(self, pad, event):
        self.info("%s event:%r" % (pad, event.type))
        return True

gobject.type_register(MySink)


src = gst.element_factory_make('fakesrc')
gst.info('About to create MySink')
sink = MySink()

pipeline = gst.Pipeline()
pipeline.add(src, sink)

src.link(sink)

pipeline.set_state(gst.STATE_PLAYING)

gobject.MainLoop().run()
