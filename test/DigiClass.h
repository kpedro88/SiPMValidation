//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Aug 10 20:39:06 2016 by ROOT version 6.06/07
// from TTree tree/tree
// found on file: QIE11digis_step1.root
//////////////////////////////////////////////////////////

#ifndef DigiClass_h
#define DigiClass_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"

class DigiClass {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   vector<int>     *adc;
   vector<int>     *adcR;
   Int_t           event;
   Int_t           ieta;
   Int_t           iphi;
   Int_t           depth;

   // List of branches
   TBranch        *b_adc;   //!
   TBranch        *b_adcR;   //!
   TBranch        *b_event;   //!
   TBranch        *b_ieta;   //!
   TBranch        *b_iphi;   //!
   TBranch        *b_depth;   //!

   DigiClass(TTree *tree=0);
   virtual ~DigiClass();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef DigiClass_cxx
DigiClass::DigiClass(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("QIE11digis_step1.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("QIE11digis_step1.root");
      }
      f->GetObject("tree",tree);

   }
   Init(tree);
}

DigiClass::~DigiClass()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t DigiClass::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t DigiClass::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void DigiClass::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   adc = 0;
   adcR = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("adc", &adc, &b_adc);
   fChain->SetBranchAddress("adcR", &adcR, &b_adcR);
   fChain->SetBranchAddress("event", &event, &b_event);
   fChain->SetBranchAddress("ieta", &ieta, &b_ieta);
   fChain->SetBranchAddress("iphi", &iphi, &b_iphi);
   fChain->SetBranchAddress("depth", &depth, &b_depth);
   Notify();
}

Bool_t DigiClass::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void DigiClass::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t DigiClass::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef DigiClass_cxx
