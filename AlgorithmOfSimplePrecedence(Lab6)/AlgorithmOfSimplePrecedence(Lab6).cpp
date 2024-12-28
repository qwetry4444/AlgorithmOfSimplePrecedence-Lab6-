#define _CRT_SECURE_NO_DEPRECATE
#include <iostream>
#include <string>
#include <stack>
#include <unordered_map>
#include <bitset>


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
	int triadNumber;
	Symbol(char _symbol, Relation _rel, std::string _id, unsigned _value) : symbol(_symbol), rel(_rel), id(_id), value(_value) {}
	Symbol() : symbol('\0'), rel(Relation{}), id(""), value(0) {}
};

std::stack<Symbol> stack;

std::string Native = "=;|&~()";
const char alpabet[] = "LSETM=;|&~()IC#";

FILE* fi, * foScanner, * foTriads;
int cc = 0;
Symbol c;


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

	Triad() : number(triadNumber) {}
};

std::unordered_map<std::string, unsigned> variables;
std::unordered_map<int, Triad> triads;

void addTriad(Triad triad) {
	triads[triad.number] = triad;
	triadNumber++;
}

void OnError(const char* msg, const char* param);
void fprintfTriads();
void fprintfTriad(Triad triad);
std::string toBinary(unsigned number);
void OpenFile(FILE*& file, const char* fileName, const char* mode, const char* errorMessage);
std::string numberToTriadNumber(int triadNumber);

void Get(void)
{
	cc = fgetc(fi);
}

void GetLex(void)
{
	c = Symbol();

	while (isspace(cc)) {
		fprintf(foScanner, "%c", cc);
		Get();
	}

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
		OnError("Unknown symbol\n", NULL);
	fprintf(foScanner, "%c", c.symbol);
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


void popAndAssign(Symbol& target) {
	if (!stack.empty()) {
		target = stack.top();
		stack.pop();
	}
}

void OnReduce(int ruleNumber) {
	if (ruleNumber < 0 || ruleNumber >= std::size(rules)) {
		OnError("Invalid rule number", nullptr);
		return;
	}

	Symbol newSymbol;
	Symbol t, e, i, m;
	Triad triad;

	switch (ruleNumber) {
	case 0: // L -> _
		return;

	case 1: // L -> LS
		newSymbol.symbol = 'L';
		stack.pop(); // Удаляем S
		stack.pop(); // Удаляем L
		break;

	case 2: // L -> S
		popAndAssign(newSymbol);
		newSymbol.symbol = 'L';
		break;

	case 3: // S -> I=E;
		newSymbol.symbol = 'S';
		stack.pop(); // Удаляем ';'
		popAndAssign(e);
		stack.pop(); // Удаляем '='
		popAndAssign(i);

		variables[i.id] = e.value;
		i.triadNumber = triadNumber;

		addTriad(Triad('V', i.id, "¤"));
		addTriad(Triad('=', numberToTriadNumber(i.triadNumber), numberToTriadNumber(e.triadNumber)));
		break;

	case 4: // E -> E|T
		newSymbol.symbol = 'E';
		popAndAssign(t);
		stack.pop(); // Удаляем '|'
		popAndAssign(e);

		triad = Triad('|', numberToTriadNumber(e.triadNumber), numberToTriadNumber(t.triadNumber));
		newSymbol.triadNumber = triad.number;
		addTriad(triad);
		break;

	case 5: // E -> T
		popAndAssign(t);
		newSymbol = t;
		newSymbol.symbol = 'E';
		break;

	case 6: // T -> T&M
		newSymbol.symbol = 'T';
		popAndAssign(m);
		stack.pop(); // Удаляем '&'
		popAndAssign(t);

		newSymbol.value = t.value & m.value;
		triad = Triad('&', numberToTriadNumber(t.triadNumber), numberToTriadNumber(m.triadNumber));
		newSymbol.triadNumber = triad.number;
		addTriad(triad);
		break;

	case 7: // M -> ~M
		newSymbol.symbol = 'M';
		popAndAssign(m);
		stack.pop(); // Удаляем '~'

		newSymbol.value = ~m.value;
		triad = Triad('~', numberToTriadNumber(m.triadNumber), "¤");
		newSymbol.triadNumber = triad.number;
		addTriad(triad);
		break;

	case 8: // T -> M
		popAndAssign(m);
		newSymbol = m;
		newSymbol.symbol = 'T';
		break;

	case 9: // M -> (E)
		stack.pop(); // Удаляем ')'
		popAndAssign(e);
		stack.pop(); // Удаляем '('
		newSymbol = e;
		newSymbol.symbol = 'M';
		break;

	case 10: // M -> I
		popAndAssign(newSymbol);

		if (variables.count(newSymbol.id) == 0) {
			OnError("Variable '%s' used before initialization", newSymbol.id.c_str());
		}
		newSymbol.symbol = 'M';

		newSymbol.triadNumber = triadNumber;
		addTriad(Triad('V', newSymbol.id, "¤"));
		break;

	case 11: // M -> C
		popAndAssign(newSymbol);
		newSymbol.symbol = 'M';

		newSymbol.triadNumber = triadNumber;
		addTriad(Triad('C', toBinary(newSymbol.value), "¤"));
		break;

	default:
		OnError("Unknown rule number", nullptr);
		return;
	}

	if (!stack.empty()) {
		newSymbol.rel = getRelation(stack.top().symbol, newSymbol.symbol);
	}
	else {
		newSymbol.rel = None;
	}
	stack.push(newSymbol);
}


int g() {
	std::string omega;
	int currentRuleNumber = -1;
	std::stack<Symbol> tempStack = stack;
	
	Symbol stackTop;
	do {
		stackTop = tempStack.top();
		omega = tempStack.top().symbol + omega;
		if (stackTop.rel == Dual || stackTop.rel == Before) {
			for (int i = 0; i < std::size(rules); i++) {
				if (omega == rules[i].right) {
					currentRuleNumber = i;
				}
			}
		}

		tempStack.pop();
	} while (!tempStack.empty() && stackTop.rel != Before);
		
	return currentRuleNumber;
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

void ProcessAction(Action currentAction, bool& skipNextGetLex, bool& exitLoop, int& ruleNumber) {
	switch (currentAction) {
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
}

int main()
{
	OpenFile(fi, "test1.txt", "rb", "Input file open error.");
	OpenFile(foScanner, "test1_resultScanner.txt", "wb", "Output file open error.");
	OpenFile(foTriads, "test1_resultTriads.txt", "wb", "Output file open error.");

	Action currentAction;
	int ruleNumber = -1;
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

		ProcessAction(currentAction, skipNextGetLex, exitLoop, ruleNumber);
	} while (!exitLoop);
	fprintfTriads();
}


void OpenFile(FILE*& file, const char* fileName, const char* mode, const char* errorMessage) {
	file = fopen(fileName, mode);
	if (!file) {
		throw std::runtime_error(errorMessage);
	}
}

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

void OnError(const char* msg, const char* param)
{
	printf("\nError: ");
	printf(msg);
	printf("\n");
	fclose(fi);
	fclose(foTriads);
	fclose(foScanner);
	exit(7);
}

std::string numberToTriadNumber(int triadNumber) {
	return "^" + std::to_string(triadNumber);
}