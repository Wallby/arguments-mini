#ifndef ARGUMENTS_MINI_H
#define ARGUMENTS_MINI_H

#define ARGUMENTS_MINI_VERSION 0.2.1

#ifdef __cplusplus
#define AM_FUNCTION extern "C"
#else
#define AM_FUNCTION
#endif


AM_FUNCTION void am_set_on_print(void(*a)(char*));
AM_FUNCTION void am_unset_on_print();

// NOTE: won't call if argv is invalid|same argument is supplied twice
// NOTE: returns 0 to indicate error (will discontinue parsing)
//       returns 1 to continue parsing
//int(*on_argument_parsed)(char* argument);
AM_FUNCTION void am_set_on_argument_parsed(int(*a)(char*));
AM_FUNCTION void am_unset_on_argument_parsed();

// NOTE: won't call if argv is invalid|parameter with same parametername is..
//       .. supplied twice
// NOTE: returns 0 to indicate error (will discontinue parsing)
//       returns 1 to continue parsing
//int(*on_parameterwithoutvalue_parsed)(char* parametername);
// NOTE: --<parametername>
AM_FUNCTION void am_set_on_parameterwithoutvalue_parsed(int(*a)(char*));
AM_FUNCTION void am_unset_on_parameterwithoutvalue_parsed();

// NOTE: won't call if argv is invalid|parameter with same parametername is..
//       .. supplied twice
// NOTE: returns 0 to indicate error (will discontinue parsing)
//       returns 1 to continue parsing
// NOTE: if arg == -<parametername>= (i.e. no value specified).. value == NULL
//int(*on_parameterwithvalue_parsed)(char* parametername, char* value);
// NOTE: -<parametername>=<value>
AM_FUNCTION void am_set_on_parameterwithvalue_parsed(int(*a)(char*,char*));
AM_FUNCTION void am_unset_on_parameterwithvalue_parsed();

// NOTE: https://stackoverflow.com/questions/7091236/maximum-length-of-argv-parameters
// NOTE: parameter(s) can be anywhere (not required to be before argument(s)..
//       .. if any)
// NOTE: returns 0 if..
//       .. any syntax error(s) in argv
//       .. duplicate argument(s) is/are supplied
//       .. duplicate parameter(s) is/are supplied
//       returns 1 if succeeded
//       returns -1 if "no code executed" (e.g. if argc == 1*)
// NOTE: * argv[0] is ignored (i.e. argc is expected to be >= 1)
//       ^
//       "works well for linux + windows"..? (see https://unix.stackexchange.com/questions/315812/why-does-argv-include-the-program-name)
// NOTE: will check if argv is valid before calling any on_*_parsed callback
//       ^
//       so.. if any on_*_parsed callback is called..
//       .. syntax is valid
//       .. no duplicate argument(s) is/are supplied
//       .. no duplicate parameter(s) is/are supplied
AM_FUNCTION int am_parse(int argc, char** argv);

#endif
