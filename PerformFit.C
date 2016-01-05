//Author: Karl Smith
//Editted by WAP

#include <fstream>
#include <sstream>
#include <string>
// #include <vector>
#include "TF1.h"
#include "TFitResult.h"
#include "TGraphErrors.h"
#include "TStyle.h" 
#include "TMatrixD.h"
// #include "MatrixFunctions.h"

#if defined(__CINT__) && !defined(__MAKECINT__) 
	#include "PeakFit.so"
#else
	#include "PeakFit.h"
#endif

TGraphErrors *PerformFit(const char *expFilename, const char *simFilename) {
	gStyle->SetOptFit(11111);

	TGraphErrors *graph = new TGraphErrors();
	
	std::ifstream file(expFilename);

	std::string line;
	while(std::getline(file, line)) {
		
		std::istringstream linestr;
		linestr.str(line);
		unsigned int barNum;
		if (!(linestr >> barNum)) continue;
		//printf("Found bar %d\n",barNum);

		float x0 = -1;
		double A = 0.0, dA = 100.0;
		double sumA = 0.0, dSumA = 0.0, dSumAcalc = 0.0;
		bool first = true;
		while (linestr >> x0 >> A >> dA) {
			if (first) {
				first = false;
				continue;
			}
			//maybe set all dA to 300 or if too large set to sqrt(A)
			if (dA < 50) {dA = 50 + sqrt(A);}
			if (dA > 500) {dA = 300 + sqrt(A);}
			sumA += A;
			dSumAcalc = dSumA;
			dSumA = sqrt(pow(dSumAcalc,2) + pow(dA,2));
		}
		//Insert the point into the graph.
		int point = graph->GetN();
		graph->SetPoint(point, barNum, sumA);
		graph->SetPointError(point, 0, dSumA);
	}

	file.close();


	graph->SetMarkerStyle(kOpenCircle);
	graph->Draw("AP");

	PeakFit *fit = new PeakFit(simFilename);
	TF1 *f = new TF1("fit",fit,0,41,fit->GetMaxNumStates());
	f->SetLineColor(kBlue);

	TFitResultPtr fitResult = graph->Fit(f,"EMS");	


	const int numStates = fit->GetMaxNumStates();
	for (int i=0;i<numStates;i++) {
		fit->GetComponent(i,f->GetParameter(i))->Draw("SAME");
	}

	printf("\n");

	//Do something interesting with the fit results see https://root.cern.ch/doc/master/classROOT_1_1Fit_1_1FitResult.html
	//For example print the Chi Sq
	printf("The Chi2 is: %f / %u\n", fitResult->Chi2(), fitResult->Ndf());


//define matrix and vector variables

	int nparStates = fitResult->NPar();
	double fitvalues[nparStates];
	double totalerror;
	TMatrixDSym Covar(nparStates);
	TMatrixDSym Corr(nparStates);

//set matrix and vectors from fit

		for (int i=0;i<nparStates;i++) {
			fitvalues[i] = f->GetParameter(i);
		}


	Covar = fitResult->GetCovarianceMatrix();
	Corr = fitResult->GetCorrelationMatrix();
/*
//Calculate total error from Covar matrix and fitvalues vector
	totalerror = fitvalues * Covar * fitvalues;

	//GetCovarMatrix
	//GetVarianceMatrix
	//Print them
	//do math with vector of results GetComponents
	//Output result of fit plus uncertainty

*/
	printf("The Covariance Matrix:\n");
	printf("Status: ");
	switch(fitResult->CovMatrixStatus()) {
		case 0: 
			printf("Not Calculated\n");
			break;
		case 1:
			printf("Approximated\n");
			break;
		case 2:
			printf("Made Positive Definate\n");
			break;
		case 3:
			printf("Accurate\n");
			break;
		default:
			printf("Unknown\n");
			break;
	}
	for (unsigned int i=0;i<fitResult->NPar();i++) {
		for (unsigned int j=0;j<fitResult->NPar();j++) {
			printf("%8e, ",fitResult->CovMatrix(i,j));
		}
		printf("\n");
	}
	for (unsigned int i=0;i<nparStates;i++) {
		for (unsigned int j=0;j<nparStates;j++) {
			printf("%8e, ",Covar(i,j));
		}
		printf("\n");
	}
	printf("The Correlation Matrix:\n");
	for (unsigned int i=0;i<fitResult->NPar();i++) {
		for (unsigned int j=0;j<fitResult->NPar();j++) {
			printf("%8e, ",fitResult->Correlation(i,j));
		}
		printf("\n");
	}
	

	return graph;
}
