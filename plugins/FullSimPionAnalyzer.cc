// Global FWCore classes
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"

// system include files
#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

//Ecal Rec hits 
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"

//Ecal det id
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"

//Hcal Rec hits
#include "DataFormats/HcalRecHit/interface/HcalRecHitCollections.h"
#include "DataFormats/HcalRecHit/interface/HBHERecHit.h"
#include "DataFormats/HcalRecHit/interface/HFRecHit.h"

//Hcal det id
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"

//cell geometry
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"

//gen particles
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TMath.h"

class FullSimPionAnalyzer : public edm::EDAnalyzer {
	public:
		explicit FullSimPionAnalyzer(const edm::ParameterSet&);
		~FullSimPionAnalyzer();
		static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
		
	private:
		virtual void beginJob();
		virtual void analyze(const edm::Event&, const edm::EventSetup&);
		virtual void endJob();
	
		virtual void beginRun(edm::Run const&, edm::EventSetup const&);
		virtual void endRun(edm::Run const&, edm::EventSetup const&);
		virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
		virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

		double phi(double x, double y);
		double DeltaPhi(double phi1, double phi2);
		double DeltaR(double phi1, double eta1, double phi2, double eta2);		
		
		//member variables
		TFile* out_file;
		TTree* tree;
		double e_ecal, e_hcal, e_hcal_raw;
		double gen_eta, gen_phi, gen_en, gen_pt;
		int event;
		std::string outname;
		double dRcut;
		edm::EDGetTokenT<EcalRecHitCollection> tok_EB_;
		edm::EDGetTokenT<EcalRecHitCollection> tok_EE_;
		edm::EDGetTokenT<HBHERecHitCollection> tok_HBHE_;
		edm::EDGetTokenT<HFRecHitCollection> tok_HF_;
		edm::EDGetTokenT<reco::GenParticleCollection> tok_gen_;
};

using namespace std;
using namespace edm;

double FullSimPionAnalyzer::phi(double x, double y) {
	double phi_ = atan2(y, x);
	return (phi_>=0) ?  phi_ : phi_ + 2*TMath::Pi();
}
double FullSimPionAnalyzer::DeltaPhi(double phi1, double phi2) {
	double phi1_= phi( cos(phi1), sin(phi1) );
	double phi2_= phi( cos(phi2), sin(phi2) );
	double dphi_= phi1_-phi2_;
	if( dphi_> TMath::Pi() ) dphi_-=2*TMath::Pi();
	if( dphi_<-TMath::Pi() ) dphi_+=2*TMath::Pi();

	return dphi_;
}
double FullSimPionAnalyzer::DeltaR(double phi1, double eta1, double phi2, double eta2){
	double dphi = DeltaPhi(phi1,phi2);
	double deta = eta2 - eta1;
	double dR2 = dphi*dphi + deta*deta;
	return sqrt(dR2);
}

FullSimPionAnalyzer::FullSimPionAnalyzer(const edm::ParameterSet& iConfig) { 
	outname = iConfig.getParameter<string>("fileName");	
	dRcut = iConfig.getParameter<double>("dRcut");

	tok_EB_ = consumes<EcalRecHitCollection>(InputTag("ecalRecHit","EcalRecHitsEB"));
	tok_EE_ = consumes<EcalRecHitCollection>(InputTag("ecalRecHit","EcalRecHitsEE"));
	tok_HBHE_ = consumes<HBHERecHitCollection>(InputTag("hbhereco"));
	tok_HF_ = consumes<HFRecHitCollection>(InputTag("hfreco"));
	tok_gen_ = consumes<reco::GenParticleCollection>(InputTag("genParticles"));

	out_file = TFile::Open(outname.c_str(), "RECREATE");
	tree = new TTree("tree","tree");
	tree->Branch("event",&event,"event/I");
	tree->Branch("ecal",&e_ecal,"e_ecal/D");
	tree->Branch("hcal",&e_hcal,"e_hcal/D");
	tree->Branch("hcal_raw",&e_hcal_raw,"e_hcal_raw/D");
	tree->Branch("eta",&gen_eta,"gen_eta/D");
	tree->Branch("phi",&gen_phi,"gen_phi/D");
	tree->Branch("energy",&gen_en,"gen_en/D");
	tree->Branch("pt",&gen_pt,"gen_pt/D");

	event = 0;
}

FullSimPionAnalyzer::~FullSimPionAnalyzer() { 
	out_file->cd();
	
	tree->Write();
	
	out_file->Close();
}

// ------------ method called for each event  ------------
void
FullSimPionAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
	edm::ESHandle<CaloGeometry> geometry;
	iSetup.get<CaloGeometryRecord>().get (geometry);

	double sum_ecal, sum_hcal, sum_hcal_raw;
	sum_ecal = sum_hcal = sum_hcal_raw = 0;
	double s_eta, s_phi, s_en, s_pt;
	s_eta = s_phi = s_en = s_pt = 0;
	
	//Access to RecHits information
	Handle<EcalRecHitCollection> EERecHits;
	iEvent.getByToken(tok_EE_, EERecHits);
	Handle<EcalRecHitCollection> EBRecHits;
	iEvent.getByToken(tok_EB_, EBRecHits);
	
	Handle<HBHERecHitCollection> HBHERecHits;
	iEvent.getByToken(tok_HBHE_, HBHERecHits);
	Handle<HFRecHitCollection> HFRecHits;
	iEvent.getByToken(tok_HF_, HFRecHits);
	
	//Access to GenParticles
	Handle<reco::GenParticleCollection> genParticles;
    iEvent.getByToken(tok_gen_, genParticles);
	for(const auto& p : *genParticles.product()){
		s_eta = p.eta();
		s_phi = p.phi();
		s_en = p.energy();
		s_pt = p.pt();

		//get ECAL rec energy for this event
		for (EcalRecHitCollection::const_iterator hit = EERecHits->begin(); hit!=EERecHits->end(); ++hit) {
			EEDetId cell(hit->id());
			const CaloCellGeometry* cellGeometry = geometry->getSubdetectorGeometry(cell)->getGeometry(cell);
			double h_eta = cellGeometry->getPosition().eta();
			double h_phi = cellGeometry->getPosition().phi();
			double en = hit->energy();
			double dR = DeltaR(s_phi,s_eta,h_phi,h_eta);
			
			if(dR < dRcut) sum_ecal += en;
		}

		for (EcalRecHitCollection::const_iterator hit = EBRecHits->begin(); hit!=EBRecHits->end(); ++hit) {
			EBDetId cell(hit->id());
			const CaloCellGeometry* cellGeometry = geometry->getSubdetectorGeometry(cell)->getGeometry(cell);
			double h_eta = cellGeometry->getPosition().eta();
			double h_phi = cellGeometry->getPosition().phi();
			double en = hit->energy();
			double dR = DeltaR(s_phi,s_eta,h_phi,h_eta);
			
			if(dR < dRcut) sum_ecal += en;
		}
	
		//get HCAL rec energy for this event
		for (HBHERecHitCollection::const_iterator hit = HBHERecHits->begin(); hit!=HBHERecHits->end(); ++hit) {
			HcalDetId cell(hit->id());
			const CaloCellGeometry* cellGeometry = geometry->getSubdetectorGeometry(cell)->getGeometry(cell);
			double h_eta = cellGeometry->getPosition().eta();
			double h_phi = cellGeometry->getPosition().phi();
			double en = hit->energy();
			double raw = hit->eraw();
			double dR = DeltaR(s_phi,s_eta,h_phi,h_eta);
			
			if(dR < dRcut) {
				sum_hcal += en;
				sum_hcal_raw += raw;
			}
		}

		for (HFRecHitCollection::const_iterator hit = HFRecHits->begin(); hit!=HFRecHits->end(); ++hit) {
			HcalDetId cell(hit->id());
			const CaloCellGeometry* cellGeometry = geometry->getSubdetectorGeometry(cell)->getGeometry(cell);
			double h_eta = cellGeometry->getPosition().eta();
			double h_phi = cellGeometry->getPosition().phi();
			double en = hit->energy();
			double dR = DeltaR(s_phi,s_eta,h_phi,h_eta);
			
			if(dR < dRcut) {
				sum_hcal += en;
				sum_hcal_raw += en;
			}
		}
		
		e_ecal = sum_ecal;
		e_hcal = sum_hcal;
		e_hcal_raw = sum_hcal_raw;
		gen_eta = s_eta;
		gen_phi = s_phi;
		gen_en = s_en;
		gen_pt = s_pt;
		
		tree->Fill();
	}
	
	++event;
}

// ------------ method called once each job just before starting event loop  ------------
void 
FullSimPionAnalyzer::beginJob() { }

// ------------ method called once each job just after ending the event loop  ------------
void 
FullSimPionAnalyzer::endJob() {
}

// ------------ method called when starting to processes a run  ------------
void 
FullSimPionAnalyzer::beginRun(edm::Run const&, edm::EventSetup const&) {

}

// ------------ method called when ending the processing of a run  ------------
void 
FullSimPionAnalyzer::endRun(edm::Run const&, edm::EventSetup const&) { 

}

// ------------ method called when starting to processes a luminosity block  ------------
void 
FullSimPionAnalyzer::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) { }

// ------------ method called when ending the processing of a luminosity block  ------------
void 
FullSimPionAnalyzer::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) { }

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
FullSimPionAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(FullSimPionAnalyzer);
