#ifndef PEAKFIT_H
#define PEAKFIT_H

#include <vector>
#include "TF1.h"
#include "TGraph.h"

class PeakFit {
	public:
		PeakFit(const char* filename);
		virtual ~PeakFit();
		double operator() (double *x, double *p);
		double GetComponent(unsigned int barNum, unsigned int state);
		unsigned int GetNumBars() {return components_.size();};
		unsigned int GetNumStates(unsigned int barNum);
		unsigned int GetMaxNumStates() {return maxNumStates_;};
		TF1 *GetComponent(unsigned int state, double scale);
		TGraph *GetFitVsAngle(double *scale, const Float_t barAngles[][2]);
		TGraph *GetComponentVsAngle(unsigned int state, double scale, const Float_t barAngles[][2]);

	private:
		std::vector< int > GetBarMap( const Float_t barAngles[][2]);
		std::vector< std::vector<float> > components_; //Vector with indicesof bar, state.
		void LoadComponents(const char* filename);
		unsigned int maxNumStates_;
		std::vector <TF1*> fitComp_;
};

#endif
