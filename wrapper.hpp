#ifndef __PTF_WRAPPER__
#define __PTF_WRAPPER__


#include <vector>
#include <array>
#include <string>
#include <utility>
#include <exception>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <iostream>
#include <fstream>
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "config.hpp"


// using namespace std;
// using namespace boost;


namespace PTF {


enum Gantry {
  Gantry0,
  Gantry1
};


typedef struct PMTChannel {
  int pmt;
  int channel;
} PMTChannel;


typedef struct PhidgetReading {
  double Bx[10];
  double By[10];
  double Bz[10];
} PhidgetReading;


namespace Private {
  typedef struct PMTSet {
    int      channel;
    double*  data{nullptr};
    TBranch* branch{nullptr};

    // PMTSet(int _channel, double* _data = nullptr, TBranch* _branch = nullptr);
  } PMTSet;

  typedef struct PhidgetSet {
    PhidgetReading data;
    TBranch*       branchX{nullptr};
    TBranch*       branchY{nullptr};
    TBranch*       branchZ{nullptr};
  } PhidgetSet;
}


typedef struct GantryPos {
  double x;
  double y;
  double z;
  double theta;
  double phi;
} GantryPos;


typedef struct Wrapper {
  Wrapper(size_t maxSamples, size_t sampleSize, const std::vector<PMTChannel>& activeChannels, const std::vector<int>& phidgets);
  Wrapper(size_t maxSamples, size_t sampleSize, const std::vector<PMTChannel>& activeChannels, const std::vector<int>& phidgets, const std::string& fileName, const std::string& treeName = "scan_tree");
  ~Wrapper();

private:
  /* Private data */
  TFile* file{0};
  TTree* tree{0};
  size_t maxSamples;
  size_t sampleSize;
  size_t entry{UINT32_MAX};

  // data
  std::unordered_map<int, Private::PMTSet*>     pmtData;
  std::unordered_map<int, Private::PhidgetSet*> phidgetData;
  
  GantryPos g0;
  GantryPos g1;
  size_t    numEntries;
  size_t    numSamples;

  /* Private methods */

  // Gets the data pointer for the specified pmt
  // Returns nullptr if not found
  double* getDataForPmt(int pmt) const;

  // Sets the pointers in the tree to the newly opened tree
  // Returns false on failure, true on success
  bool setDataPointers();

  // Sets all branch pointers to nullptr
  // Returns false on failure, true on success
  bool unsetDataPointers();

public:
  // Public interface

  // Opens the selected file, and loads the first entry
  void openFile(const std::string& fileName, const std::string& treeName = "scan_tree");
  bool isFileOpen() const;
  // Closes the currently open file and deletes the tree.
  // Does nothing if the file is already closed 
  void closeFile();

  // Returns -1 on not found
  int getChannelForPmt(int pmt) const;
  // Ditto
  int getPmtForChannel(int channel) const;

  // Functions for getting/setting data entry
  // throws `NoFileIsOpen` if no file is open.
  size_t getCurrentEntry() const;
  // ditto
  size_t getNumEntries() const;
  void setCurrentEntry(size_t entry);  // Throws exception on invalid entry

  /* Reading data */

  // Throws on file not open
  size_t getNumSamples() const;

  // Gets the data for a given sample on the current
  // Throws on invalid sample or file not open
  double* getPmtSample(int pmt, size_t sample) const;

  // Returns the length of the samples
  int getSampleLength() const;

  GantryPos getDataForCurrentEntry(Gantry whichGantry) const;

  PhidgetReading getReadingForPhidget(int phidget) const;
} Wrapper;


namespace Exceptions {
  class FileDoesNotExist : public std::runtime_error {
  public:
    FileDoesNotExist(const std::string& msg) : runtime_error(msg) {}
  };

  class InvalidTreeName : public std::runtime_error {
  public:
    InvalidTreeName(const std::string& msg) : runtime_error(msg) {}
  };

  class NoFileIsOpen : public std::runtime_error {
  public:
    NoFileIsOpen() : runtime_error("No file is open.") {}
  };

  class EntryOutOfRange : public std::runtime_error {
  public:
    EntryOutOfRange() : runtime_error("Entry is out of range.") {}
  };

  class SampleOutOfRange : public std::runtime_error {
  public:
    SampleOutOfRange() : runtime_error("Sample is out of range.") {}
  };

  class InvalidPhidget : public std::runtime_error {
  public:
    InvalidPhidget() : runtime_error("No such phidget.") {}
  };

  class InvalidPMT : public std::runtime_error {
  public:
    InvalidPMT() : runtime_error("No such PMT.") {}
  };

  class DataPointerError : public std::runtime_error {
  public:
    DataPointerError() : runtime_error("Error while setting data pointers.") {}
  };

  class CSVFileError : public std::runtime_error {
  public:
    CSVFileError() : runtime_error("Error while trying to open CSV file.") {}
  };
} // end namespace Exceptions


} // end namespace PTF


// template <typename T, typename Ts>
// bool has(Ts _variant) {
//   if (boost::get<T>(&_variant)) {
//     return true;
//   } else {
//     return false;
//   }
// }


#endif // __PTF_WRAPPER__
