Command Interface and Macro Files
---------------------------------
Issuing Commands to RAT
```````````````````````
Interactive
'''''''''''
RAT is controlled through a text-command interface. If RAT is executed without any command-line options, it will start in interactive mode. The interactive shell has a command history and understands most of the keyboard navigation shortcuts available in tcsh. If you would like RAT to execute some commands before starting the interactive session, you can place those commands into a file called prerun.mac in the current directory. The interactive session can be terminated by typing exit.

Macro Files
'''''''''''

If RAT is started with one or more files listed on the command line::

    rat macro1.mac macro2.mac macro3.mac

then RAT will start in batch mode and execute each command found in the macro files sequentially. When all commands have been executed, RAT will terminate. If one of these macro filenames is "-", then RAT will start an interactive session when it reaches that point in the file list. Note that prerun.mac will not be loaded in this case.

Command Syntax
''''''''''''''

RAT uses the GEANT4 command interface, so the syntax of commands is identical to any other GEANT4 application. Comments are preceded by # and blank lines are ignored. Commands themselves consist of the command name followed by zero or more parameters, separated by spaces. For example, the command::

    /rat/procset update 5

starts with the command name ``/rat/procset`` followed by two parameters, "update" and the number 5. Whitespace is generally not important, but commands may not have leading whitespace. This means you cannot indent commands to aid in readability, for example.

Commands are organized into a filesystem-like hierarchy of directories and sub-directories. (PAW users should find this very familiar.) RAT automatically contains all the standard GEANT4 commands, as well as any additional commands defined by GLG4sim. This allows RAT to execute GLG4sim macro files, and the goal is to preserve this compatibility as long as it is feasible. All RAT-specific commands are confined to the /rat/ command subdirectory.

Tips for Macro Files
````````````````````
Layout
''''''

The order of commands in a macro file can be important, so it is best to follow this convention for organizing your macro files::


    # example.mac - Example Macro File
    # author: Bill Bobb
    
    # ---------------------------------
    
    # Set parameters here
    
    # ---------------------------------
    
    /run/initialize
    
    # BEGIN EVENT LOOP
    
    # Issue /rat/proc commands here to add processors to the event loop
    
    # END EVENT LOOP
    
    # ---------------------------------
    
    # Run commands to start appropriate event generator (Gsim, InROOT, etc.)


Inclusion
'''''''''

You can include one macro file in another using the /control/execute command. For example::

    /control/execute setup.mac

The included macro is immediately read and executed in place.
