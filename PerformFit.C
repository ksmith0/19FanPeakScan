//Author: Karl Smith
//Editted by WAP

#include <fstream>
#include <sstream>
#include <string>
#include "TF1.h"
#include "TFitResult.h"
#include "TGraphErrors.h"
#include "TStyle.h" 
#include "TMatrix.h"
#include "TAxis.h"

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
			if (dA > 200) {dA = 200 + sqrt(A);}
			dA += 100;
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

	PeakFit *func = new PeakFit(simFilename);
	TF1 *fFirst = new TF1("fitFirst",func,0,41,func->GetMaxNumStates());
	const int numStates = func->GetMaxNumStates();
	const unsigned int numParams = fFirst->GetNpar();

	//Estimate the constants
	for (unsigned int i=0;i<numParams;i++) {
		fFirst->SetParameter(i,0.9 * graph->GetMaximum() / numStates);
	}

	TF1 *f = new TF1("fit",func,0,41,func->GetMaxNumStates());
	f->SetLineColor(kBlue);


	graph->Fit(fFirst,"QR");	
	for (unsigned int i=0;i<numParams;i++) {
		double par = fFirst->GetParameter(i);
		if (par < 0) par *= -1;
		f->SetParameter(i,par);
	}
	TFitResultPtr fitResult = graph->Fit(f,"VRMES");	

	//Report fit results.
	printf("\nRESULTS\nNO. NAME %-12s %-12s %-12s %-12s\n","VALUE","ERROR","MINOS LOWER","MINOS UPPER");
	for (unsigned int i=0;i<numParams;i++) {
		printf("%3d %4s % 12.5e % 12.5e",i,f->GetParName(i),f->GetParameter(i),f->GetParError(i));
		if (fitResult->HasMinosError(i)) {
			if (fitResult->LowerError(i)) printf(" % 12.5e", fitResult->LowerError(i));
			else printf("%12s"," ");
			if (fitResult->UpperError(i)) printf(" % 12.5e", fitResult->UpperError(i));
			else printf("%12s"," ");
			printf("\n");
		}
		else printf("\n");
	}

	//Plot the components
	for (int i=0;i<numStates;i++) {
		func->GetComponent(i,f->GetParameter(i))->Draw("SAME");
	}

	//Re scale the y axis to show from 0 to max.
	graph->GetYaxis()->SetLimits(0,graph->GetYaxis()->GetXmax());
	graph->GetYaxis()->SetRangeUser(0,graph->GetYaxis()->GetXmax());

	printf("\n");

	//Do something interesting with the fit results see https://root.cern.ch/doc/master/classROOT_1_1Fit_1_1FitResult.html
	//For example print the Chi Sq
	printf("The Chi2 is: %f / %u\n", fitResult->Chi2(), fitResult->Ndf());

//define matrix and vector variables

	TMatrix fitvalues(1,numStates);
	TMatrix errvalues(1,numStates);
	TMatrix fitvaluesT(numStates,1);

	double totalerror;

//set matrix and vectors from fit


		for (unsigned int i=0;i<numParams;i++) {
			fitvalues(0,i) = abs(f->GetParameter(i));
			errvalues(0,i) = f->GetParError(i);
		}


//	TMatrix fitvaluesT(1,numParams);

// TMatrix result(Vmat, kTransposeMult,TMatrix(M,kMult,Vmat) );

//	fitvaluesT(Transpose(fitvalues));

	TMatrixDSym covar = fitResult->GetCovarianceMatrix();
	covar.Print();


	TMatrixDSym corr = fitResult->GetCorrelationMatrix();

	TMatrix tempp(corr,TMatrix::kMultTranspose,errvalues);
	tempp.Print();

	errvalues.Print();

	TMatrix temmp(errvalues,TMatrix::kMult,tempp);
	temmp.Print();

	TMatrix vartotal(errvalues,TMatrix::kMult,TMatrix(corr,TMatrix::kMultTranspose,errvalues) );

	vartotal.Print();

	double stddevtotal = sqrt(vartotal(0,0));


//	totalerror = fitvalues * covar * fitvaluesT;
/*
//Calculate total error from Covar matrix and fitvalues vector
	

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


	for (unsigned int i=0;i<numParams;i++) {
		for (unsigned int j=0;j<numParams;j++) {
			printf("% 8e, ",covar(i,j));
		}
		printf("\n");
	}
	printf("The Correlation Matrix:\n");
	for (unsigned int i=0;i<fitResult->NPar();i++) {
		for (unsigned int j=0;j<fitResult->NPar();j++) {
			printf("% 8e, ",fitResult->Correlation(i,j));
		}
		printf("\n");
	}

	double sigtotal = 0.0;

		printf("The Fit Values matrix transposed [1,n]:\n");
	for (unsigned int i=0;i<fitResult->NPar();i++) {
			printf("%8e, ",fitvalues(0,i));
			sigtotal += fitvalues(0,i);
		printf("\n");
	}

	printf("Total cross section:\n");
	printf("%8e, ",sigtotal);

	printf("\n Total standard dev:\n");
	printf("%8e, ",stddevtotal);
	printf("\n graph:\n");

	return graph;
}
