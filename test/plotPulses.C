#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TGraphErrors.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TPaveText.h>

#ifndef DigiClass_cxx
#define DigiClass_cxx
#include "DigiClass.h"

#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <utility>
#include <algorithm>

using namespace std;

void DigiClass::Loop() {}

class DigiClass2 : public DigiClass {
	public:
		DigiClass2(TTree *tree=0) : DigiClass(tree), norm(false) {}
		pair<TH2F*,TGraphErrors*> Loop(int cut, bool rechit){
			if (fChain == 0) return make_pair((TH2F*)NULL,(TGraphErrors*)NULL);

			vector<string> bin_names;
			vector<vector<int>> bin_vals;
			
			Long64_t nentries = fChain->GetEntriesFast();

			Long64_t nbytes = 0, nb = 0;
			for (Long64_t jentry=0; jentry<nentries;jentry++) {
				Long64_t ientry = LoadTree(jentry);
				if (ientry < 0) break;
				nb = fChain->GetEntry(jentry);   nbytes += nb;
				if((rechit && adcR->at(4) <= cut) || (!rechit && adc->at(4) <= cut)) continue; //cut on SOI
				
				stringstream ss;
				ss << "i#eta = " << ieta << " i#phi = " << iphi << " d = " << depth;
				bin_names.push_back(ss.str());
				if(rechit) bin_vals.push_back(*adcR);
				else bin_vals.push_back(*adc);
			}
			
			if(bin_names.size()==0) return make_pair((TH2F*)NULL,(TGraphErrors*)NULL);
			
			//convert arrays into TH2
			vector<double> xvals(10,0), xerrs(10,0.5), sum(10,0), sum2(10,0), stdev(10,0), sump(10,0), summ(10,0);
			TH2F* htemp = new TH2F("h2","",10,-0.5,9.5,bin_names.size(),0,bin_names.size());
			for(unsigned y = 0; y < htemp->GetNbinsY(); ++y){
				for(unsigned x = 0; x < htemp->GetNbinsX(); ++x){
					htemp->SetBinContent(x+1,y+1,bin_vals[y][x]);
					sum[x] += bin_vals[y][x];
					sum2[x] += bin_vals[y][x]*bin_vals[y][x];
				}
			}
			//bin labels
			for(unsigned y = 0; y < htemp->GetNbinsY(); ++y){
				htemp->GetYaxis()->SetBinLabel(y+1,bin_names[y].c_str());
			}
			//compute average and std dev
			for(unsigned x = 0; x < 10; ++x){
				sum[x] /= bin_names.size();
				sum2[x] /= bin_names.size();
				stdev[x] = sqrt(sum2[x]-sum[x]*sum[x]);
				xvals[x] = x;
				sump[x] = sum[x]+stdev[x];
				summ[x] = sum[x]-stdev[x];
			}
			//normalize if desired
			if(norm){
				double theNorm = sum[4];
				for(unsigned x = 0; x < 10; ++x){
					sum[x] /= theNorm;
					stdev[x] /= theNorm;
					sump[x] = sum[x]+stdev[x];
					summ[x] = sum[x]-stdev[x];
				}	
			}
			//make graph
			TGraphErrors* gtemp = new TGraphErrors(10,xvals.data(),sum.data(),xerrs.data(),stdev.data());
			gtemp->SetMinimum(*min_element(summ.begin(),summ.end()));
			gtemp->SetMaximum(*max_element(sump.begin(),sump.end()));
			
			return make_pair(htemp,gtemp);
		}
		
		bool norm;
};

void plotPulses(vector<string> fnames, vector<string> legnames, vector<string> outnames, vector<int> cuts, vector<Color_t> colors, bool norm=false, bool curve=false, bool rechit=false){
	vector<TFile*> files;
	vector<TGraphErrors*> profiles;
	string poutname = "profile";
    if(rechit) poutname += "_rechit";
	if(curve) poutname += "_curve";
	if(norm) poutname += "_norm";
	
	TLegend* leg = new TLegend(0.2,0.9-0.05*fnames.size(),0.5,0.9);
	leg->SetFillColor(0);
	leg->SetBorderSize(0);
	leg->SetTextFont(42);
	leg->SetTextSize(0.05);
	
	double ymax = 0, ymin = 1e10;
	for(unsigned f = 0; f < fnames.size(); ++f){
		files.push_back(TFile::Open(fnames[f].c_str()));
		TFile* file = files.back();
		if(!file) {
			profiles.push_back(NULL);
			continue;
		}
		TTree* tree = (TTree*)file->Get("tree");
		DigiClass2* dc = new DigiClass2(tree);
		dc->norm = norm;
		pair<TH2F*,TGraphErrors*> res = dc->Loop(cuts[f],rechit);
		TH2F* h2 = res.first;
		h2->SetName(("h2"+outnames[f]).c_str());
		h2->GetXaxis()->SetTitle("TS");
		h2->GetYaxis()->SetTitle("");
		
		TCanvas* can = new TCanvas(outnames[f].c_str(),outnames[f].c_str(),700,500);
		can->SetMargin(0.25,0.15,0.15,0.1);
		h2->Draw("colz");
		
		stringstream ss;
		ss << legnames[f] << ", adc[4] > " << cuts[f];
		TPaveText* pave = new TPaveText(0.25,0.925,0.75,0.975,"NDC");
		pave->AddText(ss.str().c_str());
		pave->SetFillColor(0);
		pave->SetBorderSize(0);
		pave->SetTextFont(42);
		pave->SetTextSize(0.05);
		pave->Draw("same");
		
		can->Print((outnames[f]+".png").c_str(),"png");
		//can->Close();
		
		profiles.push_back(res.second); //std dev as error bar
		poutname += "_"+outnames[f];
		TGraphErrors* profile = profiles.back();
		profile->SetLineColor(colors[f]);
		profile->SetMarkerColor(colors[f]);
		if(curve) leg->AddEntry(profile,legnames[f].c_str(),"l");
		else leg->AddEntry(profile,legnames[f].c_str(),"pel");
		
		if(profile->GetMaximum()>ymax) ymax = profile->GetMaximum();
		if(profile->GetMinimum()<ymin) ymin = profile->GetMinimum();
	}
	
	TCanvas* can = new TCanvas(poutname.c_str(),poutname.c_str());
	TH1F* haxis = new TH1F("axis","",10,-0.5,9.5);
	haxis->GetXaxis()->SetTitle("TS");
	haxis->GetYaxis()->SetTitle("#LTadc#GT #pm #sigma_{adc}");
	if(norm) haxis->GetYaxis()->SetTitle("#LTadc#GT #pm #sigma_{adc} (a.u.)");
	haxis->GetYaxis()->SetRangeUser(max(ymin*0.9,0.0),ymax*1.1);
	haxis->Draw();
	for(unsigned p = 0; p < profiles.size(); ++p){
		if(!profiles[p]) continue;
		if(curve) {
			profiles[p]->SetLineWidth(2);
			profiles[p]->SetLineStyle(p+1);
			profiles[p]->Draw("lx same");
		}
		else profiles[p]->Draw("pz same");
	}
	leg->Draw("same");
	can->Print((poutname+".png").c_str(),"png");
}

/*
root -b -q -l 'plotPulses.C+(\
{"QIE11digis_step1.root","QIE11digis_step1_noDC.root","QIE11digis_step1_noCT.root","QIE11digis_step1_noDCCT.root","/uscms_data/d3/pedrok/hf/sipm/CMSSW_6_2_0_SLHC28/src/test/SiPMValidation/test/HcalUpgradedigis_step1.root"},\
{"81X","81X (no DC)","81X (no CT)","81X (no DC/CT)","62XSLHC"},\
{"81X","81X_noDC","81X_noCT","81X_noDCCT","62XSLHC"},\
{150,150,150,150,125},\
{kBlack,kBlue,kMagenta,kRed,kGreen+1})'

root -b -q -l 'plotPulses.C+(\
{"QIE11digis_step1.root","QIE11digis_step1_noDC.root","QIE11digis_step1_noCT.root","/uscms_data/d3/pedrok/hf/sipm/oldpulse/CMSSW_8_1_X_2016-08-10-1100/src/test/SiPMValidation/test/QIE11digis_step1.root","/uscms_data/d3/pedrok/hf/sipm/CMSSW_6_2_0_SLHC28/src/test/SiPMValidation/test/HcalUpgradedigis_step1.root"},\
{"81X","81X (no DC)","81X (no CT)","81X (old pulse)","62XSLHC"},\
{"81X","81X_noDC","81X_noCT","81X_oldpulse","62XSLHC"},\
{150,150,150,150,125},\
{kBlack,kBlue,kMagenta,kRed,kGreen+1})'

root -b -q -l 'plotPulses.C+(\
{"QIE11digis_step1.root","QIE11digis_step1_noDC.root","QIE11digis_step1_noCT.root","/uscms_data/d3/pedrok/hf/sipm/oldpulse/CMSSW_8_1_X_2016-08-10-1100/src/test/SiPMValidation/test/QIE11digis_step1.root","/uscms_data/d3/pedrok/hf/sipm/CMSSW_6_2_0_SLHC28/src/test/SiPMValidation/test/HcalUpgradedigis_step1.root","/uscms_data/d3/pedrok/raddam/muons/CMSSW_7_5_0/src/test/TB_S_tuple.root"},\
{"81X","81X (no DC)","81X (no CT)","81X (old pulse)","62XSLHC","TB data"},\
{"81X","81X_noDC","81X_noCT","81X_oldpulse","62XSLHC","TBdata"},\
{150,150,150,150,125,150},\
{kBlack,kBlue,kMagenta,kRed,kGreen+1,kOrange+2})'
*/

#endif
