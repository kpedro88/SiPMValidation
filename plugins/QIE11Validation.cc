// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include <iostream>

// include files to support the generation of TTrees. 
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TROOT.h>

//include files to support access to the QIE11 digis
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/HcalDigi/interface/QIE11DataFrame.h"

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

class QIE11Validation : public edm::stream::EDAnalyzer<> {
   public:
      explicit QIE11Validation(const edm::ParameterSet&);
      ~QIE11Validation();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginStream(edm::StreamID) override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endStream() override;

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
    std::vector<double>* fC;
    int event, ieta, iphi, depth, type;

	edm::EDGetTokenT<QIE11DigiCollection> tok_QIE11_;

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
QIE11Validation::QIE11Validation(const edm::ParameterSet& iConfig) : adc(NULL), fC(NULL), event(0), ieta(0), iphi(0), depth(0), type(0)
{
	edm::InputTag inputQIE11 = iConfig.getParameter<edm::InputTag>("QIE11tag");
	tok_QIE11_= consumes<QIE11DigiCollection>(inputQIE11);

	outputfile_ = iConfig.getParameter<std::string>("rootOutputFile");

	tf1 = new TFile(outputfile_.c_str(), "RECREATE");
	tt1 = new TTree("tree","tree");

	tt1->Branch("adc","std::vector<int>",&adc);
	tt1->Branch("fC","std::vector<double>",&fC);
	tt1->Branch("event",&event,"event/I");
	tt1->Branch("ieta",&ieta,"ieta/I");
	tt1->Branch("iphi",&iphi,"iphi/I");
	tt1->Branch("depth",&depth,"depth/I");
	tt1->Branch("type",&type,"type/I");

	event = 0;
}


QIE11Validation::~QIE11Validation()
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
QIE11Validation::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	iSetup.get<HcalDbRecord>().get(conditions);

	edm::Handle<QIE11DigiCollection> DigiQIE11;
	iEvent.getByToken(tok_QIE11_,DigiQIE11);
	const QIE11DigiCollection *DigiQIE11Collection = DigiQIE11.product () ;

	for (auto j : *DigiQIE11Collection){
		QIE11DataFrame digi(j);

		HcalDetId cell(digi.id());
		ieta = cell.ieta();
		iphi = cell.iphi();
		depth = cell.depth();

		const HcalSiPMParameter* sipmPar = conditions->getHcalSiPMParameter(cell);
		type = sipmPar->getType();

		const HcalQIECoder* channelCoder = conditions->getHcalCoder(cell);
		const HcalQIEShape* shape = conditions->getHcalShape(channelCoder);
		HcalCoderDb coder(*channelCoder, *shape);
		CaloSamples tool;
		coder.adc2fC(digi, tool);

		delete adc; adc = new std::vector<int>(10,0);
		delete fC; fC = new std::vector<double>(10,0.0);
//		bool doFill = false;
		bool doFill = true;
		for (int k=0; k<digi.samples(); k++) {
			QIE11DataFrame::Sample sam = digi[k];
			adc->at(k) = sam.adc();
			fC->at(k) = tool[k];
//			if(sam.soi() && sam.adc() > 150) doFill = true;
		}
		if(doFill) {
			tt1->Fill(); //Fill the tree
		}

	} // Dataframe with collection
	++event;
} //QIE11Validation::analyze

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void
QIE11Validation::beginStream(edm::StreamID)
{
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void
QIE11Validation::endStream() {
}

// ------------ method called when starting to processes a run  ------------
/*
void
QIE11Validation::beginRun(edm::Run const&, edm::EventSetup const&)
{ 
}
*/
 
// ------------ method called when ending the processing of a run  ------------
/*
void
QIE11Validation::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
void
QIE11Validation::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
QIE11Validation::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
QIE11Validation::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}
//define this as a plug-in
DEFINE_FWK_MODULE(QIE11Validation);
