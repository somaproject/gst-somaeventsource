
import sys, os, time, thread
import gobject
import pygst
pygst.require("0.10")
import gst

class CLI_Main:
    def __init__(self, filename):
        self.player = gst.Pipeline("player")
        self.filesink = gst.element_factory_make("filesink", "filesink")
        self.filesink.set_property("location", filename)

#        self.somasrc = gst.element_factory_make("videotestsrc", "ses")
        self.somasrc = gst.element_factory_make("somaeventsource", "ses")

        self.player.add(self.somasrc, self.filesink)
        gst.element_link_many(self.somasrc, self.filesink)
        
        bus = self.player.get_bus()
        bus.add_signal_watch()
        bus.connect("message", self.on_message)
        
    def on_message(self, bus, message):
        t = message.type
        if t == gst.MESSAGE_EOS:
            self.player.set_state(gst.STATE_NULL)
            self.playmode = False
        elif t == gst.MESSAGE_ERROR:
            self.player.set_state(gst.STATE_NULL)
            err, debug = message.parse_error()
            print "Error: %s" % err, debug
            self.playmode = False
            
    def start(self):
        self.playmode = True
        self.player.set_state(gst.STATE_PLAYING)
        while self.playmode:
            time.sleep(1)
                    
        time.sleep(1)
        loop.quit()

filename = "/tmp/testevent.dat"
mainclass = CLI_Main(filename)
thread.start_new_thread(mainclass.start, ())
gobject.threads_init()
loop = gobject.MainLoop()
loop.run()


