#include <arguments_mini.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// NOTE: for 11/12/13.. th (exceptions)
//       for 1,21,31,etc... st
//       for 2,22,32,etc... nd
//       for 3,23,33,etc... rd
//       otherwise.. th
#define ORDINAL_NUMBER_SUFFIX(a) \
	((a == 11) | (a == 12) | (a == 13) ? "th" : \
	(a%10) == 1 ? "st" : \
	(a%10) == 2 ? "nd" : \
	(a%10) == 3 ? "rd" : \
	"th")

struct info_about_memory_allocation_t
{
	void* a;
	int numBytesAllocated;
};
struct info_about_memory_allocation_t* infoPerMemoryAllocation = NULL;
int numMemoryAllocations = 0; //< really # tracked memory allocations (excluding memory allocations for infoPerMemoryAllocation)
size_t numBytesAllocated;

// NOTE: thanks to https://stackoverflow.com/questions/1208644/how-can-i-get-the-size-of-a-memory-block-allocated-using-malloc
void* __real_malloc(size_t a);
void __real_free(void* a);

void* __wrap_malloc(size_t a)
{
	void* b = __real_malloc(a);

	struct info_about_memory_allocation_t* c = infoPerMemoryAllocation;
	//infoPerMemoryAllocation = new info_about_memory_allocation_t[numMemoryAllocations + 1];
	infoPerMemoryAllocation = (struct info_about_memory_allocation_t*)__real_malloc(sizeof(struct info_about_memory_allocation_t) * (numMemoryAllocations + 1));

	if(c != NULL)
	{
		memcpy(infoPerMemoryAllocation, c, sizeof(struct info_about_memory_allocation_t) * numMemoryAllocations);
		__real_free(c);
	}

	if((b == NULL) | (infoPerMemoryAllocation == NULL))
	{
		fputs("error: out of memory (test will be inaccurate)\n", stdout); //< not sure if that is the only case __real_malloc would return NULL..?
		if(b == NULL)
		{
			__real_free(b);
		}
		if(infoPerMemoryAllocation == NULL)
		{
			__real_free(infoPerMemoryAllocation);
		}
		exit(1); //< i.e. continuing "like this" would not make a whole lot of sense..?
	}

	++numMemoryAllocations;
	infoPerMemoryAllocation[numMemoryAllocations - 1].a = b;
	infoPerMemoryAllocation[numMemoryAllocations - 1].numBytesAllocated = a;
	numBytesAllocated += a;

	return b;
}

void __wrap_free(void* a)
{
	int i;
	for(i = 0; i < numMemoryAllocations; ++i)
	{
		if(infoPerMemoryAllocation[i].a == a)
		{
			break;
		}
	}
	if(i == numMemoryAllocations)
	{
		fputs("warning: attempted to free memory that was never allocated\n", stdout);
		exit(1); //< i.e. continuing "like this" would not make a whole lot of sense..?
	}

	numBytesAllocated -= infoPerMemoryAllocation[i].numBytesAllocated;
	if((numMemoryAllocations - 1) > 0)
	{
		struct info_about_memory_allocation_t* b = infoPerMemoryAllocation;
		//infoPerMemoryAllocation = new info_about_memory_allocation_t[numMemoryAllocations - 1];
		infoPerMemoryAllocation = (struct info_about_memory_allocation_t*)__real_malloc(sizeof(struct info_about_memory_allocation_t) * (numMemoryAllocations - 1));

		int numElementsBefore = i;
		int numElementsAfter = numMemoryAllocations - numElementsBefore - 1;

		if(numElementsBefore > 0)
		{
			memcpy(infoPerMemoryAllocation, b, sizeof(struct info_about_memory_allocation_t) * numElementsBefore);
		}
		if(numElementsAfter > 0)
		{
			memcpy(infoPerMemoryAllocation + numElementsBefore, b + numElementsBefore + 1, sizeof(struct info_about_memory_allocation_t) * numElementsAfter);
		}

		__real_free(b);
	}
	else
	{
		__real_free(infoPerMemoryAllocation);
		infoPerMemoryAllocation = NULL;
	}
	--numMemoryAllocations;

	__real_free(a);
}

//*********************************** tests ***********************************

// NOTE: test 1 tests..
//       .. if argv[0]==argument is ignored
//       .. if two arguments, green and blue, get passed to..
//          .. on_argument_parsed in correct order
//       .. if am_parse returns 1 w. argv == { "red", "green", "blue" }
int test_1_on_argument_parsed_called;
int test_1_argv0_passed; //< "== test_1_argumentred_passed"
int test_1_argumentgreen_passed;
int test_1_argumentblue_passed;
int test_1_order_is_correct;
int test_1_unintended_argument;

int test_1_on_argument_parsed(char* argument)
{
	test_1_on_argument_parsed_called = 1;
	if(strcmp(argument, "red") == 0)
	{
		fputs("error: argv[0] was passed to on_argument_parsed\n", stdout);
		test_1_argv0_passed = 1;
		return 0;
	}
	else if(strcmp(argument, "green") == 0)
	{
		if(test_1_argumentblue_passed == 1)
		{
			test_1_order_is_correct = 0;
			fputs("error: on_argument_parsed was called more than once but in wrong order\n", stdout);
			return 0;
		}
		test_1_argumentgreen_passed = 1;
		return 1;
	}
	else if(strcmp(argument, "blue") == 0)
	{
		test_1_argumentblue_passed = 1;
		return 1;
	}
	
	printf("error: on_argument_parsed was called with an unintended argument (%s)\n", argument);
	test_1_unintended_argument = 1;
	return 0;
}

void test_1_on_print(char* a)
{
	printf("from arguments-mini... %s\n", a);
}

int test_1()
{
	test_1_on_argument_parsed_called = 0;
	test_1_argv0_passed = 0;
	test_1_argumentgreen_passed = 0;
	test_1_argumentblue_passed = 0;
	test_1_order_is_correct = 1;
	test_1_unintended_argument = 0;

	char* argv[] = {
			"red",
			"green",
			"blue"
		};
	int argc = 3;
	
	am_set_on_print(&test_1_on_print); //< should not be called
	am_set_on_argument_parsed(&test_1_on_argument_parsed);
	int a = am_parse(argc, argv, 0, 0);
	am_unset_on_argument_parsed();
	am_unset_on_print();
	
	if(test_1_on_argument_parsed_called == 0)
	{
		// also.. fputs in on_argument_parsed
		printf("error: on_argument_parsed was not called (am_parse returned %i)\n", a);
		return 0;
	}
	
	if(a == 1)
	{
		if(test_1_argv0_passed == 1 | test_1_unintended_argument == 1)
		{
			// also.. fputs in on_parameterwithoutvalue_parsed
			fputs("error: am_parse returned 1 though on_argument_parsed returned 0\n", stdout);
			return 0;
		}
		
		if(test_1_argumentgreen_passed == 0 | test_1_argumentblue_passed == 0)
		{
			// if test_1_on_argument_parsed_called != 0,..
			// .. test_1_unintended_parameter != 1 and either for green/blue..
			// .. on_argument_parsed was not called (not possible that for..
			// .. both on_argument_parsed was not called as then required..
			// .. that test_1_unintended_argument != 1 |..
			// ..test_1_on_argument_parsed_called == 0..?)
			fputs("error: on_argument_parsed was not called for all arguments\n", stdout);
			return 0;
		}
		
		if(test_1_order_is_correct == 0)
		{
			// also.. fputs in on_parameterwithoutvalue_parsed
			fputs("error: am_parse returned 1 though on_argument_parsed returned 0\n", stdout);
			return 0;
		}
		
		return 1;
	}
	
	if(test_1_argv0_passed == 1) //< argv[0] was passed to on_argument_parsed
	{
		return 0; //< fputs in on_argument_parsed
	}
	if(test_1_unintended_argument == 1) //< an argument other than "a","b" was passed to on_argument_parsed
	{
		return 0; //< fputs in on_argument_parsed
	}
	if(test_1_order_is_correct == 0)
	{
		return 0; //< fputs in on_argument_parsed
	}
	
	printf("error: unforeseen error happened (am_parse returned %i)\n", a);
	
	return 0;
}

// NOTE: test 2 tests..
//       .. if argv[0]==parameterwithoutvalue is ignored
//       .. if two parameterswithoutvalue, --apple and --pear, get passed..
//          .. to on_parameterwithoutvalue_parsed in correct order
//       .. if am_parse returns 1 w. argv == { "--banana", "--pear",..
//          .. "--apple" }
int test_2_on_parameterwithoutvalue_parsed_called;
int test_2_argv0_passed; //< "== test_2_bananaparameter_passed"
int test_2_appleparameter_passed;
int test_2_pearparameter_passed;
int test_2_order_is_correct;
int test_2_unintended_parameter;

int test_2_on_parameterwithoutvalue_parsed(char* parametername)
{
	test_2_on_parameterwithoutvalue_parsed_called = 1;
	if(strcmp(parametername, "banana") == 0)
	{
		fputs("error: argv[0] was passed to on_parameterwithoutvalue_parsed\n", stdout);
		test_2_argv0_passed = 1;
		return 0;
	}
	if(strcmp(parametername, "pear") == 0)
	{
		if(test_2_appleparameter_passed == 1)
		{
			test_2_order_is_correct = 0;
			fputs("error: on_parameterwithoutvalue_parsed was called more than once but in wrong order\n", stdout);
			return 0;
		}
		
		test_2_pearparameter_passed = 1;
		return 1;
	}
	if(strcmp(parametername, "apple") == 0)
	{
		test_2_appleparameter_passed = 1;
		return 1;
	}
	
	printf("error: on_parameterwithoutvalue_parsed was called with an unintended parametername (--%s)\n", parametername);
	test_2_unintended_parameter = 1;
	
	return 0;
}

void test_2_on_print(char* a)
{
	printf("from arguments-mini... %s\n", a);
}

int test_2()
{
	test_2_on_parameterwithoutvalue_parsed_called = 0;
	test_2_argv0_passed = 0;
	test_2_appleparameter_passed = 0;
	test_2_pearparameter_passed = 0;
	test_2_order_is_correct = 1;
	test_2_unintended_parameter = 0;

	char* argv[] = {
			// NOTE: not sure if argv[0] == "--banana" would ever happen if..
			//       .. argv isn't modified*, though testing such that if..
			//       .. argv is modified* might reduce struggle if first..
			//       .. time user tries to manually supply argv..?
			//       * or e.g. set w. a variable
			"--banana",
			"--pear",
			"--apple"
		};
	int argc = 3;

	am_set_on_print(&test_2_on_print); //< should not be called
	am_set_on_parameterwithoutvalue_parsed(&test_2_on_parameterwithoutvalue_parsed);
	int a = am_parse(argc, argv, 0, 0);
	am_unset_on_parameterwithoutvalue_parsed();
	am_unset_on_print();
	
	if(test_2_on_parameterwithoutvalue_parsed_called == 0)
	{
		printf("error: on_parameterwithoutvalue_parsed was not called (am_parse returned %i)\n", a);
		return 0;
	}
	
	if(a == 1)
	{
		if(test_2_argv0_passed == 1 | test_2_unintended_parameter == 1)
		{
			// also.. fputs in on_parameterwithoutvalue_parsed
			fputs("error: am_parse returned 1 though on_parameterwithoutvalue_parsed returned 0\n", stdout);
			return 0;
		}
		
		if(test_2_appleparameter_passed == 0 | test_2_pearparameter_passed == 0)
		{
			// if test_2_on_parameterwithoutvalue_parsed_called != 0,..
			// .. test_2_unintended_parameter != 1 and either for --apple/..
			// ..--pear on_parameterwithoutvalue_parsed was not called (not..
			// .. possible that for both on_parameterwithoutvalue_parsed..
			// .. was not called as then required that..
			// .. test_2_unintended_parameter != 1 |..
			// ..test_2_on_parameterwithoutvalue_parsed_called == 0..?)
			fputs("error: on_parameterwithoutvalue_parsed was not called for all parameters\n", stdout);
			return 0;
		}
		
		if(test_2_order_is_correct == 0)
		{
			// also.. fputs in on_parameterwithoutvalue_parsed
			fputs("error: am_parse returned 1 though on_parameterwithoutvalue_parsed returned 0\n", stdout);
			return 0;
		}
		
		return 1;
	}
	
	if(test_2_argv0_passed == 1) //< argv[0] was passed to on_parameterwithoutvalue_parsed
	{
		return 0; //< fputs in on_parameterwithoutvalue_parsed
	}
	if(test_2_unintended_parameter == 1)
	{
		return 0; //< fputs in on_parameterwithoutvalue_parsed
	}
	if(test_2_order_is_correct == 0)
	{
		return 0; //< fputs in on_parameterwithoutvalue_parsed
	}
	
	printf("error: unforeseen error happened (am_parse returned %i)\n", a);
	
	return 0;
}

// NOTE: test 3 tests..
//       .. if argv[0]==parameterwithvalue is ignored
//       .. if two parameterswithvalue, -shape=cube and -position, get..
//          .. passed to on_parameterwithvalue_parsed each with correct..
//          .. value and in correct order
//       .. if am_parse returns 1 w. argv == { "-size=1mx1mx1m",..
//          .. "-shape=cube", "-position=" }
int test_3_on_parameterwithvalue_parsed_called;
int test_3_argv0_passed;
int test_3_shapeparameter_passed;
int test_3_shapeparametervalue_is_correct;
int test_3_positionparameter_passed;
int test_3_positionparametervalue_is_correct;
int test_3_order_is_correct;
int test_3_unintended_parameter;

int test_3_on_parameterwithvalue_parsed(char* parametername, char* value)
{
	test_3_on_parameterwithvalue_parsed_called = 1;
	if(strcmp(parametername, "size") == 0)
	{
		fputs("error: argv[0] was passed to on_parameterwithvalue_parsed\n", stdout);
		test_3_argv0_passed = 1;
		return 0;
	}
	else if(strcmp(parametername, "shape") == 0)
	{
		if(test_3_positionparameter_passed == 1)
		{
			test_3_order_is_correct = 0;
			fputs("error: on_parameterwithvalue_parsed was called more than once but in wrong order\n", stdout);
			return 0;
		}
		
		test_3_shapeparameter_passed = 1;
		
		//if(value == NULL | strcmp(value, "cube") != 0)
		if(value == NULL || strcmp(value, "cube") != 0)
		{
			char a[128];
			if(value != NULL)
			{
				snprintf(a, 128, "value %s", value);
			}
			
			printf("error: on_parameterwithvalue_parsed was called for -shape= with unintended %s\n", value == NULL ? "NULL value" : a);
			return 0;
		}
		
		test_3_shapeparametervalue_is_correct = 1;
		
		return 1;
	}
	else if(strcmp(parametername, "position") == 0)
	{
		test_3_positionparameter_passed = 1;
		
		if(value != NULL)
		{
			printf("error: on_parameterwithvalue_parsed was called for -position= with unintended non NULL value (%p, should be NULL)\n", value);
			return 0;
		}
		
		test_3_positionparametervalue_is_correct = 1;
		return 1;
	}
	
	printf("error: on_parameterwithvalue_parsed was called with an unintended parametername (-%s=%s)\n", parametername, value == NULL ? "" : value);
	test_3_unintended_parameter = 1;
	
	return 0;	
}

void test_3_on_print(char* a)
{
	printf("from arguments-mini... %s\n", a);
}

int test_3()
{
	test_3_on_parameterwithvalue_parsed_called = 0;
	test_3_argv0_passed = 0;
	test_3_shapeparameter_passed = 0;
	test_3_shapeparametervalue_is_correct = 0;
	test_3_positionparameter_passed = 0;
	test_3_positionparametervalue_is_correct = 0;
	test_3_order_is_correct = 1;
	test_3_unintended_parameter = 0;
	
	char* argv[] = {
			// NOTE: not sure if argv[0] == "-size=1mx1mx1m" would ever..
			//       .. happen if argv isn't modified*, though testing such..
			//       .. that if argv is modified* might reduce struggle if..
			//       .. first time user tries to manually supply argv..?
			//       * or e.g. set w. a variable
			"-size=1mx1mx1m",
			"-shape=cube",
			"-position=" //< empty value is ok..?
		};
	int argc = 3;

	am_set_on_print(&test_3_on_print); //< should not be called
	am_set_on_parameterwithvalue_parsed(&test_3_on_parameterwithvalue_parsed);
	int a = am_parse(argc, argv, 0, 0);
	am_unset_on_parameterwithvalue_parsed();
	am_unset_on_print();
	
	if(test_3_on_parameterwithvalue_parsed_called == 0)
	{
		printf("error: on_parameterwithvalue_parsed was not called (am_parse returned %i)\n", a);
		return 0;
	}
	
	if(a == 1)
	{
		if(test_3_argv0_passed == 1 | test_3_unintended_parameter == 1)
		{
			// also.. fputs in on_parameterwithvalue
			fputs("error: am_parse returned 1 though on_parameterwithvalue_parsed returned 0\n", stdout);
			return 0;
		}
		
		if(test_3_shapeparameter_passed == 0 | test_3_positionparameter_passed == 0)
		{
			// if test_3_on_parameterwithvalue_parsed_called != 0,..
			// .. test_3_unintended_parameter != 1 and either for -shape=/..
			// ..-position= on_parameterwithvalue_parsed was not called (not..
			// .. possible that for both on_parameterwithvalue_parsed was..
			// .. not called as then required that..
			// .. test_3_unintended_parameter != 1 |..
			// .. test_3_on_parameterwithvalue_parsed_called == 0)
			fputs("error: on_parameterwithoutvalue_parsed was not called for all parameters\n", stdout);
			return 0;
		}
		
		if(test_3_shapeparametervalue_is_correct != 1 | test_3_positionparametervalue_is_correct != 1 | test_3_order_is_correct == 0)
		{
			// also.. fputs in on_parameterwithvalue_parsed
			fputs("error: am_parse returned 1 though on_parameterwithvalue_parsed returned 0\n", stdout);
			return 0;
		}
		
		return 1;
	}
	
	if(test_3_argv0_passed == 1) //< argv[0] was passed to on_parameterwithvalue_parsed
	{
		return 0; //< fputs in on_parameterwithvalue_parsed
	}
	if(test_3_unintended_parameter == 1)
	{
		return 0; //< fputs in on_parameterwithvalue_parsed
	}
	if(test_3_shapeparametervalue_is_correct != 1 | test_3_positionparametervalue_is_correct != 1 | test_3_order_is_correct == 0)
	{
		return 0; //< fputs in on_parameterwithvalue_parsed
	}
	
	printf("error: unforeseen error happened (am_parse returned %i)\n", a);
	
	return 0;
}

// TODO: test 4 tests..
//       .. if am_parse returns 0 w. argv == "test test" and bAllowDuplicateArguments == 0
//       .. if am_parse returns 1 w. argv == "test test" and bAllowDuplicateArguments == 1
//       .. if am_parse returns 0 w. argv == "--test --test" and bAllowDuplicateParameters == 0
//       .. if am_parse returns 0 w. argv == "-test=test -test=test" and bAllowDuplicateParameters == 0
//       .. if am_parse returns 0 w. argv == "-test=test --test" and bAllowDuplicateParameters == 0
//       .. if am_parse returns 1 w. argv == "--test --test" and bAllowDuplicateParameters == 1
//       .. if am_parse returns 1 w. argv == "-test=test -test=test" and bAllowDuplicateParameters == 1
//       .. if am_parse returns 1 w. argv == "-test=test --test" and bAllowDuplicateParameters == 1

//*****************************************************************************

//int main(int, char**)
int main(int unusedArgc, char** unusedArgv)
{
// NOTE: will run test_##a() multiple times to assure that it is repeatable
#define TEST(b) \
	{ \
		int a; \
		fputs("test " #b " started\n", stdout); \
		int i; \
		for(i = 0; i < 10; ++i) \
		{ \
			size_t numBytesAllocatedBeforeTest##b = numBytesAllocated; \
			a = test_##b(); \
			if(a != 1) \
			{ \
				break; \
			} \
			size_t numBytesAllocatedAfterTest##b = numBytesAllocated; \
			size_t numBytesLeakedByTest##b = numBytesAllocatedAfterTest##b - numBytesAllocatedBeforeTest##b; \
			if(numBytesAllocatedAfterTest##b != numBytesAllocatedBeforeTest##b) \
			{ \
				printf("error: test " #b " leaked %zi bytes of memory %i%s time\n", numBytesLeakedByTest##b, i + 1, ORDINAL_NUMBER_SUFFIX(i+1)); \
				return 1; \
			} \
		} \
		if(a != 1) \
		{ \
			printf("error: test " #b " failed %i%s time\n", i + 1, ORDINAL_NUMBER_SUFFIX(i+1)); \
			return 1; \
		} \
		else \
		{ \
			fputs("test " #b " succeeded\n", stdout); \
		} \
	}
	
	size_t numBytesAllocatedBefore = numBytesAllocated;

	TEST(1);
	TEST(2);
	TEST(3);
	
	size_t numBytesAllocatedAfter = numBytesAllocated;
	size_t numBytesLeaked = numBytesAllocatedAfter - numBytesAllocatedBefore;
	if(numBytesAllocatedAfter != numBytesAllocatedBefore)
	{
		printf("error: %zi bytes of memory leaked\n", numBytesLeaked);
	}

	fputs("all tests succeeded\n", stdout);

	return 0;
}
