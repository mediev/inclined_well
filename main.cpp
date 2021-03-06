#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <vector>
#include <iomanip>

#include "paralution.hpp"

#include "src/WellFlow.h"
#include "src/utils/perf-utils.hpp"

#include "src/inclined_sum/InclinedSum.hpp"
#include "src/inclined_sum/Inclined3dSum.h"
#include "src/inclined_sum/VerticalDirichlet.h"
#include "src/inclined_sum/Frac2dSum.hpp"
#include "src/inclined_sum/neumann/VerticalNeumann.h"
#include "src/inclined_sum/neumann/VerticalNeumannUniformBoundaries.h"
#include "src/inclined_sum/neumann/VerticalNeumannGaussBoundaries.h"
#include "src/inclined_sum/welltests/HorizontalLogDerivation.hpp"

using namespace std;
using namespace paralution;

/*void testNeumann()
{
	WellFlow solver("task/config2d.xml");
	double p_bhp;

	auto t = measure_time(
		[&]() {
		VerticalNeumann inclSum(solver.getProps(), solver.getWell());
		solver.setSummator(&inclSum);
		const Parameters* props = solver.getProps();
		p_bhp = solver.getP_bhp() * props->p_dim / BAR;
	}, 1);

	print_test_results("PERF_TEST", t);

	cout << "P_bhp = " << p_bhp << endl;
}
void testNeumannUniformBoundary()
{
	WellFlow solver("task/config2d.xml");
	double p_bhp;
	double p_avg;

	auto t = measure_time(
		[&]() {
		VerticalNeumannUniformBoundaries inclSum(solver.getProps(), solver.getWell());
		solver.setSummator(&inclSum);
		const Parameters* props = solver.getProps();
		p_bhp = solver.getP_bhp() * props->p_dim / BAR;
		p_avg = inclSum.getPresAvg() * props->p_dim / BAR;
	}, 1);

	print_test_results("PERF_TEST", t);

	cout << "P_bhp = " << p_bhp << endl;
	cout << "P_avg = " << p_avg << endl;
}
void testNeumannGaussBoundary()
{
	WellFlow solver("task/config2d.xml");
	double p_bhp;
	double p_avg;

	auto t = measure_time(
		[&]() {
		VerticalNeumannGaussBoundaries inclSum(solver.getProps(), solver.getWell());
		solver.setSummator(&inclSum);
		const Parameters* props = solver.getProps();
		p_bhp = solver.getP_bhp() * props->p_dim / BAR;
		p_avg = inclSum.getPresAvg() * props->p_dim / BAR;
	}, 1);

	print_test_results("PERF_TEST", t);

	cout << "P_bhp = " << p_bhp << endl;
	cout << "P_avg = " << p_avg << endl;
}*/
void test3D()
{
	WellFlow solver("task/config3d_new.xml");
	auto well = const_cast<Well*>(solver.getWell(0));
	auto summator = const_cast<BaseSum*>(solver.getSummator(0));
	const MainProperties* props = solver.getProps();
	double p_bhp, p1, ccf, ccf_peac, mult;

	auto t = measure_time(
		[&]() {

		const double dx = 100.0 / props->x_dim;
		p_bhp = solver.getP_bhp();
		auto rc = well->getGeomProps()->rc;
		p1 = summator->getPressure({ rc.x - dx, rc.y, rc.z });
		ccf = props->x_dim * props->x_dim * props->x_dim * props->rate * props->visc / (p_bhp - p1 - props->rate * props->visc / 4.0 / props->kx / props->sizes.z) * 86400 * BAR * 1000.0;
		ccf_peac = props->x_dim * props->x_dim * props->x_dim * 2.0 * M_PI * props->kx * well->getGeomProps()->length / log(0.198 * dx / well->getGeomProps()->rw) * 86400 * BAR * 1000.0;
		mult = ccf / ccf_peac;
	}, 1);

	print_test_results("PERF_TEST", t);
	cout << setprecision(4);
	cout << "P_bhp = \t" << p_bhp * props->p_dim / BAR << endl;
	cout << "Pi = \t\t" << p1 * props->p_dim / BAR << endl;
	cout << "CCF = \t\t" << ccf << endl;
	cout << "CCF_Peaceman = \t" << ccf_peac << endl;
	cout << "MULT = \t\t" << mult << endl;
}
/*void testDirichlet()
{
	WellFlow solver("task/horizontal.xml");
	auto summator = const_cast<BaseSum*>(solver.getSummator(0));
	auto well = const_cast<Well*>(solver.getWell(0));
	double p_bhp, p1, p2, p3, p4;

	auto t = measure_time(
		[&]() {
		//InclinedSum inclSum(solver.getProps(), solver.getWell());
		//solver.setSummator(&inclSum);
		const MainProperties* props = solver.getProps();

		auto rc = well->getGeomProps()->rc;		rc.y += well->getGeomProps()->rw;
		const double dx = 0.909 / props->x_dim;
		
		p_bhp = solver.getP_bhp() * props->p_dim / BAR;
		p1 = summator->getPressure({ rc.x - dx, rc.y, rc.z }) * props->p_dim / BAR;
		p2 = summator->getPressure({ rc.x + dx, rc.y, rc.z }) * props->p_dim / BAR;
		p3 = summator->getPressure({ rc.x, rc.y - dx, rc.z }) * props->p_dim / BAR;
		p4 = summator->getPressure({ rc.x, rc.y + dx, rc.z }) * props->p_dim / BAR;
		well->writeRates(props);
	}, 1);

	print_test_results("PERF_TEST", t);
	cout << "P_bhp = " << p_bhp << endl;
	cout << "P_x- = " << p1 << endl;
	cout << "P_x+ = " << p2 << endl;
	cout << "P_y- = " << p3 << endl;
	cout << "P_y+ = " << p4 << endl;
}*/
/*void testHorizontalLogDerivative()
{
	WellFlow solver("task/horizontal.xml");

	HorizontalLogDerivation inclSum(solver.getProps(), solver.getWell());
	solver.setSummator(&inclSum);
	const Parameters* props = solver.getProps();

	ofstream file;
	file.open("P.txt", ofstream::out);

	//const double finalTime = 20.0 * 86400.0;
	double initStep = 1000.0;
	const int size = 100;
	//const double pres_stat = solver.getP_bhp();

	for (int i = 1; i <= size; i++)
	{
		inclSum.setTime(initStep);
		file << initStep / 3600.0 << "\t" <<
				solver.getP_bhp() * props->p_dim / BAR << endl;
//		cout << "P_bhp = " << solver.getP_bhp() * props->p_dim / BAR << "\t" << endl;
		initStep *= 1.2;
	}

	file.close();
}*/
void testFrac2D()
{
	WellFlow solver("task/frac2d.xml");
	double p_bhp;

	auto t = measure_time(
		[&]() {
		const MainProperties* props = solver.getProps();
		p_bhp = solver.getP_bhp() * props->p_dim / BAR;
	}, 1);

	print_test_results("PERF_TEST", t);

	cout << setprecision(4);
	cout << "P_bhp = " << p_bhp << endl;
}
void testVerticalDirichlet()
{
	WellFlow solver("task/vertical.xml");
	double p_bhp, p_an, p1, p2, p3, p4;
	auto well = const_cast<Well*>(solver.getWell(0));
	const auto summator1 = static_cast<const VerticalDirichlet*>(solver.getSummator(0));
	auto summator = const_cast<BaseSum*>(solver.getSummator(0));
	const MainProperties* props = solver.getProps();
	const double dx = 100.0 / props->x_dim;

	auto t = measure_time(
		[&]() {
		p_bhp = solver.getP_bhp() * props->p_dim / BAR;
		p_an = summator1->getAnalyticalPres(0) * props->p_dim / BAR;
		auto rc = well->getGeomProps()->rc;		rc.y += well->getGeomProps()->rw;
		p1 = summator->getPressure({ rc.x - dx, rc.y, rc.z }) * props->p_dim / BAR;
		p2 = summator->getPressure({ rc.x + dx, rc.y, rc.z }) * props->p_dim / BAR;
		p3 = summator->getPressure({ rc.x, rc.y - dx, rc.z }) * props->p_dim / BAR;
		p4 = summator->getPressure({ rc.x, rc.y + dx, rc.z }) * props->p_dim / BAR;
	}, 1);

	print_test_results("PERF_TEST", t);
	cout << setprecision(6);
	cout << "P_bhp = " << p_bhp << endl;
	cout << "P_ana = " << p_an << endl;
	cout << "P_x- = " << p1 << endl;
	cout << "P_x+ = " << p2 << endl;
	cout << "P_y- = " << p3 << endl;
	cout << "P_y+ = " << p4 << endl;
}

int main(int argc, char* argv[])
{
	init_paralution();
	//testFrac2D();
	//testDirichlet();
	//testVerticalDirichlet();
	test3D();
	stop_paralution();

	return 0;
}