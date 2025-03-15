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
#include <omp.h>

using namespace std;

#ifdef L3
#define NUM_PAPI_EVENTS 3
#else
#define NUM_PAPI_EVENTS 2
#endif

struct Statistics
{
	double time = 0.0;
	double mflops = 0.0;
	long long values[NUM_PAPI_EVENTS] = {0};
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

void print_time_diff(double ti, double tf)
{
	cout << "Time: "
		 << fixed << setw(3) << setprecision(3)
		 << tf - ti
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

	#ifdef L3
	ret = PAPI_add_event(event_set, PAPI_L3_TCM);
	if (ret != PAPI_OK)
		cout << "ERROR: PAPI_L3_TCM" << endl;
	#endif

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

	#ifdef L3
	ret = PAPI_remove_event(event_set, PAPI_L3_TCM);
	if (ret != PAPI_OK)
		cout << "FAIL remove event" << endl;
	#endif

	ret = PAPI_destroy_eventset(&event_set);
	if (ret != PAPI_OK)
		cout << "FAIL destroy" << endl;
}

void on_mult(int m, int n, int p, int event_set, Statistics &stats)
{
	int ret;
	double ti, tf;
	double temp;
	int i, j, k;

	double *mat_a = init_array(m, n, true);
	double *mat_b = init_array(n, p, true);
	double *mat_c = init_array(m, p, false);

	if (!mat_a || !mat_b || !mat_b)
	{
		stats.time = -1.0;
		stats.mflops = -1.0;
		memset(stats.values, 0, sizeof(stats.values));
	}

	ret = PAPI_start(event_set);
	if (ret != PAPI_OK)
		cout << "ERROR: Start PAPI" << endl;

	ti = omp_get_wtime();

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

	tf = omp_get_wtime();

	ret = PAPI_stop(event_set, stats.values);
	if (ret != PAPI_OK)
		cout << "ERROR: Stop PAPI" << endl;

	print_time_diff(ti, tf);

	cout << "Result matrix: ";
	print_first_elems(mat_c, p);

	stats.time = tf - ti;
	stats.mflops = (2.0 * m * n * p) / (stats.time * 1e6);

	cout << "L1 DCM: " << stats.values[0] << '\n'
		 << "L2 DCM: " << stats.values[1] << '\n';

	#ifdef L3
	cout << "L3 TCM: " << stats.values[2] << '\n';
	#endif

	cout << flush;

	ret = PAPI_reset(event_set);
	if (ret != PAPI_OK)
		cout << "FAIL reset" << endl;

	free(mat_a);
	free(mat_b);
	free(mat_c);
}

void on_mult_line(int m, int n, int p, int event_set, Statistics &stats)
{
	int ret;
	double ti, tf;
	int i, j, k;

	double *mat_a = init_array(m, n, true);
	double *mat_b = init_array(n, p, true);
	double *mat_c = init_array(m, p, false);

	if (!mat_a || !mat_b || !mat_b)
	{
		stats.time = -1.0;
		stats.mflops = -1.0;
		memset(stats.values, 0, sizeof(stats.values));
	}

	ret = PAPI_start(event_set);
	if (ret != PAPI_OK)
		cout << "ERROR: Start PAPI" << endl;

	ti = omp_get_wtime();

	for (i = 0; i < m; i++)
	{
		for (k = 0; k < n; k++)
		{
			for (j = 0; j < p; j++)
				mat_c[i * p + j] += mat_a[i * n + k] * mat_b[k * p + j];
		}
	}

	tf = omp_get_wtime();

	ret = PAPI_stop(event_set, stats.values);
	if (ret != PAPI_OK)
		cout << "ERROR: Stop PAPI" << endl;

	print_time_diff(ti, tf);

	cout << "Result matrix: ";
	print_first_elems(mat_c, p);

	stats.time = tf - ti;
	stats.mflops = (2.0 * m * n * p) / (stats.time * 1e6);

	cout << "L1 DCM: " << stats.values[0] << '\n'
		 << "L2 DCM: " << stats.values[1] << '\n';

	#ifdef L3
	cout << "L3 TCM: " << stats.values[2] << '\n';
	#endif

	cout << flush;

	ret = PAPI_reset(event_set);
	if (ret != PAPI_OK)
		cout << "FAIL reset" << endl;

	free(mat_a);
	free(mat_b);
	free(mat_c);
}

void on_mult_block(int m, int n, int p, int block_size, int event_set, Statistics &stats)
{
	int ret;
	double ti, tf;
	int I, J, K, I_end, J_end, K_end, i, j, k;

	double *mat_a = init_array(m, n, true);
	double *mat_b = init_array(n, p, true);
	double *mat_c = init_array(m, p, false);

	if (!mat_a || !mat_b || !mat_b)
	{
		stats.time = -1.0;
		stats.mflops = -1.0;
		memset(stats.values, 0, sizeof(stats.values));
	}

	ret = PAPI_start(event_set);
	if (ret != PAPI_OK)
		cout << "ERROR: Start PAPI" << endl;

	ti = omp_get_wtime();

	for (I = 0; I < m; I += block_size)
	{
		I_end = min(I + block_size, m);
		for (K = 0; K < n; K += block_size)
		{
			K_end = min(K + block_size, n);
			for (J = 0; J < p; J += block_size)
			{
				J_end = min(J + block_size, p);
				for (i = I; i < I_end; i++)
				{
					for (k = K; k < K_end; k++)
					{
						for (j = J; j < J_end; j++)
						{
							mat_c[i * p + j] += mat_a[i * n + k] * mat_b[k * p + j];
						}
					}
				}
			}
		}
	}

	tf = omp_get_wtime();

	ret = PAPI_stop(event_set, stats.values);
	if (ret != PAPI_OK)
		cout << "ERROR: Stop PAPI" << endl;

	print_time_diff(ti, tf);

	cout << "Result matrix: ";
	print_first_elems(mat_c, p);

	stats.time = tf - ti;
	stats.mflops = (2.0 * m * n * p) / (stats.time * 1e6);

	cout << "L1 DCM: " << stats.values[0] << '\n'
		 << "L2 DCM: " << stats.values[1] << '\n';

	#ifdef L3
	cout << "L3 TCM: " << stats.values[2] << '\n';
	#endif

	cout << flush;

	ret = PAPI_reset(event_set);
	if (ret != PAPI_OK)
		cout << "FAIL reset" << endl;

	free(mat_a);
	free(mat_b);
	free(mat_c);
}


void on_mult_line_parallel_1(int m, int n, int p, int event_set, Statistics &stats)
{
	int ret;
	double ti, tf;
	int i, j, k;

	double *mat_a = init_array(m, n, true);
	double *mat_b = init_array(n, p, true);
	double *mat_c = init_array(m, p, false);

	if (!mat_a || !mat_b || !mat_b)
	{
		stats.time = -1.0;
		stats.mflops = -1.0;
		memset(stats.values, 0, sizeof(stats.values));
	}

	ret = PAPI_start(event_set);
	if (ret != PAPI_OK)
		cout << "ERROR: Start PAPI" << endl;

	ti = omp_get_wtime();

	#pragma omp parallel for private(i, k, j)
	for (i = 0; i < m; i++)
	{
		for (k = 0; k < n; k++)
		{
			for (j = 0; j < p; j++)
				mat_c[i * p + j] += mat_a[i * n + k] * mat_b[k * p + j];
		}
	}

	tf = omp_get_wtime();

	ret = PAPI_stop(event_set, stats.values);
	if (ret != PAPI_OK)
		cout << "ERROR: Stop PAPI" << endl;

	print_time_diff(ti, tf);

	cout << "Result matrix: ";
	print_first_elems(mat_c, p);

	stats.time = tf - ti;
	stats.mflops = (2.0 * m * n * p) / (stats.time * 1e6);

	cout << "L1 DCM: " << stats.values[0] << '\n'
		 << "L2 DCM: " << stats.values[1] << '\n';

	#ifdef L3
	cout << "L3 TCM: " << stats.values[2] << '\n';
	#endif

	cout << flush;

	ret = PAPI_reset(event_set);
	if (ret != PAPI_OK)
		cout << "FAIL reset" << endl;

	free(mat_a);
	free(mat_b);
	free(mat_c);
}

void on_mult_line_parallel_2(int m, int n, int p, int event_set, Statistics &stats)
{
	int ret;
	double ti, tf;
	int i, j, k;

	double *mat_a = init_array(m, n, true);
	double *mat_b = init_array(n, p, true);
	double *mat_c = init_array(m, p, false);

	if (!mat_a || !mat_b || !mat_b)
	{
		stats.time = -1.0;
		stats.mflops = -1.0;
		memset(stats.values, 0, sizeof(stats.values));
	}

	ret = PAPI_start(event_set);
	if (ret != PAPI_OK)
		cout << "ERROR: Start PAPI" << endl;

	ti = omp_get_wtime();

	#pragma omp parallel private(i, k)
	for (i = 0; i < m; i++)
	{
		for (k = 0; k < n; k++)
		{
			#pragma omp for private(j)
			for (j = 0; j < p; j++)
				mat_c[i * p + j] += mat_a[i * n + k] * mat_b[k * p + j];
		}
	}

	tf = omp_get_wtime();

	ret = PAPI_stop(event_set, stats.values);
	if (ret != PAPI_OK)
		cout << "ERROR: Stop PAPI" << endl;

	print_time_diff(ti, tf);

	cout << "Result matrix: ";
	print_first_elems(mat_c, p);

	stats.time = tf - ti;
	stats.mflops = (2.0 * m * n * p) / (stats.time * 1e6);

	cout << "L1 DCM: " << stats.values[0] << '\n'
		 << "L2 DCM: " << stats.values[1] << '\n';

	#ifdef L3
	cout << "L3 TCM: " << stats.values[2] << '\n';
	#endif

	cout << flush;

	ret = PAPI_reset(event_set);
	if (ret != PAPI_OK)
		cout << "FAIL reset" << endl;

	free(mat_a);
	free(mat_b);
	free(mat_c);
}

void print_usage(const string &program_name)
{
	cout << "Usage: " << program_name << " <output-file> [(<op> <m> <n> <p> [<block-size>])]\n"
		 << "  <output-file> : Output filename\n"
		 << "  <op>          : Operation mode: 1, 2, 3, 4, 5\n"
		 << "  <m>           : Number of rows in matrix A\n"
		 << "  <n>           : Number of columns in matrix A = number of rows in matrix B\n"
		 << "  <p>           : Number of columns in matrix B\n"
		 << "  <size>        : Size of matrix\n"
		 << "  <block-size>  : Size of a block" << endl;
}

ofstream create_file(const string &file_name)
{
	bool file_exists = ifstream(file_name).good();
	ofstream file(file_name, ios::out | ios::app);

	if (!file_exists) {
		#ifdef L3
			file << "OPERATION_MODE,SIZE,M,N,P,TIME,L1 DCM,L2 DCM,L3 TCM,MFLOPS" << endl;
		#else
			file << "OPERATION_MODE,SIZE,M,N,P,TIME,L1 DCM,L2 DCM,MFLOPS" << endl;
		#endif
	}

	return file;
}

template <typename T>
int safe_get_cin(T &var) {
	if (cin >> var)
		return 0;

	if (cin.eof())
		exit(EXIT_SUCCESS);

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

	int op, m, n, p, block_size = 0;
	int event_set = setup_papi();
	Statistics stats;

	if (argc >= 3) {
		char *endptr;

		op = strtol(argv[2], &endptr, 10);
		if (endptr == argv[2] || op < 0 || op > 5)
		{
			cout << "Invalid operation" << endl;
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}

		if ((op != 3 && argc != 6) || (op == 3 && argc != 7))
		{
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}

		m = strtol(argv[3], &endptr, 10);
		if (endptr == argv[3] || m <= 0)
		{
			cout << "Invalid value for m" << endl;
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}

		n = strtol(argv[4], &endptr, 10);
		if (endptr == argv[4] || n <= 0)
		{
			cout << "Invalid value for n" << endl;
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}

		p = strtol(argv[5], &endptr, 10);
		if (endptr == argv[5] || p <= 0)
		{
			cout << "Invalid value for p" << endl;
			print_usage(argv[0]);
			exit(EXIT_FAILURE);
		}

		if (op == 3)
		{
			block_size = strtol(argv[6], &endptr, 10);
			if (endptr == argv[6] || block_size <= 0)
			{
				cout << "Invalid block size" << endl;
				print_usage(argv[0]);
				exit(EXIT_FAILURE);
			}
		}
		else
		{
			block_size = 0;
		}

		switch (op)
		{
			case 1:
				on_mult(m, n, p, event_set, stats);
				break;
			case 2:
				on_mult_line(m, n, p, event_set, stats);
				break;
			case 3:
				on_mult_block(m, n, p, block_size, event_set, stats);
				break;
			case 4:
				on_mult_line_parallel_1(m, n, p, event_set, stats);
				break;
			case 5:
				on_mult_line_parallel_2(m, n, p, event_set, stats);
				break;
			default:
				return 1;
		}

		file << op << ','
			 << m << ','
			 << n << ','
			 << p << ','
			 << block_size << ','
			 << stats.time << ','
			 << stats.values[0] << ','
			 << stats.values[1] << ',';

		#ifdef L3
		file << stats.values[2] << ',';
		#endif

		file << stats.mflops
			 << endl;

	} else {

		while (true) {
			cout << "\n1. Multiplication\n"
				<< "2. Line Multiplication\n"
				<< "3. Block Multiplication\n"
				<< "4. Line Multiplication Parallel 1\n"
				<< "5. Line Multiplication Parallel 2\n"
				<< "0. Exit\n"
				<< "Operation ? " << flush;

			if (safe_get_cin(op) != 0 || op < 0 || op > 5)
			{
				cout << "Invalid operation" << endl;
				continue;
			}
			if (op == 0)
				break;

			cout << "Number of rows in matrix A ? ";
			if (safe_get_cin(m) != 0 || m <= 0)
			{
				cout << "Invalid size" << endl;
				continue;
			}

			cout << "Number of columns in matrix A = number of rows in matrix B ? ";
			if (safe_get_cin(n) != 0 || n <= 0)
			{
				cout << "Invalid size" << endl;
				continue;
			}

			cout << "Number of columns in matrix B ? ";
			if (safe_get_cin(p) != 0 || p <= 0)
			{
				cout << "Invalid size" << endl;
				continue;
			}

			if (op == 3)
			{
				cout << "Block size ? ";
				if (safe_get_cin(block_size) != 0 || block_size <= 0)
				{
					cout << "Invalid block size" << endl;
					continue;
				}
			}
			else
			{
				block_size = 0;
			}

			switch (op)
			{
				case 1:
					on_mult(m, n, p, event_set, stats);
					break;
				case 2:
					on_mult_line(m, n, p, event_set, stats);
					break;
				case 3:
					on_mult_block(m, n, p, block_size, event_set, stats);
					break;
				case 4:
					on_mult_line_parallel_1(m, n, p, event_set, stats);
					break;
				case 5:
					on_mult_line_parallel_2(m, n, p, event_set, stats);
					break;
				default:
					return 1;
			}

			file << op << ','
				 << m << ','
				 << n << ','
				 << p << ','
				 << block_size << ','
				 << stats.time << ','
				 << stats.values[0] << ','
				 << stats.values[1] << ',';

			#ifdef L3
			file << stats.values[2] << ',';
			#endif

			file << stats.mflops
				 << endl;
		}
	}

	cleanup_papi(event_set);

	return 0;
}
