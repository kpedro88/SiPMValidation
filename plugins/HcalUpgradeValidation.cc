// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include <iostream>

// include files to support the generation of TTrees. 
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TROOT.h>

//include files to support access to the digis
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/HcalUpgradeDataFrame.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
 
//#include "DQMServices/Core/interface/DQMStore.h"
 
#include "DataFormats/HcalDetId/interface/HcalElectronicsId.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"
 
#include <vector>
#include <utility>
#include <ostream>
#include <string>
#include <algorithm>
#include <cmath>

//adc2fC stuff
#include "CalibFormats/HcalObjects/interface/HcalDbService.h"
#include "CondFormats/HcalObjects/interface/HcalQIEShape.h"
#include "CalibFormats/HcalObjects/interface/HcalCoderDb.h"
#include "CalibFormats/HcalObjects/interface/HcalDbRecord.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/ESHandle.h"

//
// class declaration
//

class HcalUpgradeValidation : public edm::EDAnalyzer {
   public:
      explicit HcalUpgradeValidation(const edm::ParameterSet&);
      ~HcalUpgradeValidation();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;

      //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

      // ----------member data ---------------------------
	std::string outputfile_;

	TFile *tf1;
	TTree *tt1;

	//Branchs
    std::vector<int>* adc;
    int event, ieta, iphi, depth;

	edm::EDGetTokenT<HcalUpgradeDigiCollection> tok_HcalUpgrade_;

	//adc2fC stuff
	edm::ESHandle<HcalDbService> conditions;

};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
HcalUpgradeValidation::HcalUpgradeValidation(const edm::ParameterSet& iConfig) : adc(NULL), event(0), ieta(0), iphi(0), depth(0)
{
	edm::InputTag inputHcalUpgrade = iConfig.getParameter<edm::InputTag>("HcalUpgradetag");
	tok_HcalUpgrade_= consumes<HcalUpgradeDigiCollection>(inputHcalUpgrade);

	outputfile_ = iConfig.getParameter<std::string>("rootOutputFile");

	tf1 = new TFile(outputfile_.c_str(), "RECREATE");
	tt1 = new TTree("tree","tree");

	tt1->Branch("adc","std::vector<int>",&adc);
	tt1->Branch("event",&event,"event/I");
	tt1->Branch("ieta",&ieta,"ieta/I");
	tt1->Branch("iphi",&iphi,"iphi/I");
	tt1->Branch("depth",&depth,"depth/I");

	event = 0;
}


HcalUpgradeValidation::~HcalUpgradeValidation()
{
 
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)

	tf1->cd();
	tt1->Write();
	tf1->Write();
	tf1->Close();
}


//
// member functions
//

// ------------ method called on each new Event  ------------
void
HcalUpgradeValidation::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	iSetup.get<HcalDbRecord>().get(conditions);

	edm::Handle<HcalUpgradeDigiCollection> DigiHcalUpgrade;
	iEvent.getByToken(tok_HcalUpgrade_,DigiHcalUpgrade);
	const HcalUpgradeDigiCollection *DigiHcalUpgradeCollection = DigiHcalUpgrade.product () ;

	for (auto digi : *DigiHcalUpgradeCollection){
		HcalDetId cell(digi.id());
		if(cell.subdet()!=HcalEndcap) continue;
		ieta = cell.ieta();
		iphi = cell.iphi();
		depth = cell.depth();

		delete adc; adc = new std::vector<int>(10,0);
//		bool doFill = false;
		bool doFill = true;
		for (int k=0; k<digi.size(); k++) {
			adc->at(k) = digi.adc(k);
//			if(k==digi.presamples() && digi.adc(k) > 100) doFill = true;
		}
		if(doFill) {
			tt1->Fill(); //Fill the tree
		}

	} // Dataframe with collection
	++event;
} //HcalUpgradeValidation::analyze

// ------------ method called when starting to processes a run  ------------
/*
void
HcalUpgradeValidation::beginRun(edm::Run const&, edm::EventSetup const&)
{ 
}
*/
 
// ------------ method called when ending the processing of a run  ------------
/*
void
HcalUpgradeValidation::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
void
HcalUpgradeValidation::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
HcalUpgradeValidation::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
HcalUpgradeValidation::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}
//define this as a plug-in
DEFINE_FWK_MODULE(HcalUpgradeValidation);
