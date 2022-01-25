#include "arguments_mini.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>


// NOTE: for 11/12/13.. th (exceptions)
//       for 1,21,31,etc... st
//       for 2,22,32,etc... nd
//       for 3,23,33,etc... rd
//       otherwise.. th
#define ORDINAL_NUMBER_SUFFIX(a) \
	((a == 11) | (a == 12) | (a == 1) ? "th" : \
	(a%10) == 1 ? "st" : \
	(a%10) == 2 ? "nd" : \
	(a%10) == 3 ? "rd" : \
	"th")

#define ARGUMENTS_MINI_ALWAYS_INLINE gnu::always_inline

[[ARGUMENTS_MINI_ALWAYS_INLINE]]
inline void* operator new[](std::size_t a)
{
	return malloc(a);
}
[[ARGUMENTS_MINI_ALWAYS_INLINE]]
inline void operator delete(void* a, std::size_t)
{
    free(a); //< there is no check here on purpose, please catch this issue elsewhere
}

namespace
{
	// NOTE: T& is a reference to adress "decay issue" (where T[] becomes T*) as explained here http://www.cplusplus.com/articles/D4SGz8AR/
	template<typename T, int A>
	int length(T (&a)[A])
	{
		return sizeof a/sizeof *a;
	}

	int length(char* a)
	{
		return strlen(a);
	}
}

//*****************************************************************************

namespace
{
	void(*on_print)(char* a);
	int(*on_argument_parsed)(char* argument);
	int(*on_parameterwithoutvalue_parsed)(char* parametername);
	int(*on_parameterwithvalue_parsed)(char* parametername, char* value);
}

extern "C" void am_set_on_print(void(*a)(char*))
{
	on_print = a;
}
extern "C" void am_unset_on_print()
{
	on_print = NULL;
}

extern "C" void am_set_on_argument_parsed(int(*a)(char*))
{
	on_argument_parsed = a;
}
extern "C" void am_unset_on_argument_parsed()
{
	on_argument_parsed = NULL;
}

extern "C" void am_set_on_parameterwithoutvalue_parsed(int(*a)(char*))
{
	on_parameterwithoutvalue_parsed = a;
}
extern "C" void am_unset_on_parameterwithoutvalue_parsed()
{
	on_parameterwithoutvalue_parsed = NULL;
}

extern "C" void am_set_on_parameterwithvalue_parsed(int(*a)(char*,char*))
{
	on_parameterwithvalue_parsed = a;
}
extern "C" void am_unset_on_parameterwithvalue_parsed()
{
	on_parameterwithvalue_parsed = NULL;
}

//*****************************************************************************

namespace
{
	enum
	{
		EArgvElementType_Argument = 0,
		EArgvElementType_Parameterwithvalue,
		EArgvElementType_Parameterwithoutvalue
	};

	/*
	// NOTE: expects parameter to be in format --<parametername>
	void parse_parameterwithoutvalue(char* parameter, int* indexToParametername)
	{
		*indexToParametername = 2;
	}
	*/
	// NOTE: expects parameter to be in format -<parametername>=<value>
	void parse_parameterwithvalue(char* parameter, /*int* indexToParametername,*/ int* parameternameLength, /*int* indexToValue,*/ int* valueLength)
	{
		int indexToParametername = 1;
		//*indexToParametername = 1;

		int a = length(parameter);
		for(int i = 0; i < a; ++i)
		{
			if(parameter[i] == '=')
			{
				*parameternameLength = ((i-1)+1) - indexToParametername;

				//*indexToValue = i + 1;
				int indexToValue = i + 1;
				*valueLength = a - indexToValue;
			}
		}
	}
}

extern "C" int am_parse(int argc, char** argv)
{
	//if(argc == 0) //< not checked as if directly supplying argc+argv from main.. "couldn't occur"..?

	if(argc == 1) //< first argument is ignored
	{
		return -1;
	}

	int numArguments = 0;
	int* indexPerArgument = NULL;

	int numParameterswithvalue = 0;
	int* indexPerParameterwithvalue = NULL;

	int numParameterswithoutvalue = 0;
	int* indexPerParameterwithoutvalue = NULL;

	// NOTE: returns 0 if failed (i.e. am_parse should then return 0)
	//       returns 1 if succeeded
	int(*attempt_to_parse_arg)(char**, int, int*, int**, int*, int**, int*, int**) = [](char** argv, int i, int* numArguments, int** indexPerArgument, int* numParameterswithvalue, int** indexPerParameterwithvalue, int* numParameterswithoutvalue, int** indexPerParameterwithoutvalue)
		{
			int type; //< one of EArgvElementType

			// NOTE: on windows,linux.. guaranteed that length(argv[i]) > 1..?..
			//       .. (i.e. empty string shouldn't end up in argv..?)
			int a = length(argv[i]);

			// 1. check syntax correctness + determine parameter type
			if(argv[i][0] == '-')
			{
				// i.e. argv[i] == "-" //< < parameter is missing parametername
				if(a == 1)
				{
					if(on_print != NULL)
					{
						char b[128];
						snprintf(b, 128, "error: %i%s element in argv is parameter missing parametername\n", i+1, ORDINAL_NUMBER_SUFFIX(i+1));
						on_print(b);
					}

					return 0;
				}

				if(argv[i][1] == '-')
				{
					// i.e. argv[i] == "--" //< parameter is missing parametername
					if(a == 2)
					{
						if(on_print != NULL)
						{
							char b[128];
							snprintf(b, 128, "error: %i%s element in argv is parameter missing parametername\n", i+1, ORDINAL_NUMBER_SUFFIX(i+1));
							on_print(b);
						}

						return 0;
					}

					// i.e. argv[i] start with "---"
					if(argv[i][2] == '-')
					{
						if(on_print != NULL)
						{
							char b[128];
							snprintf(b, 128, "error: invalid parameter %s\n", argv[i]);
							on_print(b);
						}

						return 0;
					}

					type = EArgvElementType_Parameterwithoutvalue;
				}
				else
				{
					type = EArgvElementType_Parameterwithvalue;

					int bIsMissingEqualsCharacter = 1;
					for(int j = 1; j < a; ++j)
					{
						if(argv[i][j] == '=')
						{
							bIsMissingEqualsCharacter = 0;
						}
					}
					if(bIsMissingEqualsCharacter == 1)
					{
						if(on_print != NULL)
						{
							char b[128];
							// i.e. -<parametername>
							//       ^
							//       +1
							snprintf(b, 128, "error: '-' parameter \"%s\" is missing '=' sign", argv[i] + 1);
							on_print(b);
						}

						return 0;
					}
				}
			}
			else //< argument
			{
				type = EArgvElementType_Argument;
			}

			// 2. check for forbidden '=' character in parameterwithoutvalue/argument
			if((type == EArgvElementType_Argument) | (type == EArgvElementType_Parameterwithoutvalue))
			{
				int j;
				if(type == EArgvElementType_Argument)
				{
					j = 0;
				}
				else
				{
					// i.e. --<parametername>
					//        ^
					//        +2
					j = 2;
				}

				for(; j < a; ++j)
				{
					if(argv[i][j] == '=') //< forbidden in all but parameterwithvalue
					{
						if(on_print != NULL)
						{
							char b[128];
							snprintf(b, 128, "error: '=' in %s\n", type == EArgvElementType_Argument ? "argument" : "'--' parameter");
							on_print(b);
						}

						return 0;
					}
				}
			}

			// 3. check if argument/parameter isn't duplicate + if..
			// .. successful register argument/parameter
			switch(type)
			{
			case EArgvElementType_Argument:
				{
					for(int j = 0; j < *numArguments; ++j)
					{
						if(strcmp(argv[i], argv[(*indexPerArgument)[j]]) == 0)
						{
							if(on_print != NULL)
							{
								char c[128];
								snprintf(c, 128, "error: duplicate argument \"%s\"\n", argv[i]);

								on_print(c);
							}
							return 0;
						}
					}

					int* b = *indexPerArgument;
					//*indexPerArgument = new int[(*numArguments) + 1];
					*indexPerArgument = (int*)new char[sizeof(int) * ((*numArguments) + 1)];
					if(b != NULL)
					{
						memcpy(*indexPerArgument, b, (*numArguments) * sizeof(int));
						delete b;
					}
					++(*numArguments);
					(*indexPerArgument)[(*numArguments) - 1] = i;
				}
				break;
			case EArgvElementType_Parameterwithoutvalue:
				{
					int bIsDuplicate = 0;

					int parameternameLength = a - 2;

					for(int j = 0; j < *numParameterswithoutvalue; ++j)
					{
						// i.e. --<parametername>
						//        ^
						//        +2
						if(strcmp(argv[i] + 2, argv[(*indexPerParameterwithoutvalue)[j]] + 2) == 0)
						{
							bIsDuplicate = 1;
						}
					}
					for(int j = 0; j < *numParameterswithvalue; ++j)
					{
						struct
						{
							int parameternameLength;
							int valueLength;
						} c;

						parse_parameterwithvalue(argv[(*indexPerParameterwithvalue)[j]], &c.parameternameLength, &c.valueLength);

						if(parameternameLength != c.parameternameLength)
						{
							continue;
						}

						if(memcmp(argv[i] + 2, argv[(*indexPerParameterwithvalue)[j]] + 1, parameternameLength) == 0)
						{
							bIsDuplicate = 1;
						}
					}

					if(bIsDuplicate == 1)
					{
						if(on_print != NULL)
						{
							char c[128];
							snprintf(c, 128, "error: duplicate parameter \"%%.%is\"\n", parameternameLength);
							// ^
							// limit # characters to print

							char d[128];
							// i.e. --<parametername>
							//        ^
							//        +2
							snprintf(d, 128, c, argv[i] + 2);

							on_print(d);
						}
						return 0;
					}

					int* b = *indexPerParameterwithoutvalue;
					//*indexPerParameterwithoutvalue = new int[(*numParameterswithoutvalue) + 1];
					*indexPerParameterwithoutvalue = (int*)new char[sizeof(int) * ((*numParameterswithoutvalue) + 1)];
					if(b != NULL)
					{
						memcpy(*indexPerParameterwithoutvalue, b, (*numParameterswithoutvalue) * sizeof(int));
						delete b;
					}
					++(*numParameterswithoutvalue);

					(*indexPerParameterwithoutvalue)[(*numParameterswithoutvalue) - 1] = i;
				}
				break;
			case EArgvElementType_Parameterwithvalue:
				{
					int bIsDuplicate = 0;

					struct
					{
						int parameternameLength;
						int valueLength;
					} b;

					parse_parameterwithvalue(argv[i], &b.parameternameLength, &b.valueLength);

					for(int j = 0; j < *numParameterswithvalue; ++j)
					{
						struct
						{
							int parameternameLength;
							int valueLength;
						} d;

						parse_parameterwithvalue(argv[(*indexPerParameterwithvalue)[j]], &d.parameternameLength, &d.valueLength);

						if(b.parameternameLength != d.parameternameLength)
						{
							continue;
						}

						// i.e. -<parametername>=<value>
						//       ^
						//       +1
						if(memcmp(argv[i] + 1, argv[(*indexPerParameterwithvalue)[j]] + 1, b.parameternameLength) == 0)
						{
							bIsDuplicate = 1;
						}
					}

					for(int j = 0; j < (*numParameterswithoutvalue); ++j)
					{
						if(memcmp(argv[i] + 1, argv[(*indexPerParameterwithoutvalue)[j]] + 2, b.parameternameLength) == 0)
						{
							bIsDuplicate = 1;
						}
					}

					if(bIsDuplicate == 1)
					{
						if(on_print != NULL)
						{
							char d[128];
							snprintf(d, 128, "error: duplicate parameter \"%%.%is\"\n", b.parameternameLength);
							// ^
							// limit # characters to print

							char e[128];
							// i.e. -<parametername>=<value>
							//       ^
							//       +1
							snprintf(e, 128, d, argv[i] + 1);

							on_print(e);
						}
						return 0;
					}

					int* c = *indexPerParameterwithvalue;
					//*indexPerParameterwithvalue = new int[(*numParameterswithvalue) + 1];
					*indexPerParameterwithvalue = (int*)new char[sizeof(int) * ((*numParameterswithvalue) + 1)];
					if(c != NULL)
					{
						memcpy(*indexPerParameterwithvalue, c, (*numParameterswithvalue) * sizeof(int));
						delete c;
					}
					++(*numParameterswithvalue);

					(*indexPerParameterwithvalue)[(*numParameterswithvalue) - 1] = i;
				}
				break;
			}

			return 1;
		};

	// NOTE: returns 1 if succeeded
	//       returns 0 if on_<argument|parameterwithoutvalue|..
	//       ..parameterwithvalue>_parsed returned 0
	//       should never return -1 (that would be a bug)
	int(*on_arg_parsed)(char**, int, int, int*, int, int*, int, int*) = [](char** argv, int i, int numArguments, int* indexPerArgument, int numParameterswithvalue, int* indexPerParameterwithvalue, int numParameterswithoutvalue, int* indexPerParameterwithoutvalue)
		{
			for(int j = 0; j < numArguments; ++j)
			{
				if(indexPerArgument[j] == i)
				{
					if(on_argument_parsed != NULL)
					{
						if(on_argument_parsed(argv[j]) == 0)
						{
							return 0;
						}
					}
				}
				return 1;
			}
			for(int j = 0; j < numParameterswithoutvalue; ++j)
			{
				if(indexPerParameterwithoutvalue[j] == i)
				{
					if(on_parameterwithoutvalue_parsed != NULL)
					{
						// i.e. --<parametername>
						//        ^
						//        +2
						if(on_parameterwithoutvalue_parsed(argv[i]+2) == 0)
						{
							return 0;
						}
					}
					return 1;
				}
			}
			for(int j = 0; j < numParameterswithvalue; ++j)
			{
				if(indexPerParameterwithvalue[j] == i)
				{
					if(on_parameterwithvalue_parsed != NULL)
					{
						int parameternameLength;
						int valueLength;

						parse_parameterwithvalue(argv[i], &parameternameLength, &valueLength);

						char* parametername = new char[parameternameLength + 1];
						// i.e. -<parametername>=<value>
						//       ^
						//       +1
						memcpy(parametername, argv[i]+1, parameternameLength);
						parametername[parameternameLength] = '\0';

						char* value = NULL;
						if(valueLength > 0) //< allow empty value..?
						{
							value = new char[valueLength + 1];
							// i.e. -<parametername>=<value>
							//       ^              ^^
							//       ^              ^+1
							//       +1             +parameternameLength
							memcpy(value, argv[i]+1+parameternameLength+1, valueLength);
							value[valueLength] = '\0';
						}

						if(on_parameterwithvalue_parsed(parametername, value) == 0)
						{
							return 0;
						}

						delete parametername;
						if(value != NULL)
						{
							delete value;
						}
					}
					return 1;
				}
			}

			return -1;
		};

	int bSuccess = 1;

	//while#0(1)
	while(1)
	{
		for(int i = 1; i < argc; ++i) //< skip 1st element of argv
		{
			//if(attempt_to_parse_arg(argv, i) == 0)
			if(attempt_to_parse_arg(argv, i, &numArguments, &indexPerArgument, &numParameterswithvalue, &indexPerParameterwithvalue, &numParameterswithoutvalue, &indexPerParameterwithoutvalue) == 0)
			{
				bSuccess = 0;
				//break#0;
				break;
			}
		}
		if(bSuccess != 1)
		{
			break;
		}

		// to assure correct order of argument|parameter(s)..
		for(int i = 1; i < argc; ++i) //< skip 1st element of argv
		{
			//if(on_arg_parsed(argv, i) != 1)
			if(on_arg_parsed(argv, i, numArguments, indexPerArgument, numParameterswithvalue, indexPerParameterwithvalue, numParameterswithoutvalue, indexPerParameterwithoutvalue) != 1)
			{
				bSuccess = 0;
				//break#0;
				break;
			}
		}
		//if(bSuccess != 1)
		//{
		//	break;
		//}

		break;
	}

	if(numArguments > 0)
	{
		delete indexPerArgument;
		//indexPerArgument = NULL;
	}
	if(numParameterswithoutvalue > 0)
	{
		delete indexPerParameterwithoutvalue;
		//indexPerParameterwithoutvalue = NULL;
	}
	if(numParameterswithvalue > 0)
	{
		delete indexPerParameterwithvalue;
		//indexPerParameterwithvalue = NULL;
	}

	return bSuccess == 1 ? 1 : 0;
}
