// LIBRARIES
#include <stdlib.h>
#include <stdio.h>

#include "Parsing.h"

// CONSTANTS
#define INCREMENT 20 // the dynamic array will increase it's size at this rate

// READING FUNCTIONS

int readLine (char** pp)//We receive the memory direction of a pointer, so we save it on a double pointer
{
	char* string = (char*) malloc(INCREMENT * sizeof(char));
	int size = 0;
	char temp;
	do
	{
		// We read a character
		temp = getchar();

		// If it's the end of the line, we're done
		if (temp == '\n') break;

		string[size] = temp;
		size++;

		// If the array is full, we'll increase its capacity (doing this here because I want to make sure there's space for '\0')
		if (size%INCREMENT == 0 && size != 0)
			string = (char*) realloc(string, (size+INCREMENT)*sizeof(char));

	} while (True);

	string[size] = '\0'; // so it's a proper string

	*pp = string;
	return size;
}

boolean translate(char** pString, int* pSize)
{
	char* newString = malloc(*pSize * sizeof(char));
	int newSize = 0;

	char symbol;
	int index;
	for(index = 0; index < *pSize; index++)
	{
		symbol = (*pString)[index];

		if (symbol == '(' || symbol == ')' || symbol == '&' || symbol == '|' || symbol == '~' || (symbol >= 'a' && symbol <= 'z'))
			newString[newSize] = symbol;
		else if (symbol == '-')
		{
			if (*pSize > index+1 && (*pString)[index+1] == '>')
			{
				newString[newSize] = '>';
				index = index+1; // this should skip the next cycle
			}
			else
			{
				printf("Syntax error! Suggested correction: ->\n");
				printf("You may try again, by the way! ^^\n");

				free(*pString); // getting rid of trash so we can reuse the memory
				*pSize = -1;
				return False;
			}
		}
		else if (symbol == '<')
		{
			if (*pSize > index+2 && (*pString)[index+1] == '-' && (*pString)[index+2] == '>')
			{
				newString[newSize] = '?';
				index = index+2;
			}
			else
			{
				printf("Syntax error! Suggested correction: <->\n");
				printf("You may try again, by the way! ^^\n");

				free(*pString); // getting rid of trash so we can reuse the memory
				*pSize = -1;
				return False;
			}
		}
		else if(symbol == 'V')
			newString[newSize] = '|';
		else if(symbol == '^')
			newString[newSize] = '&';
		else if (symbol == ' ') continue;
		else
		{
			printf("An invalid character was found in your input! D:\n");
			printf("You may try again, though! ^^\n");

			free(*pString); // getting rid of trash so we can reuse the memory
			*pSize = -1;
			return False;
		}
		newSize++; // In all of the above cases a character was added
	}

	// We'll get rid of the old string
	free(*pString);

	// Add a '\0' at the end
	realloc(newString, newSize+1 * sizeof(char));
	newString[newSize] = '\0';

	// And update our string and its size
	*pString = newString;
	*pSize = newSize;

	return True;
}

// STACK
struct strNode {
	char data;
	struct strNode * prior, * leftChild, * rightChild;
};


//SemanticTree
struct strSignedNode {
	char data;
	struct strSignedNode * leftChild, * rightChild;
	char sign;//'+' = 43, '-' = 45
};
typedef struct strSignedNode * SignedNode;

typedef struct strNode * Node;

struct stack {
	long size;
	Node Top;
};

typedef struct stack * Stack;

Stack stack_create()
{
	return (Stack) calloc(1, sizeof(struct stack));
}

void treeDestroy(Node root)
{
	if(root)
	{
		treeDestroy(root->leftChild);
		treeDestroy(root->rightChild);
		free(root);
	}
}

boolean stackDestroy(Stack s)
{
	if(!s) return False;

	Node current = s->Top;
	Node prior;
	while (current)
	{
		prior = current->prior;

		treeDestroy(current);

		current = prior;
	}

	free(s);
	return True;
}

void stack_add_node(Stack s, Node n)
{
	if(s->size > 0) n->prior = s->Top;

	// Either way, since a node was added
	s->Top = n;
	s->size++;
}

boolean stack_add_char(Stack s, char symbol)
{
	// Validating s isn't NULL
	if(!s) return False;

	// We'll create the node
	Node n = (Node) calloc(1, sizeof(struct strNode));
	if(!n) return False;
	n->data = symbol;

	// We'll add the node
	stack_add_node(s, n);

	return True;
}


Node stack_remove_node(Stack s)
{
	// Validations
	if(!s || s->size == 0) return NULL;

	// Unlinking the top node
	Node NodeHolder = s->Top;

	s->Top = s->Top->prior;
	s->size--;

	NodeHolder->prior = NULL;

	// Returning the top node
	return NodeHolder;
}

boolean isComplete(Node n)
{
	if(n->data >= 'a' && n->data <= 'z') return True;

	if (n->data == '&' || n->data == '|' || n->data == '?' || n->data == '>')
	{
		if(n->leftChild && n->rightChild) return True;
		return False;
	}

	if (n->data == '~')
	{
		if(n->rightChild) return True;
		return False;
	}

	return False;
}

boolean transfer(Stack origin, Stack destiny)
{
	if(isComplete(origin->Top) || origin->Top->data == '(' || origin->Top->data == ')')
		stack_add_node(destiny, stack_remove_node(origin));
	else
	{
		if (origin->Top->data == '~')
		{
			// Assigning operand to the operator
			if(destiny->size == 0) return False;
			if(!isComplete(destiny->Top)) return False;
			origin->Top->rightChild = stack_remove_node(destiny);

			// Transferring the -now complete- operand node
			stack_add_node(destiny, stack_remove_node(origin));
		}
		else
		{
			// Assigning right operand to the operator
			if(destiny->size == 0) return False;
			if(!isComplete(destiny->Top)) return False;
			origin->Top->rightChild = stack_remove_node(destiny);

			// Transferring the operand node
			stack_add_node(destiny, stack_remove_node(origin));

			// Assigning left operand to operator
			if(origin->size == 0) return False;
			if(!isComplete(origin->Top)) return False;
			Node holder = stack_remove_node(origin);
			if((origin->Top) && (origin->Top->data == '~')) {
				origin->Top->rightChild = holder;
				destiny->Top->leftChild = stack_remove_node(origin);
			}
			else
				destiny->Top->leftChild = holder;
		}
	}

	return True;
}


void print(Node n)
{
	if(n == NULL)
		return;
	print(n->leftChild);
	print(n->rightChild);
	if(n->data == '>')
		printf("->");
	else if(n->data == '?')
		printf("<->");
	else if(n->data == '|')
		printf("V");
	else if(n->data == '&')
		printf("^");
	else
		printf("%c", n->data);

	return;
}


void polacaInversa(Node n)
{
	if(n == NULL)
		return;
	print(n);
	printf("\n");
	return;
}


void removeDoubleImp(Node n)
{
	if(n == NULL)
		return;
	if(n->data == '?')//Has a double implication
	{
		n->data = '&';
		Node newNode1 = (Node) malloc(sizeof(struct strNode));
		Node newNode2 = (Node) malloc(sizeof(struct strNode));
		newNode1->data = newNode2->data = '>';
		newNode1->prior = newNode2->prior = NULL;
		newNode1->leftChild = n->leftChild;
		newNode1->rightChild = n->rightChild;
		newNode2->leftChild = n->rightChild;
		newNode2->rightChild = n->leftChild;
		n->leftChild = newNode1;
		n->rightChild = newNode2;
	}
	removeDoubleImp(n->leftChild);
	removeDoubleImp(n->rightChild);
}


void removeSimpleImp(Node n)
{
	if(n == NULL)
		return;

	//printf("%c has changed to ", n->data);

	if(n->data == '>')//Has a simple implication
	{
		n->data = '|';
		Node newNode = (Node) malloc(sizeof(struct strNode));
		newNode->data = '~';
		newNode->prior = NULL;
		newNode->leftChild = NULL;
		newNode->rightChild = n->leftChild;
		n->leftChild = newNode;
	}

	//printf("%c\n", n->data);

	removeSimpleImp(n->leftChild);
	removeSimpleImp(n->rightChild);
}

SignedNode copyTree(Node n, char sign) {
	if(n == NULL) return NULL;

	if(n->data != '~')
	{
		// We'll create a new node to save the current node's data
		SignedNode sn = (SignedNode) calloc(1, sizeof(struct strSignedNode));
		sn->data = n->data;

		sn->leftChild = copyTree(n->leftChild, '+');
		sn->rightChild = copyTree(n->rightChild, '+');
		sn->sign = sign;
		return sn;
	}
	else
		return copyTree(n->rightChild, (sign == '+')? '-':'+');
}

void signPrint(SignedNode n) {
	if(n == NULL)
		return;
	signPrint(n->leftChild);
	if(n->sign == '-')
		printf("~");
	printf("%c", n->data);
	signPrint(n->rightChild);
}


void deMorgan(SignedNode n) {
	if(n == NULL)
		return;
	if((n->data == '|') || (n->data == '&'))
		if(n->sign == '-') {
			if(n->leftChild->sign == '-')
				n->leftChild->sign = '+';
			else
				n->leftChild->sign = '-';
			if(n->rightChild->sign == '-')
				n->rightChild->sign = '+';
			else
				n->rightChild->sign = '-';
			if(n->data == '|')
				n->data = '&';
			else
				n->data = '|';
			n->sign = '+';
		}
	deMorgan(n->leftChild);
	deMorgan(n->rightChild);
}

void getSubtree(SignedNode node, SignedNode* arrSubtrees, int* sizeOfArray)
{
	if (!node) return;

	if(node->data != '&')
	{
		arrSubtrees[*sizeOfArray] = node;
		(*sizeOfArray)++;

		// We'll make sure there's space for more
		if((*sizeOfArray)%INCREMENT == 0 && (*sizeOfArray) != 0) realloc(arrSubtrees, ((*sizeOfArray)+INCREMENT)*sizeof(SignedNode));

		return;
	}

	getSubtree(node->leftChild, arrSubtrees, sizeOfArray);
	getSubtree(node->rightChild, arrSubtrees, sizeOfArray);
}

SignedNode duplicateTree(SignedNode n)
{
	if(n == NULL) return NULL;

	SignedNode sn = (SignedNode) calloc(1, sizeof(struct strSignedNode));
	sn->data = n->data;
	sn->sign = n->sign;

	sn->leftChild = duplicateTree(n->leftChild);
	sn->rightChild = duplicateTree(n->rightChild);
	return sn;
}

void connectToLeaves(SignedNode node, SignedNode Subtree)
{
	if(!node) return;

	// If it's a leave, we'll append the subtree and return
	if(node->rightChild == NULL)
	{
		node->rightChild = duplicateTree(Subtree); // Somewhat arbitrary
	}
	else
	{
		connectToLeaves(node->leftChild, Subtree);
		connectToLeaves(node->rightChild, Subtree);
	}
}

boolean closes(SignedNode n, SignedNode* arrPreviousN, int* pSize)
{
	// NULL isn't part of a path so it's not considered "unclosed" (/open)
	if(n == NULL) return True;

	// If there's a disjunction we must check both paths
	if(n->data == '|')
	{
		if(closes(n->leftChild, arrPreviousN, pSize) == False) return False;
		if(closes(n->rightChild, arrPreviousN, pSize) == False) return False;

		// If both paths are closed, the whole | is
		return True;
	}
	else // if it's a letter
	{
		// If its contrary is in the array, path's closed, so we'll RETURN True
		int i;
		for (i = 0; i < *pSize; i++)
			if(arrPreviousN[i]->data == n->data && arrPreviousN[i]->sign != n->sign)
				return True;

		// If we're in a leaf, this is an open path (RETURN False)
		if(n->rightChild == NULL) return False;

		// Else we'll add it (check it isn't full now)
		arrPreviousN[i] = n;
		(*pSize)++;

		if((*pSize)%INCREMENT == 0 && (*pSize) != 0)
			realloc(arrPreviousN, ((*pSize)+INCREMENT)*sizeof(SignedNode));


		// Since it wasn't a leaf, we'll keep looking (in rightChild)
		boolean aheadIsClosed = closes(n->rightChild, arrPreviousN, pSize);

		// If it IS closed, delete yourself from the array
		if(aheadIsClosed)
			(*pSize)--;

		// RETURN whatever the recursive call returned
		return aheadIsClosed;
	}
}

void destroySignedTree(SignedNode n) {
	if(!n) return;
	destroySignedTree(n->leftChild);
	destroySignedTree(n->rightChild);
	free(n);
}






boolean hasAND(SignedNode n)
{
	if(n)
	{
		if(n->data == '&') return True;
		if(hasAND(n->leftChild) == True) return True;
		if(hasAND(n->rightChild) == True) return True;
		return False;
	}
	return False;
}

SignedNode genSemanticTree2(SignedNode n)
{
	// We want all the memory add. of the children of nodes containing &
	SignedNode* arrSubtrees = (SignedNode*) calloc(INCREMENT, sizeof(SignedNode));

	// We'll fill the array
	int subtreeQuantity = 0;
	getSubtree(n, arrSubtrees, &subtreeQuantity);

	// We'll check each subtree for ANDs
	int currentSubtree;
	for (currentSubtree = 0; currentSubtree < subtreeQuantity; currentSubtree++)
	{
		if(arrSubtrees[currentSubtree]->data == '|')
		{
			if(hasAND((arrSubtrees[currentSubtree])->leftChild) == True)
			{
				SignedNode deleteMe = (arrSubtrees[currentSubtree])->leftChild;
				(arrSubtrees[currentSubtree])->leftChild = genSemanticTree2(deleteMe);
				destroySignedTree(deleteMe);
			}
			if(hasAND((arrSubtrees[currentSubtree])->rightChild) == True)
			{
				SignedNode deleteMe = (arrSubtrees[currentSubtree])->rightChild;
				(arrSubtrees[currentSubtree])->rightChild = genSemanticTree2(deleteMe);
				destroySignedTree(deleteMe);
			}
		}
	}

	// We'll start our semantic tree with our first subtree
	SignedNode SemanticTree = duplicateTree(arrSubtrees[0]); // we could use the original one but it's "messier"

	// We'll connect the following subtrees in each of our main tree's leaves
	for(currentSubtree = 1; currentSubtree < subtreeQuantity; currentSubtree++)
	{
		connectToLeaves(SemanticTree, arrSubtrees[currentSubtree]);
	}

	// We'll free the memory
	free(arrSubtrees);

	// We now have our semantic tree! F yeah! :'D
	return SemanticTree;
}

void genSemanticTree(Node SyntaxTree)
{
	removeDoubleImp(SyntaxTree);
	removeSimpleImp(SyntaxTree);
	SignedNode SignedSyntaxTree = copyTree(SyntaxTree, '+');
	deMorgan(SignedSyntaxTree);

	SignedNode SemanticTree = genSemanticTree2(SignedSyntaxTree);
	// No more ANDs after this point

	// We'll see if all the paths close

	int sizeOfArray = 0; // couple things for the recursive "closes" function
	SignedNode* arrPrevious = (SignedNode*) malloc(INCREMENT*sizeof(SignedNode));

	if (closes(SemanticTree, arrPrevious, &sizeOfArray))
	{
		printf("Verdict: Correct");//\n\nSemantic Tree (dbug - in order):\n
		//signPrint(SemanticTree);
		printf("\n");
	}
	else
	{
		printf("Verdict: Incorrect :(");//\n\nSemantic Tree (dbug - in order):\n
		//signPrint(SemanticTree);
		printf("\n");
	}

	//We'll free the memory :D
	free(arrPrevious);
	treeDestroy(SyntaxTree);
	destroySignedTree(SemanticTree);
	destroySignedTree(SignedSyntaxTree);
}

boolean parse(char* statement)
{
	// Creating both stacks we'll be working with
	Stack A = stack_create();
	Stack B = stack_create();

	int strIndex = 0;
	char currentChar = statement[strIndex];
	while(currentChar != '\0')
	{
		// We'll add the current character to A
		stack_add_char(A, currentChar);

		// If the character is a ")" we'll start moving nodes from A to B
		if(A->Top->data == ')')
		{
			while(A->Top->data != '(')
			{
				if(transfer(A, B) == False) {
					stackDestroy(A);
					stackDestroy(B);
					return False;
				}
				if(A->size == 0) {
					stackDestroy(A);
					stackDestroy(B);
					return False;
				}
			}

			stack_remove_node(A); // "(" is removed from A
			transfer(B, A); // ideally a now complete node containing a binary operand is being transfered back
			stack_remove_node(B); // ideally ")" is being removed
			if(B->size > 0) {
				stackDestroy(A);
				stackDestroy(B);
				return False;
			}
		}

		// We'll check the next character
		strIndex++;
		currentChar = statement[strIndex];
	}

	// One last transfer from A to B
	while(A->size > 0)
	{
		if(transfer(A, B) == False) {
			stackDestroy(A);
			stackDestroy(B);
			return False;
		}
	}

	if(B->size == 1)
	{
		Node SyntaxTree = stack_remove_node(B);
		stackDestroy(A);
		stackDestroy(B);
		printf("\nReverse Polish Notation:\n");
		polacaInversa(SyntaxTree);
		printf("\n");
		genSemanticTree(SyntaxTree);
		printf("\n");
		return True; // Success!
	}
	else {
		stackDestroy(A);
		stackDestroy(B);
		return False;
	}
}






void concatenate(char ** pStatement, char * statement) {
	if(!statement) return;
	int pStatementSize = 0, statementSize = 0;

	for(;(*pStatement)[pStatementSize] != '\0'; pStatementSize++);
	for(; statement[statementSize] != '\0'; statementSize++);

	char * newString = (char *) malloc((pStatementSize + statementSize + 1) * sizeof(char));
	int i;
	for(i = 0; i < pStatementSize; i++)
		newString[i] = (*(pStatement))[i];
	for(i = pStatementSize; i < (pStatementSize + statementSize); i++)
		newString[i] = statement[i - pStatementSize];
	newString[i] = '\0';
	free(*pStatement);
	*pStatement = newString;
}
