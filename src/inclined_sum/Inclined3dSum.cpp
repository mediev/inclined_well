#include <new>
#include <cassert>

#include "src/inclined_sum/Inclined3dSum.h"

Inclined3dSum::Inclined3dSum(const SummatorProperties& _sprops, const MainProperties* _props, const Well* _well) : BaseSum(_sprops, _props, _well)
{
}
Inclined3dSum::~Inclined3dSum()
{
}
double Inclined3dSum::getPressure(const Point& point)
{
	double sum = 0.0;
	double sum_prev_m, sum_prev_n;
	double buf, F1, F2, denom1, denom2;
	int break_idx_m = 0, break_idx_n = 0;

	double sum1 = 0.0, sum2 = 0.0;
	std::vector<double> f2;		f2.resize(sprops.K);
	for (int arr_idx = 0; arr_idx < sprops.K; arr_idx++)
	{
		const WellSegment& seg = well->segs[arr_idx];
		const Point& r = point;
		const Point rad = gprops->r2 - gprops->r1;

		f2[arr_idx] = sum_prev_m = 0.0;

		break_idx_m = 0;
		for (int m = 1; m <= sprops.M; m++)
		{
			sum_prev_n = 0;	 break_idx_n = 0;
			for (int n = 1; n <= sprops.M; n++)
			{
				F1 = 0.0;
				denom1 = M_PI * (double)m / props->sizes.x * rad.x - M_PI * (double)n / props->sizes.y * rad.y;
				denom2 = M_PI * (double)m / props->sizes.x * rad.x + M_PI * (double)n / props->sizes.y * rad.y;
				if (fabs(denom1) < EQUALITY_TOLERANCE)
					F1 += cos(M_PI * (double)m * seg.r1.x / props->sizes.x - M_PI * (double)n * seg.r1.y / props->sizes.y) * (seg.tau2 - seg.tau1) / 2.0;
				else
					F1 += (sin(M_PI * (double)m * seg.r2.x / props->sizes.x - M_PI * (double)n * seg.r2.y / props->sizes.y) -
						sin(M_PI * (double)m * seg.r1.x / props->sizes.x - M_PI * (double)n * seg.r1.y / props->sizes.y)) / denom1 / 2.0;
				if (fabs(denom2) < EQUALITY_TOLERANCE)
					F1 += -cos(M_PI * (double)m * seg.r1.x / props->sizes.x + M_PI * (double)n * seg.r1.y / props->sizes.y) * (seg.tau2 - seg.tau1) / 2.0;
				else
					F1 += -(sin(M_PI * (double)m * seg.r2.x / props->sizes.x + M_PI * (double)n * seg.r2.y / props->sizes.y) -
						sin(M_PI * (double)m * seg.r1.x / props->sizes.x + M_PI * (double)n * seg.r1.y / props->sizes.y)) / denom2 / 2.0;

				buf = M_PI * M_PI * ((double)m * (double)m / props->sizes.x / props->sizes.x + (double)n * (double)n / props->sizes.y / props->sizes.y);
				f2[arr_idx] += 1.0 / 2.0 * F1 * sin(M_PI * (double)m * r.x / props->sizes.x) *	sin(M_PI * (double)n * r.y / props->sizes.y) * exp(-sprops.xi_c * buf) / buf;

				for (int l = 1; l <= sprops.L; l++)
				{
					auto getFoo = [=, this]() -> double
					{
						const Point point1 = M_PI * seg.r1 / props->sizes;
						const Point point2 = M_PI * seg.r2 / props->sizes;
						const Point p[] = { { (double)m, (double)n, -(double)l },
											{ (double)m, -(double)n, (double)l },
											{ (double)m, -(double)n, -(double)l },
											{ (double)m, (double)n, (double)l } };
						const double mult[4] = { -1.0, 1.0, 1.0, -1.0 };
						double denom;
						double sum = 0.0;
						for (int i = 0; i < 4; i++)
						{
							denom = M_PI * p[i] * (rad / props->sizes);
							if (fabs(denom) < EQUALITY_TOLERANCE)
								sum += mult[i] * cos(p[i] * seg.r1) * (seg.tau2 - seg.tau1);
							else
								sum += mult[i] * (sin(p[i] * point2) - sin(p[i] * point1)) / denom;
						}
						return 1.0 / 4.0 * sum;
					};

					F2 = getFoo();
					buf = M_PI * M_PI * ((double)m * (double)m / props->sizes.x / props->sizes.x +
						(double)n * (double)n / props->sizes.y / props->sizes.y + (double)l * (double)l / props->sizes.z / props->sizes.z);
					f2[arr_idx] += F2 * sin(M_PI * (double)m * r.x / props->sizes.x) *	sin(M_PI * (double)n * r.y / props->sizes.y) *
						cos(M_PI * (double)l * r.z / props->sizes.z) * exp(-sprops.xi_c * buf) / buf;
				}

				if (fabs(f2[arr_idx] - sum_prev_n) > f2[arr_idx] * EQUALITY_TOLERANCE)
				{
					sum_prev_n = f2[arr_idx];
					break_idx_n = 0;
				}
				else
					break_idx_n++;

				if (break_idx_n > 1)
				{
					//std::cout << m << std::endl;
					//break;
				}
			}

			if (fabs(f2[arr_idx] - sum_prev_m) > f2[arr_idx] * EQUALITY_TOLERANCE)
			{
				sum_prev_m = f2[arr_idx];
				break_idx_m = 0;
			}
			else
				break_idx_m++;

			if (break_idx_m > 1)
			{
				//std::cout << m << std::endl;
				//break;
			}
		}
	}

	for (int k = 0; k < sprops.K; k++)
	{
		const WellSegment& seg = well->segs[k];
		sum1 += f2[k] * seg.rate / seg.length;
	}
	sum1 *= (8.0 * props->visc * gprops->length / props->sizes.x / props->sizes.y / props->sizes.z / props->kx);


	std::vector<double> f3;		f3.resize(sprops.K);
	for (int arr_idx = 0; arr_idx < sprops.K; arr_idx++)
	{
		const WellSegment& seg = well->segs[arr_idx];
		const Point& r = point;

		f3[arr_idx] = 0.0;

		for (int p = -sprops.I; p <= sprops.I; p++)
		{
			for (int q = -sprops.I; q <= sprops.I; q++)
			{
				for (int s = -50 * sprops.I; s <= 50 * sprops.I; s++)
				{
					Point pt((double)p, (double)q, (double)s);
					auto getFoo = [=, this](const Point& point) -> double
					{
						Point vv1, vv2;
						vv1 = (r - point + 2.0 * product(pt, props->sizes));	vv1 = product(vv1, vv1) / 4.0;
						vv2 = (r + point + 2.0 * product(pt, props->sizes));	vv2 = product(vv2, vv2) / 4.0;
						double bbuf1, bbuf2, bbuf3, bbuf4, bbuf5, bbuf6, bbuf7, bbuf8;
						bbuf1 = vv1.x + vv1.y + vv1.z;		bbuf2 = vv1.x + vv1.y + vv2.z;
						bbuf3 = vv1.x + vv2.y + vv1.z;		bbuf4 = vv1.x + vv2.y + vv2.z;
						bbuf5 = vv2.x + vv1.y + vv1.z;		bbuf6 = vv2.x + vv1.y + vv2.z;
						bbuf7 = vv2.x + vv2.y + vv1.z;		bbuf8 = vv2.x + vv2.y + vv2.z;
						return	erfc(sqrt(bbuf1 / sprops.xi_c)) / sqrt(bbuf1)
							+ erfc(sqrt(bbuf2 / sprops.xi_c)) / sqrt(bbuf2)
							- erfc(sqrt(bbuf3 / sprops.xi_c)) / sqrt(bbuf3)
							- erfc(sqrt(bbuf4 / sprops.xi_c)) / sqrt(bbuf4)
							- erfc(sqrt(bbuf5 / sprops.xi_c)) / sqrt(bbuf5)
							- erfc(sqrt(bbuf6 / sprops.xi_c)) / sqrt(bbuf6)
							+ erfc(sqrt(bbuf7 / sprops.xi_c)) / sqrt(bbuf7)
							+ erfc(sqrt(bbuf8 / sprops.xi_c)) / sqrt(bbuf8);
					};

					Double2d* integrand = new Double2d[PART_SIZE + 1];
					for (int i = 0; i < PART_SIZE + 1; i++)
					{
						Point point = seg.r1 + (double)i * (seg.r2 - seg.r1) / (double)PART_SIZE;
						double tau = seg.tau1 + (double)i * (seg.tau2 - seg.tau1) / (double)PART_SIZE;
						integrand[i].x = tau;
						double qwe = getFoo(point);
						integrand[i].y = getFoo(point);
					}
					integr = new Integral(integrand, PART_SIZE + 1);
					f3[arr_idx] += integr->Calculate(seg.tau1, seg.tau2);

					delete integrand;
					delete integr;
				}
			}
		}
	}

	for (int k = 0; k < sprops.K; k++)
	{
		const WellSegment& seg = well->segs[k];
		sum2 += f3[k] * seg.rate / seg.length;
	}
	sum2 *= (props->visc * gprops->length / 8.0 / M_PI / props->kx);

	return sum1 + sum2;
}
double Inclined3dSum::get2D(int seg_idx) const
{
	double sum = 0.0;
	for (int k = 0; k < sprops.K; k++)
	{
		const WellSegment& seg = well->segs[k];
		sum += F2d[seg_idx * sprops.K + k] * seg.rate / seg.length;
	}

	sum *= (8.0 * props->visc * gprops->length / props->sizes.x / props->sizes.y / props->sizes.z / props->kx);
	return sum;
}
double Inclined3dSum::get3D(int seg_idx) const
{
	double sum = 0.0;
	for (int k = 0; k < sprops.K; k++)
	{
		const WellSegment& seg = well->segs[k];
		sum += F3d[seg_idx * sprops.K + k] * seg.rate / seg.length;
	}

	sum *= (props->visc * gprops->length / 8.0 / M_PI / props->kx);
	return sum;
}
void Inclined3dSum::prepare()
{
	size = segs->size() * sprops.K;
	F2d = new double[size];		F3d = new double[size];
	prepareDirect();	prepareFourier();
}
void Inclined3dSum::prepareDirect()
{
	double sum = 0.0;
	double sum_prev_m, sum_prev_n;
	double buf, F1, F2, denom1, denom2;
	int break_idx_m = 0, break_idx_n = 0;

	for (int arr_idx = 0; arr_idx < size; arr_idx++)
	{
		const WellSegment& seg = well->segs[arr_idx % sprops.K];
		const Point& r = (*segs)[int((double)(arr_idx) / (double)(sprops.K))]->r_bhp;
		const Point rad = gprops->r2 - gprops->r1;

		F2d[arr_idx] = sum_prev_m = 0.0;

		break_idx_m = 0;
		for (int m = 1; m <= sprops.M; m++)
		{
			sum_prev_n = 0;	 break_idx_n = 0;
			for (int n = 1; n <= sprops.M; n++)
			{
				F1 = 0.0;
				denom1 = M_PI * (double)m / props->sizes.x * rad.x - M_PI * (double)n / props->sizes.y * rad.y;
				denom2 = M_PI * (double)m / props->sizes.x * rad.x + M_PI * (double)n / props->sizes.y * rad.y;
				if (fabs(denom1) < EQUALITY_TOLERANCE)
					F1 += cos(M_PI * (double)m * seg.r1.x / props->sizes.x - M_PI * (double)n * seg.r1.y / props->sizes.y) * (seg.tau2 - seg.tau1) / 2.0;
				else
					F1 += (sin(M_PI * (double)m * seg.r2.x / props->sizes.x - M_PI * (double)n * seg.r2.y / props->sizes.y) -
						sin(M_PI * (double)m * seg.r1.x / props->sizes.x - M_PI * (double)n * seg.r1.y / props->sizes.y)) / denom1 / 2.0;
				if (fabs(denom2) < EQUALITY_TOLERANCE)
					F1 += -cos(M_PI * (double)m * seg.r1.x / props->sizes.x + M_PI * (double)n * seg.r1.y / props->sizes.y) * (seg.tau2 - seg.tau1) / 2.0;
				else 
					F1 += -(sin(M_PI * (double)m * seg.r2.x / props->sizes.x + M_PI * (double)n * seg.r2.y / props->sizes.y) -
						sin(M_PI * (double)m * seg.r1.x / props->sizes.x + M_PI * (double)n * seg.r1.y / props->sizes.y)) / denom2 / 2.0;

				/*F1 = ((sin(M_PI * (double)m * seg.r2.x / props->sizes.x - M_PI * (double)n * seg.r2.y / props->sizes.y) -
						sin(M_PI * (double)m * seg.r1.x / props->sizes.x - M_PI * (double)n * seg.r1.y / props->sizes.y)) /
						(M_PI * (double)m / props->sizes.x * rad.x - M_PI * (double)n / props->sizes.y * rad.y) -
					(sin(M_PI * (double)m * seg.r2.x / props->sizes.x + M_PI * (double)n * seg.r2.y / props->sizes.y) -
						sin(M_PI * (double)m * seg.r1.x / props->sizes.x + M_PI * (double)n * seg.r1.y / props->sizes.y)) /
						(M_PI * (double)m / props->sizes.x * rad.x + M_PI * (double)n / props->sizes.y * rad.y)) / 2.0;*/
				buf = M_PI * M_PI * ((double)m * (double)m / props->sizes.x / props->sizes.x + (double)n * (double)n / props->sizes.y / props->sizes.y);
				F2d[arr_idx] += 1.0 / 2.0 * F1 * sin(M_PI * (double)m * r.x / props->sizes.x) *	sin(M_PI * (double)n * r.y / props->sizes.y) * exp(-sprops.xi_c * buf) / buf;

				for (int l = 1; l <= sprops.L; l++)
				{
					auto getFoo = [=, this]() -> double
					{
						const Point point1 = M_PI * seg.r1 / props->sizes;
						const Point point2 = M_PI * seg.r2 / props->sizes;
						const Point p[] = { { (double)m, (double)n, -(double)l  },
											{ (double)m, -(double)n, (double)l  },
											{ (double)m, -(double)n, -(double)l },
											{ (double)m, (double)n, (double)l	} };
						const double mult[4] = { -1.0, 1.0, 1.0, -1.0 };
						double denom;
						double sum = 0.0;
						for (int i = 0; i < 4; i++)
						{
							denom = M_PI * p[i] * (rad / props->sizes);
							if (fabs(denom) < EQUALITY_TOLERANCE)
								sum += mult[i] * cos(p[i] * seg.r1) * (seg.tau2 - seg.tau1);
							else
								sum += mult[i] * (sin(p[i] * point2) - sin(p[i] * point1)) / denom;
						}
						return 1.0 / 4.0 * sum;
					};

					F2 = getFoo();
					buf = M_PI * M_PI * ((double)m * (double)m / props->sizes.x / props->sizes.x +
						(double)n * (double)n / props->sizes.y / props->sizes.y + (double)l * (double)l / props->sizes.z / props->sizes.z);
					F2d[arr_idx] += F2 * sin(M_PI * (double)m * r.x / props->sizes.x) *	sin(M_PI * (double)n * r.y / props->sizes.y) *
						cos(M_PI * (double)l * r.z / props->sizes.z) * exp(-sprops.xi_c * buf) / buf;
				}
			}
		}
	}
}
void Inclined3dSum::prepareFourier()
{
	for (int arr_idx = 0; arr_idx < size; arr_idx++)
	{
		const WellSegment& seg = well->segs[arr_idx % sprops.K];
		const Point& r = (*segs)[int((double)(arr_idx) / (double)(sprops.K))]->r_bhp;

		F3d[arr_idx] = 0.0;

		for (int p = -sprops.I; p <= sprops.I; p++)
		{
			for (int q = -sprops.I; q <= sprops.I; q++)
			{
				for (int s = -50 * sprops.I; s <= 50 * sprops.I; s++)
				{
					Point pt((double)p, (double)q, (double)s);
					auto getFoo = [=, this](const Point& point) -> double
					{
						Point vv1, vv2;
						vv1 = (r - point + 2.0 * product(pt, props->sizes));	vv1 = product(vv1, vv1) / 4.0;
						vv2 = (r + point + 2.0 * product(pt, props->sizes));	vv2 = product(vv2, vv2) / 4.0;
						double bbuf1, bbuf2, bbuf3, bbuf4, bbuf5, bbuf6, bbuf7, bbuf8;
						bbuf1 = vv1.x + vv1.y + vv1.z;		bbuf2 = vv1.x + vv1.y + vv2.z;
						bbuf3 = vv1.x + vv2.y + vv1.z;		bbuf4 = vv1.x + vv2.y + vv2.z;
						bbuf5 = vv2.x + vv1.y + vv1.z;		bbuf6 = vv2.x + vv1.y + vv2.z;
						bbuf7 = vv2.x + vv2.y + vv1.z;		bbuf8 = vv2.x + vv2.y + vv2.z;
						return	erfc(sqrt(bbuf1 / sprops.xi_c)) / sqrt(bbuf1)
							+ erfc(sqrt(bbuf2 / sprops.xi_c)) / sqrt(bbuf2)
							- erfc(sqrt(bbuf3 / sprops.xi_c)) / sqrt(bbuf3)
							- erfc(sqrt(bbuf4 / sprops.xi_c)) / sqrt(bbuf4)
							- erfc(sqrt(bbuf5 / sprops.xi_c)) / sqrt(bbuf5)
							- erfc(sqrt(bbuf6 / sprops.xi_c)) / sqrt(bbuf6)
							+ erfc(sqrt(bbuf7 / sprops.xi_c)) / sqrt(bbuf7)
							+ erfc(sqrt(bbuf8 / sprops.xi_c)) / sqrt(bbuf8);
					};

					Double2d* integrand = new Double2d[PART_SIZE + 1];
					for (int i = 0; i < PART_SIZE + 1; i++)
					{
						Point point = seg.r1 + (double)i * (seg.r2 - seg.r1) / (double)PART_SIZE;
						double tau = seg.tau1 + (double)i * (seg.tau2 - seg.tau1) / (double)PART_SIZE;
						integrand[i].x = tau;
						double qwe = getFoo(point);
						integrand[i].y = getFoo(point);
					}
					integr = new Integral(integrand, PART_SIZE + 1);
					F3d[arr_idx] += integr->Calculate(seg.tau1, seg.tau2);

					delete integrand;
					delete integr;
				}
			}
		}
	}
}