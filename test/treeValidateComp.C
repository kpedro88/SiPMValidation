#include <TROOT.h>
#include <TFile.h>
#include <TTree.h>
#include <TH1.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TPad.h>
#include <TDirectory.h>
#include <TStyle.h>
#include <TPaveStats.h>
#include <TAxis.h>
#include <TLine.h>

#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

void clean(string& line, char alt='_'){
	string illegalChars = "\\/:?\"<>|()";
	string::iterator it;
	for (it = line.begin(); it != line.end(); ++it){
		bool found = illegalChars.find(*it) != string::npos;
		if(found){
			*it = alt;
		}
	}
}

//run example:
//root -l 'treeValidateComp.C+("step2new","step2","subdet==2&&M0>0.01&&chi2<500",{"M2","M0","chi2","time"},1)'
//root -l 'treeValidateComp.C+("tree_pion_step2new","tree_pion_step2","ecal<1",{"hcal","hcal_raw"},0)'
void treeValidateComp(string f_new, string f_ref, string cut, vector<string> qtys, bool reco = false, string tag = "HLT"){
	string treename = reco ? "Events" : "tree";
	//open files
	TFile* file[2];
	TTree* tree[2];
	file[0] = TFile::Open((f_new+".root").c_str());
	tree[0] = (TTree*)file[0]->Get(treename.c_str());
	file[1] = TFile::Open((f_ref+".root").c_str());
	tree[1] = (TTree*)file[1]->Get(treename.c_str());
	
	//aliases
	if(reco){
		string fulltag = "HBHERecHitsSorted_hbhereco__"+tag+".obj.obj";
		for(int f = 0; f < 2; f++){
			tree[f]->SetAlias("subdet",(fulltag+".id().subdet()").c_str());
			tree[f]->SetAlias("M2",(fulltag+".energy()").c_str());
			tree[f]->SetAlias("M0",(fulltag+".eraw()").c_str());
			tree[f]->SetAlias("chi2",(fulltag+".chi2()").c_str());
			tree[f]->SetAlias("time",(fulltag+".time()").c_str());
		}
	}
	
	Color_t colors[2] = {kRed, kBlack};
	Color_t fitcolors[2] = {kMagenta, kBlue};
	Int_t styles[2] = {2,1};
	string hname[2] = {"new","ref"};

	//axis ranges for each histo
	int nqty = qtys.size();
	vector<TH1F*> hist[2];
	hist[0] = vector<TH1F*>(nqty,NULL); hist[1] = vector<TH1F*>(nqty,NULL);
	vector<TH1F*> hrat(nqty,NULL);
	vector<TLine*> line(nqty,NULL);
	vector<double> xmax(nqty,0);
	vector<double> xmin(nqty,1e10);
	vector<double> ymax(nqty,0);
	vector<double> ymin(nqty,1e10);
	
	//initial loop over qtys to get x-axis limits
	for(int q = 0; q < nqty; q++){
		double x1 = 1e10;
		double x2 = -1e10;
		
		for(int f = 0; f < 2; f++){
			stringstream histname;
			string qtystmp = qtys[q]; clean(qtystmp);
			histname << "htmp_" << qtystmp << "_" << f;
			tree[f]->Draw((qtys[q]+">>"+histname.str()).c_str(),cut.c_str(),"goff");
			TH1F* htmp = (TH1F*)gDirectory->Get((histname.str()).c_str());
			if(htmp->GetXaxis()->GetXmin() < x1) x1 = htmp->GetXaxis()->GetXmin();
			if(htmp->GetXaxis()->GetXmax() > x2) x2 = htmp->GetXaxis()->GetXmax();
		}
		
		xmin[q] = x1;
		xmax[q] = x2;
	}
	
	//second loop over qtys to get histos and y-axis limits
	for(int q = 0; q < nqty; q++){
		double y1 = 1e10;
		double y2 = -1e10;
		
		for(int f = 0; f < 2; f++){
			stringstream histname;
			string qtystmp = qtys[q]; clean(qtystmp);
			histname << "h_" << qtystmp << "_" << f;
			stringstream drawname;
			drawname << qtys[q] << ">>" << histname.str() << "(100," << xmin[q] << "," << xmax[q] << ")";
			tree[f]->Draw((drawname.str()).c_str(),cut.c_str(),"goff");
			hist[f][q] = (TH1F*)gDirectory->Get((histname.str()).c_str());
			if(hist[f][q]->GetMinimum(0.0) < y1) y1 = hist[f][q]->GetMinimum(0.0);
			if(hist[f][q]->GetMaximum() > y2) y2 = hist[f][q]->GetMaximum();
		}
		
		ymin[q] = y1;
		ymax[q] = y2;
	}
	
	//final loop to draw histos with ratio
	for(int q = 0; q < nqty; q++){
		//setup canvas with histo and ratio areas
		string cname = qtys[q]; clean(cname);
		TCanvas* can = new TCanvas(cname.c_str(),cname.c_str(),700,750);
		TPad* pad1;
		can->cd();
		pad1 = new TPad("graph","",0,0.27,1.0,1.0);
		pad1->SetMargin(0.15,0.05,0.02,0.05);
		pad1->Draw();
		pad1->SetTicks(1,1);
		if(reco) pad1->SetLogy();
		TPad* pad2;
		can->cd();
		pad2 = new TPad("dmc","",0,0,1.0,0.25);
		pad2->SetMargin(0.15,0.05,0.35,0.05);
		pad2->Draw();
		pad2->SetTicks(1,1);
		pad1->cd();

		//histo format
		for(int f = 0; f < 2; f++){
			hist[f][q]->SetTitle("");
			hist[f][q]->SetName(hname[f].c_str());
			//hist[f][q]->GetXaxis()->SetTitle(qtys[q].c_str());
			hist[f][q]->GetXaxis()->SetTitle("");
			hist[f][q]->GetYaxis()->SetRangeUser(ymin[q]*0.9,ymax[q]*1.1);
			hist[f][q]->SetLineWidth(2);
			hist[f][q]->SetLineStyle(styles[f]);
			hist[f][q]->SetLineColor(colors[f]);
			hist[f][q]->SetStats(kTRUE);
			gStyle->SetOptStat("nemr");
			if(!reco) hist[f][q]->Fit("gaus","0Q");
			
			hist[f][q]->GetXaxis()->SetLabelOffset(999);
			//hist[f][q]->GetYaxis()->SetTitleOffset(1.1);
			hist[f][q]->GetYaxis()->SetTitleSize(32/(pad1->GetWh()*pad1->GetAbsHNDC()));
			hist[f][q]->GetYaxis()->SetLabelSize(28/(pad1->GetWh()*pad1->GetAbsHNDC()));
			hist[f][q]->GetYaxis()->SetTickLength(12/(pad1->GetWh()*pad1->GetAbsHNDC()));
			hist[f][q]->GetXaxis()->SetTickLength(12/(pad1->GetWh()*pad1->GetAbsHNDC()));
		}
		
		//draw histos
		hist[1][q]->Draw("hist");
		if(!reco){
			TF1* ftmp = hist[1][q]->GetFunction("gaus");
			ftmp->SetLineColor(fitcolors[1]);
			ftmp->SetLineStyle(styles[1]);
			ftmp->SetLineWidth(2);
			ftmp->Draw("same");
		}
		pad1->Modified(); pad1->Update();
		TPaveStats *ps1 = (TPaveStats*)hist[1][q]->GetListOfFunctions()->FindObject("stats");
		double x1_1 = ps1->GetX1NDC(); double x2_1 = ps1->GetX2NDC();
		//cout << "x1 = " << ps1->GetX1NDC() << ", x2 = " << ps1->GetX2NDC() << endl;
		
		hist[0][q]->Draw("hist sames");
		if(!reco){
			TF1* ftmp = hist[0][q]->GetFunction("gaus");
			ftmp->SetLineColor(fitcolors[0]);
			ftmp->SetLineStyle(styles[0]);
			ftmp->SetLineWidth(2);
			ftmp->Draw("same");
		}
		pad1->Modified(); pad1->Update();
		TPaveStats *ps = (TPaveStats*)hist[0][q]->GetListOfFunctions()->FindObject("stats");
			
		//move stat box
		ps->SetX1NDC(x1_1-(x2_1-x1_1)); ps->SetX2NDC(x1_1);
		ps->SetTextColor(colors[0]);
		ps->SetLineColor(colors[0]);
		pad1->Modified(); pad1->Update();
		
		//make ratio
		hrat[q] = (TH1F*)hist[0][q]->Clone();
		hrat[q]->SetStats(kFALSE);
		hrat[q]->Divide(hist[1][q]); //new/ref
		hrat[q]->GetXaxis()->SetTitle(qtys[q].c_str());
		hrat[q]->GetYaxis()->SetTitle("new/ref");
		hrat[q]->SetTitle("");
		hrat[q]->SetMarkerStyle(20);
		hrat[q]->SetMarkerColor(kBlack);
		hrat[q]->SetMarkerSize(1.25);
		hrat[q]->SetLineColor(kBlack);
		hrat[q]->SetFillColor(0);
		hrat[q]->GetXaxis()->SetLabelOffset(0.005);
		hrat[q]->GetYaxis()->SetTitleOffset(0.35);
		hrat[q]->GetXaxis()->SetLabelColor(1);
		hrat[q]->GetYaxis()->SetLabelColor(1);
		hrat[q]->GetXaxis()->SetTitleSize(32/(pad2->GetWh()*pad2->GetAbsHNDC()));
		hrat[q]->GetXaxis()->SetLabelSize(28/(pad2->GetWh()*pad2->GetAbsHNDC()));
		hrat[q]->GetYaxis()->SetTitleSize(32/(pad2->GetWh()*pad2->GetAbsHNDC()));
		hrat[q]->GetYaxis()->SetLabelSize(28/(pad2->GetWh()*pad2->GetAbsHNDC()));
		hrat[q]->GetYaxis()->SetTickLength(6/(pad2->GetWh()*pad2->GetAbsHNDC()));
		hrat[q]->GetXaxis()->SetTickLength(12/(pad2->GetWh()*pad2->GetAbsHNDC()));
		hrat[q]->GetYaxis()->SetNdivisions(503);
		hrat[q]->GetYaxis()->SetRangeUser(0,2);

		//draw ratio
		pad2->cd();
		hrat[q]->Draw("PE");
		
		//line
		line[q] = new TLine(hrat[q]->GetXaxis()->GetXmin(),1,hrat[q]->GetXaxis()->GetXmax(),1);
		line[q]->SetLineStyle(2);
		line[q]->SetLineWidth(1);
		line[q]->SetLineColor(kBlack);
		line[q]->Draw("same");
		
		//save image
		can->Print((cname + ".png").c_str(),"png");
	}
	
}
