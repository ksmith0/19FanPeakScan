//#define USESPLINE //Enable the use of splines in the function.

#include "PeakFit.h"
#include <fstream>
#include <sstream>
#include <string>
#ifdef USESPLINE
#include "TSpline.h"
#endif

PeakFit::PeakFit(const char* filename) :
	maxNumStates_(0)
{
	LoadComponents(filename);
}

void PeakFit::LoadComponents(const char* filename) {
	while (!fitComp_.empty()) {
		delete fitComp_.back();
		fitComp_.pop_back();
	}

	std::ifstream file(filename);

	std::string line;
	while(std::getline(file, line)) {
		
		std::istringstream linestr;
		linestr.str(line);
		unsigned int barNum;
		linestr >> barNum;
		//printf("Found bar %d\n",barNum);

		if (components_.size() <= barNum) components_.resize(barNum+1);

		float excitedState = -1;
		double countsPerRx = 0, uncertainty = 100;
		while (linestr >> excitedState >> countsPerRx >> uncertainty) {
			//printf("%f %e %e\n",excitedState,countsPerRx, uncertainty);
			components_[barNum].push_back(countsPerRx);
		}
		if (components_[barNum].size() > maxNumStates_) maxNumStates_ = components_[barNum].size();
	}

	file.close();

	fitComp_.resize(maxNumStates_, NULL);
}

double PeakFit::GetComponent(unsigned int barNum, unsigned int state) {
	if (barNum >= components_.size()) {
		//printf("ERROR: Requested bar number, %d, invalid!\n", barNum);
		return 0;
	}
	if (state >= components_[barNum].size()) {
		//printf("ERROR: Requested state, %d, invalid!\n", state);
		return 0;
	}
	return components_[barNum][state];
}

unsigned int PeakFit::GetNumStates(unsigned int barNum) {
	if (barNum >= components_.size()) {
		return 0;
	}
	return components_[barNum].size();
}

TF1 *PeakFit::GetComponent(unsigned int state, double scale) {
	if (state > maxNumStates_) return NULL;

	if (!fitComp_[state]) {
		fitComp_[state] = new TF1(Form("comp%d",state),this,0,components_.size(),maxNumStates_);
	}

	fitComp_[state]->SetParameter(state,scale);

	return fitComp_[state];
}
std::vector< int > PeakFit::GetBarMap(const Float_t barAngles[][2]) {
	std::vector< int > barMap;

	float lastAngle = 0;
	while (barMap.size() < GetNumBars()) {
		int minIndex = -1;
		for (unsigned int bar=0; bar < GetNumBars(); bar++) {
			float angle = barAngles[bar][0];
			if (angle <= lastAngle) continue;
			if (minIndex < 0) minIndex = bar;
			else if (angle < barAngles[minIndex][0]) minIndex = bar;
		}
		barMap.push_back(minIndex);
		lastAngle = barAngles[minIndex][0];
	}

	return barMap;
}
TGraph *PeakFit::GetFitVsAngle(double *scales, const Float_t barAngles[][2]) {
	std::vector< int > barMap = GetBarMap(barAngles);

	TGraph *gr = new TGraph();

	for (unsigned int barIndex = 0; barIndex < GetNumBars(); barIndex++) {
		int bar = barMap[barIndex];
		double value = 0;
		for (unsigned int state = 0; state < GetNumStates(bar); state++) {
			value += components_[bar][state] * scales[state];
		}
		std::cout << "Bar Index: " << barIndex << " Bar " << bar << " value " << value << "\n";
		gr->SetPoint(gr->GetN(), barAngles[bar][0], value);

	}
	gr->SetLineColor(kBlue);
	gr->SetLineWidth(2);

	return gr;
}
TGraph *PeakFit::GetComponentVsAngle(unsigned int state, double scale, const Float_t barAngles[][2]) {
	std::vector< int > barMap = GetBarMap(barAngles);
	TGraph *gr = new TGraph();

	for (unsigned int barIndex = 0; barIndex < GetNumBars(); barIndex++) {
		int bar = barMap[barIndex];
		gr->SetPoint(gr->GetN(), barAngles[bar][0], components_[bar][state] * scale);
	}
	gr->SetLineColor(kRed);
	gr->SetLineWidth(2);

	return gr;
}

PeakFit::~PeakFit() {
	while (!fitComp_.empty()) {
		delete fitComp_.back();
		fitComp_.pop_back();
	}

}

/**We treat the x variable as a bar and cast it to an integer to round down. After 
 * ensuring the bar number is valid we sum the components multiplied by a scaling
 * factor chosen from the provided parameters.
 *
 * \return The value of function with the specified bar number (x) and parameters p.
 */
double PeakFit::operator() (double *x, double *p) {
	//Compute the integer value of x by rounding
	unsigned int bar = x[0] + 0.5;
	if (bar >= components_.size()) return 0;

	double retVal = 0;
#ifdef USESPLINE
	double yVal[components_.size()];
	double xVal[components_.size()];
	for (bar = 0; bar < components_.size(); bar++) {
		xVal[bar] = bar;
		retVal = 0;
#endif

		for (unsigned int state=0; state < components_[bar].size(); state++) {
			retVal += fabs(p[state]) * components_[bar][state];
		}

#ifdef USESPLINE
		yVal[bar] = retVal;
	}

	TSpline3 spline("spline",xVal,yVal,components_.size());
	retVal = spline.Eval(x[0]);
#endif

	return retVal;
}

