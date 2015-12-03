#include "PeakFit.h"
#include <fstream>
#include <sstream>
#include <string>

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
	for (unsigned int i=0;i<components_[bar].size();i++) {
		retVal += fabs(p[i+1]) * components_[bar][i];
	}
	return retVal;
}

