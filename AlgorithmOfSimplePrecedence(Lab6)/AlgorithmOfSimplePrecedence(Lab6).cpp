#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <string>
#include <stack>

#define MAX_ID_LEN 32
#define MAX_STACK_LEN 250

typedef enum Relation
{
	None = ' ', Before = '<', Together = '=', After = '>', Dual = '%'
} Relation;

struct Symbol
{
	char symbol;
	Relation rel;
	std::string id;
	int value;

	Symbol(char _symbol, Relation _rel, std::string _id, int _value) : symbol(_symbol), rel(_rel), id(_id), value(_value) {}
	Symbol() : symbol('\0'), rel(Relation{}), id(""), value(0) {}
};

std::string Native = "=;|&~()";

FILE* fi = 0;
FILE* fo = 0;
int cc = 0;   
Symbol c;    


std::stack<Symbol> stack;

void printStack() {
	fprintf(fo, "Содержимое стека:\n");
	while (!stack.empty()) {
		const Symbol& top = stack.top();
		fprintf(fo, "%c: %s = %d\n", top.symbol, top.id.c_str(), top.value); 
		stack.pop();
	}
}
const char alpabet[] = "LSETMIC=|&~();#";

typedef struct Rule
{
	char left;
	std::string right;
} Rule;

Rule rules[] =
{
	{'_', "#L#"},
	{'L', "LS"}, {'L', "S"},
	{'S', "I=E;"},
	{'E', "E|T"}, {'E', "T"},
	{'T', "T&M"}, {'T', "M"},
	{'M', "~M"}, {'M', "(E)"}, {'M', "I"}, {'M', "C"}
};

Relation matrix[15][15] = 
{ 
	{None, Together, None, None, None, None, None, None, None, None, None, None, Before, None, Together },
	{None, None, None, None, None, None, None, None, None, None, None, None, After, None, After },
	{None, None, None, None, None, None, Together, Together, None, None, None, Together, None, None, None },
	{None, None, None, None, None, None, After, After, Together, None, None, After, None, None, None },
	{None, None, None, None, None, None, After, After, After, None, None, After, None, None, None },
	{None, None, Dual, Before, Before, None, None, None, None, Before, Before, None, Before, Before, None },
	{None, None, None, None, None, None, None, None, None, None, None, None, After, None, After },
	{None, None, None, Together, Before, None, None, None, None, Before, Before, None, Before, Before, None },
	{None, None, None, None, Together, None, None, None, None, Before, Before, None, Before, Before, None },
	{None, None, None, None, Together, None, None, None, None, Before, Before, None, Before, Before, None },
	{None, None, Dual, Before, Before, None, None, None, None, Before, Before, None, Before, Before, None },
	{None, None, None, None, None, None, After, After, After, None, None, After, None, None, None },
	{None, None, None, None, None, Together, After, After, After, None, None, After, None, None, None },
	{None, None, None, None, None, None, After, After, After, None, None, After, None, None, None },
	{Dual, Before, None, None, None, None, None, None, None, None, None, None, Before, None, None }
};

void Error(const char* msg, const char* param)
{
	printf("\nError: ");
	printf(msg);
	printf("\n");
	fclose(fi);
	fclose(fo);
	exit(7);
}

void Get(void)
{
	cc = fgetc(fi);
}

Symbol GetLex(void)
{
	while (isspace(cc)) Get();
	c = Symbol();
	if (isalpha(cc) || cc == '_')
	{
		c.id = cc;
		Get();
		while (isalnum(cc) || cc == '_')
		{
			c.id += cc;
			Get();
		}
		c.symbol = 'I';
	}
	else if (isdigit(cc))
	{
		std::string buffer;
		buffer = cc;
		Get();
		while (isdigit(cc))
		{
			buffer += cc;
			Get();
		}
		c.value = std::stoi(buffer, nullptr, 2);
		c.symbol = 'C';
	}
	else if (Native.find(cc) != std::string::npos)
	{
		c.symbol = cc;
		Get();
	}
	else if (cc == EOF)
		c.symbol = '#';
	else
		Error("Unknown symbol \'%s\'\n", (char*)&cc);
	stack.push(c);
	return c;
}

void RunScanner(void)
{
	Get();
	do
	{
		GetLex();
	} while (c.symbol != '#');
}

int charIndex(char c)
{
	const char* t;
	t = strchr(alpabet, c);
	if (t)
		return t - alpabet;
	else
		Error("Unknown symbol", NULL);
}

Relation f(char x, char y)
{
	return matrix[charIndex(x)][charIndex(y)];
}

int g()
{
	std::string buffer;
	buffer = c.symbol;
	while (stack.top().rel != Before) {
		buffer = stack.top().symbol + buffer;
		stack.pop();
	}

	for (size_t i = 0; i < std::size(rules); i++) {
		if (rules[i].right == buffer) {
			return static_cast<int>(i);
		}
	}
	return -1;
}

/* функция свертки (в стеке) */

/* функция семантики */

//n = g(y);
//
//switch (n)
//{
//case 1: /* E+T */
//	z.value = e.value + t.value; /* e and t - symbols from stack,
//												 z - new symbol that will pushed to stack */
//	break;
//	/* ... */
//}
//Rule rules[] =
//{
//	{'_', "#L#"},
//	{'L', "LS"}, {'L', "S"},
//	{'S', "I=E;"},
//	{'E', "E|T"}, {'E', "T"},
//	{'T', "T&M"}, {'T', "M"},
//	{'M', "~M"}, {'M', "(E)"}, {'M', "I"}, {'M', "C"}
//};

void Convolution() {

}



void Semantics(int ruleNumber) {
	switch (ruleNumber)
	{
	case 1:
		break;	
	case 2:


	case 3:
		c.symbol = 'L';
		c.rel = f(stack.top().symbol, c.symbol);
		break;
	case 6:
		c.symbol = 'E';
		c.rel = f(stack.top().symbol, c.symbol);
		break;
	case 8:
		c.symbol = 'T';
		c.rel = f(stack.top().symbol, c.symbol);
		break;
	case 11:
	case 12:
		c.symbol = 'M';
		c.rel = f(stack.top().symbol, c.symbol);
		break;

	default:
		break;
	}
}



int main()
{
	fi = fopen("test1.txt", "rb");
	if (!fi)
	{
		fprintf(stderr, "Input file open error.\n");
		return 1;
	}
	fo = fopen("test1_result.txt", "wb");
	if (!fo)
	{
		fprintf(stderr, "Output file open error.\n");
		return 2;
	}
	RunScanner();
	printStack();

	//int ruleNumber;
	//stack.push(Symbol('#', None, "", 0));
	//GetLex();
	//do {
	//	c.rel = f(stack.top().symbol, c.symbol);

	//	if (stack.top().symbol = 'S' && c.symbol == '#') {

	//	}

	//	if (c.rel == None) {
	//		Error("Wrong syntax", NULL);
	//	}

	//	if (c.rel == Before || c.rel == Together) {
	//		stack.push(c);
	//	}

	//	if (c.rel == After) {
	//		ruleNumber = g();
	//		if (ruleNumber == -1) {
	//			Error("Wrong syntax", NULL);
	//		}
	//		Semantics(ruleNumber);
	//	}
	//} while (GetLex().symbol != '#');
}

