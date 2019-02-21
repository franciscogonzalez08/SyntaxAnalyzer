// LIBRARIES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Parsing.h"

// MAIN
int main(void) {
	setbuf(stdout, NULL);

	int numLines;

	printf("You may use lower-case letters for your propositions, &, |, -> and <->\n\n");


	// We'll save the input in a dynamic array and check it only has valid characters
	char* statement = NULL;
	int statementSize;

	char * entry = "";
	boolean validCharacters;

	//Logic propositions
	printf("Number of logic propositions: ");
	scanf("%d", &numLines);
	for(int i = 0; i < numLines; i++) {
		do
		{
			statementSize = readLine(&statement);
			validCharacters = translate(&statement, &statementSize);

			// If they're trolling and just gave us an enter...
			if(statementSize == 0) validCharacters = False;
		} while (validCharacters == False);
			concatenate(&entry, statement);
			concatenate(&entry, "&");
	}
	//Conclusion
	printf("\nConclusion: ");
	do
	{
		statementSize = readLine(&statement);
		validCharacters = translate(&statement, &statementSize);

		// If they're trolling and just gave us an enter...
		if(statementSize == 0) validCharacters = False;
	} while (validCharacters == False);
	concatenate(&entry, "~");
	concatenate(&entry, statement);
	//puts(entry);
	if (!parse(entry)) printf("Syntax error\n");
	system("PAUSE");
	return EXIT_SUCCESS;
}

