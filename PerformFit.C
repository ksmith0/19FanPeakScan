//Author: Karl Smith

#include <fstream>
#include <sstream>
#include <string>
#include "TF1.h"
#include "TGraphErrors.h"

#if defined(__CINT__) && !defined(__MAKECINT__) 
	#include "PeakFit.so"
#else
	#include "PeakFit.h"
#endif

TGraphErrors *PerformFit(const char *expFilename, const char *simFilename) {
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
		double A = 0, dA = 100;
		double sumA = 0, dSumA = 0;
		while (linestr >> x0 >> A >> dA) {
			sumA += A;
			dSumA += sqrt(pow(dSumA,2) + pow(dA,2));
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

	graph->Fit(f,"EM");	

	return graph;
}
