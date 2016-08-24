#include <new>

#include "src/inclined_sum/Inclined3dSum.h"

Inclined3dSum::Inclined3dSum(const Parameters* _props, const Well* _well) : BaseSum(_props, _well)
{
}

Inclined3dSum::~Inclined3dSum()
{
}

double Inclined3dSum::get2D(int seg_idx)
{
	double sum = 0.0;

	for (int k = 0; k < props->K; k++)
	{
		const WellSegment& seg = well->segs[k];
		sum += F2d[seg_idx * props->K + k] * seg.rate / seg.length;
	}

	sum *= (props->visc * props->sizes.x / M_PI / M_PI / props->sizes.z / props->kx / sin(props->alpha));

	return sum;
}

double Inclined3dSum::get3D(int seg_idx)
{
	double sum = 0.0;

	for (int k = 0; k < props->K; k++)
	{
		const WellSegment& seg = well->segs[k];
		sum += F3d[seg_idx * props->K + k] * seg.rate / seg.length;
	}

	sum *= (2.0 * props->visc / M_PI / props->sizes.x / props->sizes.z / props->kx / cos(props->alpha));

	return sum;
}

void Inclined3dSum::prepare()
{
	prepare2D();	prepare3D();
}

void Inclined3dSum::prepare2D()
{

}

void Inclined3dSum::prepare3D()
{
	double a[2], b[2], c[2], d[2], e[2], f[2];
	double A, B1, B2;
	double sum1, sum2;
	double buf1, buf2;
	int k;

	for (int arr_idx = 0; arr_idx < props->K * props->K; arr_idx++)
	{
		const WellSegment seg = well->segs[arr_idx % props->K];
		const Point& point = well->segs[int((double)(arr_idx) / (double)(props->K))].r_bhp;

		F3d[arr_idx] = sum1 = sum2 = 0.0;
	
		for (int p = 1; p <= props->I; p++)
		{
			for (int q = 1; q <= props->I; q++)
			{
				for (int r = 1; r <= props->I; r++)
				{
					a[0] = props->r1.x - props->r2.x;	b[0] = point.x + 2.0 * (double)(p)* props->sizes.x - props->r1.x;
					a[1] = props->r2.x - props->r1.x;	b[1] = point.x + 2.0 * (double)(p)* props->sizes.x + props->r1.x;
					c[0] = props->r1.y - props->r2.y;	d[0] = point.y + 2.0 * (double)(q)* props->sizes.y - props->r1.y;
					c[1] = props->r2.y - props->r1.y;	d[1] = point.y + 2.0 * (double)(q)* props->sizes.y + props->r1.y;
					e[0] = props->r1.z - props->r2.z;	f[0] = point.z + 2.0 * (double)(r)* props->sizes.z - props->r1.z;
					e[1] = props->r2.z - props->r1.z;	f[1] = point.z + 2.0 * (double)(r)* props->sizes.z + props->r1.z;

					// First
					A = (a[0] * a[0] * (d[0] * d[0] + f[0] * f[0]) -
						2.0 * a[0] * b[0] * (c[0] * d[0] + e[0] * f[0]) +
						b[0] * b[0] * (c[0] * c[0] + e[0] * e[0]) +
						(e[0] * d[0] - c[0] * f[0]) * (e[0] * d[0] - c[0] * f[0])) / 
						(4.0 * (a[0] * a[0] + c[0] * c[0] + e[0] * e[0]));
					B1 = a[0] * (a[0] * +b[0]) + c[0] * (c[0] * +d[0]) + e[0] * (e[0] * +f[0]) / 
						(2.0 * sqrt(a[0] * a[0] + c[0] * c[0] + e[0] * e[0]));
					B2 = a[0] * (a[0] * +b[0]) + c[0] * (c[0] * +d[0]) + e[0] * (e[0] * +f[0]) /
						(2.0 * sqrt(a[0] * a[0] + c[0] * c[0] + e[0] * e[0]));
				}
			}
		}

	}
}