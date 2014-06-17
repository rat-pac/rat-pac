from ratproc.base import Processor
from rat import ROOT

class DeltaT(Processor):
    def __init__(self):
        # Calculates the time since the last event in ns.
        Processor.__init__(self)
        self.t0 = ROOT.TTimeStamp()
        self.first = True

    def event(self, ds, ev):
        t1 = ev.GetUTC()
        dt = 0.0
        if self.first:
            self.first = False
        else:
            dt += (t1.GetSec() - self.t0.GetSec()) * 1.0e9
            dt += t1.GetNanoSec() - self.t0.GetNanoSec()
        ev.SetDeltaT(dt)
        self.t0 = t1
        return 0
