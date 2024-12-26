#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <string>
#include <stack>
#include <unordered_map>
#include <bitset>

#define MAX_ID_LEN 32
#define MAX_STACK_LEN 250

typedef enum Relation
{
	None = ' ', Before = '<', Together = '=', After = '>', Dual = '%'
} Relation;

enum Action
{
	Shift, Reduce, Stop, Error
};

struct Symbol
{
	char symbol;
	Relation rel;
	std::string id;
	unsigned value;

	Symbol(char _symbol, Relation _rel, std::string _id, unsigned _value) : symbol(_symbol), rel(_rel), id(_id), value(_value) {}
	Symbol() : symbol('\0'), rel(Relation{}), id(""), value(0) {}
};

std::string Native = "=;|&~()";

FILE* fi, *fo, *foTriads;
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
const char alpabet[] = "LSETM=;|&~()IC#";

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
	{'T', "T&M"}, {'M', "~M"}, {'T', "M"},
	 {'M', "(E)"}, {'M', "I"}, {'M', "C"}
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


int triadNumber = 1;
struct Triad {
	int number;
	char operation;
	std::string operand1;
	std::string operand2;

	Triad(char _operation, std::string _operand1, std::string _operand2)
		: number(triadNumber), operation(_operation), operand1(_operand1), operand2(_operand2) { }

	Triad() {}
};
std::unordered_map<int, Triad> triads;

std::string toBinary(unsigned number) {
	int bitCount = (number == 0) ? 1 : static_cast<int>(std::log2(number)) + 1;
	return std::bitset<32>(number).to_string().substr(32 - bitCount);
}

void fprintfTriad(Triad triad) {
	fprintf(foTriads, "%d:\t%c(%s, %s)\n", triad.number, triad.operation, triad.operand1.c_str(), triad.operand2.c_str());
}

void fprintfTriads() {
	for (const auto& [key, value] : triads) {
		fprintfTriad(value);
	}
}

void addTriad(Triad triad) {
	triads[triad.number] = triad;
	triadNumber++;
}


void OnError(const char* msg, const char* param)
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
	c = Symbol();
	while (isspace(cc)) Get();
	if (isalpha(cc) || cc == '_')
	{
		c.id = cc;
		Get();
		while (cc == '0' || cc == '1')
		{
			c.id += cc;
			Get();
		}
		c.symbol = 'I';
	}
	else if (cc == '0' || cc == '1')
	{
		std::string buffer;
		buffer = cc;
		Get();
		while (cc == '0' || cc == '1')
		{
			buffer += cc;
			Get();
		}
		c.symbol = 'C';
		c.value = std::stoi(buffer, nullptr, 2);
	}
	else if (Native.find(cc) != std::string::npos)
	{
		c.symbol = cc;
		Get();
	}
	else if (cc == EOF)
		c.symbol = '#';
	else
		OnError("Unknown symbol \'%s\'\n", (char*)&cc);
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
		OnError("Unknown symbol", NULL);
}

Relation getRelation(char x, char y)
{
	return matrix[charIndex(x)][charIndex(y)];
}


void OnReduce(int ruleNumber) {
	Symbol newSymbol = stack.top();
	Symbol t;
	Symbol e;
	Symbol i;
	Symbol m;
	switch (ruleNumber)
	{
	case 0:
		break;	

	case 1:
		newSymbol.symbol = 'L';
		stack.pop();
		stack.pop();
		break;

	case 2:
		newSymbol = stack.top();
		newSymbol.symbol = 'L';
		stack.pop();
		break;

	case 3:
		newSymbol.symbol = 'S';
		stack.pop();
		e = stack.top();
		stack.pop();
		stack.pop();
		i = stack.top();
		stack.pop();
		newSymbol.id = i.id;
		newSymbol.value = e.value;
		addTriad(Triad('=', i.id, toBinary(e.value)));
		break;
	
	case 4:
		newSymbol.symbol = 'E';
		t = stack.top();
		stack.pop();
		stack.pop();
		e = stack.top();
		stack.pop();
		newSymbol.value = t.value | e.value;
		addTriad(Triad('|', std::to_string(e.value), std::to_string(t.value)));
		break;

	case 5:
		newSymbol.symbol = 'E';
		stack.pop();
		break;

	case 6:
		newSymbol.symbol = 'T';
		m = stack.top();
		stack.pop();
		t = stack.top();
		stack.pop();
		newSymbol.value = m.value & t.value;
		addTriad(Triad('&', std::to_string(m.value), std::to_string(t.value)));
		break;

	case 7:
		newSymbol.symbol = 'M';
		m = stack.top();
		stack.pop();
		stack.pop();
		newSymbol.value = ~m.value;
		if (m.id.empty())
			addTriad(Triad('~', toBinary(m.value), "¤"));
		else
			addTriad(Triad('~', m.id, "¤"));
		break;

	case 8:
		newSymbol.symbol = 'T';
		stack.pop();
		break;

	case 9:
		newSymbol.symbol = 'M';
		stack.pop();
		e = stack.top();
		stack.pop();
		newSymbol.value = e.value;
		break;

	case 10:
		newSymbol = stack.top();
		newSymbol.symbol = 'M';
		stack.pop();
		addTriad(Triad('V', newSymbol.id, "¤"));
		break;

	case 11:
		newSymbol = stack.top();
		newSymbol.symbol = 'M';
		stack.pop();
		addTriad(Triad('C', toBinary(newSymbol.value), "¤"));
		break;
	}

	newSymbol.rel = getRelation(stack.top().symbol, newSymbol.symbol);
	stack.push(newSymbol);
}

int g() {
	std::string omega;

	std::stack<Symbol> tempStack = stack;
	while (!tempStack.empty()) {
		omega = tempStack.top().symbol + omega;
		tempStack.pop();
	}

	for (int i = 0; i < std::size(rules); ++i) {
		if (omega.size() >= rules[i].right.size() &&
			omega.substr(omega.size() - rules[i].right.size()) == rules[i].right) {
			return i;
		}
	}

	return -1;
}

Action f() {
	if (stack.top().symbol == 'L' && c.symbol == '#') {
		return Stop;
	}
	if (c.rel == None) {
		return Action::Error;
	}
	if (c.rel == Before || c.rel == Together || c.rel == Dual) {
		return Shift;
	}
	if (c.rel == After) {
		return Action::Reduce;
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

	foTriads = fopen("test1_resultTriads.txt", "wb");
	if (!foTriads)
	{
		fprintf(stderr, "Output file open error.\n");
		return 3;
	}

	

	Action currentAction;
	int ruleNumber;
	bool skipNextGetLex = false;
	bool exitLoop = false;

	stack.push(Symbol('#', None, "", 0));
	Get();
	
	do {
		if (!skipNextGetLex) {
			GetLex(); 
			c.rel = getRelation(stack.top().symbol, c.symbol);
		}
		skipNextGetLex = false; 
		currentAction = f();
		

		switch (currentAction)
		{
		case Action::Shift:
			stack.push(c);
			break;
		case Action::Reduce:
			ruleNumber = g();
			if (ruleNumber == -1) {
				OnError("No rule", NULL);
			}
			OnReduce(ruleNumber);
			c.rel = getRelation(stack.top().symbol, c.symbol);
			skipNextGetLex = true;
			break;
		case Action::Stop:
			exitLoop = true;
			break;
		case Action::Error:
			OnError("No relation", NULL);
			break;
		}
	} while (!exitLoop);

	fprintfTriads();
}

