C++ Style Guidelines
--------------------
C++ coding style debates are a never-ending source of flamewars, so it's best
to keep guides like this short.  For some really interesting discussion of
C++ style, take a look at these two books:

* `Effective C++: 55 Specific Ways to Improve Your Programs and Designs, 3rd Edition <http://www.awprofessional.com/bookstore/product.asp?isbn=0321334876/>`_
  by Scott Meyers
* `C++ Coding Standards <http://www.gotw.ca/publications/c++cs.htm>`_  by
  Herb Sutter and Andrei Alexandrescu

That said, there are a few additional things to emphasize for RAT.

Filenames
`````````
1. Class declarations should be placed in header files in the relevant
   subdirectory of src. Header filenames should end with .hh and be protected
   against multiple inclusion with the standard #ifndef/#define trick. The
   convention is to #define __namespace_class__; for example
   #define __RAT_CountProc__ or #define __RAT_DS_PMT__.
2. Class definitions (when needed) should placed in the same subdirectory
   of src. Source filenames should end with .cc.
3. When possible, the both the declaration and definition files should have
   the same name as the class they contain.

Identifiers
```````````
1. Class names and method names should be written in CamelCase (acronyms
   like RAT can stay all upper case).
2. Class member variables should be written in CamelCase, but with an
   initial "f", as in fNhitThreshold, fExample.
3. Local variable names should be written in lowerCamelCase.
4. Classes should be defined under namespace RAT.
5. These rules do not apply to classes used in the data structure.

Data Structure Classes
``````````````````````
1. They must be subclasses of TObject.
2. Member variables should be in lowerCamelCase.

Comments
````````
1. A Doxygen-compatible comment with both brief and full description should
precede each class declaration in a header file.
2. A brief comment should precede each method declaration (public, protected,
or private) in the header file explaining how to use the method.  If there
is a method definition in a source file, it should be preceded by a detailed
comment giving further information on how to use the method and explaining
the overall implementation.

General Tips
````````````
Avoid ROOT collection classes
'''''''''''''''''''''''''''''
As anyone who has tried to use them can attest, the ROOT collection classes
(like TObjArray and TClonesArray) are extremely awkward to use.  This is
because they are trying to tackle the problem of storing different kinds of
objects in the same list, and so make you use pointers everywhere.  Anything
you want to put in a collection has to be a subclass of TObject, and when
you extract an item from the collection, you have to recast back to whatever
kind of object you thought you had in the array.  This is all very messy
and prone to error, and it makes you use pointers more than you should
(see later item).

Instead, use the Standard Template Library (STL).  It provides arrays, linked
lists, maps, etc., all ready to go. Note that if you really do need a list of
heterogenous objects, you are still better off using a vector<> of pointers
to a common base class rather than using the ROOT collections, which force
you to make the very complicated TObject the base of your class hierarchy.)

Avoid C-style arrays
''''''''''''''''''''
C-style arrays are problematic because:

* They require you to separately know the size of the array.  (Especially
  annoying if you need to pass the array to another function.)
* You can't resize them.
* The C++ standard does not allow you to have variable-sized automatic arrays::

    void do_something useful(int number_of_foos) {
      int foo[number_of_foos];  // ILLEGAL
      // ... etc ...
    }

Much as before, STL vectors work just like arrays, remember their size, and
allow things like::

    void do_something useful(int number_of_foos) {
      vector<int> bar(number_of_foos);
      vector<int> baz(bar.size() * 2);
      // ... etc ...
    }

Avoid pointers when you can
```````````````````````````
Pointers are both essential in C++ and the thing most likely to make you
crazy (a bad combination). Thankfully, in many cases, pointers are completely
unnecessary:

* Pass by reference - If you want pass a large object to a function but
  don't want it to be copied, use the & operator in the function declaration::

    int foo(MyObject &obj)  {
      // Do something
    }

* Passing arrays - Use the vector<> template and pass it by reference.

Basically, you only need pointers when you must allocate new memory from the
heap or need to manipulate a derived class without knowing which derived
class it is (ex: a pointer to a RAT::Processor when you don't know which
particular processor it is).  If you pass pointers between functions, be
sure to make it clear who "owns" the object the pointer points to, and
thus who is responsible for eventually deleting it.

