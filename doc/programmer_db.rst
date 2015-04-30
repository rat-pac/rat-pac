Accessing RATDB
---------------
If you are unfamiliar with using RATDB, you should go read the description of
RATDB in the User's Guide for an explanation of the layout and terminology.

How do I access the data from inside my code?
`````````````````````````````````````````````
Assuming the database has been already loaded (see section "How do I load
data into RATDB?"), accessing the data inside your program requires only a
few steps:

1. #include <RAT/DB.hh> at the top of your source file.
2. Get a pointer to the global database.
3. Obtain a link to the table you want to access.
4. Read the value of the field using your link.

Here is a code snippet that shows how this works::

    RAT::DB *db = RAT::DB::Get();
    RAT::DBLinkPtr lmedia = db->GetLink("MEDIA", "acrylic");  // Link to MEDIA[acrylic]
    double rindex = lmedia->GetD("index_of_refraction");

Of course, writing those three lines over and over again would be extremely
cumbersome, so it is better to obtain the link in your class constructor
and just use it in your other methods. A longer example::

    #include <RAT/DB.hh>

    class Dummy {
    public:
      Dummy();
      virtual ~Dummy();
      float SpeedOfLight();
    
    protected:
      DBLinkPtr lmedia_acrylic;
    };
    
    Dummy::Dummy() {
      lmedia_acrylic = DB::Get()->GetLink("MEDIA", "acrylic");
      // Don't need to save the RAT::DB pointer. All you need is the
      // link object. You can leave off the index if you just want
      // empty index ""
    }
    
    Dummy::~Dummy() {
      // You don't have to delete your links!  Automatically handled for you.
    }

    float Dummy::SpeedOfLight() {
      return 299792458.0 / lmedia_acrylic->GetF("index_of_refraction");
    }

Note that you can get a link from the database at any time and save it for
when you need it, even if the table you want hasn't been loaded yet.  This
works because the table isn't looked up until you call one of the Get methods
on the link object.  The link knows how to find the field in the appropriate
table (checking the user, time and default planes).

The indirection of the link might seem like overkill, but it allows for the
case where the value you are looking up changes during processing. For
example, if you were processing a very long stretch of data, the attenuation
length in the mineral oil might not be constant. Each time you call GetD(),
the link will find the attenuation length for the current "detector" time.
Figuring out how to do this fast (with caching or load-on-demand) becomes
an implementation detail you don't have to worry about.

Get Methods
'''''''''''
There is one Get method in the DBLink class for each data type, so you must
know the data type of the field you are accessing (which you should anyway).
The simple types have the names GetI(), GetD(), and GetS() and return
int, double and std::string, as you might expect. There are no floats in the
database -- they are always promoted to doubles.

The array types are a little more complicated. To avoid copying large amounts
of data around, these Get methods return a ''const reference'' to the array.
Rather than explain what that means exactly, it's probably easier to show an
example::

    RAT::DBLinkPtr lgeo_pmt = RAT::DB::Get()->GetLink("GEO_PMT");
    const std::vector<double>& pmtx = lgeo_pmt->GetDArray("xpos");
    const std::vector<double>& pmty = lgeo_pmt->GetDArray("ypos");
    const std::vector<double>& pmtz = lgeo_pmt->GetDArray("zpos");
  
    std::cout << "PMT 0 is at " << pmtx[0] << "," << pmty[0] << "," 
              << pmtz[0] << std::endl;

You can use the reference to the vector just like an array. The const
qualifier means that the compiler will forbid you from replacing elements
of the array. This is a good thing as the physical array will be shared
with other objects in the program that might not appreciate you changing
it. If you do want a copy of the array for some reason, you can just do this::

    std::vector<double> pmtx = lgeo_pmt->GetDArray("xpos");

You are now free to do whatever you want to pmtx. Don't do this too often,
though, as copying the contents of the array could be rather slow if it is a
big array.

As you might have guessed, the names of the Get methods for the array types
follow a similar pattern: GetIArray(), GetDArray(), GetSArray().

