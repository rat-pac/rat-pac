from ratproc.base import Processor
from rat import ratiter, ROOT

class Hist(Processor):
    def __init__(self, selector, xbin, xmin, xmax, title=None, interval=10, wait=False):
        '''Creates and draws a histogram as the RAT job progresses.'''
        Processor.__init__(self)
        self.selector = selector
        if title is None:
            title = selector
    
        self.count = 0
        self.interval = interval

        self.canvas = ROOT.TCanvas('c'+ROOT.TUUID().AsString(), 
                                   self.selector, 800, 600)
        self.hist = ROOT.TH1D('', title, xbin, xmin, xmax)
        self.hist.Draw()
        self.canvas.Update()
        self.wait = wait

    def dsevent(self, ds):
        self.count += 1

        for v in ratiter(ds, self.selector):
            self.hist.Fill(v)

        if self.count % self.interval == 0:
            self.canvas.cd()
            self.hist.Draw()
            self.canvas.Update()

        return Processor.OK

    def finish(self):
        if self.wait:
            raw_input('Press enter to close window')
