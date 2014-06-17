Input Producers
---------------

InROOT
``````
The InROOT event producer reads events from a ROOT format data file, as produced by [wiki:UserGuideOutRoot OutROOT], and passes them one at a time to the event loop.

Command
'''''''

::

    /rat/inroot/read filename


* filename - name of ROOT file to open.  Note the lack of quotation marks.

InNet
`````

The InNet event producer listens to a network socket and passes events it receives to the event loop.  Multiple remote hosts may all send events at once.  !InNet will process them in roughly first-come-first-serve order, but no attempt is made at strong fairness.  Runs forever until manually terminated with Ctrl-C.

InNet has '''NO SECURITY''' and will accept connections from any computer on the network.  You should only use it on computers which are isolated from the rest of the Internet by a firewall.

Command
'''''''

::

   /rat/innet/listen port

* port - integer, TCP/IP port number to listen for events on
