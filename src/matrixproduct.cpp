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

// Initialize matrix
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

// Displays 10 elements of the result matrix to verify correctness
void print_first_elems(double *mat)
{
	for (int j = 0; j < 10; j++)
		cout << mat[j] << " ";
	cout << endl;
}

template <typename Function>
double timeFunc(Function function, long long *values, int m, int n, int p)
{
	double *mat_A = init_array(m, p, true);
	double *mat_B = init_array(p, n, true);
	double *mat_C = init_array(m, n, false);
	if (!mat_A || !mat_B || !mat_C)
		return -1.0;

	int EventSet = PAPI_NULL;
	int ret;

	ret = PAPI_library_init(PAPI_VER_CURRENT);
	if (ret != PAPI_VER_CURRENT)
		std::cout << "FAIL" << endl;

	ret = PAPI_create_eventset(&EventSet);
	if (ret != PAPI_OK)
		std::cout << "ERROR: create eventset" << endl;

	ret = PAPI_add_event(EventSet, PAPI_L1_DCM);
	if (ret != PAPI_OK)
		std::cout << "ERROR: PAPI_L1_DCM" << endl;

	ret = PAPI_add_event(EventSet, PAPI_L2_DCM);
	if (ret != PAPI_OK)
		std::cout << "ERROR: PAPI_L2_DCM" << endl;

	ret = PAPI_start(EventSet);
	if (ret != PAPI_OK)
		std::cout << "ERROR: Start PAPI" << endl;

	SYSTEMTIME Time1 = clock();
	function(mat_A, mat_B, mat_C);
	SYSTEMTIME Time2 = clock();

	print_time_diff(Time1, Time2);

	print_first_elems(mat_C);

	ret = PAPI_stop(EventSet, values);
	if (ret != PAPI_OK)
		std::cout << "ERROR: Stop PAPI" << endl;

	ret = PAPI_reset(EventSet);
	if (ret != PAPI_OK)
		std::cout << "FAIL reset" << endl;

	ret = PAPI_remove_event(EventSet, PAPI_L1_DCM);
	if (ret != PAPI_OK)
		std::cout << "FAIL remove event" << endl;

	ret = PAPI_remove_event(EventSet, PAPI_L2_DCM);
	if (ret != PAPI_OK)
		std::cout << "FAIL remove event" << endl;

	ret = PAPI_destroy_eventset(&EventSet);
	if (ret != PAPI_OK)
		std::cout << "FAIL destroy" << endl;

	free(mat_A);
	free(mat_B);
	free(mat_C);

	return (double)(Time2 - Time1) * 1000 / CLOCKS_PER_SEC;
}

double OnMult(int m, int n, int p, long long *values)
{
	double temp;
	int i, j, k;

	auto execMult = [&](double *mat_A, double *mat_B, double *mat_C)
	{
		for (i = 0; i < m; i++)
		{
			for (j = 0; j < p; j++)
			{
				temp = 0;
				for (k = 0; k < n; k++)
					temp += mat_A[i * n + k] * mat_B[k * p + j];
				mat_C[i * p + j] = temp;
			}
		}
	};

	return timeFunc(execMult, values, m, n, p);
}

double OnMultLine(int m, int n, int p, long long *values)
{
	int i, j, k;

	auto execMult = [&](double *mat_A, double *mat_B, double *mat_C)
	{
		for (i = 0; i < m; i++)
		{
			for (k = 0; k < n; k++)
			{
				for (j = 0; j < p; j++)
					mat_C[i * p + j] += mat_A[i * n + k] * mat_B[k * p + j];
			}
		}
	};

	return timeFunc(execMult, values, m, n, p);
}

double OnMultBlock(int m, int n, int p, int bkSize, long long *values)
{
	int row, col, i, k, j;

	int endRow = ceil((double)m / bkSize);
	int endCol = ceil((double)p / bkSize);

	auto execMult = [&](double *mat_A, double *mat_B, double *mat_C)
	{
		for (row = 0; row < endRow; ++row)
		{
			for (col = 0; col < endCol; ++col)
			{
				for (i = row * bkSize; i < min((row + 1) * bkSize, m); ++i)
				{
					for (k = 0; k <= n - 1; ++k)
					{
						for (j = col * bkSize; j < min((col + 1) * bkSize, p); ++j)
						{
							mat_C[i * p + j] += mat_A[i * n + k] * mat_B[k * p + j];
						}
					}
				}
			}
		}
	};

	return timeFunc(execMult, values, m, n, p);
}

void handle_error(int retval)
{
	printf("PAPI error %d: %s\n", retval, PAPI_strerror(retval));
	exit(1);
}

void init_papi()
{
	int retval = PAPI_library_init(PAPI_VER_CURRENT);
	if (retval != PAPI_VER_CURRENT && retval < 0)
	{
		printf("PAPI library version mismatch!\n");
		exit(1);
	}
	if (retval < 0)
		handle_error(retval);

	std::cout << "PAPI Version Number: MAJOR: " << PAPI_VERSION_MAJOR(retval)
			  << " MINOR: " << PAPI_VERSION_MINOR(retval)
			  << " REVISION: " << PAPI_VERSION_REVISION(retval) << "\n";
}

void printUsage(const string &programmName)
{
	std::cout << "Usage: " << programmName << " <op> <lin> <col> <output> [blockSize]" << endl
			  << "  <op> 		: Opration mode: 1, 2, 3 (required)" << endl
			  << "  <lin>       : Number of lines (required)" << endl
			  << "  <col>       : Number of columns (required)" << endl
			  << "  <output>    : Path to output filename (required)" << endl
			  << "  [blockSize] : Size of a block (optional)" << endl;
}

std::ofstream createFile(const string &fileName)
{
	std::ofstream file(fileName, std::ios::out | std::ios::app);
	
	if (!std::filesystem::exists(fileName))
		file << "OPERATION_MODE,SIZE,BLOCK_SIZE,TIME,L1 DCM,L2 DCM" << endl;

	return file;
}

int main(int argc, char *argv[])
{
	if (argc < 4 || argc > 5)
	{
		printUsage(argv[0]);
		exit(EXIT_FAILURE);
	}

	int op = std::atoi(argv[1]);
	int size = std::atoi(argv[2]);
	std::ofstream file = createFile(argv[3]);
	int blockSize = op == 3 ? std::atoi(argv[4]) : 0;
	double time = 0.0;
	long long values[2];

	switch (op)
	{
	case 1:
		time = OnMult(size, size, size, values);
		break;
	case 2:
		time = OnMultLine(size, size, size, values);
		break;
	case 3:
		time = OnMultBlock(size, size, size, blockSize, values);
		break;
	default:
		printUsage(argv[0]);
		exit(EXIT_FAILURE);
	}

	file << op << ','
		 << size << ','
		 << blockSize << ','
		 << time << ','
		 << values[0] << ','
		 << values[1]
		 << endl;

	file.close();

	return 0;
}
