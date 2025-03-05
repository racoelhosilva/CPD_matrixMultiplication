#include <stdio.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <cstdlib>
#include <papi.h>
#include <filesystem>

using namespace std;

#define SYSTEMTIME clock_t

struct Statistics
{
	double time = 0.0;
	double mflops = 0.0;
	long long values[2] = {0, 0};
};

double *init_array(int m, int n, bool fill)
{
	double *mat = (double *)calloc(m * n, sizeof(double));
	if (mat == NULL)
	{
		perror("Error in init_array");
		return NULL;
	}

	if (!fill)
		return mat;

	for (int i = 0; i < m; i++)
	{
		for (int j = 0; j < n; j++)
		{
			mat[i * n + j] = i + j + 1;
		}
	}

	return mat;
}

void print_time_diff(SYSTEMTIME ti, SYSTEMTIME tf)
{
	cout << "Time: "
		 << fixed << setw(3) << setprecision(3)
		 << (double)(tf - ti) / CLOCKS_PER_SEC
		 << " seconds\n";
}

void print_first_elems(double *mat, int n)
{
	for (int j = 0; j < min(n, 10); j++)
		cout << mat[j] << " ";
	cout << endl;
}

template<typename Function>
Statistics measure_exec(Function function, int m, int n, int p)
{
	double *mat_a = init_array(m, p, true);
	double *mat_b = init_array(p, n, true);
	double *mat_c = init_array(m, n, false);

	if (!mat_a || !mat_b || !mat_b)
		return {-1.0, -1.0};

	int event_set = PAPI_NULL;
	int ret;

	ret = PAPI_library_init(PAPI_VER_CURRENT);
	if (ret != PAPI_VER_CURRENT)
		std::cout << "FAIL" << endl;

	ret = PAPI_create_eventset(&event_set);
	if (ret != PAPI_OK)
		std::cout << "ERROR: create eventset" << endl;

	ret = PAPI_add_event(event_set, PAPI_L1_DCM);
	if (ret != PAPI_OK)
		std::cout << "ERROR: PAPI_L1_DCM" << endl;

	ret = PAPI_add_event(event_set, PAPI_L2_DCM);
	if (ret != PAPI_OK)
		std::cout << "ERROR: PAPI_L2_DCM" << endl;

	ret = PAPI_start(event_set);
	if (ret != PAPI_OK)
		std::cout << "ERROR: Start PAPI" << endl;

	Statistics statistics;

	SYSTEMTIME ti = clock();
	function(mat_a, mat_b, mat_c);
	SYSTEMTIME tf = clock();

	print_time_diff(ti, tf);

	cout << "Result matrix: ";
	print_first_elems(mat_c, p);

	statistics.time = (double)(tf - ti) / CLOCKS_PER_SEC;
	statistics.mflops = (2.0 * m * n * p) / (statistics.time * 1e6);

	ret = PAPI_stop(event_set, statistics.values);
	if (ret != PAPI_OK)
		std::cout << "ERROR: Stop PAPI" << endl;

	ret = PAPI_reset(event_set);
	if (ret != PAPI_OK)
		std::cout << "FAIL reset" << endl;

	ret = PAPI_remove_event(event_set, PAPI_L1_DCM);
	if (ret != PAPI_OK)
		std::cout << "FAIL remove event" << endl;

	ret = PAPI_remove_event(event_set, PAPI_L2_DCM);
	if (ret != PAPI_OK)
		std::cout << "FAIL remove event" << endl;

	ret = PAPI_destroy_eventset(&event_set);
	if (ret != PAPI_OK)
		std::cout << "FAIL destroy" << endl;

	free(mat_a);
	free(mat_b);
	free(mat_c);

	return statistics;
}

Statistics on_mult(int m, int n, int p)
{
	double temp;
	int i, j, k;

	auto exec_mult = [&](double *mat_a, double *mat_b, double *mat_c)
	{
		for (i = 0; i < m; i++)
		{
			for (j = 0; j < p; j++)
			{
				temp = 0;
				for (k = 0; k < n; k++)
					temp += mat_a[i * n + k] * mat_b[k * p + j];
				mat_c[i * p + j] = temp;
			}
		}
	};

	return measure_exec(exec_mult, m, n, p);
}

Statistics on_mult_line(int m, int n, int p)
{
	int i, j, k;

	auto exec_mult = [&](double *mat_a, double *mat_b, double *mat_c)
	{
		for (i = 0; i < m; i++)
		{
			for (k = 0; k < n; k++)
			{
				for (j = 0; j < p; j++)
					mat_c[i * p + j] += mat_a[i * n + k] * mat_b[k * p + j];
			}
		}
	};

	return measure_exec(exec_mult, m, n, p);
}

Statistics on_mult_block(int m, int n, int p, int block_size)
{
	int I, J, K, i, j, k;

	auto exec_mult = [&](double *mat_a, double *mat_b, double *mat_c)
	{
		for (I = 0; I < m; I += block_size)
		{
			for (K = 0; K < n; K += block_size)
			{
				for (J = 0; J < p; J += block_size)
				{
					for (i = I; i < min(I + block_size, m); i++)
					{
						for (k = K; k < min(K + block_size, n); k++)
						{
							for (j = J; j < min(J + block_size, p); j++)
							{
								mat_c[i * p + j] += mat_a[i * n + k] * mat_b[k * p + j];
							}
						}
					}
				}
			}
		}
	};

	return measure_exec(exec_mult, m, n, p);
}

void print_usage(const string &program_name)
{
	std::cout << "Usage: " << program_name << " <op> <size> <output> [block-size]" << endl
			  << "  <op>         : Operation mode: 1, 2, 3 (required)" << endl
			  << "  <size>       : Size of matrix (required)" << endl
			  << "  <output>     : Output filename (required)" << endl
			  << "  [block-size] : Size of a block (optional)" << endl;
}

std::ofstream create_file(const string &file_name)
{
	bool fileExists = std::filesystem::exists(file_name);
	std::ofstream file(file_name, std::ios::out | std::ios::app);

	if (!fileExists)
		file << "OPERATION_MODE,SIZE,BLOCK_SIZE,TIME,L1 DCM,L2 DCM,MFLOPS" << std::endl;

	return file;
}

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	int op = std::atoi(argv[1]);
	if ((op != 3 && argc != 4) || (op == 3 && argc != 5))
	{
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	int size = std::atoi(argv[2]);
	std::ofstream file = create_file(argv[3]);
	int block_size = op == 3 ? std::atoi(argv[4]) : 0;
	Statistics statistics;

	switch (op)
	{
	case 1:
		statistics = on_mult(size, size, size);
		break;
	case 2:
		statistics = on_mult_line(size, size, size);
		break;
	case 3:
		statistics = on_mult_block(size, size, size, block_size);
		break;
	default:
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	file << op << ','
		 << size << ','
		 << block_size << ','
		 << statistics.time << ','
		 << statistics.values[0] << ','
		 << statistics.values[1] << ','
		 << statistics.mflops
		 << endl;

	file.close();

	return 0;
}
