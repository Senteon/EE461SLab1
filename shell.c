#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <readline/readline.h>
#include <readline/history.h>

int main()
{
	char* input;
	while (1)
	{
		input = readline("#");
		printf("%s\n", input);
	}
	return 1;
}
