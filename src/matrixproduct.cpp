#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <time.h>
#include <cstdlib>
#include <papi.h>
#include <filesystem>

using namespace std;

#define SYSTEMTIME clock_t

// Initialize matrix
// Print first elements

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
			mat[i * n + j] = i + j + 1; // TODO(Process-ing): Check if value matters
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
void print_first_elems(double *mat, int n, int m)
{
	for (int j = 0; j < min(m, 10); j++)
		cout << mat[j] << " ";
	cout << endl;
}

double OnMult(int m_ar, int m_br, int m_cr)
{
	SYSTEMTIME Time1, Time2;

	double temp;
	int i, j, k;

	double *pha, *phb, *phc;

	// TODO(mm): Move to main? PAPI will start count here.
	pha = init_array(m_ar, m_br, true);
	phb = init_array(m_br, m_cr, true);
	phc = init_array(m_ar, m_cr, false);

	if (pha == NULL || phb == NULL || phc == NULL)
		return -1.0;

	Time1 = clock();

	for (i = 0; i < m_ar; i++)
	{
		for (j = 0; j < m_cr; j++)
		{
			temp = 0;
			for (k = 0; k < m_br; k++)
				temp += pha[i * m_br + k] * phb[k * m_cr + j];
			phc[i * m_cr + j] = temp;
		}
	}

	Time2 = clock();
	print_time_diff(Time1, Time2);

	// TODO(Process-ing): Is this necessary?
	cout << "Result matrix: " << endl;
	print_first_elems(phc, m_ar, m_cr);

	free(pha);
	free(phb);
	free(phc);

	return (double)(Time2 - Time1) * 1000 / CLOCKS_PER_SEC; // in milliseconds
}

double OnMultLine(int m_ar, int m_br, int m_cr)
{
	SYSTEMTIME Time1, Time2;

	double temp;
	int i, j, k;

	double *pha, *phb, *phc;

	pha = init_array(m_ar, m_br, true);
	phb = init_array(m_br, m_cr, true);
	phc = init_array(m_ar, m_cr, false);

	if (pha == NULL || phb == NULL || phc == NULL)
		return -1.0;

	Time1 = clock();

	for (i = 0; i < m_ar; i++)
	{
		for (k = 0; k < m_br; k++)
		{
			// TODO (Process-ing): Remove temporary variable (maybe?)
			for (j = 0; j < m_cr; j++)
				phc[i * m_cr + j] += pha[i * m_br + k] * phb[k * m_cr + j];
		}
	}

	Time2 = clock();
	print_time_diff(Time1, Time2);

	// TODO(Process-ing): Is this necessary?
	cout << "Result matrix: " << endl;
	print_first_elems(phc, m_ar, m_cr);

	free(pha);
	free(phb);
	free(phc);

	return (double)(Time2 - Time1) * 1000 / CLOCKS_PER_SEC; // in milliseconds
}

double OnMultBlock(int m_ar, int m_br, int m_cr, int bkSize)
{
	SYSTEMTIME Time1, Time2;

	double temp;
	int I, J, K, i, j, k;

	if (m_ar % bkSize != 0 || m_br % bkSize != 0 || m_cr % bkSize != 0)
		return -1.0;

	double *pha, *phb, *phc;

	pha = init_array(m_ar, m_br, true);
	phb = init_array(m_br, m_cr, true);
	phc = init_array(m_ar, m_cr, false);

	if (pha == NULL || phb == NULL || phc == NULL)
		return -1.0;

	Time1 = clock();

	// TODO(mm): use for loops from OmMultLine?
	for (I = 0; I < m_ar; I += bkSize)
	{
		for (J = 0; J < m_cr; J += bkSize)
		{
			for (K = 0; K < m_br; K += bkSize)
			{
				for (i = I; i < I + bkSize; i++)
				{
					for (j = J; j < J + bkSize; j++)
					{
						temp = 0;
						for (k = K; k < K + bkSize; k++)
						{
							temp += pha[i * m_br + k] * phb[k * m_cr + j];
						}
						phc[i * m_cr + j] += temp;
					}
				}
			}
		}
	}

	// But It's Honest Work
	// int a_bk = m_ar / bkSize, b_bk = m_br / bkSize, c_bk = m_cr / bkSize;
	// for (int a = 0; a < a_bk; a++)
	// {
	// 	for (int b = 0; b < b_bk; b++)
	// 	{
	// 		for (int c = 0; c < c_bk; c++)
	// 		{
	// 			for (i = 0; i < bkSize; i++)
	// 			{
	// 				for (j = 0; j < bkSize; j++)
	// 				{
	// 					temp = 0;
	// 					for (k = 0; k < bkSize; k++)
	// 						temp += pha[(a * bkSize + i) * m_br + c * bkSize + k] * phb[(k + c * bkSize) * m_cr + b * bkSize + j];
	// 					phc[(i + a * bkSize) * m_cr + b * bkSize + j] += temp;
	// 				}
	// 			}
	// 		}
	// 	}
	// }

	Time2 = clock();
	print_time_diff(Time1, Time2);

	cout << "Result matrix: " << endl;
	print_first_elems(phc, m_ar, m_cr);

	free(pha);
	free(phb);
	free(phc);

	return (double)(Time2 - Time1) * 1000 / CLOCKS_PER_SEC; // in milliseconds
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
	namespace fs = std::filesystem;

	if (fs::exists(fileName))
	{
		std::ofstream file(fileName, std::ios::out | std::ios::app);
		return file;
	}

	fs::path p(fileName);

	if (!p.parent_path().empty())
	{
		std::error_code ec;
		fs::create_directories(p.parent_path(), ec);
	}

	std::ofstream file(fileName, std::ios::out | std::ios::app);
	file << "OPERATION_MODE,LIN,COL,BLOCK_SIZE,TIME,L1 DCM,L2 DCM" << endl;
	return file;
}

int main(int argc, char *argv[])
{
	if (argc < 5 || argc > 6)
	{
		printUsage(argv[0]);
		exit(EXIT_FAILURE);
	}

	int op = std::atoi(argv[1]);
	// TODO(mm): matrix sizes?
	int lin = std::atoi(argv[2]);
	int col = std::atoi(argv[3]);
	int blockSize = op == 3 ? std::atoi(argv[5]) : 0;
	double time = 0.0;
	std::ofstream file = createFile(argv[4]);

	int EventSet = PAPI_NULL;
	long long values[2];
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

	switch (op)
	{
	case 1:
		time = OnMult(lin, col, lin);
		break;
	case 2:
		time = OnMultLine(lin, col, lin);
		break;
	case 3:
		time = OnMultBlock(lin, col, lin, blockSize);
		// OnMultBlock(lin, col, blockSize);
		break;
	default:
		printUsage(argv[0]);
		exit(EXIT_FAILURE);
	}

	ret = PAPI_stop(EventSet, values);
	if (ret != PAPI_OK)
		std::cout << "ERROR: Stop PAPI" << endl;

	file << op << ','
		 << lin << ','
		 << col << ','
		 << blockSize << ','
		 << time << ','
		 << values[0] << ','
		 << values[1] << endl;

	file.close();
	printf("L1 DCM: %lld \n", values[0]);
	printf("L2 DCM: %lld \n", values[1]);

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

	return 0;
}
