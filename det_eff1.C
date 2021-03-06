/********SK PMT signal analyzer script*********/
#include "wrapper.hpp"
#include <string>
#include <iostream>
#include <ostream>
#include <fstream>
#include <unordered_set>
#include "TH1D.h"
#include "TFile.h"
#include "TF1.h"
#include <TVirtualFitter.h>
#include<TFitter.h>
#include <cmath>
#include "TMath.h"
#include <TStyle.h>


using namespace std;




string* name_sum;
//Gaussian fitting function
Double_t fitfunc(Double_t *x, Double_t *par)
{
  Double_t arg=0;
  if(par[2]!=0) arg=(x[0]-par[1])/par[2];
  Double_t gfunc=-par[0]*TMath::Exp(-0.5*arg*arg)+par[3];
  return gfunc;
}





int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "Enter the run number.";
  }
  // Opening  a root file
  TFile outFile("histograms_average_waveform.root", "RECREATE");
  
  const string run_no = argv[1];
  string root_f = "/home/shahin/ptf-analysis-2/data/out_run0" + run_no + ".root";
  string csv_f  = "/home/shahin/ptf-analysis-2/data/out_run0" + run_no + ".csv";
  
  vector<int> phidgets = {0, 1, 3};
  vector<PTF::PMTChannel> activeChannels = {
    {0, 0}
    //   {1, 4},
    //   {2, 5},
    //   {3, 6},
    //   {4, 7}, 
    //   {5, 8},
    //   {6, 9},
    //   {7, 10}
  };
  PTF::Wrapper wrapper = PTF::Wrapper(16384, 70, activeChannels, phidgets);
  
  unordered_set<int> skipLines = {};// {962,1923,2884,5240,6201,9611,10572,11533,12494,13455,15811,16771};
  
  wrapper.openFile(root_f, "scan_tree");
  
  cerr << "Num entries: " << wrapper.getNumEntries() << endl << endl;
  
  uint32_t lines = 0;
  const uint32_t freq = 100;



  
  /*************First loop on number of points on the PMT***************/
 
  for (int i = 0; i < wrapper.getNumEntries(); i++) {
    if (i % freq == 0 || i == wrapper.getNumEntries() - 1) {
      cerr << "Entry " << i << "/" << wrapper.getNumEntries() << "\u001b[34;1m (" << (((double)i)/wrapper.getNumEntries()*100) << "%)\u001b[0m\033[K";
      if (skipLines.find(i) != skipLines.end()) {
        cerr << "\u001b[31;1m Skipping...\u001b[0m\r";
        continue;
      } else {
        cerr << "\r";
      }
    }
    
    if (skipLines.find(i) != skipLines.end()) continue;
    
    lines++;
    wrapper.setCurrentEntry(i);
    
    auto location = wrapper.getDataForCurrentEntry(PTF::Gantry1);
    
    cerr << location.x << "," << location.y << "," << location.z << endl;
    
    for (auto  Channel : activeChannels)
      {
	//get number of samples
	int  numSamples= wrapper.getNumSamples();



	
	// Creating two arrays or vectors which store bin content and its squared
	double	arr1[70];
	double arr2[70];
	double sigma = 0;
	double mean;
        vector <TH1D*> hist;
	for(int k=0;k<70;k++){
	  arr1[k] = 0;
	  arr2[k] = 0;
	}


	
	/*****************Second loop; looping over the number of samples***************/
	for ( int j=0; j<10; j++) {
	  string name_sum = "single_pulse" + std::to_string(i)+to_string(j); 
	  auto  single_pulse=new TH1D(name_sum.c_str(), "Pulse waveform; Sample length;Collected charges",70,0, 70);
	  //  Getting  pmt sample
	  hist.push_back(single_pulse);
	  double* pmtsample=wrapper.getPmtSample( Channel.pmt, j);
	  /*********************Third loop; looping over the length of sample******************/
	  for(int k=0; k<wrapper.getSampleLength(); k++){
	    //Getingt bin content for h
	    double bin_value = single_pulse->GetBinContent(k+1);
	    // Calculating new bin content
	    bin_value = bin_value + pmtsample[k];
	    cout<<bin_value << " " << pmtsample[k] <<endl;
	    //Set new bin content
	    single_pulse->SetBinContent(k+1,bin_value);
	    //Storing the numbers from histogram to arrays
	    arr1[k]+=bin_value;
	    arr2[k]+=(bin_value)*(bin_value);
	  }


	  
	  // Setting the fitting parameters
	  TF1 *f1=new TF1("m1",fitfunc,20,50,4);
	  f1->SetParameters(0.06,35.0,10.0,8135.0);
	  single_pulse->Fit("m1","","",20,50);
	  gStyle->SetOptFit(1011);
	}



	
	/************Looping over sample length**********/ 
	for(int k=0;k<70;k++){ 
	  mean=arr1[k]/10;
	  sigma=sqrt(arr2[k]/10+(mean*mean)/10-2*arr2[k]*mean);
	  cout<< mean << " " <<sigma<<endl;

	}
	for(int m=0; m<hist.size();m++)
	  hist.at(m)->Write();
	
      }
	
    outFile.cd();
 
    
  }
  outFile.Close();
  
  
  cout<<"Done"<<endl; 
}



