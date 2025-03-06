#include <stdio.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <cstdlib>
#include <papi.h>
#include <cstring>
#include <limits>

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
	double *mat = (double *)malloc(m * n * sizeof(double));
	if (mat == NULL)
	{
		perror("Error in init_array");
		return NULL;
	}

	if (!fill)
	{
		memset(mat, 0, m * n * sizeof(double));
		return mat;
	}

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

int setup_papi()
{
	int event_set = PAPI_NULL;
	int ret;

	ret = PAPI_library_init(PAPI_VER_CURRENT);
	if (ret != PAPI_VER_CURRENT)
		cout << "FAIL" << endl;

	ret = PAPI_create_eventset(&event_set);
	if (ret != PAPI_OK)
		cout << "ERROR: create eventset" << endl;

	ret = PAPI_add_event(event_set, PAPI_L1_DCM);
	if (ret != PAPI_OK)
		cout << "ERROR: PAPI_L1_DCM" << endl;

	ret = PAPI_add_event(event_set, PAPI_L2_DCM);
	if (ret != PAPI_OK)
		cout << "ERROR: PAPI_L2_DCM" << endl;

	return event_set;
}

void cleanup_papi(int &event_set)
{
	int ret;
	ret = PAPI_remove_event(event_set, PAPI_L1_DCM);
	if (ret != PAPI_OK)
		cout << "FAIL remove event" << endl;

	ret = PAPI_remove_event(event_set, PAPI_L2_DCM);
	if (ret != PAPI_OK)
		cout << "FAIL remove event" << endl;

	ret = PAPI_destroy_eventset(&event_set);
	if (ret != PAPI_OK)
		cout << "FAIL destroy" << endl;
}

template <typename Function>
void measure_exec(Function function, int m, int n, int p, int event_set, Statistics &stats)
{
	int ret;

	double *mat_a = init_array(m, p, true);
	double *mat_b = init_array(p, n, true);
	double *mat_c = init_array(m, n, false);

	if (!mat_a || !mat_b || !mat_b)
	{
		stats.time = -1.0;
		stats.mflops = -1.0;
		memset(stats.values, 0, sizeof(stats.values));
	}

	ret = PAPI_start(event_set);
	if (ret != PAPI_OK)
		cout << "ERROR: Start PAPI" << endl;

	SYSTEMTIME ti = clock();
	function(mat_a, mat_b, mat_c);
	SYSTEMTIME tf = clock();

	ret = PAPI_stop(event_set, stats.values);
	if (ret != PAPI_OK)
		cout << "ERROR: Stop PAPI" << endl;

	print_time_diff(ti, tf);

	cout << "Result matrix: ";
	print_first_elems(mat_c, p);

	stats.time = (double)(tf - ti) / CLOCKS_PER_SEC;
	stats.mflops = (2.0 * m * n * p) / (stats.time * 1e6);

	cout << "L1 DCM: " << stats.values[0] << '\n'
		 << "L2 DCM: " << stats.values[1] << endl;

	ret = PAPI_reset(event_set);
	if (ret != PAPI_OK)
		cout << "FAIL reset" << endl;

	free(mat_a);
	free(mat_b);
	free(mat_c);
}

void on_mult(int m, int n, int p, int event_set, Statistics &stats)
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

	return measure_exec(exec_mult, m, n, p, event_set, stats);
}

void on_mult_line(int m, int n, int p, int event_set, Statistics &stats)
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

	return measure_exec(exec_mult, m, n, p, event_set, stats);
}

void on_mult_block(int m, int n, int p, int block_size, int event_set, Statistics &stats)
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

	return measure_exec(exec_mult, m, n, p, event_set, stats);
}

void print_usage(const string &program_name)
{
	cout << "Usage: " << program_name << " <output-file> [(<op> <size> [<block-size>])]\n"
		 << "  <op>          : Operation mode: 1, 2, 3 (required)\n"
		 << "  <size>        : Size of matrix (required)\n"
		 << "  <output-file> : Output filename (required)\n"
		 << "  <block-size>  : Size of a block (optional)" << endl;
}

ofstream create_file(const string &file_name)
{
	bool file_exists = ifstream(file_name).good();
	ofstream file(file_name, ios::out | ios::app);

	if (!file_exists)
		file << "OPERATION_MODE,SIZE,BLOCK_SIZE,TIME,L1 DCM,L2 DCM,MFLOPS" << endl;

	return file;
}

int execute_operation(int op, int size, int block_size, ofstream &file, int event_set)
{
	Statistics stats;

	switch (op)
	{
		case 1:
			on_mult(size, size, size, event_set, stats);
			break;
		case 2:
			on_mult_line(size, size, size, event_set, stats);
			break;
		case 3:
			on_mult_block(size, size, size, block_size, event_set, stats);
			break;
		default:
			return 1;
	}

	file << op << ','
		 << size << ','
		 << block_size << ','
		 << stats.time << ','
		 << stats.values[0] << ','
		 << stats.values[1] << ','
		 << stats.mflops
		 << endl;

	return 0;
}

template <typename T>
int safe_get_cin(T &var, const string &error_message) {
	if (cin >> var)
		return 0;

	if (cin.eof())
		exit(EXIT_SUCCESS);

	cout << error_message << endl;
	cin.clear();
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	return 1;
}

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		print_usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	ofstream file = create_file(argv[1]);
	if (!file.is_open())
	{
		cout << "Error opening file";
		exit(EXIT_FAILURE);
	}

	int op, size, block_size;
	int event_set = setup_papi();

	if (argc >= 3) {
		char *endptr;

		op = strtol(argv[2], &endptr, 10);
		if (endptr == argv[2] || op < 0 || op > 3)
		{
			cout << "Invalid operation" << endl;
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}

		if ((op != 3 && argc != 4) || (op == 3 && argc != 5))
		{
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}

		size = strtol(argv[3], &endptr, 10);
		if (endptr == argv[3] || size <= 0)
		{
			cout << "Invalid size" << endl;
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}

		if (op == 3)
		{
			block_size = strtol(argv[4], &endptr, 10);
			if (endptr == argv[4] || block_size <= 0)
			{
				cout << "Invalid block size" << endl;
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
			}
		}

		if (execute_operation(op, size, block_size, file, event_set) != 0)
			exit(EXIT_FAILURE);

	} else {

		while (true) {
			cout << "\n1. Multiplication\n"
				<< "2. Line Multiplication\n"
				<< "3. Block Multiplication\n"
				<< "0. Exit\n"
				<< "Operation ? " << flush;

			if (safe_get_cin(op, "Invalid operation") != 0)
				continue;
			if (op == 0)
				break;

			cout << "Matrix size ? ";
			if (safe_get_cin(size, "Invalid size") != 0)
				continue;

			if (op == 3)
			{
				cout << "Block size ? ";
				if (safe_get_cin(block_size, "Invalid block size") != 0)
					continue;
			}

			if (execute_operation(op, size, block_size, file, event_set) != 0)
				exit(EXIT_FAILURE);
		}
	}

	cleanup_papi(event_set);

	return 0;
}
