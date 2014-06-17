import inspect

class Processor(object):
    '''Base class for processors providing default, empty implementations of
    both required methods.'''

    OK = 0
    FAIL = 1
    ABORT = 2

    def dsevent(self, ds):
        '''Called for every event produced by the event loop.

        Default implementation calls event() method for every trigger.

        Override with your own implementation of ``dsevent()`` if you want to
        run code for every physics event, or override ``event()`` to run code
        for every trigger, but not both.'''
        overall_result = self.OK

        for i in xrange(ds.GetEVCount()):
            result = self.event(ds, ds.GetEV(i))
            if result == Processor.ABORT:
                return Processor.ABORT
            elif result == Processor.FAIL:
                overall_result = Processor.FAIL # failure is OR over triggers

        return overall_result

    def event(self, ds, ev):
        '''Called for every triggered event in data structure.'''
        pass

    def finish(self):
        '''Called at end of job.'''
        pass
