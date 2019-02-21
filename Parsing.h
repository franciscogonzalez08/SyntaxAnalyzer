#ifndef SRC_PARSING_H_
#define SRC_PARSING_H_

typedef enum {False, True} boolean;

// FUNCTION HEADERS
void concatenate(char ** pStatement, char * statement);
int readLine (char** pp); // saves dynamic string into a given pointer passed by reference; returns size of said string
boolean translate(char** pString, int* pSize); // validates (T/F), removes spaces and changes "->" to ">" and "<->" to "?"
boolean parse(char* statement);

#endif /* SRC_PARSING_H_ */
