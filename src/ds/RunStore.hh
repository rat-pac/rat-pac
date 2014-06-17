/**
 * @class DS::RunStore
 * Run-level data structure helper class
 *
 * @author Stan Seibert <sseibert@lanl.gov>
 *
 * The RunStore coordinates the movement of run-level information
 * throughout the program.  Unless you have a really good reason,
 * you should always access the RunStore through the static methods,
 * and not create an instance of it yourself.
 *
 * This code handles DS::Run objects, which should be used to store
 * all run-level information that you wish to carry with the output
 * file.  In ROOT files, DS::Run objects are stored in a tree named
 * "runT" which has a single branch called "run" of type DS::Run.
 *
 * The current behavior of the RunStore is to cache all run records
 * in memory on demand, with no bound on memory usage of the cache.
 * Records are never flushed from the cache, and are written to the
 * output tree at the last possible moment.  This behavior may be
 * fine-tuned in the future.
 *
 * The rules for using the RunStore depend on who you are:
 *
 *   Generic Processor
 *   -----------------
 *
 *   Given an event, you can obtain the run information for that
 *   event by calling RunStore::GetRun() with a pointer to the event
 *   (DS::Root).  If the information cannot be found in the cache, or
 *   in a TTree registered to handle this information, a null pointer
 *   is returned.
 *
 *
 *   Output Processor
 *   ----------------
 *
 *   When opening an output ROOT file, the output processor must
 *   create a run tree.  The tree is registered by calling
 *   SetWriteTree().  Before closing the file containing the TTree,
 *   the output processor needs to call FlushWriteTree() to ensure
 *   all cached runs are written to the tree.
 *
 *
 *   Event Producer
 *   --------------
 * 
 *   A producer of events (Gsim/inroot/etc) needs to either register
 *   an input TTree of run information using SetReadTree(), or needs
 *   to populate the cache with run records using AddNewRun().  The
 *   former is used by inroot, and the latter is used by Gsim.  In
 *   special circumstances, you might want to populate the cache from
 *   a tree (but not leave it permanently assocated), in which case
 *   you should use PreloadFromTree().
 */

#ifndef __RAT_DS_RunStore__
#define __RAT_DS_RunStore__

#include <TTree.h>
#include <RAT/DS/Run.hh>
#include <map>

namespace RAT {
  namespace DS {

class RunStore;
class Root;

class RunRecord {
public:
  RunRecord() : run(0), writtenToDisk(false) {}
  ~RunRecord() { delete run; }
  Run* run;
  bool writtenToDisk;
};

class RunStore {
  /** Static interface */
public:
  /**
   * Get a pointer to the global RunStore instance.
   *
   * You generally don't need to use this function.
   */
  inline static RunStore* Get() { 
    if (!fgStore) {
      fgStore = new RunStore();
    }
    return fgStore; 
  }

  /**
   * Get the Run object associated with event ds.
   *
   *  If the run information is in the cache, it is returned,
   *  otherwise a potentially slow search is made through the
   *  input tree of run records, if available.  If the record
   *  cannot be found, 0 is returned.
   */
  inline static Run* GetRun(Root* ds) { return Get()->InstanceGetRun(ds); }

  /**
   * Get the Run object associated with the given runID.
   *
   *  GetRun(Root* ds) should be used in preference to this method
   *  in case future versions of RunStore use additional criteria
   *  to identify run records, like eventID ranges.  If that happens,
   *  this method will change.
   */
  inline static Run* GetRun(int runID) { return Get()->InstanceGetRun(runID); }

  /**
   * Sets the TTree from which Run records can be read.
   *
   *  This tree will not be written to, but will be searched
   *  as needed.  No assumption is made about the numerical ordering
   *  of runs in the tree.
   *
   *  The caller retains ownership of the tree, although this class
   *  will call GetEntry(), SetBranchAddress(), and SetBranchStatus()
   *  on the tree at will.
   */
  inline static void SetReadTree(TTree* tree) {
    Get()->InstanceSetReadTree(tree);
  }

  /**
   * Sets the TTree to which Run records are written.
   *
   *  All records loaded by RunStore, either from the read tree,
   *  or new records added with AddNewRun(), will be written to
   *  the write tree.  The only exception to this rule are run records
   *  added using PreloadFromTree() with alreadWrittenToDisk==true.
   *
   *  Run records are written to the output tree before being removed
   *  from the cache.   Currently records are never written to the
   *  cache, so only FlushWriteTree() actually triggers a write.
   *
   *  The caller retains ownership of the tree.
   */
  inline static void SetWriteTree(TTree* tree) {
    Get()->InstanceSetWriteTree(tree);
  }

  /** Writes all unwritten run records to the write tree.
   *
   *  This function must be called before closing the write tree.
   *  A flag is kept for each run record indicating whether it has
   *  already been written to disk, so FlushWriteTree() can be called 
   *  many times without duplication of run records in the write tree.
   *
   *  Note that this function flushes run records from the cache
   *  into the tree.  It does not Write() the tree to an output 
   *  root file, which is still the responsibility of the caller
   *  to ensure tree contents are flushed to disk.
   */
  inline static void FlushWriteTree() { Get()->InstanceFlushWriteTree(); }

  /** Adds a new run record to the cache.
   *
   *  The run record is added to the cache for later access by 
   *  GetRun(), and the record will be sent to the write tree
   *  when FlushWriteTree() is called.
   *
   */
  inline static void AddNewRun(Run* run) { Get()->InstanceAddNewRun(run); }

  /** Preloads the cache from a run tree.
   *
   *  All of the run records in the tree are added to the cache.
   *  If alreadyWrittenToDisk is true, then the records will NOT
   *  be send to the write tree when FlushWriteTree() is called.
   *  This can be useful if you are appending to an existing tree,
   *  since you cannot make the same tree both a read an write
   *  tree.  Instead you can preload the cache from the tree at
   *  the beginning, then write only the new run records to it.
   */
  inline static void
  PreloadFromTree(TTree* tree, bool alreadyWrittenToDisk=false) {
    Get()->InstancePreloadFromTree(tree, alreadyWrittenToDisk);
  }

protected:
  static RunStore* fgStore;

public:
  /** You probably want the static interface above. */
  RunStore();
  virtual ~RunStore();

  Run* InstanceGetRun(Root* ds);
  Run* InstanceGetRun(int runID);
  void InstanceSetReadTree(TTree *tree);
  void InstanceSetWriteTree(TTree *tree);
  void InstanceFlushWriteTree();
  void InstanceAddNewRun(Run* run);
  void InstancePreloadFromTree(TTree* tree, bool writtenToDisk=false);
  
  ClassDef(RunStore, 1)

protected:
  TTree* fReadTree;
  Run* fReadRun;
  TTree* fWriteTree;
  Run* fWriteRun;
  std::map<int, RunRecord*> fCache;

};

  } // namespace DS
} // namespace RAT

#endif

