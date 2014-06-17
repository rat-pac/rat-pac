from ratproc.base import Processor

class Count(Processor):
    def __init__(self, interval=1):
        '''Counts events and prints updates at specified interval, just like
        the C++ RAT processor.'''
        Processor.__init__(self)
        self.count = 0
        self.triggers = 0
        self.interval = interval

    def dsevent(self, ds):
        self.count += 1
        self.triggers += ds.GetEVCount()

        if self.count % self.interval == 0:
            print 'PyCountProc: Event %d (%d triggered events)' % (self.count, self.triggers)
        return 0

    def finish(self):
        print 'PyCountProc: Total # of events %d (%d triggers)' % (self.count, self.triggers)
