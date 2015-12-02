#include "PeakFit.h"
#include <fstream>
#include <sstream>
#include <string>

PeakFit::PeakFit(const char* filename) {
	LoadComponents(filename);
}

void PeakFit::LoadComponents(const char* filename) {
	std::ifstream file(filename);

	std::string line;
	while(std::getline(file, line)) {
		
		std::istringstream linestr;
		linestr.str(line);
		unsigned int barNum;
		linestr >> barNum;
		//printf("Found bar %d\n",barNum);

		if (components_.size() <= barNum) components_.resize(barNum+1);

		int excitedState = -1;
		double countsPerRx = 0, uncertainty = 100;
		while (linestr >> excitedState >> countsPerRx >> uncertainty) {
			//printf("%d %e %e\n",excitedState,countsPerRx, uncertainty);
			components_[barNum].push_back(countsPerRx);
		}
	}

	file.close();
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

PeakFit::~PeakFit() {

}

double PeakFit::operator() (double *x, double *p) {
	return 0;
}

