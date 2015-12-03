#ifndef PEAKFIT_H
#define PEAKFIT_H

#include <vector>

class PeakFit {
	public:
		PeakFit(const char* filename);
		virtual ~PeakFit();
		double operator() (double *x, double *p);
		double GetComponent(unsigned int barNum, unsigned int state);
		unsigned int GetNumBars() {return components_.size();};
		unsigned int GetNumStates(unsigned int barNum);
		unsigned int GetMaxNumStates();

	private:
		std::vector<std::vector<float>> components_;
		void LoadComponents(const char* filename);
};

#endif
