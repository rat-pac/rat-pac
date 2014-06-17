#!/usr/bin/env ipython
import ROOT
from IPython.iplib import InteractiveShell
import IPython.Magic

############ Prefilter to understand Python/CINT command mode ###########
def prefilter_cint(self, line, continuation):
    if line:
        if line.startswith(".r"):
            print "ROOT mode"
            if self._cint_mode == self._cint_PYTHON:
                self._cint_mode = self._cint_ROOT
                self._cint_oldprompt = self.outputcache.prompt1.p_template
                self.outputcache.prompt1.p_template = 'Root [\#]:'
                self.outputcache.prompt1.set_p_str()
            return ""
        elif line.startswith(".p"):
            print "Python mode"
            if self._cint_mode == self._cint_ROOT:
                self.outputcache.prompt1.p_template = self._cint_oldprompt
                self._cint_mode = self._cint_PYTHON
                self.outputcache.prompt1.set_p_str()
            return ""
        elif line.startswith(".q"):
            return self._cint_orig_prefilter("%Quit", continuation)
        else:
            if self._cint_mode == self._cint_ROOT:
                return self._cint_orig_prefilter("%cint " + line,
                                                 continuation)
            else:
                return self._cint_orig_prefilter(line, continuation)
    else:
        return self._cint_orig_prefilter(line, continuation)

# Setup interactive shell to use our prefilter
InteractiveShell._cint_orig_prefilter = InteractiveShell.prefilter
InteractiveShell._cint_ROOT = 1
InteractiveShell._cint_PYTHON = 2
InteractiveShell._cint_mode = InteractiveShell._cint_PYTHON
InteractiveShell.prefilter = prefilter_cint

############## Magic command to execute CINT commands ###############

def magic_cint(self, parameter_s=''):
    """Execute command in ROOT's CINT C++ interpreter

        %cint <command>"""
    
    ROOT.gInterpreter.ProcessLine(parameter_s)
    return None

# Setup magic
InteractiveShell.magic_cint = magic_cint
del magic_cint
