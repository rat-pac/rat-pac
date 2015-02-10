Extending the Data Structure
----------------------------
Planning
````````
In order to extend the data structure (RAT::DS), you must decide several
things:

What sort of data would you like to record?
'''''''''''''''''''''''''''''''''''''''''''
If it is an atomic value, like a float or an integer, your task will be easy.
If it is several pieces of data, it may make more sense to make a new class
to put in the event data structure.

The only types allowed in the data structure are standard C++ atomic types
like int and double (and ROOT-specific typedefs to them), STL containers,
like vector and map, and classes which follow the RAT::DS style. That is,
they are subclasses of TObject which follow the RAT style conventions.
Note that C-style arrays and ROOT container classes (e.g. TClonesArray)
are not allowed!

What do you want to call it?
''''''''''''''''''''''''''''
Short (but not cryptic) names are preferred, and it helps to make them unique
over all branches of the event data structure. This simplifies plotting with
TTree::Draw() in ROOT later, so that the branch need not be specified to
disambiguate a name.

Where will this data be created and where will it be used?
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
Data stored in the event structure must be "created" somewhere, either in a
generator or a processor.  You should decide where the code which computes
this information should go.  If the information is at all optional, it's
best to put the computation in a separate processor so users who don't
need the data can skip it and save running time.

Also remember that RAT::DS is the interface by which processors "communicate"
with each other. Intermediate calculations should not be stored in the data
structure unless they would be of interest to an end user.  However, any
data from one processor that is the input to the calculation of another
processor should be stored in the data structure.

Adding Existing Data Types to Objects
`````````````````````````````````````
If you are adding data of an already existing data type, you can just edit
the appropriate header file and recompile. No other action is required,
except editing the appropriate processor or generators to actually put
some data there!

Creating a New Class for the Data Structure
```````````````````````````````````````````
If you need to create a new class, you need to:

Make the header file
''''''''''''''''''''
Your new class must subclass TObject and be declared
within the namespace RAT::DS. For ease, copy an existing event class and edit
it. Do not forget to put the new class name in the !ClassDef macro.

Note that DS class members should not use a prefix like "f" (that is, use
"pmtCount" rather than "fPMTCount"), since they are part of the user
interface, in a sense. Data members should be private, with the interface
defined by "getter" and "setter" methods. The name of the file should
match the name of the class.

For example, a class with PMT information might look like this::

    /**
     * @class RAT::DS::PMT
     * @author A. Hamsterton <hamster@urodents.edu>
     * @brief Example of PMT information
     *
     * This is a long description of the class. This whole
     * block is here so we can generate Doxygen documentation.
     */

    #ifndef __RAT_DS_PMT___
    #define __RAT_DS_PMT___
    
    #include <TObject.h>
    #include <vector>

    namespace RAT {
      namespace DS {

    class MyClass : public TObject {
    public:
      MyClass() {}
      virtual ~MyClass() {}

      /// PMT channel number
      int GetPMTID() { return pmtID; }
      void SetPMTID(int _pmtID) { pmtID = _pmtID; }

      /// Digitized hit time information
      int GetADCTime() { return adcTime; }
      void SetADCTime(int _adcTime) { adcTime = _adcTime; }

      /// Digitized hit charge information
      int GetADCCharge() { return adcCharge; }
      void SetADCCharge(int _adcCharge) { adcCharge = _adcCharge; }

      /// True hit time information
      int GetTime() { return time; }
      void SetTime(int _time) { time = _time; }

      /// True hit charge information
      int GetCharge() { return charge; }
      void SetCharge(int _charge) { charge = _charge; }

    private:
      int pmtID;  ///< PMT channel number;
      int adcTime;  ///< Digitized hit time
      int adcCharge;  ///< Digitized hit charge
      float time;  ///< True hit time
      float charge;  ///< True hit charge

      ClassDef(PMT, 1)
    };

      }  // namespace DS
    }  // namespace RAT

    #endif  // __RAT_DS_PMT__

Make an implementation
''''''''''''''''''''''
Make in implementation in a cc file with the same name as the header, if
necessary. Remember that DS classes should not do very much work -- they are
vessels for data which is computed elsewhere (such as a generator or
processor). Never should a DS class itself be computing the data that it
stores.

Add to CINT
'''''''''''
Edit the src/ds/LinkDef.h file to ensure your new class is added to the
CINT dictionary, which is needed for ROOT macros to work properly. You will
need to add a line that looks like::

    #pragma link C++ class RAT::DS::PMT;

And further down, there needs to be a line like::

    #pragma link C++ class vector<RAT::DS::PMT>;

Add to SConstruct
'''''''''''''''''
Add your class name to the cint_cls list in the $RATROOT/SConstruct file.
This will ensure the build system invokes rootcint on your new event class
and generates the code that allows the event data structure to be streamed
to disk or over network connections.

Recompile
'''''''''
Run scons to recompile.

Update Documentation
````````````````````
Don't forget to update the documentation in $RATROOT/doc to reflect the
changes you have made to the data structure! Remember to explain what the
data is and what units it is stored in.

