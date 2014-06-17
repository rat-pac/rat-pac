from ratproc.base import Processor
from rat import ROOT, RAT, ratiter
from rat.parser import create_evaluation_tree
from array import array
import operator
import itertools

class NtupleWriter(object):
    def __init__(self, ntuple, field_selectors):
        self.ntuple = ntuple
        self.field_selectors = field_selectors
        self.field_contents = array('f', [0.0]*len(self.field_selectors))
        self.count = 0
        self.eval_tree = create_evaluation_tree(*field_selectors)

    def fill(self, ds):
        for row in self.eval_tree.eval(ds):
            for i, value in enumerate(row):
                if value is None:
                    value = -9999999.0
                self.field_contents[i] = value
            self.ntuple.Fill(self.field_contents)
            self.count += 1
        
    def write(self):
        self.ntuple.Write()

class Ntuple(Processor):
    def __init__(self, filename, *ntuples):
        self.filename = filename
        self.ntuple_names = ntuples
        self.first_event = True

    def create_ntuple(self, name):
        db = RAT.DB.Get()
        lntuple = db.GetLink("NTUPLE", name)
        # Python lists are better than STL vectors
        fields = list(lntuple.GetSArray("fields"))

        # Deinterleave field names and selectors
        field_names = fields[::2]
        field_selectors = fields[1::2]

        assert len(field_names) == len(field_selectors)

        N = ROOT.TNtuple(name, "RAT reduced ntuple",
                         ":".join(field_names))
        return NtupleWriter(N, field_selectors)

    def dsevent(self, ds):
        if self.first_event:
            print 'ntuple: Writing to', self.filename
            self.f = ROOT.TFile.Open(self.filename, "RECREATE")
            self.ntuples = [ self.create_ntuple(name)
                             for name in self.ntuple_names ]

            self.first_event = False

        for writer in self.ntuples:
            writer.fill(ds)

        return Processor.OK

    def finish(self):
        self.f.cd()
        for writer in self.ntuples:
            print 'ntuple: Wrote %d entries to %s' % (writer.count, writer.ntuple.GetName())
            writer.write()
        self.f.Close()
